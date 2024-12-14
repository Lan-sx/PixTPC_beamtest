/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Created          : 2024-12-13 21:22
 * Filename         : PixHitRecoCCAProcessor.h
 * Description      : Pixel TPC hits reco. based on 
 *                    Connected Component Analysis (CCA)
 * Update           : 
 * ******************************************************************/
#ifndef __PixHitRecoCCAProcessor_H__
#define __PixHitRecoCCAProcessor_H__ 1
//std
#include <iostream>
#include <string>
#include <numeric>
#include <memory>

//ROOT
#include "TFile.h"
#include "TTree.h"

//User
#include "JsonStruct.h"
#include "Processor.h"
#include "PixelMatrix.h"
#include "BeamUnities.h"
#include "PixelTPCdata.h"
#include "MCTrackdata.h"

class PixHitRecoCCAProcessor : public Processor
{
public:
    //Ctor for json manager
    PixHitRecoCCAProcessor(TaskConfigStruct::PixTPChitRecoParsList recoparaslist);
    //Dtor
    ~PixHitRecoCCAProcessor();
    
    enum ClusterAlgo
    {
        CCA_DFS=0,
        CCA_TwoPass
    };

    // Implementation of virtual functions
    virtual void InitAction()  override;
    virtual void ProcessEventAction() override;
    virtual void EndAction() override;
    
    virtual void DebugPrint() override;

    //Public Methods
    void EnableProceesorDebug() { fIsdebug = true; }
    
protected:
    // Step I, clustering
    void TPChitsClusterFinder(int algo);
    // Step II, 
    //void SearchPeaks();

    //Debug vars
    //TH1D** fHistRecohitsArray;
    //void InitialHistRecohits(int numofhist);
    //Show methods for debug
    //Plot a canvas of the first event with pixel response and reco hits
    //void ShowDebugCanvas();
    //TODO Show raw dE/dx distribution histogram

private:
    //std::string fInputfileName;
    std::string fInputBranchName;
    std::string fOutputfileName;
    std::string fOutputBranchName;
    //pointers for I/O
    TFile* f_file_in;
    TTree* f_tree_in;
    PixelTPCdata* f_PixTPCdata;
    
};

#endif


