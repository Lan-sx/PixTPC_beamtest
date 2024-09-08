/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-05 16:36
 * Filename         : GemSimData.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "GenSimData.h"


GenSimData::GenSimData(int Nevts) : fNevts(Nevts),fGas(nullptr), fCmp(nullptr), fSensor(nullptr),
                                    fTrkProjxy(nullptr),fPixelResponse(nullptr)
{
    fMat10x300 = new PixelMatrix;
    fMat10x300->ResizeTo(__ROW__,__COL__);
    //this->EnableDebugging();        
}

GenSimData::~GenSimData()
{ 
    delete fGas;
    delete fCmp;
    delete fSensor;
    delete fMat10x300;
}

void GenSimData::InitGasCmpSensor()
{
    //Initial Gas 
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
    for(int i=0;i<fNevts;++i)
    {
        if(i%100==0)
            std::printf("=====>New Track%d %s \n",i,particleName.c_str());

        this->NewTrack(0.225,0.,DriftLength,0.,0.,1.,0); 

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
                        (*fMat10x300)(rowcolpair.first,rowcolpair.second)+=1;
                    }
                }
                //End of a Avalance
            }
            //End of a Cluster
        }
        //End of a Track
    }

    std::printf("=============> End of GenTracks! \n");
}





