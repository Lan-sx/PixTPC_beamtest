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
#include "TRandom3.h"

//Users
#include "Processor.h"
#include "GenSimData.h"

using PixJson = nlohmann::json;

//extern global chip chn mapping, .csv file must be provided in task.json file
extern std::vector<std::pair<int,int>> GlobalMaps;

//define namespace for json parser
namespace TaskConfigStruct
{
//structure for Generate simulation data
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
    //Ctors and Dtor
    ProcessManager();
    ProcessManager(std::string taskjsonfile);
    ~ProcessManager() {}

    //TaskType declare
    enum CEPCPixtpcTaskType 
    {
        Rawdata2ROOT=0,
        GenMCdata,
        TPChitReco
    };

    // test function for initial Global maps
    inline void InitialMapsManually(std::string globalmapsfile);
    // Processor Manager 
    int CEPCPixtpcRun();
    
    // TODO testing
    void AddProcessor(Processor* processor);
    void StartProcessing();

protected:
    // Initial GlobalMaps from json
    void InitialMapsFromJson();
    // Default, print usage info
    void PrintUsage();
    // Generate MC data, CEPCPixtpcTaskType: GenMCdata, 1 in json
    void StartGenMCdata();

private:
    std::string fTaskjsonfile;
    //Nlohmann json parser
    PixJson fPixJsonParser;
};

inline void ProcessManager::InitialMapsManually(std::string globalmapsfile)
{
    GlobalMaps = BeamUnities::CreateChipChnToRowColMap(globalmapsfile);
}

#endif
