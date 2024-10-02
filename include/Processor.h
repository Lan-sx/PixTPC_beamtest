/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:13
 * Filename         : Processor.h
 * Description      : Processor base class
 * Update           : 
 * ******************************************************************/
#ifndef __Processor_H__
#define __Processor_H__ 1
//std 
#include <iostream>
#include <string>

//ROOT CERN
#include "TObject.h"
#include "TTree.h"
#include "TFile.h"


class Processor : public TObject 
{
public:
    Processor(std::string processorname,int processorid);
    ~Processor();

    //virtual functions
    virtual void InitAction() =0;
    virtual void ProcessEventAction() =0;
    virtual void EndAction() =0;
    
    virtual void DebugPrint() =0;
    
    //Getters
    std::string GetProcessorName() { return fProcessorName+std::to_string(fProcessorId); }

protected:
    //General attributes, mainly used in task json file manager
    int fProcessorId;
    std::string fProcessorName;
};


#endif
