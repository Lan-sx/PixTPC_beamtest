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
    
    if(fEquivalentPad)
    {
        UseEquivalentPadMethod();
    }
    else
    {
        UsePixelChargeCenterMethod();
    }

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

}

void PixHitRecoSimpleProcessor::EndAction()
{
    if(fIsdebug)
        ShowDebugCanvas();
    PixTPCLog(PIXtpcINFO,Form("This is end of %s!!!",this->GetProcessorName().c_str()));
}

void PixHitRecoSimpleProcessor::ShowDebugCanvas()
{
    //Get 0-th entry from input pixelTPCdata branch
    f_tree_in->GetEntry(0);
    auto Matrix10x300 = new PixelMatrix; 
    Matrix10x300->PixelTPCdata2PixelMatrix(f_PixTPCdata);
    if(Matrix10x300->GetMaxMinElement().second < NumOfe_cut)
        PixTPCLog(PIXtpcWARNING,"The min element < NumOfe_cut, check GenSimData or Rawdata2ROOT");
    //Create a Canvas
    auto myc = new TCanvas(Form("C_%smat10x300",this->GetProcessorName().c_str()),"",300,600);
    myc->SetGrid();
    myc->SetRightMargin(0.12);
    myc->SetLogz();
    //Draw pixel projection at x-y plane
    auto htrkxy = Matrix10x300->Matrix2HistReadout();
    htrkxy->SetName(Form("Hxy_%s",this->GetProcessorName().c_str()));
    htrkxy->DrawCopy("COL Z");
    delete Matrix10x300;
    //Draw reco hits from output file 
    auto outfile = std::make_unique<TFile>(fOutputfileName.c_str(),"READ");
    outfile->cd();
    auto tr_out = dynamic_cast<TTree*>(outfile->Get("PixTPCdata"));
    auto recoHitsdata = new MCTrackdata;
    tr_out->SetBranchAddress(fOutputBranchName.c_str(),&recoHitsdata);
    tr_out->GetEntry(0);
    auto vecRecohits = recoHitsdata->GetClusterVec();
    auto gRecohits = new TGraph(vecRecohits.size());
    gRecohits->SetName(Form("Ghits%s",this->GetProcessorName().c_str()));
    for(size_t kk=0;kk<vecRecohits.size();++kk)
    {
        gRecohits->SetPoint(kk,vecRecohits.at(kk).x(),vecRecohits.at(kk).y());
    }
    LansxFormat::FormatAll(gRecohits,"%d %e %f",kRed,kFullStar,2);
    gRecohits->Draw("P");
    //delete tree and close output file
    delete tr_out;
    outfile->Close();
    //Draw Canvas
    myc->Draw();
}

void PixHitRecoSimpleProcessor::DebugPrint()
{
    if(fIsdebug)
        PixTPCLog(PIXtpcDebug,Form("There are %lld entries in input file!!!",f_tree_in->GetEntries()));
}

void PixHitRecoSimpleProcessor::InitialHistRecohits(int numofhist)
{
    fHistRecohitsArray = new TH1D*[numofhist];
    for(int ihist=0; ihist < numofhist; ++ihist)
    {
        fHistRecohitsArray[ihist] = new TH1D(Form("h%s_%d",this->GetProcessorName().c_str(),ihist),"",200,0,2);
    }
}

