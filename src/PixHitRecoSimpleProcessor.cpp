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

PixHitRecoSimpleProcessor::PixHitRecoSimpleProcessor(TaskConfigStruct::PixTPChitRecoParsList recoparaslist) :
    Processor("PixHitRecoSimpleProcessor",recoparaslist.Processorid),
    fEquivalentPad(false),fNumOfColMerge(recoparaslist.NumOfColMerge),
    fInputBranchName(recoparaslist.Inputbranch),
    fOutputfileName(recoparaslist.Outputfile),fOutputBranchName(recoparaslist.Outputbranch)
{
    fIsdebug = recoparaslist.Isdebug;
    fEquivalentPad = recoparaslist.EquivalentPad;
    
    f_file_in = TFile::Open(recoparaslist.Inputfile.data());
    if(!f_file_in)
        throw std::runtime_error("INPUT FILE ERROR!");

    f_tree_in = dynamic_cast<TTree*>(f_file_in->Get("PixTPCdata"));
}

PixHitRecoSimpleProcessor::PixHitRecoSimpleProcessor(TFile* filein,TTree* treein) :
    Processor("PixHitRecoSimpleProcessor",0),
    fEquivalentPad(false),fNumOfColMerge(10),
    fInputBranchName("pixelTPCdata"),fOutputfileName("./RecoTest.root"),fOutputBranchName("recoHitsdata"),
    f_file_in(filein),f_tree_in(treein),
    f_PixTPCdata(nullptr)//,f_file_out(nullptr), f_tree_out(nullptr)
{
}

PixHitRecoSimpleProcessor::PixHitRecoSimpleProcessor(TFile* filein,std::string treeName) : 
    Processor("PixHitRecoSimpleProcessor",0), 
    fEquivalentPad(false),fNumOfColMerge(10),
    f_file_in(filein),f_tree_in(nullptr),
    fInputBranchName("pixelTPCdata"),fOutputfileName("./RecoTest.root"),fOutputBranchName("recoHitsdata"),
    f_PixTPCdata(nullptr)//,f_file_out(nullptr), f_tree_out(nullptr)
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
    if(!f_tree_in->GetBranch(fInputBranchName.c_str()))
        throw std::runtime_error("ERROR INPUT Branch Name");
    else
        f_tree_in->SetBranchAddress(fInputBranchName.c_str(),&f_PixTPCdata);

    PixTPCLog(PIXtpcINFO,Form("%s InitAction Done!",this->GetProcessorName().c_str()));
}

void PixHitRecoSimpleProcessor::ProcessEventAction()
{
    //TH1D** hxrecs = new TH1D*[30]; 
    //for(int i=0;i<30;++i)
    //    hxrecs[i] = new TH1D(Form("hxrec%d",i),";x [cm];Evts",100,-0.5,0.5);
    if(fEquivalentPad)
    {
        UseEquivalentPadMethod();
    }
    else
    {
        UsePixelChargeCenterMethod();
    }

#if 0
    double x_pad_pos[5] = {0.025,0.125,0.225,0.325,0.425};
    auto hmethod1 = new TH1D("hmethod1",";x_{rec}-x_{track} [cm];Evts",200,-0.5,0.5);
    //auto hmethod2 = new TH1D("hmethod2",";x_{rec}-x_{track} [cm];Evts",200,-0.5,0.5);
    //TH2Poly* htrkxy_mc1 = nullptr;
    //for(int i_entry=0; i_entry < f_tree_in->GetEntries(); ++i_entry)
    for(int i_entry=0; i_entry < f_tree_in->GetEntries(); ++i_entry)
    {
        if(i_entry%100==0)
            std::printf("##%d tracks done!\n",i_entry);

        f_tree_in->GetEntry(i_entry);

        auto Matrix10x300_MC = new PixelMatrix; 
        Matrix10x300_MC->PixelTPCdata2PixelMatrix(f_PixTPCdata);

        // Method1
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
            //std::cout<<"Method1 :"<<sumQXij/sumQij<<std::endl;
            if(colidx==140)
                hmethod1->Fill(sumQXij/sumQij-0.21);
        }
        // Method2
        for(int kk=0;kk<30;++kk)
        {
            std::array<double,5> sumQi;
            sumQi.fill(0.);
            double sumQXi(0.);

            //row direction, merge 2 rows -> 1 mm
            for(int ii=0;ii<5;++ii)
            {
                //col direction, merge 10 cols -> 5 mm
                for(int mm=0;mm<10;++mm)
                {
                    // Merge two rows Q 
                    auto Qij_12 = (*Matrix10x300_MC)(2*ii,kk*10+mm) + 
                        (*Matrix10x300_MC)(2*ii+1,kk*10+mm);
                    sumQi[ii] += Qij_12;
                }
                sumQXi += sumQi[ii]*x_pad_pos[ii]; 
            }
            auto sumQhits = std::accumulate(sumQi.begin(),sumQi.end(),0.);
            if(kk==14)
                hmethod2->Fill(sumQXi/sumQhits-0.21);
            //hxrecs[kk]->Fill(sumQXi/sumQhits-0.225);
            //std::cout<<"Method2 : "<<sumQXi/sumQhits<<std::endl;

            //if(sumQhits>0.)
            //    recoHits.push_back({sumQXi/sumQhits,0.25+0.5*kk,20.,sumQhits});
            //else
            //    recoHits.push_back({-0.1,0.5+0.25*kk,20.,sumQhits});

            //if(i_entry<10)
            //    std::cout<<recoHits.size()<<"\t"<<recoHits.at(kk).x()<<std::endl;
        }
        delete Matrix10x300_MC;
        //recoHits.clear();
    }

