/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:05
 * Filename         : ProcessManager.cpp
 * Description      : Base manager class for PixelTPC data analysis 
 * Update           : 
 * ******************************************************************/
#include "ProcessManager.h"

ProcessManager::ProcessManager() : fTaskjsonfile("")
{}

ProcessManager::ProcessManager(std::string taskjsonfile) : fTaskjsonfile(taskjsonfile)
{
    std::ifstream fin(fTaskjsonfile.data());
    if(!fin.is_open())
    {
        throw std::runtime_error("Task Json file DOES NOT EXIT!");
    }

    fPixJsonParser = PixJson::parse(fin);
    
    if(!fPixJsonParser.is_object())
    {
        throw std::runtime_error("Task Json file Can not be parsered!");
    }
    fin.close();
}


int ProcessManager::CEPCPixtpcRun()
{
    std::string TaskComments = fPixJsonParser.at("Comments");
    int Tasktype = fPixJsonParser.at("Tasktype");
    this->InitialMapsFromJson();
    PixTPCLog(PIXtpcINFO,"==============================================",false);
    PixTPCLog(PIXtpcINFO,Form("> Start a Task: %s",TaskComments.c_str()),false);
    PixTPCLog(PIXtpcINFO,"==============================================",false);
    
    switch(Tasktype)
    {
        case Rawdata2ROOT:
            std::printf("@@@Raw data to root file!\n");
            this->StartUnpackage();
            break;
        case GenMCdata:
            std::printf("@@@GenMCdata!\n");
            this->StartGenMCdata();
            break;
        case TPCEvtsReco:
            std::printf("@@@TPC evts reconstruction!\n");
            this->StartRecoPixTPCEvts();
            break;
        case TPCcalibration:
            std::printf("@@@TPC calibration!\n");
            break;
        default:
            this->PrintUsage();
            PixTPCLog(PIXtpcINFO,"Dummy task!",false);
    }

    PixTPCLog(PIXtpcINFO,Form("> End of Task:%s",TaskComments.c_str()),false);
    return 0;
}

void ProcessManager::InitialMapsFromJson()
{
    if(fPixJsonParser.contains("GlobalChipChnMaps")) 
    {
        //from csv file
        GlobalMaps = BeamUnities::CreateChipChnToRowColMap(fPixJsonParser.at("GlobalChipChnMaps"));
    }else if(fPixJsonParser.contains("GlobalChipChnMapsJson"))
    {
        //from json config file
        auto map_pathjsonObj = fPixJsonParser.at("GlobalChipChnMapsJson");
        auto map_path = map_pathjsonObj.get<std::string>();
        //std::vector<int> vGlobalIdx;
        std::vector<std::pair<int,int>> vMaps(__ROW__*__COL__,{-1,-1});
        std::ifstream ifs(map_path.c_str());
        if(!ifs.is_open())
        {
            throw std::runtime_error("Json Map file does not exist");
        }

        std::string line;
        while(std::getline(ifs,line))
        {
            auto chipchnmapsJsonObj = PixJson::parse(line);
            TaskConfigStruct::ChipChnMaps_V1 pixelIdx = chipchnmapsJsonObj.get<TaskConfigStruct::ChipChnMaps_V1>();
            if(pixelIdx.isActived)
            {
                vMaps.at(pixelIdx.globalIdx)= std::make_pair(pixelIdx.chipchnIdx[0],
                                                             pixelIdx.chipchnIdx[1]);
                //vGlobalIdx.emplace_back(pixelIdx.globalIdx);
            }
        }
        //PixTPCLog(PIXtpcDebug,Form("Global Idx size = %zu",vGlobalIdx.size()),false);
        ifs.close();

        GlobalMaps = vMaps;
        PixTPCLog(PIXtpcINFO,"GlobalMaps initialized by json config file",false);
#if 0
        PixTPCLog(PIXtpcDebug,"End = !!!!!!!",false);
        std::ofstream ofs_csv("./test.csv");
        for(size_t idx=0; idx<__ROW__*__COL__;++idx)
        {
            ofs_csv<<idx<<","<<GlobalMaps.at(idx).first<<","<<GlobalMaps.at(idx).second<<std::endl;
        }
        ofs_csv.close();
        PixTPCLog(PIXtpcDebug,"test.csv write done!",false);
#endif
    }
    else
    {
        PixTPCLog(PIXtpcERROR,"No Chip Chn map file",true);
        throw std::runtime_error("No Chip Chn map file in task json");
    }
        
}

