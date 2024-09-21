/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-20 21:33
 * Filename         : PixHitRecoSimpleProcessor.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "PixHitRecoSimpleProcessor.h"

#include "TCanvas.h"
#include "TH1D.h"
#include "TGraph.h"
#include "Lansxlogon.h"

PixHitRecoSimpleProcessor::PixHitRecoSimpleProcessor(TFile* filein,TTree* treein) :
                           Processor("PixHitRecoSimpleProcessor"),
                           f_file_in(filein),f_tree_in(treein),
                           f_file_out(nullptr), f_tree_out(nullptr),f_PixTPCdata(nullptr)
{
}

PixHitRecoSimpleProcessor::PixHitRecoSimpleProcessor(TFile* filein,std::string treeName) : 
                           Processor("PixHitRecoSimpleProcessor"), 
                           f_file_in(filein),f_tree_in(nullptr),
                           f_file_out(nullptr), f_tree_out(nullptr),f_PixTPCdata(nullptr)
{
    f_tree_in = dynamic_cast<TTree*>(f_file_in->Get(treeName.data()));
}


PixHitRecoSimpleProcessor::~PixHitRecoSimpleProcessor()
{
    delete f_tree_in;
    delete f_file_in;
    if(f_PixTPCdata)
        delete  f_PixTPCdata;
}


void PixHitRecoSimpleProcessor::InitAction()
{
    f_file_in->cd();
    f_PixTPCdata = new PixelTPCdata(__NumChip__);
    f_tree_in->SetBranchAddress("pixelTPCdata",&f_PixTPCdata);
}

void PixHitRecoSimpleProcessor::ProcessEventAction()
{
    //std::printf("=====> %lld\n", f_tree_in->GetEntries()) ;
    //f_tree_in->GetEntry(0);
    
    std::vector<FourVector> recoHits;
    TH1D** hxrecs = new TH1D*[30]; 
    for(int i=0;i<30;++i)
        hxrecs[i] = new TH1D(Form("hxrec%d",i),";x [cm];Evts",100,-0.5,0.5);

    for(int i_entry=0; i_entry < f_tree_in->GetEntries(); ++i_entry)
    {
        if(i_entry%100==0)
            std::printf("##%d tracks done!\n",i_entry);
        
        f_tree_in->GetEntry(i_entry);
        auto Matrix10x300_MC = new PixelMatrix; 
        Matrix10x300_MC->PixelTPCdata2PixelMatrix(f_PixTPCdata);

        for(int colidx =0;  colidx < __COL__; colidx=colidx+10)
        {
            double sumQij(0.),sumQXij(0.),sumQYij(0.);
            for(int colsub=0;colsub<10;++colsub)
            {
                int cols = colidx + colsub;
                for(int rowidx =0; rowidx < __ROW__; ++rowidx)
                {
                    auto Qij = (*Matrix10x300_MC)(rowidx,cols);
                    if(Qij !=0 )
                    {
                        auto xpyp_pair = BeamUnities::RowColIdx2Position(rowidx,cols);
                        sumQij += Qij;
                        sumQXij += Qij * xpyp_pair.first;
                        sumQYij += Qij * xpyp_pair.second;
                    }
                }
            }
            auto recoXp = sumQXij/sumQij;
            auto recoYp = sumQYij/sumQij;
            auto recodE = sumQij;
            recoHits.push_back({recoXp,recoYp,20.,recodE});
        }
        if(i_entry<10)
            std::cout<<recoHits.size()<<"\t"<<recoHits.at(0).x()<<std::endl;
        //hxrec->Fill(recoHits.at(14).x()-0.225);
        for(int i=0;i<30;++i)
        {
            hxrecs[i]->Fill(recoHits.at(i).x()-0.225);
        }
        delete Matrix10x300_MC;
        recoHits.clear();
    }

    //std::cout<<"Number of Reco Hits: "<< recoHits.size()<<std::endl;
    //int cnts=0;
    //auto grRec = new TGraph(30);
    //for(auto hit : recoHits)
    //{
    //    if(cnts < 10)
    //        std::cout<<"("<<hit.x()<<","<<hit.y()<<","<<hit.z()<<")"<<", and dE "<<hit.e()<<std::endl;

    //    grRec->SetPoint(cnts,hit.x(),hit.y());
    //    cnts++;
    //}
    
    //std::vector<double> rms_x_rec;
    auto grxrec_rms = new TGraph(30);
    for(int i=0;i<30;++i)
    {
        grxrec_rms->SetPoint(i,i,hxrecs[i]->GetRMS());
        delete hxrecs[i];
    }
    delete []hxrecs;

    auto cc1 = new TCanvas("cc1","cc1",800,600);
    //cc1->SetLogz();
    cc1->DrawFrame(0,0.09,30,0.015,"5 GeV/#it{c} e^{-},B=1T;hit id;RMS of (x_{rec}-x_{track}) [cm]");
    cc1->SetGrid();
    LansxFormat::FormatAll(grxrec_rms,"%d %e %f",kOrange-3,kFullStar,2);
    grxrec_rms->Draw("P");
    //hxrec->DrawCopy();
    //auto htrkxy_mc1 = Matrix10x300_MC->Matrix2HistReadout();
    //htrkxy_mc1->Draw("COL");
}

void PixHitRecoSimpleProcessor::EndAction()
{
    this->DebugPrint();
}

void PixHitRecoSimpleProcessor::DebugPrint()
{
    std::printf("$$$$$$$Debug Print: This is end of %s !!!\n",this->GetProcessorName().c_str()); 
}