#endif

#if 0
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
    //hxrec->Fill(recoHits.at(14).x()-0.225);
    for(int i=0;i<30;++i)
    {
        hxrecs[i]->Fill(recoHits.at(i).x()-0.225);
    }
#endif

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
    //auto grxrec_rms = new TGraph(30);
    //for(int i=0;i<30;++i)
    //{
    //    grxrec_rms->SetPoint(i,i,hxrecs[i]->GetRMS());
    //    delete hxrecs[i];
    //}
    //delete []hxrecs;

    //auto cc1 = new TCanvas("cc1","cc1",800,600);
    //cc1->SetGrid();
    //hmethod1->DrawCopy();
    //hmethod2->SetLineColor(kRed);
    //hmethod2->DrawCopy("SAMES");
    //delete hmethod1;
    //delete hmethod2;
    //cc1->SetLogz();
    //cc1->DrawFrame(0,0.09,30,0.015,"5 GeV/#it{c} e^{-},B=1T;hit id;RMS of (x_{rec}-x_{track}) [cm]");
    //cc1->SetGrid();
    //LansxFormat::FormatAll(grxrec_rms,"%d %e %f",kOrange-3,kFullStar,2);
    //grxrec_rms->Draw("P");
    //hxrecs[14]->DrawCopy();
    //auto htrkxy_mc1 = Matrix10x300_MC->Matrix2HistReadout();
    //htrkxy_mc1->Draw("COL");
    //LansxFormat::FormatAll(grRec,"%d %e",kRed,kFullTriangleDown);
    //grRec->Draw("P");
}

void PixHitRecoSimpleProcessor::EndAction()
{
    PixTPCLog(PIXtpcINFO,Form("This is end of %s!!!",this->GetProcessorName().c_str()));
}

void PixHitRecoSimpleProcessor::DebugPrint()
{
    if(fIsdebug)
        PixTPCLog(PIXtpcDebug,Form("There are %lld entries in input file!!!",f_tree_in->GetEntries()));
}

//void PixHitRecoSimpleProcessor::InitialOutputFile()
//{
//     
//}

