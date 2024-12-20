/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-05 16:36
 * Filename         : GemSimData.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "GenSimData.h"

GenSimData::GenSimData(int Nevts) : fUsingPixNoiseMaps(false),fNevts(Nevts),fGas(nullptr), fCmp(nullptr), fSensor(nullptr),
                                    fTrkProjxy(nullptr),fPixelResponse(nullptr)
{
    if(!fUsingPixNoiseMaps)
        std::printf("$GenSimData: Using default noise map for all pixels\n");
    else
        std::printf("$GenSimData: External noise map needed~\n");
    //this->EnableDebugging();        
    
    fParticle = "e-";
    fTrkInitialPos[0] = 0.225;
    fTrkInitialPos[1] = 0.;
    fTrkInitialPos[2] = 20.;
    fTrkInitialDir = {0.,1.,0};

    fMomentum = 5.e+9; // 5 GeV/c
    fMomentumReso = 0.001;
}

GenSimData::GenSimData(int Nevts, double DriftLength) : fUsingPixNoiseMaps(false),fNevts(Nevts),fGas(nullptr), fCmp(nullptr), fSensor(nullptr),
                                    fTrkProjxy(nullptr),fPixelResponse(nullptr)

{
    if(!fUsingPixNoiseMaps)
        std::printf("$GenSimData: Using default noise map for all pixels\n");
    else
        std::printf("$GenSimData: External noise map needed~\n");
    //this->EnableDebugging();        
    fParticle = "e-";
    fTrkInitialPos[0] = 0.225;
    fTrkInitialPos[1] = 0.;
    fTrkInitialPos[2] = DriftLength;

    fTrkInitialDir = {0.,1.,0};
    fMomentum = 5.e+9; // 5 GeV/c
    fMomentumReso =0.001; 
}

GenSimData::~GenSimData()
{ 
    fvecMat10x300Q.clear();
    fvecMat10x300T.clear();
    fvecTrackdatas.clear();
    delete fGas;
    delete fCmp;
    delete fSensor;
    //delete fMat10x300;
}

void GenSimData::SetTrkInitialPosition(TVector3 &xp)
{
    fTrkInitialPos = xp;
}

void GenSimData::SetTrkInitialDirection(TVector3 &direction)
{
    fTrkInitialDir = direction;
}

void GenSimData::InitGasCmpSensor()
{
    //Initial Gas, TDR gas, T2K
    fGas = new Garfield::MediumMagboltz("ar",95.,"CF4",3.,"isobutane",2.);
    fGas->SetPressure(760.);
    fGas->SetTemperature(293.15);

    //Initial Cmp
    fCmp = new Garfield::ComponentConstant;
    fCmp->SetMedium(fGas);
    fCmp->SetArea(-1,-1,0,3,20,MaxDriftLength);
    fCmp->SetElectricField(0,0,230.);
    fCmp->SetMagneticField(0,0,1);
    
    //Initial Sensor
    fSensor = new Garfield::Sensor;
    fSensor->AddComponent(fCmp);
}

void GenSimData::GenTracksFromJson()
{
    GenTracks(fParticle,fMomentum);
}

