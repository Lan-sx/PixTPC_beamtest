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

//ROOT
#include "TMatrixDSparse.h"
#include "TH2Poly.h"

//Users
#include "PixelTPCdata.h"
#include "BeamUnities.h"

class PixelMatrix : public TMatrixDSparse
{
public:
    PixelMatrix();
    PixelMatrix(PixelTPCdata* pixeltpcdata);
    ~PixelMatrix();
    
    TH2Poly* Matrix2HistReadout();
    TH2Poly* GetHistReadout();

protected:
    //Create readout pixel array (10 x 300) using TH2Poly 
    void CreateReadoutPixelArray();

private:
    PixelTPCdata *fPixelTPCdata;
    TH2Poly* fHistreadout;
};


#endif
