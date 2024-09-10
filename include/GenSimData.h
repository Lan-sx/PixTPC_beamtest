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
#include <vector>
#include <memory>

//ROOT 
#include "TFile.h"
#include "TTree.h"
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

class PixelMatrix;

class GenSimData : public Garfield::TrackHeed
{
public:
    GenSimData(int Nevts=1);
    ~GenSimData();

    //Set method
    
    //Generate tracks
    void GenTracks(std::string particleName="e-",double Mom=5e+9,double DriftLength=5.);
    //Save tracks data to PixelTPCdata
    void WritePixelTPCdata(std::string filename, const std::vector<std::pair<int,int>> vMaps);
    
    std::shared_ptr<PixelMatrix> GetPixelMatrix(int i);
    TGraph* GetProjTrk() { return fTrkProjxy; }
    TH2D* GetPixelResponse() { return fPixelResponse; }

protected:
    void InitGasCmpSensor();

private:
    int fNevts;
    std::vector<std::shared_ptr<PixelMatrix>> fvecMat10x300Q;
    std::vector<std::shared_ptr<PixelMatrix>> fvecMat10x300T;

    Garfield::MediumMagboltz* fGas;
    Garfield::ComponentConstant* fCmp;
    Garfield::Sensor* fSensor;
    TGraph* fTrkProjxy;
    TH2D* fPixelResponse;
};

#endif