void GenSimData::GenTracks(std::string particleName,double Mom)
{
    //Initial General settings
    InitGasCmpSensor();
    //Initial TrackHeed
    this->SetSensor(fSensor);
    this->SetParticle(particleName);
    //this->SetMomentum(Mom);
    auto DriftLength = fTrkInitialPos.Z();

    // Polya dist from Yue Chang Simu.
    auto fpolya = new TF1("fpolya","[0]*pow(1+[1],1+[1])*pow(x/[2],[1])*exp(-(1+[1])*x/[2])/TMath::Gamma(1+[1])",0,10000);
    fpolya->SetParameter(0,3802);
    fpolya->SetParameter(1,0.487);
    fpolya->SetParameter(2,2092);

    //Drift T sigma v.s. DriftLength
    auto fsigmaTvsZ = new TF1("fsigmaTvsZ","0.0001167+0.003273*sqrt(x)",0.,300);
    auto sigmaT = fsigmaTvsZ->Eval(DriftLength); // [us]
    auto Tdrift = DriftLength/Velo_e;            // [us] 
    
    fTrkInitialPos[2] = DriftLength;
    //New Track
    for(int itrk=0; itrk<fNevts; ++itrk)
    {
        if(itrk%100==0)
            std::printf("=====>New Track%d %s \n",itrk,particleName.c_str());

        double momentumWithDispersion = gRandom->Gaus(fMomentum,fMomentum*fMomentumReso);
        this->SetMomentum(momentumWithDispersion);

        this->NewTrack(fTrkInitialPos[0],fTrkInitialPos[1],fTrkInitialPos[2],0.,
                       fTrkInitialDir[0],fTrkInitialDir[1],fTrkInitialDir[2]); 
        
        //Matrixes for saving readout pixels info
        auto Mat10x300Q_i = std::make_shared<PixelMatrix>();
        auto Mat10x300T_i = std::make_shared<PixelMatrix>();
        Mat10x300Q_i->ResizeTo(__ROW__,__COL__);
        Mat10x300T_i->ResizeTo(__ROW__,__COL__);
        //Vectors for saving MC track data
        std::vector<FourVector> vecClusters;

        double xc(0.),yc(0.),zc(0.),tc(0.),ec(0.),extra(0.);
        int nc(0);
        //Loop all clusters and electron in each cluster
        while(this->GetCluster(xc,yc,zc,tc,nc,ec,extra))
        {
            //push MC track/clusters data
            ROOT::Math::XYZTVector clusterinfo(xc,yc,zc,ec);
            vecClusters.push_back(clusterinfo);
            for(int k=0;k<nc;++k)
            {
                double xe(0.),ye(0.),ze(0.),te(0.),ee(0.);
                double dx(0.),dy(0.),dz(0.);
                this->GetElectron(k,xe,ye,ze,te,ee,dx,dy,dz);
                
                //Drift smear
                auto xe_afterdrift = gRandom->Gaus(xe,X_Cd*sqrt(DriftLength));
                auto ye_afterdrift = gRandom->Gaus(ye,Y_Cd*sqrt(DriftLength));
                auto te_afterdrift = gRandom->Gaus(Tdrift,sigmaT);
                
                //Avalance 
                int Gain = int(fpolya->GetRandom());
                for(int mm=0;mm < Gain;++mm)
                {
                    // Avalance gap smear
                    auto xe_afteraval = gRandom->Gaus(xe_afterdrift,Sigma_aval);
                    auto ye_afteraval = gRandom->Gaus(ye_afterdrift,Sigma_aval);
                    
                    auto rowcolpair = BeamUnities::Position2RowCol(xe_afteraval,ye_afteraval);
                    if(rowcolpair.first >=0 && rowcolpair.second >= 0)
                    {
                        (*Mat10x300Q_i)(rowcolpair.first,rowcolpair.second)+=1;
                        (*Mat10x300T_i)(rowcolpair.first,rowcolpair.second) = te_afterdrift;
                    }
                }
                //End of a Avalance
            }
            //End of a Cluster
        }
        fvecMat10x300Q.push_back(Mat10x300Q_i);
        fvecMat10x300T.push_back(Mat10x300T_i);
        fvecTrackdatas.push_back(vecClusters);
        //Debug print, print cluster info
        if(this->m_debug)
            std::printf("=====> %zu clusters in this track\n",vecClusters.size());
        //End of a Track
    }
    delete fpolya;
    delete fsigmaTvsZ;
    //std::printf("=============> End of GenTracks! \n");
    PixTPCLog(PIXtpcINFO,"End of GenTracks!",false);
}

std::shared_ptr<PixelMatrix> GenSimData::GetPixelMatrix_withoutNoise(int i)
{
    //std::printf("========> Test Get!\n");
    if(i < 0 || i > fvecMat10x300Q.size() ) 
        throw std::out_of_range("!!!Error!!! out of range in GetPixelMatrix");
    
    return fvecMat10x300Q[i];
}

