/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-20 21:26
 * Filename         : PixHitRecoSimpleProcessor.h
 * Description      : 
 * Update           : 
 * ******************************************************************/
#ifndef __PixHitRecoSimpleProcessor_H__
#define __PixHitRecoSimpleProcessor_H__
//std
#include <iostream>
#include <string>

//ROOT
#include "TFile.h"
#include "TTree.h"

//User
#include "Processor.h"
#include "PixelMatrix.h"
#include "BeamUnities.h"
#include "PixelTPCdata.h"
#include "MCTrackdata.h"

class PixHitRecoSimpleProcessor : public Processor
{
public:
    PixHitRecoSimpleProcessor(TFile* filein,TTree* treein);
    PixHitRecoSimpleProcessor(TFile* filein,std::string treeName="PixTPCdata");
    ~PixHitRecoSimpleProcessor();

    // Implementation of virtual functions
    virtual void InitAction()  override;
    virtual void ProcessEventAction() override;
    virtual void EndAction() override;
    
    virtual void DebugPrint() override;

private:
    //pointers for I/O
    TFile* f_file_in;
    TFile* f_file_out;
    TTree* f_tree_in;
    TTree* f_tree_out;
    PixelTPCdata* f_PixTPCdata;
    
    //pointers for Debug
    //TH2Poly* fHistReadout;
};

#endif



