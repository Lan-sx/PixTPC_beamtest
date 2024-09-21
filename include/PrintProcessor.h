/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:27
 * Filename         : PrintProcessor.h
 * Description      : simple print processor class
 * Update           : 
 * ******************************************************************/
#ifndef __PrintProcessor_H__
#define __PrintProcessor_H__ 1
//std
#include <iostream>

//ROOT CERN
#include "TH1D.h"
#include "TGraphErrors.h"

//
#include "Processor.h"
#include "PixelTPCdata.h"

class PrintProcessor : public Processor 
{
public:
    PrintProcessor(TFile *filein, TTree *ftreein);
    ~PrintProcessor();

    // Implementation of virtual functions
    virtual void InitAction()  override;
    virtual void ProcessEventAction() override;
    virtual void EndAction() override;
    
    virtual void DebugPrint() override;

    //public methods
    TH1D* GetHistQ() { return f_histQ; }
    TH1D* GetHistT() { return f_histT; }
    TGraphErrors* GetGrQ() { return fgrQ; }

private:
    TFile* f_file;
    TTree* f_tree;
    PixelTPCdata* f_PixTPCdata;
    
    TH1D *f_histQ,*f_histT;
    TH1D** fhQ128;
    TGraphErrors* fgrQ;
};

#endif