TCanvas* GenSimData::ShowPixelResponseWithoutNoise(int trkid)
{
    // Show a Hist without noise, used for debug
    auto myc = new TCanvas(Form("cc_histwithoutnoise%d",trkid),Form("cc_histwithoutnoise%d",trkid),300,600);
    myc->SetGrid();

    myc->SetRightMargin(0.12);
    myc->SetGrid();
    myc->SetLogz();
    auto mat10x300 = this->GetPixelMatrix_withoutNoise(trkid);
    if(mat10x300->GetMaxMinElement().second > NumOfe_cut)
        PixTPCLog(PIXtpcWARNING,"The min element > NumOfe_cut",true);
    auto htrkxy = mat10x300->Matrix2HistReadout();
    htrkxy->DrawClone("COL Z");

    return myc;
}

void GenSimData::WritePixelTPCdata(std::string filename)
{
    //Save MC data
    auto outfile = std::make_unique<TFile>(filename.data(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","tpc channel data"); 
    
    auto mcTrackdata =  new MCTrackdata;
    auto pixeltpcdata = new PixelTPCdata(__NumChip__);
    tr_out->Branch("pixelTPCdata",&pixeltpcdata);
    tr_out->Branch("mcTrackdata",&mcTrackdata);
    
    //start save MC data 
    for(size_t itrk=0; itrk < fNevts; ++itrk)
    {
        if(itrk%500==0)
            std::printf("=====>Track%d saved!\n",int(itrk));
        pixeltpcdata->SetTiggleID(itrk);
        mcTrackdata->SetTrkID(itrk);
        //Loop std::vector<std::vector<TVector3>> fvecTrackdatas and fill mcTrackdata
        mcTrackdata->FillClusters(fvecTrackdatas.at(itrk));
        //Loop track sparse matrix and fill pixelTPCdata
        auto mat10x300Q_i = fvecMat10x300Q.at(itrk);
        auto mat10x300T_i = fvecMat10x300T.at(itrk);
        auto rowIdx = mat10x300Q_i->GetRowIndexArray();
        auto colIdx = mat10x300Q_i->GetColIndexArray();
        auto pData =  mat10x300Q_i->GetMatrixArray();
    
        for(int irow=0;irow < mat10x300Q_i->GetNrows(); ++irow)
        {
            const int sIdx = rowIdx[irow];
            const int eIdx = rowIdx[irow+1];
            //std::cout<<"====> "<<sIdx<<"\t"<<eIdx<<std::endl;
            for(int idx = sIdx; idx<eIdx;++idx)
            {
                const int icol = colIdx[idx];
                auto chipchnpair = BeamUnities::RowColIdx2ChipChn(irow,icol,GlobalMaps);
                auto pixelQ = (*mat10x300Q_i)(irow,icol);
                //TODO pixelT info update... T fluctuation in amp gap and unit 
                auto pixelT = (*mat10x300T_i)(irow,icol);
                // Add electronic noise and NumOfe_cut for all pixels
                if(!fUsingPixNoiseMaps)
                {
                    //using default settings, all pixels are same
                    auto pixelBaselineQ = gRandom->Gaus(Mean_Pixbaseline,Sigma_Pix);
                    auto pixelQ_withNoise = pixelBaselineQ + gRandom->Gaus(pixelQ,Sigma_electronics*pixelQ);
                    //applying number of electrons cut for all pixels
                    if(pixelQ_withNoise > NumOfe_cut)
                        (*pixeltpcdata)(chipchnpair.first,chipchnpair.second).push_back(std::make_pair(pixelT,pixelQ_withNoise));
                }
                else
                {
                    //TODO, creating noise maps for all pixels
                }
            } 
        }// end of sparse matrix
        tr_out->Fill();
        //std::printf("##################=====> Debug Pirnt Fill 0\n");
        pixeltpcdata->ClearPixelTPCdata(__NumChip__);
        mcTrackdata->ClearMCTrackdata();
    }// end of tracks
    
    //Write root file 
    tr_out->Write();
    outfile->Close();

    //std::printf("======>MC data Write Done!\n");
    PixTPCLog(PIXtpcINFO,"MC data Write Done!",false);
}