void PixHitRecoSimpleProcessor::UsePixelChargeCenterMethod()
{
    //Create output root file
    auto outfile = std::make_unique<TFile>(fOutputfileName.c_str(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","reco hits data");
    auto recoHitsdata = new MCTrackdata;
    tr_out->Branch(fOutputBranchName.c_str(),&recoHitsdata);
    std::vector<FourVector> vRecoHits;
    
    if(__COL__%fNumOfColMerge!=0)
    {
        fNumOfColMerge=10;
        PixTPCLog(PIXtpcWARNING,"__COL__ % NumOfColMerge!=0, NumOfColMerge set to 10");
    }

    if(fIsdebug)
        InitialHistRecohits(__COL__/fNumOfColMerge);

    int NumofEntries = fIsdebug ? int(0.4*f_tree_in->GetEntries()) : f_tree_in->GetEntries();

    for(int i_entry=0; i_entry < NumofEntries; ++i_entry)
    {
        if(i_entry%100==0)
            std::printf("##%d tracks hits reconstructed !\n",i_entry);

        f_tree_in->GetEntry(i_entry);

        recoHitsdata->SetTrkID(i_entry);
        
        auto Matrix10x300 = new PixelMatrix; 
        Matrix10x300->PixelTPCdata2PixelMatrix(f_PixTPCdata);

        int hit_id =0;
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
            {
                if(fIsdebug)
                    fHistRecohitsArray[hit_id]->Fill(sumQXij/sumQij);
                vRecoHits.push_back({sumQXij/sumQij,sumQYij/sumQij,0.,sumQij/double(fNumOfColMerge)});
            }
            else
            {
                if(fIsdebug)
                    fHistRecohitsArray[hit_id]->Fill(-0.1);
                vRecoHits.push_back({-0.1,-1.,0.,0.});
            }
            hit_id++;
        }// end of a track
        if(fIsdebug && i_entry<10)
        {
            std::cout<<"DebugPrint reco hit:"<<"("<<vRecoHits.at(0).x()<<","<<vRecoHits.at(0).y()<<","<<vRecoHits.at(0).z()<<")"<<"\n"
                <<"("<<vRecoHits.at(1).x()<<","<<vRecoHits.at(1).y()<<","<<vRecoHits.at(1).z()<<")"<<std::endl;
        }
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
    
    //Write debug histograms and delete 
    if(fIsdebug)
    {
        for(int ihist=0; ihist < __COL__/fNumOfColMerge ; ++ihist)
        {
            fHistRecohitsArray[ihist]->Write();
            //delete fHistRecohitsArray[ihist];
        }
        delete []fHistRecohitsArray;
        PixTPCLog(PIXtpcDebug,"Debug histograms write done!");
    }

    outfile->Close();
    
    PixTPCLog(PIXtpcINFO,"Reco data Write Done, using merged cols Pixel Charge center method!");
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
    {
        fNumOfColMerge=10;
        PixTPCLog(PIXtpcWARNING,"__COL__ % NumOfColMerge!=0, NumOfColMerge set to 10");
    }

    if(fIsdebug)
        InitialHistRecohits(__COL__/fNumOfColMerge);

    int NumofEntries = fIsdebug ? int(0.4*f_tree_in->GetEntries()) : f_tree_in->GetEntries();

    for(int i_entry=0; i_entry < NumofEntries; ++i_entry)
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
            {
                if(fIsdebug)
                    fHistRecohitsArray[kk]->Fill(sumQXi/sumQhits);

                vRecoHits.push_back({sumQXi/sumQhits,fNumOfColMerge*0.05/2+fNumOfColMerge*0.05*kk,0.,sumQhits/double(fNumOfColMerge)});
            }
            else
            {
                if(fIsdebug)
                    fHistRecohitsArray[kk]->Fill(-0.1);

                vRecoHits.push_back({-0.1,fNumOfColMerge*0.05/2+fNumOfColMerge*0.05*kk,0.,sumQhits});
            }
        }// end of a track
        if(fIsdebug && i_entry<10)
        {
            std::cout<<"DebugPrint reco hit:"<<"("<<vRecoHits.at(0).x()<<","<<vRecoHits.at(0).y()<<","<<vRecoHits.at(0).z()<<")"<<"\n"
                <<"("<<vRecoHits.at(1).x()<<","<<vRecoHits.at(1).y()<<","<<vRecoHits.at(1).z()<<")"<<std::endl;
        }
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
    //Write debug histograms and delete 
    if(fIsdebug)
    {
        for(int ihist=0; ihist < __COL__/fNumOfColMerge ; ++ihist)
        {
            fHistRecohitsArray[ihist]->Write();
            //delete fHistRecohitsArray[ihist];
        }
        delete []fHistRecohitsArray;
        PixTPCLog(PIXtpcDebug,"Debug histograms write done!");
    }

    outfile->Close();

    PixTPCLog(PIXtpcINFO,"Reco data Write Done, using EquivalentPad method!");
}


