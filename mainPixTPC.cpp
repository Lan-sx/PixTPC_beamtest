/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-05 10:00
 * Filename         : mainPixTPC.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/

//std
#include <iostream>

// ROOT CERN
#include "TFile.h"
#include "TTree.h"
#include "TLegend.h"
#include "TApplication.h"
#include "Lansxlogon.h"

// User
#include "Rawdata2ROOT.h"
#include "ProcessManager.h"
#include "PrintProcessor.h"
#include "BeamUnities.h"
#include "GenSimData.h"
#include "PixelMatrix.h"

std::vector<std::pair<int,int>> GlobalMaps;

using namespace std;

void FillPixelTPCdata()
{
    try {
        Rawdata2ROOT* myConvert = new Rawdata2ROOT("/mnt/d/Data/experiment/DESYBeamTest/test/TEPIX_test_canwen/0619_new_4/data_lg_300ns.dat_r.dat");
        myConvert->DoUnpackage();
        delete myConvert;
    }catch (const std::exception& e)
    {
        std::cerr<< "Error: "<< e.what() <<std::endl;
    }
}

void test01()
{
    auto file1 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/build/Test_lg_300ns.root");
    if(!file1) return;
    auto tree1 = dynamic_cast<TTree*>(file1->Get("PixTPCdata"));
    auto processor1 = new PrintProcessor(file1,tree1);
    auto file2 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/build/Test_hg_20ns.root");
    if(!file2) return;
    auto tree2 = dynamic_cast<TTree*>(file2->Get("PixTPCdata"));
    auto processor2 = new PrintProcessor(file2,tree2);

    auto pm = new ProcessManager;
    pm->AddProcessor(processor1);
    pm->AddProcessor(processor2);
    pm->StartProcessing();
    
    auto myc =new TCanvas("myc","myc",800,600);
    myc->SetGrid();
    processor1->GetHistQ()->Draw();
    auto myc1 =new TCanvas("myc1","myc1",800,600);
    myc1->SetGrid();
    myc1->DrawFrame(0,3000,150,5000,";Channel;Q mean [LSB]");
    
    auto grQchn_hg = processor1->GetGrQ();
    grQchn_hg->SetName("grhg");
    auto grQchn_lg = processor2->GetGrQ();
    grQchn_lg->SetName("grlg");
    LansxFormat::FormatAll(grQchn_hg,"%a %d%e",kBlue,kBlue,kFullCircle);
    LansxFormat::FormatAll(grQchn_lg,"%a %d%e",kRed,kRed,kFullCircle);
    grQchn_hg->Draw("P");
    grQchn_lg->Draw("P");
    
    auto leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->SetFillStyle(000);
    leg->AddEntry(grQchn_hg,"High gain, 300ns","lp");
    leg->AddEntry(grQchn_lg,"Low gain, 20ns","lp");
    leg->Draw();
    delete processor1;
    delete processor2;
}

int main(int argc, char** argv)
{
    TApplication app("app",&argc,argv);
    LansxFormat::myStyle();
    //FillPixelTPCdata();
    //test01();
    
    GlobalMaps = BeamUnities::CreateChipChnToRowColMap("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/config/ChipChnMaps.csv");
    
    //auto rowcolpair = BeamUnities::Position2RowCol(0.06,0.06);
    //std::cout<<"-----> "<<rowcolpair.first<<"\t"<<rowcolpair.second<<std::endl;

    //std::cout<<"\t"<<vmaps.at(1000).first<<"\t"<<vmaps.at(1000).second<<std::endl;
    //std::cout<<"=======> "<<BeamUnities::RowColIdx2ChipChn(rowcolpair,vmaps).first<<"\t"
    //         <<BeamUnities::RowColIdx2ChipChn(rowcolpair,vmaps).second<<endl;
    //std::cout<<"### "<<BeamUnities::RowColIdx2Position(rowcolpair).first<<"\t"
    //         <<BeamUnities::RowColIdx2Position(rowcolpair).second<<std::endl;
    //std::cout<<"=======> "<<BeamUnities::ChipChn2RowCol(0,127,vmaps).first<<"\t"
    //         <<BeamUnities::ChipChn2RowCol(0,127,vmaps).second<<"\t"
    //         <<BeamUnities::ChipChn2RowCol(22,54,vmaps).first<<"\t"
    //         <<BeamUnities::ChipChn2RowCol(22,54,vmaps).second<<std::endl;
    
    auto gensim = new GenSimData(500);
    gensim->GenTracks("e-",2.7e+9,20.);
    gensim->WritePixelTPCdata("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/test/GenMCdata_test01.root",GlobalMaps);

    auto myc = new TCanvas("myc","myc",600,600);
    myc->Divide(2,1);
    myc->cd(1);
    gPad->SetRightMargin(0.12);
    gPad->SetGrid();
    gPad->SetLogz();
    auto mat10x300 = gensim->GetPixelMatrix(0);
    auto htrkxy = mat10x300->Matrix2HistReadout();
    htrkxy->Draw("COL Z");

    myc->cd(2);
    gPad->SetRightMargin(0.12);
    gPad->SetGrid();
    gPad->SetLogz();
    auto mat10x300_1 = gensim->GetPixelMatrix(1);
    auto htrkxy1 = mat10x300_1->Matrix2HistReadout();
    htrkxy1->Draw("COL Z");

    //read MC data
    auto MCfile = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/test/GenMCdata_test01.root");
    auto MCtr = dynamic_cast<TTree*>(MCfile->Get("PixTPCdata"));
    auto pixeltpcdata = new PixelTPCdata(__NumChip__);
    MCtr->SetBranchAddress("pixelTPCdata",&pixeltpcdata);
    
    auto Matrix10x300_MC = new PixelMatrix; 


    auto cc1 = new TCanvas("cc1","cc1",600,600);
    cc1->Divide(2,1);
    cc1->cd(1);
    gPad->SetLogz();
    MCtr->GetEntry(0);
    Matrix10x300_MC->PixelTPCdata2PixelMatrix(pixeltpcdata);
    auto htrkxy_mc1 = Matrix10x300_MC->Matrix2HistReadout();
    htrkxy_mc1->Draw("COL Z");

    cc1->cd(2);
    gPad->SetLogz();
    MCtr->GetEntry(1);
    Matrix10x300_MC->Zero();
    Matrix10x300_MC->PixelTPCdata2PixelMatrix(pixeltpcdata);
    auto htrkxy_mc2 = Matrix10x300_MC->Matrix2HistReadout();
    htrkxy_mc2->Draw("COL Z");

    auto Canvasfile = new TFile("../test/Canvasfile.root","RECREATE");
    Canvasfile->cd();
    myc->Write();
    cc1->Write();
    Canvasfile->Close();
    //auto mat300x10 = TMatrixDSparse(TMatrixDSparse::kTransposed,*mat10x300);
    //mat300x10.Draw("COL Z");

    delete gensim;
    std::printf("--------------------> Code end! \n");
    app.Run(kTRUE);
    return 0;
}
