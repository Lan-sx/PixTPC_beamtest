/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-07 21:30
 * Filename         : PixelMatrix.h
 * Description      : PixelMatrix class inherit from TMatrixDSparse
 * Update           : 
 * ******************************************************************/
#ifndef __PixelMatrix_H__
#define __PixelMatrix_H__ 1
//std
#include <iostream>
#include <map>

//ROOT
#include "TMatrixDSparse.h"
#include "TH2Poly.h"

//Users
#include "PixelTPCdata.h"
#include "BeamUnities.h"

extern std::vector<std::pair<int,int>> GlobalMaps;

class PixelTPCdata;

class PixelMatrix : public TMatrixDSparse
{
public:
    //Ctors
    PixelMatrix();
    PixelMatrix(int numberofChips);
    //PixelMatrix(PixelTPCdata* pixeltpcdata);
    //Dtor
    ~PixelMatrix();
    
    //public Methods
    //TODO : need to fixed
    PixelMatrix& PixelTPCdata2PixelMatrix(PixelTPCdata* pixeltpcdata, char qt='Q' );

    void SetOverTh_ID(int id) { fOverTh_ID = (id>=0&&id<4) ? id : 0; }

    //PixelMatrix -> PixelResponse at readout plane
    TH2Poly* Matrix2HistReadout();
    //Get Pixel Response Hist 
    TH2Poly* GetHistReadout();
    
    //Get Max. and Min. element in this Sparse Matrix
    std::pair<double,double> GetMaxMinElement();

protected:
    //Create readout pixel array (10 x 300) using TH2Poly 
    void CreateReadoutPixelArray();

private:
    //ID over threshold, 0,1,2,3
    int fOverTh_ID;
    //Number of Chips used
    int fNumberChipsUsed;
    PixelTPCdata *fPixelTPCdata;
    TH2Poly* fHistreadout;
};


#endif
