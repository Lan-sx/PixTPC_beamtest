/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Created          : 2024-12-13 22:10
 * Filename         : PixHitRecoCCAProcessor.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "PixHitRecoCCAProcessor.h"

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
    f_PixTPCdata = new PixelTPCdata(__NumChip__);
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
}

void PixHitRecoCCAProcessor::EndAction()
{
    spdlog::get("cepcPixTPClogger")->info("This is end of {}!!!",this->GetProcessorName());
}

void PixHitRecoCCAProcessor::DebugPrint()
{
    spdlog::get("cepcPixTPClogger")->debug(" debug print in PixHitRecoCCAProcessor!");
}


