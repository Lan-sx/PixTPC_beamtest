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
#include <numeric>
#include <memory>

//ROOT
#include "TFile.h"
#include "TTree.h"

//User
#include "Processor.h"
#include "ProcessManager.h"
#include "PixelMatrix.h"
#include "BeamUnities.h"
#include "PixelTPCdata.h"
#include "MCTrackdata.h"

//forward declare
namespace TaskConfigStruct{
    struct PixTPChitRecoParsList;
}

class PixHitRecoSimpleProcessor : public Processor
{
public:
    PixHitRecoSimpleProcessor(TFile* filein,TTree* treein);
    PixHitRecoSimpleProcessor(TFile* filein,std::string treeName="PixTPCdata");
    //Ctor for json manager
    PixHitRecoSimpleProcessor(TaskConfigStruct::PixTPChitRecoParsList recoparaslist);
    //Dtor
    ~PixHitRecoSimpleProcessor();

    // Implementation of virtual functions
    virtual void InitAction()  override;
    virtual void ProcessEventAction() override;
    virtual void EndAction() override;
    
    virtual void DebugPrint() override;
    
protected:
    //Merge fNumOfColMerge cols to calc the Charge center
    void UsePixelChargeCenterMethod();
    //Using Equivalent 1 mm x fNumOfColMerge*0.5mm pad to calc the Charge center
    void UseEquivalentPadMethod();
    
private:
    //Paras used in task json file manager
    bool fEquivalentPad;
    int fNumOfColMerge;
    //std::string fInputfileName;
    std::string fInputBranchName;
    std::string fOutputfileName;
    std::string fOutputBranchName;
    //pointers for I/O
    TFile* f_file_in;
    TTree* f_tree_in;
    PixelTPCdata* f_PixTPCdata;
    
    //pointers for Debug
    //TH2Poly* fHistReadout;
};

#endif



