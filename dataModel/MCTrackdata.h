/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-20 15:04
 * Filename         : MCTrackdata.h
 * Description      : Head file, data structure of MC tracks, used in 
 *                    GenSimData 
 * Update           : 
 * ******************************************************************/
#ifndef __MCTrackdata_H__
#define __MCTrackdata_H__ 1

#include <vector>
#include <string>
#include "Math/Vector4D.h"
#include "TObject.h"

using FourVector = ROOT::Math::XYZTVector;

class MCTrackdata : public TObject
{
public:
    MCTrackdata(std::string MCparticleName="e-");
    ~MCTrackdata() {}
    
    //Public methods to fill/clear MC track data
    MCTrackdata& SetTrkID(int trkid);
    void FillClusters(std::vector<FourVector>& cluster_pos);
    void ClearMCTrackdata();
    
    //Methods to Get MC track (or cluster) data
    std::vector<FourVector>& GetClusterVec() { return fClsVec; }
    int GetTrkID() { return fTrkID; }
    //inline std::string GetMCparticle();

private:
    //======================================================================================================
    //@Brief: MCTrackdata structure
    // std::vector<FourVector>    : vector to store all cluster position+energy loss info along MC track.
    //                              When Number of e- in a cluster > 1, save the first (x,y,z,e) info.
    //======================================================================================================
    int fTrkID;
    //std::string fMCparticleName;
    std::vector<FourVector> fClsVec;

    ClassDef(MCTrackdata,1);
};

//inline std::string MCTrackdata::GetMCparticle()
//{
//    return fMCparticleName;
//}

#endif
