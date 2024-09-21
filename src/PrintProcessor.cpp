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
                Processor("printProcessor"),f_file(filein), f_tree(treein), f_PixTPCdata(nullptr),
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
    f_PixTPCdata = new PixelTPCdata(1);
    f_tree->SetBranchAddress("pixelTPCdata",&f_PixTPCdata);
}

void PrintProcessor::ProcessEventAction()
{
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

}

void PrintProcessor::EndAction()
{
    this->DebugPrint();
}
    

void PrintProcessor::DebugPrint()
{
    //std::cout<<"=======> "<<__ROW__<<std::endl;
    //auto vmaps = CreateChipChnToRowColMap("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/config/ChipChnMaps.csv");
    //std::cout<<"\t"<<vmaps.at(2000).ChipIdx<<"\t"<<vmaps.at(2000).ChnIdx<<std::endl;
    std::printf("=======================> This is end of %s !!!\n",this->GetProcessorName().c_str()); 
}
