/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Created          : 2024-12-13 22:10
 * Filename         : PixHitRecoCCAProcessor.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "PixHitRecoCCAProcessor.h"
#include "TPolyMarker.h"
#include "Lansxlogon.h"

PixHitRecoCCAProcessor::PixHitRecoCCAProcessor(TaskConfigStruct::PixTPChitRecoParsList recoparaslist) :
    Processor("PixHitRecoCCAProcessor",recoparaslist.Processorid),
    fInputBranchName(recoparaslist.Inputbranch),
    fOutputfileName(recoparaslist.Outputfile),fOutputBranchName(recoparaslist.Outputbranch)
{
    fIsdebug = recoparaslist.Isdebug;
    
    f_file_in = TFile::Open(recoparaslist.Inputfile.data());
    if(!f_file_in)
        throw std::runtime_error("INPUT FILE ERROR!");

    f_tree_in = dynamic_cast<TTree*>(f_file_in->Get("PixTPCdata"));
    
}


PixHitRecoCCAProcessor::~PixHitRecoCCAProcessor()
{
    delete f_tree_in;
    delete f_file_in;
    if(f_PixTPCdata)
        delete  f_PixTPCdata;
}


void PixHitRecoCCAProcessor::InitAction()
{
    f_file_in->cd();
    f_PixTPCdata = new PixelTPCdata(__NumChip__);// TODO, get from json file
    if(!f_tree_in->GetBranch(fInputBranchName.c_str()))
        throw std::runtime_error("ERROR INPUT Branch Name");
    else
        f_tree_in->SetBranchAddress(fInputBranchName.c_str(),&f_PixTPCdata);

    spdlog::get("cepcPixTPClogger")->info("{} InitAction Done!",this->GetProcessorName());
}

void PixHitRecoCCAProcessor::ProcessEventAction()
{
    spdlog::get("cepcPixTPClogger")->info("Test print in PixHitRecoCCAProcessor");
    spdlog::get("cepcPixTPClogger")->info("There are {} entries in input tree:{}",f_tree_in->GetEntries(),fInputBranchName);

    f_tree_in->GetEntry(0);

    auto cc1 = new TCanvas("cc1","cc1",300,600);
    cc1->SetLogz();
    cc1->SetGrid();

    auto Matrix10x300_MC = new PixelMatrix; 
    Matrix10x300_MC->PixelTPCdata2PixelMatrix(f_PixTPCdata);
    auto htrkxy_mc1 = Matrix10x300_MC->Matrix2HistReadout();
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

void PixHitRecoCCAProcessor::EndAction()
{
    spdlog::get("cepcPixTPClogger")->info("This is end of {}!!!",this->GetProcessorName());
}

void PixHitRecoCCAProcessor::DebugPrint()
{
    spdlog::get("cepcPixTPClogger")->debug(" debug print in PixHitRecoCCAProcessor!");
}


