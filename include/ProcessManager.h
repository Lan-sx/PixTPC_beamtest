/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 15:55
 * Filename         : ProcessManager.h
 * Description      : Base manager class for PixelTPC data analysis
 * Update           : 
 * ******************************************************************/
#ifndef __ProcessManager_H__
#define __ProcessManager_H__ 1 
//std 

//ROOT CERN
#include "TObject.h"
#include "TObjArray.h"
#include "TTree.h"

//
#include "Processor.h"

class ProcessManager : public TObjArray
{
public:

    void AddProcessor(Processor* processor);
    
    void StartProcessing();

};





#endif
