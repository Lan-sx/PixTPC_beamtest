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
#include <iostream>
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
#include "TVector3.h"

//Garfield++
#include "Garfield/TrackHeed.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/ComponentConstant.hh"

//Users
#include "BeamUnities.h"
#include "PixelTPCdata.h"
#include "MCTrackdata.h"
#include "PixelMatrix.h"

// extern global var: mapping chip chn <->  (row,col) pair
extern std::vector<std::pair<int,int>> GlobalMaps;

class PixelMatrix;

class GenSimData : public Garfield::TrackHeed
{
public:
    GenSimData(int Nevts=1);
    GenSimData(int Nevts, double DriftLength);
    ~GenSimData();

    //Set methods
    inline void SetParticle(std::string particle);
    inline void SetTrkMomentums(double p, double preso=0.);
    void SetTrkInitialPosition(TVector3 &xp);
    void SetTrkInitialDirection(TVector3 &direction);
    
    //Generate tracks
    //used in ProcessManager 
    void GenTracksFromJson();
    //used for testing
    void GenTracks(std::string particleName="e-",double Mom=5e+9);
    //Save tracks data to PixelTPCdata
    void WritePixelTPCdata(std::string filename);
    // Get i-th PixelMatrix response, without noise and cuts
    std::shared_ptr<PixelMatrix> GetPixelMatrix_withoutNoise(int i);
    TGraph* GetProjTrk() { return fTrkProjxy; }
    TH2D* GetPixelResponse() { return fPixelResponse; }

protected:
    void InitGasCmpSensor();

private:
    bool fUsingPixNoiseMaps; // Is using noise maps for all pixels
    int fNevts;              // Number of tracks/events
    double fMomentum;        // Momentum of particle [eV]
    double fMomentumReso;    // Momentum resolution of particle 
    std::string fParticle;   // Particle Name 
    TVector3 fTrkInitialPos; // Track Initial Position 
    TVector3 fTrkInitialDir; // Track Initial Direction

    // vectors to save all pixels' Q,T info
    std::vector<std::shared_ptr<PixelMatrix>> fvecMat10x300Q;
    std::vector<std::shared_ptr<PixelMatrix>> fvecMat10x300T;
    // vectors to save MC track data
    std::vector<std::vector<FourVector>> fvecTrackdatas; 

    // Garfield++ pointers
    Garfield::MediumMagboltz* fGas;
    Garfield::ComponentConstant* fCmp;
    Garfield::Sensor* fSensor;

    // TODO, pointers for debugging...
    TGraph* fTrkProjxy;
    TH2D* fPixelResponse;
};

inline void GenSimData::SetParticle(std::string particle)
{
    fParticle = particle;
}

inline void GenSimData::SetTrkMomentums(double p, double preso)
{
    fMomentum = p;
    fMomentumReso = preso;
}

#endif
