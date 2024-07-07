
//std
#include <iostream>

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TApplication.h"
#include "Lansxlogon.h"

//
#include "Rawdata2ROOT.h"
#include "ProcessManager.h"
#include "PrintProcessor.h"

using namespace std;

void FillPixelTPCdata()
{
    Rawdata2ROOT* myConvert = new Rawdata2ROOT("/mnt/d/Data/experiment/DESYBeamTest/test/TEPIX_test_canwen/0619_new_4/data_lg_300ns.dat_r.dat");
    myConvert->DoUnpackage();

    delete myConvert;
}

void test01()
{
    auto file1 = TFile::Open("/mnt/d/Data/experiment/DESYBeamTest/PixelTPCexpAna/build/Test_hg_300ns.root");
    if(!file1) return;
    auto tree1 = dynamic_cast<TTree*>(file1->Get("PixTPCdata"));
    auto processor1 = new PrintProcessor(file1,tree1);
    auto pm = new ProcessManager;
    pm->AddProcessor(processor1);
    pm->StartProcessing();

    
    auto myc =new TCanvas("myc","myc",800,600);
    myc->SetGrid();
    processor1->GetHistQ()->Draw();
    auto myc1 =new TCanvas("myc1","myc1",800,600);
    myc1->SetGrid();
    processor1->GetHistT()->Draw();
    
    delete processor1;
}

int main(int argc, char** argv)
{
    TApplication app("app",&argc,argv);
    LansxFormat::myStyle();
    //FillPixelTPCdata();
    test01();
    
    std::cout<<"Code end!\n";
    app.Run(kTRUE);
    return 0;
}
