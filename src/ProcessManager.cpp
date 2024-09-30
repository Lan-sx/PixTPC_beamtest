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
    std::printf("==============================================\n");
    std::printf("> Start a Task: %s\n",TaskComments.data());
    std::printf("==============================================\n");
    
    switch(Tasktype)
    {
        case Rawdata2ROOT:
            std::printf("Raw data to root file!\n");
            break;
        case GenMCdata:
            std::printf("GenMCdata!\n");
            this->StartGenMCdata();
            break;
        case TPChitReco:
            std::printf("TPC hits reconstruction!\n");
            break;
        default:
            this->PrintUsage();
            PixTPCLog(PIXtpcINFO,"Dummy task!");
    }

    std::printf("> End of Task:%s\n",TaskComments.data());
    return 0;
}

void ProcessManager::InitialMapsFromJson()
{
    if(fPixJsonParser.contains("GlobalChipChnMaps")) 
    {
        GlobalMaps = BeamUnities::CreateChipChnToRowColMap(fPixJsonParser.at("GlobalChipChnMaps"));
    }
    else
    {
        PixTPCLog(PIXtpcERROR,"No Chip Chn map file");
        throw std::runtime_error("No Chip Chn map file in task json");
    }
        
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

void ProcessManager::PrintUsage()
{
    std::printf("========================================\n");
    std::printf("Tasktype: [0], Raw data to ROOT         \n");
    std::printf("Tasktype: [1], generate MC data         \n");
    std::printf("Tasktype: [2], TPC hits reconstruction  \n");
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
    }
}


