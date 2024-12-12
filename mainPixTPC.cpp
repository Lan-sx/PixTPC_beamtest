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
#include "TPolyMarker.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TApplication.h"
#include "Lansxlogon.h"

// User
#include "ProcessManager.h"
#include "RawdataConverter.h"
#include "PrintProcessor.h"
#include "PixHitRecoSimpleProcessor.h"
#include "BeamUnities.h"
#include "GenSimData.h"
#include "PixelMatrix.h"
#include "MCTrackdata.h"

std::vector<std::pair<int,int>> GlobalMaps;
std::vector<TaskConfigStruct::ChipChnMaps_V1> GlobalJsonMaps;

using namespace std;


void test01()
{
    printf("==== test01 ====\n");
#if 0
    auto processor3 = new PixHitRecoSimpleProcessor(file3);
    processor3->EnableProceesorDebug();

    auto pm = new ProcessManager;
    pm->AddProcessor(processor3);
    pm->StartProcessing();

    auto file1 = TFile::Open("/mnt/e/WorkSpace/GitRepo/PixTPC_beamtest/build/Test_lg_300ns.root");
    if(!file1) return;
    auto tree1 = dynamic_cast<TTree*>(file1->Get("PixTPCdata"));
    auto processor1 = new PrintProcessor(file1,tree1);
    //auto file2 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/build/Test_hg_20ns.root");
    //if(!file2) return;
    //auto tree2 = dynamic_cast<TTree*>(file2->Get("PixTPCdata"));
    //auto processor2 = new PrintProcessor(file2,tree2);
    //auto file3 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/build/GenMCdata_withNoise_Z24cm.root");
    //auto file3 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/test/MCdata_withNoise_Z24.0cm_x0.21.root");

    processor1->InitAction();
    processor1->ProcessEventAction();
    processor1->EndAction();
    auto myc1 =new TCanvas("myc1","myc1",800,600);
    myc1->SetGrid();
    myc1->DrawFrame(3000,0,5000,800,";Channel;Q mean [LSB]");
    auto hq1 = processor1->GetHistQ();
    if(!hq1)
    {
        cout<<"=======KKKKJj"<<endl;
        return;
    }
    cout<<"=========> "<<hq1->GetEntries()<<"\t"<<hq1->GetMean()<<endl;
    LansxFormat::FormatAll(hq1,"%a",kBlue);
    hq1->DrawCopy();

    auto myc2 =new TCanvas("myc2","myc2",800,600);
    myc2->SetGrid();
    myc2->DrawFrame(0,3000,150,5000,";Channel;Q mean [LSB]");

    auto grQchn_hg = processor1->GetGrQ();
    grQchn_hg->SetName("grhg");
    //auto grQchn_lg = processor2->GetGrQ();
    //grQchn_lg->SetName("grlg");
    LansxFormat::FormatAll(grQchn_hg,"%a %d%e",kBlue,kBlue,kFullCircle);
    //LansxFormat::FormatAll(grQchn_lg,"%a %d%e",kRed,kRed,kFullCircle);
    grQchn_hg->Draw("P");
    //grQchn_lg->Draw("P");

    auto leg = new TLegend(0.6,0.7,0.9,0.9);
    leg->SetFillStyle(000);
    leg->AddEntry(grQchn_hg,"High gain, 300ns","lp");
    //leg->AddEntry(grQchn_lg,"Low gain, 20ns","lp");
    leg->Draw();
    delete processor1;
#endif

}

