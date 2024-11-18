/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:42
 * Filename         : PrintProcessor.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
//std
#include <vector>

//ROOT CERN
#include "TFitResult.h"
#include "TFitResultPtr.h"

#include "PrintProcessor.h"

PrintProcessor::PrintProcessor(TFile *filein, TTree *treein) : 
                Processor("PrintProcessor",0),f_file(filein), f_tree(treein), f_PixTPCdata(nullptr),
                f_histQ(nullptr),f_histT(nullptr)
{
    fhQ128 = new TH1D*[128];
    for(int i=0;i<128;++i)
    {
        fhQ128[i] = new TH1D(Form("hQ%d",i),Form("%d-th Chn.;Q [LSB];Cnts",i),2000,3000,5000);
    }
    fgrQ = nullptr;
    f_histQ = new TH1D("hQ",";Q [LSB];Cnts",2000,3000,5000);
    f_histT = new TH1D("hT",";Q [LSB];Cnts",500,2000,3000);
}

PrintProcessor::PrintProcessor(TaskConfigStruct::PrintProcessorParsList inputPars) :
                Processor("PrintProcessor",inputPars.Processorid), f_PixTPCdata(nullptr),
                f_histQ(nullptr),f_histT(nullptr)

{
    f_InputPars = inputPars;
    fIsdebug = f_InputPars.Isdebug;
    f_file = TFile::Open(f_InputPars.Inputfile.data());
    if(!f_file)
        throw std::runtime_error("INPUT FILE ERROR");

    f_tree = dynamic_cast<TTree*>(f_file->Get("PixTPCdata"));
    //f_InputPars.NumberOfChips = inputPars.NumberOfChips; 
    //f_InputPars.HistPars = inputPars.HistPars;
}

PrintProcessor::~PrintProcessor()
{
    delete f_PixTPCdata;
    delete f_tree;
    f_file->Close();
    delete f_file;
}

void PrintProcessor::InitAction()
{
    f_file->cd();
    f_PixTPCdata = new PixelTPCdata(f_InputPars.NumberOfChips);
    if(!f_tree->GetBranch(f_InputPars.Inputbranch.c_str()) || f_tree->GetEntries()<=0)
        throw  std::runtime_error("ERROR INPUT Branch NAME Or 0 entry");
    else
        f_tree->SetBranchAddress("pixelTPCdata",&f_PixTPCdata);

    PixTPCLog(PIXtpcINFO,Form("%s InitAction Done!",this->GetProcessorName().c_str()),false);
    
}

void PrintProcessor::ProcessEventAction()
{
    auto print_type = f_InputPars.PrintType;
    
    switch(print_type)
    {
        case Print1DHistQT:
            this->Plot1DHistQT();
            break;
        case Print1DHistQT_IO:
            this->Plot1DHistQT_IO();
            break;
        case Print2DHistQT_Pos:
            this->Plot2DHistQT_Pos();
            break;
        default:
            break;
    }

#if 0
    f_tree->GetEntry(0);
    auto vecOverThresholdEvts = f_PixTPCdata->Getdata_IJ(0,0);
    if(vecOverThresholdEvts.size()>0)
    {
        std::cout<<"===============Entry 0: "<< vecOverThresholdEvts.at(0).first<<"\t"
            << vecOverThresholdEvts.at(0).second<<std::endl;
    }

    //fill hist Q
    for(auto i_entry=0; i_entry < f_tree->GetEntries(); ++i_entry)
    {
        f_tree->GetEntry(i_entry);

        //for(int i=0;i<;++i)
        //{
        f_histQ->Fill(f_PixTPCdata->Getdata_IJ(0,85).at(0).second);
        f_histT->Fill(f_PixTPCdata->Getdata_IJ(0,85).at(0).first);
        //}
        for(int i_chn=0; i_chn<128; ++i_chn)
        {
            fhQ128[i_chn]->Fill(f_PixTPCdata->Getdata_IJ(0,i_chn).at(0).second);
        }
    }


    std::vector<double> vMeanQ(128,0),vSigmaQ(128,0);
    std::vector<double> vChn(128,0);
    for(int i_chn=0;i_chn<128;++i_chn)
    {
        if(fhQ128[i_chn]->GetEntries()>1000)
        {
            auto fitRet = fhQ128[i_chn]->Fit("gaus","QS0");
            auto gausParas = fitRet->GetParams();
            auto chi2overndf = fitRet->Chi2()/fitRet->Ndf(); 
            if(chi2overndf<10)
            {
                vMeanQ[i_chn] = gausParas[1];
                vSigmaQ[i_chn] = gausParas[2];
                //std::cout<<"===>"<<i_chn<<"\t"<<gausParas[1]<<"\t"<<gausParas[2]<<" Chi2/NDF= "<<chi2overndf<<std::endl;
            }
        }
        vChn[i_chn] = i_chn;
    }

    fgrQ = new TGraphErrors(vMeanQ.size(),vChn.data(),vMeanQ.data(),nullptr,vSigmaQ.data());
#endif
}

