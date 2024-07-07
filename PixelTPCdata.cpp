/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-05 21:15
 * Filename         : PixelTPCdata.cpp
 * Description      : Src file, data structure of PixelTPC Det. R&D 
 *                    for ROOT I/O, generate dict
 * Update           : 2024-07-05 21:02: created
 * ******************************************************************/

#include "PixelTPCdata.h"

PixelTPCdata::PixelTPCdata(int NumofChips)
{
    fTestdata.resize(NumofChips);
}

PixelTPCdata& PixelTPCdata::SetTiggleID(int triggleid)
{
    fTriggleID = triggleid;
    return *this;
}

void PixelTPCdata::FillPixelTPCdata(Array128Chns &arr128chns,int chipid)
{
    fTestdata.at(chipid) = arr128chns;
}

void PixelTPCdata::ClearPixelTPCdata()
{
    fTestdata.clear();
}

Array128Chns & PixelTPCdata::Getdata_I  (int chipid)
{
    chipid = (chipid < int(fTestdata.size())) ? chipid : 0;
    return fTestdata.at(chipid);
}

VecSingleChn & PixelTPCdata::Getdata_IJ (int chipid, int chnid)
{
    chnid  = (chnid < 128) ? chnid : 0; 
    return Getdata_I(chipid).at(chnid); 
}

PairQT       & PixelTPCdata::Getdata_IJK(int chipid, int chnid, int kk)
{
    kk = (kk<4) ? kk : 0;
    return Getdata_IJ(chipid,chnid).at(kk);
}

