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
}

GenSimData::~GenSimData()
{ 
    fvecMat10x300Q.clear();
    fvecMat10x300T.clear();
    delete fGas;
    delete fCmp;
    delete fSensor;
    //delete fMat10x300;
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

void GenSimData::GenTracks(std::string particleName,double Mom, double DriftLength)
{
    //Initial General settings
    InitGasCmpSensor();
    //Initial TrackHeed
    this->SetSensor(fSensor);
    this->SetParticle(particleName);
    this->SetMomentum(Mom);
    
    // Polya dist from Yue Chang Simu.
    auto fpolya = new TF1("fpolya","[0]*pow(1+[1],1+[1])*pow(x/[2],[1])*exp(-(1+[1])*x/[2])/TMath::Gamma(1+[1])",0,10000);
    fpolya->SetParameter(0,3802);
    fpolya->SetParameter(1,0.487);
    fpolya->SetParameter(2,2092);

    //Drift T sigma v.s. DriftLength
    auto fsigmaTvsZ = new TF1("fsigmaTvsZ","0.0001167+0.003273*sqrt(x)",0.,300);
    auto sigmaT = fsigmaTvsZ->Eval(DriftLength); // [us]
    auto Tdrift = DriftLength/Velo_e;            // [us] 

    //New Track
    for(int itrk=0; itrk<fNevts; ++itrk)
    {
        if(itrk%100==0)
            std::printf("=====>New Track%d %s \n",itrk,particleName.c_str());

        this->NewTrack(0.225,0.,DriftLength,0.,0.,1.,0); 

        auto Mat10x300Q_i = std::make_shared<PixelMatrix>();
        auto Mat10x300T_i = std::make_shared<PixelMatrix>();
        Mat10x300Q_i->ResizeTo(__ROW__,__COL__);
        Mat10x300T_i->ResizeTo(__ROW__,__COL__);

        double xc(0.),yc(0.),zc(0.),tc(0.),ec(0.),extra(0.);
        int nc(0);
        //Loop all clusters and electron in each cluster
        while(this->GetCluster(xc,yc,zc,tc,nc,ec,extra))
        {
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
        //End of a Track
    }
    delete fpolya;
    std::printf("=============> End of GenTracks! \n");
}

std::shared_ptr<PixelMatrix> GenSimData::GetPixelMatrix(int i)
{
    //std::printf("========> Test Get!\n");
    if(i < 0 || i > fvecMat10x300Q.size() ) 
        throw std::out_of_range("!!!Error!!! out of range in GetPixelMatrix");
    
    return fvecMat10x300Q[i];
}

void GenSimData::WritePixelTPCdata(std::string filename)
{
    //Save MC data
    auto outfile = std::make_unique<TFile>(filename.data(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","tpc channel data"); 
        
    auto pixeltpcdata = new PixelTPCdata(__NumChip__);
    tr_out->Branch("pixelTPCdata",&pixeltpcdata);
    
    //start save MC data 
    for(size_t itrk=0; itrk < fNevts; ++itrk)
    {
        pixeltpcdata->SetTiggleID(itrk);
        
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
        pixeltpcdata->ClearPixelTPCdata(__NumChip__);
    }// end of tracks
    
    //Write root file 
    tr_out->Write();
    outfile->Close();

    std::printf("======>MC data Write Done!\n");
}