void PrintProcessor::EndAction()
{
    if(fIsdebug) 
        this->DebugPrint();
}
    

void PrintProcessor::DebugPrint()
{
    //std::cout<<"=======> "<<__ROW__<<std::endl;
    //auto vmaps = CreateChipChnToRowColMap("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/config/ChipChnMaps.csv");
    //std::cout<<"\t"<<vmaps.at(2000).ChipIdx<<"\t"<<vmaps.at(2000).ChnIdx<<std::endl;
    std::printf("=======================> This is end of %s !!!\n",this->GetProcessorName().c_str()); 
}


TCanvas* PrintProcessor::Plot2DHistQT_Pos()
{
    int plot2d_entry_id = f_InputPars.PlotPattern[0];
    if(plot2d_entry_id > f_tree->GetEntries() || plot2d_entry_id < 0)
    {
        plot2d_entry_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input EntryID (PlotPattern[0]) out of range, plot 0-th entry",false);
    }

    int plot2d_overthreshold_id = f_InputPars.PlotPattern[3];
    if(plot2d_overthreshold_id >= 4 || plot2d_overthreshold_id < 0)
    {
        plot2d_overthreshold_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input OverThresholdID (PlotPattern[3]) out of range, plot 0-th over threshold evt",false);
    }
    
    char QT_flags = 'Q';
    QT_flags = f_InputPars.PlotPattern[4]>=0 ? 'Q' : 'T';

    f_tree->GetEntry(plot2d_entry_id);

    auto mat10x300_exp = new PixelMatrix(f_InputPars.NumberOfChips);
    mat10x300_exp->SetOverTh_ID(plot2d_overthreshold_id);
    mat10x300_exp->PixelTPCdata2PixelMatrix(f_PixTPCdata,QT_flags);
    auto histxy = mat10x300_exp->Matrix2HistReadout();
    //TODO, plot histxy name

    std::string Cname = "CQvsPos"+std::to_string(fProcessorId);
    auto myc = new TCanvas(Cname.c_str(),Cname.c_str(),300,600);
    //myc->SetRightMargin(0.12);
    myc->SetGrid();
    histxy->DrawCopy("COL");
    delete mat10x300_exp;
    return myc;
}

