/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:42
 * Filename         : PrintProcessor.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/

#include "PrintProcessor.h"

PrintProcessor::PrintProcessor(TFile *filein, TTree *treein) : 
                Processor("printProcessor"),f_file(filein), f_tree(treein), f_PixTPCdata(nullptr),
                f_histQ(nullptr),f_histT(nullptr)
{
}

void PrintProcessor::InitAction()
{
    f_file->cd();
    f_PixTPCdata = new PixelTPCdata(1);
    f_tree->SetBranchAddress("pixelTPCdata",&f_PixTPCdata);

    f_histQ = new TH1D("hQ","85-th Chn. ;Q [LSB];Cnts",2000,3000,5000);
    f_histT = new TH1D("hT","85-th Chn. ;Q [LSB];Cnts",500,2000,3000);
}

void PrintProcessor::ProcessEventAction()
{
    f_tree->GetEntry(0);
    std::cout<<"===============Entry 0: "<< f_PixTPCdata->Getdata_IJK(0,0,0).first<<"\t"
                                         << f_PixTPCdata->Getdata_IJK(0,0,0).second<<std::endl;
    
    //fill hist Q
    for(auto i_entry=0; i_entry < f_tree->GetEntries(); ++i_entry)
    {
        f_tree->GetEntry(i_entry);
        
        //for(int i=0;i<;++i)
        //{
            f_histQ->Fill(f_PixTPCdata->Getdata_IJK(0,85,0).second);
            f_histT->Fill(f_PixTPCdata->Getdata_IJK(0,85,0).first);
        //}
    }

}

void PrintProcessor::EndAction()
{
    this->DebugPrint();
    delete f_PixTPCdata;
    delete f_tree;
    f_file->Close();
    delete f_file;
}
    

void PrintProcessor::DebugPrint()
{
    std::printf("=======================> Test of %s !!!\n",this->GetProcessorName().c_str()); 
}
