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

ClassImp(PixelTPCdata)

PixelTPCdata::PixelTPCdata(int NumofChips)
{
    fTestdata.resize(NumofChips);
    for(int i=0;i<NumofChips;++i)
    {
        fTestdata.at(i).resize(__NumChn__);
    }
}

PixelTPCdata& PixelTPCdata::SetTiggleID(unsigned long triggleid)
{
    fTriggleID = triggleid;
    return *this;
}

void PixelTPCdata::FillPixelTPCdata(Array128Chns &arr128chns,int chipid)
{
    fTestdata.at(chipid) = arr128chns;
}

void PixelTPCdata::ClearPixelTPCdata(int NumofChips)
{
    fTestdata.clear();
    fTestdata.resize(NumofChips);
    for(int i=0;i<NumofChips;++i)
    {
        fTestdata.at(i).resize(__NumChn__);
    }
    //std::printf("=======================Debug Print!!! ClearPixelTPCdata()!\n");
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

VecSingleChn & PixelTPCdata::operator()(int chipid, int chnid)
{
    if(chipid <0 || chipid>=fTestdata.size() || chnid <0 || chnid>=128) 
        throw std::out_of_range("Chip and Chn Idx out of range!");

    return fTestdata.at(chipid).at(chnid);
    //return fTestdata[chipid][chnid];
}

//PairQT       & PixelTPCdata::Getdata_IJK(int chipid, int chnid, int kk)
//{
//    kk = (kk<4) ? kk : 0;
//    return Getdata_IJ(chipid,chnid).at(kk);
//}