TCanvas* PrintProcessor::Plot1DHistQT_IO()
{
    int plot2d_entry_id = f_InputPars.PlotPattern[0];
    if(plot2d_entry_id > f_tree->GetEntries() || plot2d_entry_id < 0)
    {
        plot2d_entry_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input EntryID (PlotPattern[0]) out of range, plot 0-th entry",false);
    }

    int plot1d_chip_id = f_InputPars.PlotPattern[1];
    if(plot1d_chip_id >= f_InputPars.NumberOfChips || plot1d_chip_id<0)
    {
        plot1d_chip_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input ChipID (PlotPattern[1]) out of range, plot 0-th chip data",false);
    }

    //int plot2d_overthreshold_id = f_InputPars.PlotPattern[3];
    //if(plot2d_overthreshold_id >= 4 || plot2d_overthreshold_id < 0)
    //{
    //    plot2d_overthreshold_id = 0;
    //    PixTPCLog(PIXtpcWARNING,"Input OverThresholdID (PlotPattern[3]) out of range, plot 0-th over threshold evt",false);
    //}
    
    char QT_flags = 'Q';
    QT_flags = f_InputPars.PlotPattern[4]>=0 ? 'Q' : 'T';

    f_tree->GetEntry(plot2d_entry_id);
    
    auto suffix_histname = f_InputPars.HistPars.Histname;
    std::string histnameArray[4] = {suffix_histname + "0",
                                    suffix_histname + "1",
                                    suffix_histname + "2",
                                    suffix_histname + "3"
                                    };

    auto histQT_chn0  = std::make_unique<TH1D>(histnameArray[0].c_str(),
                                              f_InputPars.HistPars.Histtitle.c_str(),128,0,128);

    auto histQT_chn1  = std::make_unique<TH1D>(histnameArray[1].c_str(),
                                              f_InputPars.HistPars.Histtitle.c_str(),128,0,128);

    auto histQT_chn2  = std::make_unique<TH1D>(histnameArray[2].c_str(),
                                              f_InputPars.HistPars.Histtitle.c_str(),128,0,128);

    auto histQT_chn3  = std::make_unique<TH1D>(histnameArray[3].c_str(),
                                              f_InputPars.HistPars.Histtitle.c_str(),128,0,128);
    
    histQT_chn0->SetLineColor(ColorArray[2]);
    histQT_chn1->SetLineColor(ColorArray[3]);
    histQT_chn2->SetLineColor(ColorArray[4]);
    histQT_chn3->SetLineColor(ColorArray[5]);

    for(int chn_i=0; chn_i < __NumChn__; ++chn_i)
    {
        if((*f_PixTPCdata)(plot1d_chip_id,chn_i).size()==4)
        {
            auto histcontent0 = QT_flags == 'Q' ? (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(0).second :
                (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(0).first;

            auto histcontent1 = QT_flags == 'Q' ? (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(1).second :
                (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(1).first;

            auto histcontent2 = QT_flags == 'Q' ? (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(2).second :
                (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(2).first;
            
            auto histcontent3 = QT_flags == 'Q' ? (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(3).second :
                (*f_PixTPCdata)(plot1d_chip_id,chn_i).at(3).first;
            
            histQT_chn0->SetBinContent(chn_i+1,histcontent0);
            histQT_chn1->SetBinContent(chn_i+1,histcontent1);
            histQT_chn2->SetBinContent(chn_i+1,histcontent2);
            histQT_chn3->SetBinContent(chn_i+1,histcontent3);
        }
    }

    std::string Cname = "CQT_IO_hist"+std::to_string(fProcessorId);
    auto myc = new TCanvas(Cname.c_str(),Cname.c_str(),800,600);
    myc->DrawFrame(f_InputPars.HistPars.HistXYstart[0],
                   f_InputPars.HistPars.HistXYend[0],
                   f_InputPars.HistPars.HistXYstart[1],
                   f_InputPars.HistPars.HistXYend[1],
                   Form("Entry%d,%d-th Chip",plot2d_entry_id,plot1d_chip_id));

    myc->SetGrid();
    histQT_chn0->DrawCopy("SAME");
    histQT_chn1->DrawCopy("SAME");
    histQT_chn2->DrawCopy("SAME");
    histQT_chn3->DrawCopy("SAME");
    
    auto lg = myc->BuildLegend(0.65,0.72,0.9,0.92,"","L");
    lg->SetFillStyle(000);

    //remove "hframe" legend entry
    auto obj1 = lg->GetListOfPrimitives()->At(0);
    //std::cout<<"[==========] :"<<lg->GetListOfPrimitives()->GetSize()<<std::endl;
    auto legEntry = dynamic_cast<TLegendEntry*>(obj1);
    lg->GetListOfPrimitives()->Remove(legEntry);
    myc->Update();
    
    return  myc;

    //find globalIdx and IOidx
    //auto Q_pixel = (*f_PixTPCdata)(plot1d_chip_id,0).at(plot2d_overthreshold_id).second;
    //auto maping_i = std::find_if(GlobalJsonMaps.begin(),GlobalJsonMaps.end(),
    //                             [=](TaskConfigStruct::ChipChnMaps_V1 mapitem){ 
    //                                if(mapitem.chipchnIdx[0] == plot1d_chip_id && mapitem.chipchnIdx[1] == 0)  
    //                                    return true;
    //                                else 
    //                                    return false;
    //                             });
    //if(maping_i != GlobalJsonMaps.end())
    //    std::printf("IO idx of (chip%d,chn%d)=%s,and Q/T=%.2f\n",plot1d_chip_id,0,(*maping_i).IOidxArr[0].c_str(),Q_pixel);
    //else
    //    PixTPCLog(PIXtpcERROR,"Chip Chn NOT FOUND",false);
}

TCanvas* PrintProcessor::Plot1DHistQT()
{
    int plot1d_entry_id = f_InputPars.PlotPattern[0];
    if(plot1d_entry_id > f_tree->GetEntries())
    {
        plot1d_entry_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input EntryID (PlotPattern[0]) out of range, plot 0-th entry",false);
    }
    int entryid_start = plot1d_entry_id < 0 ? 0 : plot1d_entry_id;
    int entryid_end   = plot1d_entry_id < 0 ? f_tree->GetEntries() : plot1d_entry_id+1;

    int plot1d_chip_id = f_InputPars.PlotPattern[1];
    if(plot1d_chip_id >= f_InputPars.NumberOfChips)
    {
        plot1d_chip_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input ChipID (PlotPattern[1]) out of range, plot 0-th chip data",false);
    }
    int chipid_start = plot1d_chip_id < 0 ? 0 : plot1d_chip_id;
    int chipid_end   = plot1d_chip_id < 0 ? f_InputPars.NumberOfChips : plot1d_chip_id + 1;

    int plot1d_chn_id = f_InputPars.PlotPattern[2];
    if(plot1d_chn_id >= __NumChn__)
    {
        plot1d_chn_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input ChnID (PlotPattern[2]) out of range, plot 0-th channel data",false);
    }
    int chnid_start = plot1d_chn_id < 0 ? 0 : plot1d_chn_id;
    int chnid_end   = plot1d_chn_id < 0 ? __NumChn__ : plot1d_chn_id + 1;

    int plot1d_overtheshold_id = f_InputPars.PlotPattern[3];
    if(plot1d_overtheshold_id >= 4)
    {
        plot1d_chn_id = 0;
        PixTPCLog(PIXtpcWARNING,"Input OverThresholdID (PlotPattern[3]) out of range, plot 0-th over threshold data",false);
    }
    int overThreshold_start = plot1d_overtheshold_id < 0 ? 0 : plot1d_overtheshold_id;
    int overThreshold_end   = plot1d_overtheshold_id < 0 ? 4 : plot1d_overtheshold_id + 1;

    char QT_flags = 'Q';
    QT_flags = f_InputPars.PlotPattern[4]>=0 ? 'Q' : 'T';
    
    auto histQT = std::make_unique<TH1D>(f_InputPars.HistPars.Histname.c_str(),
                                         f_InputPars.HistPars.Histtitle.c_str(),
                                         f_InputPars.HistPars.Histbins[0],
                                         f_InputPars.HistPars.HistXYstart[0],
                                         f_InputPars.HistPars.HistXYend[0]);

    //5-fold cycle
    for(int ii0 = entryid_start; ii0 < entryid_end; ++ii0)
    {
        f_tree->GetEntry(ii0);
        for(int ii1 = chipid_start; ii1 < chipid_end; ++ii1)     
        {
            for(int ii2 = chnid_start; ii2 < chnid_end; ++ii2)
            {
                auto tmp_size = (*f_PixTPCdata)(ii1,ii2).size();
                if(tmp_size>0)
                {
                    //only for calibration signal case: 
                    for(int ii3 = overThreshold_start; ii3 < overThreshold_end; ++ii3)
                    {
                         auto histcontent = QT_flags == 'Q' ? (*f_PixTPCdata)(ii1,ii2).at(ii3).second :
                                                              (*f_PixTPCdata)(ii1,ii2).at(ii3).first;

                         histQT->Fill(histcontent);
                    }
                    //FIXME,
                    //for external trigger case: only plot 0-th over threshold QT
                }
            }
        }
    }

    std::string Cname = "CQT_hist"+std::to_string(fProcessorId);
    auto myc = new TCanvas(Cname.c_str(),Cname.c_str(),800,600);
    myc->SetGrid();
    histQT->DrawCopy();

    return  myc;
}


