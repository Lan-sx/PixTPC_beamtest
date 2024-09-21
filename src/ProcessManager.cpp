/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-07 16:05
 * Filename         : ProcessManager.cpp
 * Description      : Base manager class for PixelTPC data analysis 
 * Update           : 
 * ******************************************************************/
#include "ProcessManager.h"

void ProcessManager::AddProcessor(Processor* processor)
{
    this->Add(processor);
}

void ProcessManager::StartProcessing()
{
    for(int i=0; i<this->GetEntries();++i)
    {
        auto processor_i = dynamic_cast<Processor*>(this->At(i));
        processor_i->InitAction();
        processor_i->ProcessEventAction();
        processor_i->EndAction();
    }
}
