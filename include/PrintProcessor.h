/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:27
 * Filename         : PrintProcessor.h
 * Description      : simple print processor class
 * Update           : 20241016- a Processor for read/print/plot raw exp data
 * ******************************************************************/
#ifndef __PrintProcessor_H__
#define __PrintProcessor_H__ 1
//std
#include <iostream>
#include <memory>

//ROOT CERN
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TCanvas.h"

//Users
#include "JsonStruct.h"
#include "Processor.h"
#include "PixelTPCdata.h"

class PrintProcessor : public Processor 
{
public:
    //Default Ctor
    PrintProcessor(TFile *filein, TTree *ftreein);
    //Ctor with json struct
    PrintProcessor(TaskConfigStruct::PrintProcessorParsList inputPars);
    //Dtor
    ~PrintProcessor();

    // Implementation of virtual functions
    virtual void InitAction()  override;
    virtual void ProcessEventAction() override;
    virtual void EndAction() override;
    
    virtual void DebugPrint() override;

    //enum types
    enum PrintTypes { 
        Print1DHistQT=0,
        Print2DHistQT_Pos,
        Print2DHistQT_IO
    };
    //public methods
    //Legacy code
    TH1D* GetHistQ() { return f_histQ; }
    TH1D* GetHistT() { return f_histT; }
    TGraphErrors* GetGrQ() { return fgrQ; }

protected:
    TCanvas* Plot1DHistQT();
    TCanvas* Plot2DHistQT_Pos();

private:
    TFile* f_file;
    TTree* f_tree;
    PixelTPCdata* f_PixTPCdata;
    TaskConfigStruct::PrintProcessorParsList f_InputPars;
    
    //Legacy code
    TH1D *f_histQ,*f_histT;
    TH1D** fhQ128;
    TGraphErrors* fgrQ;
};

#endif