void PixHitRecoSimpleProcessor::UsePixelChargeCenterMethod()
{
    //Create output root file
    auto outfile = std::make_unique<TFile>(fOutputfileName.c_str(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","reco hits data");
    auto recoHitsdata = new MCTrackdata;
    tr_out->Branch(fOutputBranchName.c_str(),&recoHitsdata);
    std::vector<FourVector> vRecoHits;

    for(int i_entry=0; i_entry < f_tree_in->GetEntries(); ++i_entry)
    {
        if(i_entry%100==0)
            std::printf("##%d tracks hits reconstructed !\n",i_entry);

        f_tree_in->GetEntry(i_entry);

        recoHitsdata->SetTrkID(i_entry);
        
        auto Matrix10x300 = new PixelMatrix; 
        Matrix10x300->PixelTPCdata2PixelMatrix(f_PixTPCdata);

        for(int colidx =0;  colidx < __COL__; colidx=colidx+fNumOfColMerge)
        {
            double sumQij(0.),sumQXij(0.),sumQYij(0.);

            for(int colsub=0;colsub<fNumOfColMerge;++colsub)
            {
                int cols = colidx + colsub;
                for(int rowidx =0; rowidx < __ROW__; ++rowidx)
                {
                    auto Qij = (*Matrix10x300)(rowidx,cols);
                    if(Qij > 0. )
                    {
                        auto xpyp_pair = BeamUnities::RowColIdx2Position(rowidx,cols);
                        sumQij += Qij;
                        sumQXij += Qij * xpyp_pair.first;
                        sumQYij += Qij * xpyp_pair.second;
                    }
                }
            } // end of a hit
            //TODO: implementation time info
            if(sumQij>0)
                vRecoHits.push_back({sumQXij/sumQij,sumQYij/sumQij,0.,sumQij/double(fNumOfColMerge)});
        }// end of a track
        //Fill tree
        recoHitsdata->FillClusters(vRecoHits);
        tr_out->Fill();
        //clear hits vector
        vRecoHits.clear();
        recoHitsdata->ClearMCTrackdata();
        //delete 
        delete Matrix10x300;
    }
    
    tr_out->Write();
    delete tr_out;
    outfile->Close();
    
    PixTPCLog(PIXtpcINFO,"Reco data Write Done, using 10 cols Pixel Charge center method!");

}

void PixHitRecoSimpleProcessor::UseEquivalentPadMethod()
{
    double x_pad_pos[5] = {0.025,0.125,0.225,0.325,0.425};
    //Create output root file
    auto outfile = std::make_unique<TFile>(fOutputfileName.c_str(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","reco hits data");
    auto recoHitsdata = new MCTrackdata;
    tr_out->Branch(fOutputBranchName.c_str(),&recoHitsdata);
    std::vector<FourVector> vRecoHits;
    
    if(__COL__%fNumOfColMerge!=0)
        fNumOfColMerge=10;

    for(int i_entry=0; i_entry < f_tree_in->GetEntries(); ++i_entry)
    {
        if(i_entry%100==0)
            std::printf("##%d tracks hits reconstructed !\n",i_entry);

        f_tree_in->GetEntry(i_entry);

        recoHitsdata->SetTrkID(i_entry);

        auto Matrix10x300 = new PixelMatrix; 
        Matrix10x300->PixelTPCdata2PixelMatrix(f_PixTPCdata);

        for(int kk=0; kk < (__COL__/fNumOfColMerge); ++kk)
        {
            std::array<double,5> sumQi;
            sumQi.fill(0.);
            double sumQXi(0.);

            //row direction, merge 2 rows -> 1 mm
            for(int ii=0;ii<5;++ii)
            {
                //col direction, merge fNumOfColMerge cols -> fNumOfColMerge*0.5 mm
                for(int mm=0;mm<fNumOfColMerge;++mm)
                {
                    // Merge two rows Q 
                    auto Qij_12 = (*Matrix10x300)(2*ii,kk*fNumOfColMerge+mm) + 
                        (*Matrix10x300)(2*ii+1,kk*fNumOfColMerge+mm);
                    sumQi[ii] += Qij_12;
                }
                sumQXi += sumQi[ii]*x_pad_pos[ii]; 
            }//end of a hit
            auto sumQhits = std::accumulate(sumQi.begin(),sumQi.end(),0.);
            if(sumQhits>0.)
                vRecoHits.push_back({sumQXi/sumQhits,fNumOfColMerge*0.05/2+fNumOfColMerge*0.05*kk,0.,sumQhits/double(fNumOfColMerge)});
            else
                vRecoHits.push_back({-0.1,fNumOfColMerge*0.05/2+fNumOfColMerge*0.05*kk,0.,sumQhits});
        }// end of a track
        if(fIsdebug && i_entry<10)
            std::cout<<"DebugPrint reco yhit:"<<vRecoHits.at(0).y()<<"\t"<<vRecoHits.at(1).y()<<std::endl;

        //Fill tree
        recoHitsdata->FillClusters(vRecoHits);
        tr_out->Fill();
        //clear hits vector
        vRecoHits.clear();
        recoHitsdata->ClearMCTrackdata();
        //delete
        delete Matrix10x300;
    }

    tr_out->Write();
    delete tr_out;
    outfile->Close();

    PixTPCLog(PIXtpcINFO,"Reco data Write Done, using EquivalentPad method!");
}
