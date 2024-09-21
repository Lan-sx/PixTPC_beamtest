/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-20 15:08
 * Filename         : MCTrackdata.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "MCTrackdata.h"

ClassImp(MCTrackdata)

MCTrackdata::MCTrackdata(std::string MCparticleName) : fTrkID(0)
{
}

MCTrackdata& MCTrackdata::SetTrkID(int trkid)
{
    fTrkID = trkid;
    return *this;
}

void MCTrackdata::ClearMCTrackdata()
{
    fClsVec.clear();
}

void MCTrackdata::FillClusters(std::vector<FourVector>& cluster_pos)
{
    //fClsVec.push_back(cluster_pos);
    fClsVec=cluster_pos;
}
