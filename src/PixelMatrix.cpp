/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-07 21:31
 * Filename         : PixelMatrix.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "PixelMatrix.h"

PixelMatrix::PixelMatrix() : TMatrixDSparse(__ROW__,__COL__) , fPixelTPCdata(nullptr),
                             fHistreadout(nullptr)
{
}

//PixelMatrix::PixelMatrix(PixelTPCdata* pixeltpcdata) : TMatrixDSparse(__ROW__,__COL__),
//                                                        fPixelTPCdata(nullptr), fHistreadout(nullptr)
//{
//}

PixelMatrix::~PixelMatrix()
{
    delete fHistreadout;
}

PixelMatrix& PixelMatrix::PixelTPCdata2PixelMatrix(PixelTPCdata* pixeltpcdata,char qt)
{
    for(int chipsid=0; chipsid<__NumChip__; ++chipsid)
    {
        for(int chnid=0; chnid<__NumChn__; ++chnid)             
        {
            if((*pixeltpcdata)(chipsid,chnid).size()>0)
            {
                auto rowcolpair = BeamUnities::ChipChn2RowCol(chipsid,chnid,GlobalMaps);
                (*this)(rowcolpair.first,rowcolpair.second) = (qt=='Q') ? (*pixeltpcdata)(chipsid,chnid).at(0).second : 
                                                                          (*pixeltpcdata)(chipsid,chnid).at(0).first;
            }
        }
    }

    return *this;
}

void PixelMatrix::CreateReadoutPixelArray()
{
    fHistreadout = new TH2Poly("hPixel",";x [cm];y [cm]",-0.1,0.6,-0.1,15.5);
    fHistreadout->SetStats(kFALSE);

    for(int ii=0; ii<__ROW__; ++ii)
    {
        for(int kk=0; kk<__COL__; ++kk)
        {
            auto pixelPositionCenterpair = BeamUnities::RowColIdx2Position(ii,kk);
            double x1 = pixelPositionCenterpair.first-PixelSize/2.;
            double x2 = pixelPositionCenterpair.first+PixelSize/2.;
            double y1 = pixelPositionCenterpair.second-PixelSize/2.;
            double y2 = pixelPositionCenterpair.second+PixelSize/2.;
            fHistreadout->AddBin(x1,y1,x2,y2);
        }
    }
}

TH2Poly* PixelMatrix::Matrix2HistReadout()
{
    CreateReadoutPixelArray();

    auto rowIdx = this->GetRowIndexArray();
    auto colIdx = this->GetColIndexArray();
    auto pData = this->GetMatrixArray();

    for(int irow=0;irow < this->GetNrows(); ++irow)
    {
        const int sIdx = rowIdx[irow];
        const int eIdx = rowIdx[irow+1];
        //std::cout<<"====> "<<sIdx<<"\t"<<eIdx<<std::endl;
        for(int idx = sIdx; idx<eIdx;++idx)
        {
            const int icol = colIdx[idx];
                        
            auto xpyp = BeamUnities::RowColIdx2Position(irow,icol);
            fHistreadout->Fill(xpyp.first,xpyp.second,(*this)(irow,icol));
            //printf("data(%d,%d) = %d Mat(%d,%d)=%f\n",irow+matlocalmax.GetRowLwb(),
            //                                        icol+matlocalmax.GetColLwb(),
            //                                        data,
            //                                        irow+matlocalmax.GetRowLwb(),
            //                                        icol+matlocalmax.GetColLwb(),
            //                                        matlocalmax(irow+matlocalmax.GetRowLwb(),icol+matlocalmax.GetColLwb())
            //                                        );
        }
    }

    return fHistreadout;
}

TH2Poly* PixelMatrix::GetHistReadout()
{
    if(!fHistreadout)
    {
        CreateReadoutPixelArray();
        return fHistreadout;
    }
    return fHistreadout;
}

std::pair<double,double> PixelMatrix::GetMaxMinElement()
{
    auto pdataArray = this->GetMatrixArray(); 
    int NumOfElement = this->GetNoElements();

    auto max_ele = std::max_element(pdataArray,pdataArray+NumOfElement);
    auto min_ele = std::min_element(pdataArray,pdataArray+NumOfElement);

    return  std::make_pair(*max_ele,*min_ele);
}
