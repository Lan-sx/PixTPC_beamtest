/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-05 21:05
 * Filename         : PixelTPCdata.h
 * Description      : Head file, data structure of PixelTPC Det. R&D 
 *                    for ROOT I/O, generate dict
 * Update           : 2024-07-05 21:02: created
 *                    2024-07-06 18.07: version0_0 3-dim vector type
 *                    2024-09-19 22.05: change TYPE1 to std::vector<>
 *                    TODO, vector will used bigger memory, need to update?
 * ******************************************************************/
#ifndef __PixelTPCdata_H__
#define __PixelTPCdata_H__ 1
//std
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <stdexcept>

//ROOT CERN
#include "TObject.h"

//User 
#include "BeamUnities.h"

using PairQT= std::pair<double,double>;            // TYPE3
using VecSingleChn = std::vector<PairQT>;          // TYPE2 
//using Array128Chns = std::array<VecSingleChn,128>; // TYPE1 old version
using Array128Chns = std::vector<VecSingleChn>; // TYPE1

class PixelTPCdata : public TObject
{
private:
    // Triggle_ID (Physics Evt_ID)
    unsigned long fTriggleID; // -> TimeStamp
    // ======================================================================================================
    //@Brief: Event Info table 
    //  std::vector<TYPE1>                 : vector to store all TEPIX chips info
    //                                       maxima size == Number of TEPIX chips used
    //  TYPE1 -> std::vector<TYPE2>        : vector to store 128 channels info  
    //  TYPE2 -> std::vector<TYPE3>        : vector to store (Q,T) info (over threashold) of every channel,
    //                                       maxima size == 4
    //  TYPE3 -> std::pair<double,double> :  pair to store (Q,T) info
    // ======================================================================================================
    //std::vector<std::array<std::vector<std::pair<double,double>>,128>> fTestdata;
    std::vector<Array128Chns> fTestdata;
    
    //TODO vars need to add 
    //1. TimeStamp 
    //2. Flags etc
    //sub collections

    ClassDef(PixelTPCdata,1);

public:
    PixelTPCdata(int NumofChips=24);
    ~PixelTPCdata() {};
    
    PixelTPCdata& SetTiggleID(unsigned long triggleid);

    void FillPixelTPCdata(Array128Chns &arr128chns, int chipid);
    
    // Getters 
    unsigned long GetTriggleID() { return fTriggleID; }
    Array128Chns & Getdata_I  (int chipid);
    VecSingleChn & Getdata_IJ (int chipid, int chnid);
    //PairQT       & Getdata_IJK(int chipid, int chnid, int kk); 
    
    VecSingleChn & operator()(int chipid, int chnid);

    void ClearPixelTPCdata(int NumofChips);


};

#endif