void ProcessManager::StartUnpackage()
{
    PixTPCLog(PIXtpcDebug,"Test print in StartUnpackage()",false);
    std::string inputrawbinfile= fPixJsonParser.at("Inputfile");
    std::string outputrootfile = fPixJsonParser.at("Outputfile");
    bool isdebug = fPixJsonParser.at("Isdebug");
    PixTPCLog(PIXtpcDebug,(inputrawbinfile+" "+outputrootfile).data(),false);
    
    auto myConverter = new RawdataConverter(inputrawbinfile,outputrootfile);
    if(isdebug)
        myConverter->EnableUnpackgeDebug();
    else
        myConverter->DisableUnpackgeDebug();

    if(isdebug && fPixJsonParser.contains("Debughistconfig"))
    {
        auto parasobj = fPixJsonParser.at("Debughistconfig");
        TaskConfigStruct::HistConfigList histconfiglist = parasobj;
        myConverter->ConfigDebugHist(histconfiglist);
        PixTPCLog(PIXtpcDebug,"=======================",false);
        std::cout<<" Dim        "<<histconfiglist.Dim<<"\n"
                 <<" Histbins   ["<<histconfiglist.Histbins[0]<<","<<histconfiglist.Histbins[1]<<"]\n"
                 <<" HistXYstart["<<histconfiglist.HistXYstart[0]<<","<<histconfiglist.HistXYstart[1]<<"]\n"
                 <<" HistXYend  ["<<histconfiglist.HistXYend[0]<<","<<histconfiglist.HistXYend[0]<<"]"<<std::endl;
    }
    auto flags1 = myConverter->DoUnpackageRawdata2ROOT();
    //auto myCoverter = new RawdataConverter("/mnt/e/WorkSpace/DESYBeam_Test/test/TEPIX_test_canwen/0619_new_4/data_lg_300ns.dat_r.dat");
    //myCoverter->DoUnpackage();

    if(isdebug)
    {
        auto mycDebug = new TCanvas("mycDebug","mycDebug",800,600);
        mycDebug->SetGrid();
        auto histdebug = myConverter->GetDebugHist();
        histdebug->SetStats(0);
        histdebug->DrawCopy();
        PixTPCLog(PIXtpcDebug,Form("UnderFlow = %f , OverFlow = %f",histdebug->GetBinContent(0),histdebug->GetBinContent(histdebug->GetNbinsX()+1)),false);
    }

    delete myConverter;
    PixTPCLog(PIXtpcDebug,"End of StartUnpackage()",false);
}

void ProcessManager::StartGenMCdata()
{
    if(fPixJsonParser.contains("GenSimDataParsList"))
    {
        //1. from json to GenSimDataParsList structure
        auto paraslist = fPixJsonParser.at("GenSimDataParsList");
        TaskConfigStruct::GenSimDataParsList configlist = paraslist;
        // test print 
        //std::cout<< configlist.Isdebug<<"\n"
        //         << configlist.Nevts << "\n"
        //         << configlist.Particle<<"\n"
        //         <<"( " <<configlist.InitialPos[0]<<","<<configlist.InitialPos[1]<<","<<configlist.InitialPos[2]<<")"
        //         <<std::endl;
        
        //2. initial a GenSimData Object according to configlist
        auto gensim = new GenSimData(configlist.Nevts);
        if(configlist.Isdebug)
            gensim->EnableDebugging();
        
        gensim->SetParticle(configlist.Particle);
        gensim->SetTrkMomentums(configlist.Momentum,configlist.MomentumReso);
        TVector3 position(configlist.InitialPos),direction(configlist.InitialDir);
        gensim->SetTrkInitialPosition(position);
        gensim->SetTrkInitialDirection(direction);
        //3. simulate tracks
        gensim->GenTracksFromJson();
        //Show a hist without noise
        if(configlist.Isdebug)
        {
            auto debugCanvas = gensim->ShowPixelResponseWithoutNoise(gRandom->Integer(configlist.Nevts));
            debugCanvas->Draw();
        }
        //4. save MC data 
        std::string outputfile = fPixJsonParser.at("Outputfile");
        auto npos_root = outputfile.find(".root");
        if(npos_root != std::string::npos)
        {
            gensim->WritePixelTPCdata(outputfile);
        }
        else 
        {
            outputfile = "./Default"+configlist.Particle +  std::to_string(configlist.Nevts)+".root";
            gensim->WritePixelTPCdata(outputfile);
        }
        delete gensim;

    }
    else
    {
        throw std::runtime_error("Error! GenSimDtaParsList needed in task json file");
    }
}

void ProcessManager::StartRecoPixTPCEvts()
{
    if(fPixJsonParser.contains("RecoProcessor")) 
    {
        auto recoprocessor = fPixJsonParser.at("RecoProcessor");
        //Reco Pix TPC hits or track using different processors
        if(recoprocessor == "PixHitRecoSimpleProcessor") 
        {
            auto recoprocessorArray = fPixJsonParser.at("RecoProcessorArray");

            for(auto item : recoprocessorArray)
            {
                TaskConfigStruct::PixTPChitRecoParsList recoprocessor_i = item;
                auto pixhitrecoprocessor = new PixHitRecoSimpleProcessor(recoprocessor_i);
                this->AddProcessor(pixhitrecoprocessor);
            }
        }
        else 
        {
            //TODO using PixClusterSepRecoProcessor, under developing 
        }

        PixTPCLog(PIXtpcINFO,Form("There are %d processors added! ###Start Processing...",this->GetEntries()),false);
        this->StartProcessing();
        
    }
    else 
    {
        throw std::runtime_error("Error! No RecoProcessor in task json file");
    }
}

void ProcessManager::PrintUsage()
{
    std::printf("========================================\n");
    std::printf("Tasktype: [0], Raw data to ROOT         \n");
    std::printf("Tasktype: [1], generate MC data         \n");
    std::printf("Tasktype: [2], TPC evts reconstruction  \n");
    std::printf("Tasktype: [3], TPC calibration          \n");
    std::printf("========================================\n");
}

void ProcessManager::AddProcessor(Processor* processor)
{
    this->Add(processor);
}

void ProcessManager::StartProcessing()
{
    for(int i=0; i<this->GetEntries();++i)
    {
        auto processor_i = dynamic_cast<Processor*>(this->At(i));
        processor_i->InitAction();
        processor_i->ProcessEventAction();
        processor_i->EndAction();
        delete processor_i;
    }
}


