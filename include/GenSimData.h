/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-05 16:18
 * Filename         : GenSimData.h
 * Description      : A class to generate simu. root data (same as raw data),
 *                    1. using Garfield++ TrackHeed simu. e- 5 GeV track;
 *                    2. Drift/Avalance/  parameterization from Yue Chang
 *                    3. Get pixel response and fill PixelTPCdata
 * Update           : 
 * ******************************************************************/
#ifndef __GenSimData_H__
#define __GenSimData_H__ 1
//std

//ROOT 
#include "TRandom3.h"
#include "TF1.h"
#include "TPolyLine3D.h"
#include "TGraph.h"
#include "TH2D.h"

//Garfield++
#include "Garfield/TrackHeed.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ComponentConstant.hh"

//Users
#include "BeamUnities.h"
#include "PixelTPCdata.h"
#include "PixelMatrix.h"

class GenSimData : public Garfield::TrackHeed
{
public:
    GenSimData(int Nevts=1);
    ~GenSimData();
    
    //Generate tracks
    void GenTracks(std::string particleName="e-",double Mom=5e+9,double DriftLength=5.);
    
    PixelMatrix* GetMat10x300() { return fMat10x300; }
    TGraph* GetProjTrk() { return fTrkProjxy; }
    TH2D* GetPixelResponse() { return fPixelResponse; }

protected:
    void InitGasCmpSensor();

private:
    int fNevts;
    PixelMatrix* fMat10x300;
    Garfield::MediumMagboltz* fGas;
    Garfield::ComponentConstant* fCmp;
    Garfield::Sensor* fSensor;
    TGraph* fTrkProjxy;
    TH2D* fPixelResponse;

};

#endif
