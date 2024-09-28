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
#include <fstream>
#include <string>

//nlohmann json
#include <nlohmann/json.hpp>

//ROOT CERN
#include "TObject.h"
#include "TObjArray.h"
#include "TTree.h"

//Users
#include "Processor.h"
#include "GenSimData.h"

using PixJson = nlohmann::json;

namespace TaskConfigStruct
{

struct GenSimDataParsList
{
    bool Isdebug;
    int Nevts;
    std::string Particle;
    double Momentum;
    double MomentumReso;
    double InitialPos[3];
    double InitialDir[3];

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GenSimDataParsList,Isdebug,Nevts,Particle,Momentum,MomentumReso,InitialPos,InitialDir)
};

}


class ProcessManager : public TObjArray
{
public:
    ProcessManager();
    ProcessManager(std::string taskjsonfile);
    ~ProcessManager() {}

    enum CEPCPixtpcTaskType 
    {
        Rawdata2ROOT=0,
        GenMCdata,
        TPChitReco
    };

    // Processor Manager 
    int CEPCPixtpcRun();

    // TODO testing
    void AddProcessor(Processor* processor);
    void StartProcessing();

protected:

    void StartGenMCdata();


private:
    std::string fTaskjsonfile;
    PixJson fPixJsonParser;
};





#endif