void test02()
{
    //auto MCfile = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/test/GenMCdata_withNoise01_new.root");
    auto MCfile = TFile::Open("/mnt/e/WorkSpace/GitRepo/PixTPC_beamtest/task/Test01.root");
    auto MCtr = dynamic_cast<TTree*>(MCfile->Get("PixTPCdata"));
    auto pixeltpcdata = new PixelTPCdata(__NumChip__);
    auto mctrackdata  = new MCTrackdata;
    MCtr->SetBranchAddress("pixelTPCdata",&pixeltpcdata);
    MCtr->SetBranchAddress("mcTrackdata",&mctrackdata);
    //Get first track
    MCtr->GetEntry(0);

    auto Matrix10x300_MC = new PixelMatrix; 

    auto cc1 = new TCanvas("cc1","cc1",600,600);
    cc1->Divide(2,1);
    cc1->cd(1);
    gPad->SetLogz();
    Matrix10x300_MC->PixelTPCdata2PixelMatrix(pixeltpcdata);
    //auto mat300x10 = TMatrixDSparse(TMatrixDSparse::kTransposed,*Matrix10x300_MC);
    //mat300x10.Draw("Surf2 Z");
    auto htrkxy_mc1 = Matrix10x300_MC->Matrix2HistReadout();
    htrkxy_mc1->Draw("COL Z");

    auto vClusterdata = mctrackdata->GetClusterVec();
    auto clusterSize = vClusterdata.size();
    auto grClsTruth = new TGraph(clusterSize);
    for(size_t ii=0;ii<clusterSize;++ii)
    {
        //polytrack->SetPoint(ii,vClusterdata.at(ii).x(),
        //                       vClusterdata.at(ii).y(),
        //                       vClusterdata.at(ii).z());
        auto xyzt = vClusterdata.at(ii);
        grClsTruth->SetPoint(ii,xyzt.x(),xyzt.y());
    }
    LansxFormat::FormatAll(grClsTruth,"%d %e %f",kOrange-3,kFullStar,2);
    grClsTruth->Draw("P");

    cc1->cd(2);
    gPad->SetLogz();
    htrkxy_mc1->Draw("COL");
    //test DFS_algo
    auto clusters = BeamUnities::PixClusterFinder(*Matrix10x300_MC,true);
    int icolor = 0;
    TPolyMarker *clusterMarker = nullptr;
    for(const auto cluster : clusters)
    {
        clusterMarker = new TPolyMarker(cluster.size());
        LansxFormat::FormatAll(clusterMarker,"%d %e",ColorArray[icolor%9],kFullSquare);
        int ii=0;
        for(auto rowcolpair : cluster)
        {
            auto xpyp_pair = BeamUnities::RowColIdx2Position(rowcolpair);
            clusterMarker->SetPoint(ii,xpyp_pair.first,xpyp_pair.second);
            ii++;
        }
        clusterMarker->Draw("P");
        icolor++;
    }
     
}


int main(int argc, char** argv)
{
    int Argc=argc;
    char** Argv = new char*[Argc];
    for(int i=0;i<argc;++i)
    {
        Argv[i] = new char[strlen(argv[i]+1)];
        strcpy(Argv[i],argv[i]);
    }

    TApplication app("app",&Argc,Argv);
    LansxFormat::myStyle();

    //New log based on spdlog lib
    std::shared_ptr<spdlog::logger> cepcPixTPCconsole = spdlog::stdout_color_mt("cepcPixTPClogger");
    cepcPixTPCconsole->set_pattern("[cepcPixTPC %l]: %v");

    if(argc < 2)
    {
        PixTPCLog(PIXtpcINFO,"No task json file! Testing",false);
        auto myc = new TCanvas("myc","myc",800,600);
        myc->SetGrid();
        myc->DrawFrame(0,0,1,1,"Test;x;y");

        auto CEPCPixtpcRunManager = new ProcessManager;
        try {
            CEPCPixtpcRunManager->InitialMapsManually("/mnt/d/Data/experiment/DESYBeamTest/PixTPC_beamtest/config/ChipChnMapsV1.csv");
            //CEPCPixtpcRunManager->InitialMapsFromJson()
            test01();
            //test02();
            //test03(std::atof(argv[2]));
            delete CEPCPixtpcRunManager;
        } catch (const std::exception& e)
        {
            PixTPCLog(PIXtpcERROR,e.what(),true);
        }
    }
    else if(argc==2)
    {
        std::string taskfilestr(argv[1]);
        try {
            auto CEPCPixtpcRunManager = new ProcessManager(taskfilestr);
            CEPCPixtpcRunManager->CEPCPixtpcRun();
            delete CEPCPixtpcRunManager;
        } catch (const std::exception& e)
        {
            PixTPCLog(PIXtpcERROR,e.what(),true);
        }
    }
    else{
        PixTPCLog(PIXtpcINFO,"uncorrected input paras",true);
        return -1;
    }
    
#if 0 
    //test print for GlobalMaps
    auto rowcolpair = BeamUnities::Position2RowCol(0.06,0.06);
    std::cout<<"-----> "<<rowcolpair.first<<"\t"<<rowcolpair.second<<std::endl;

    std::cout<<"\t"<<vmaps.at(1000).first<<"\t"<<vmaps.at(1000).second<<std::endl;
    std::cout<<"=======> "<<BeamUnities::RowColIdx2ChipChn(rowcolpair,vmaps).first<<"\t"
        <<BeamUnities::RowColIdx2ChipChn(rowcolpair,vmaps).second<<endl;
    std::cout<<"### "<<BeamUnities::RowColIdx2Position(rowcolpair).first<<"\t"
        <<BeamUnities::RowColIdx2Position(rowcolpair).second<<std::endl;
    std::cout<<"=======> "<<BeamUnities::ChipChn2RowCol(0,127,vmaps).first<<"\t"
        <<BeamUnities::ChipChn2RowCol(0,127,vmaps).second<<"\t"
        <<BeamUnities::ChipChn2RowCol(22,54,vmaps).first<<"\t"
        <<BeamUnities::ChipChn2RowCol(22,54,vmaps).second<<std::endl;
#endif
    cepcPixTPCconsole->info("--------------------> Code end!");
    app.Run(kTRUE);
    return 0;
}
