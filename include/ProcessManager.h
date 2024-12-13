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
#include <vector>
#include <algorithm>

//nlohmann json
#include <nlohmann/json.hpp>

//ROOT CERN
#include "TObject.h"
#include "TObjArray.h"
#include "TTree.h"
#include "TRandom3.h"

//Users
#include "JsonStruct.h"
#include "RawdataConverter.h"
#include "Processor.h"
#include "PixHitRecoSimpleProcessor.h"
#include "PixHitRecoCCAProcessor.h"
#include "PrintProcessor.h"
#include "GenSimData.h"

using PixJson = nlohmann::json;

//Legacy map
//extern global chip chn mapping, .csv file must be provided in task.json file
extern std::vector<std::pair<int,int>> GlobalMaps;
//extern global json mapping, .json file muse be provided in task.json file
extern std::vector<TaskConfigStruct::ChipChnMaps_V1> GlobalJsonMaps;

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
        TPCEvtsReco,
        TPCcalibration
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
    //Convert raw binary data to .root data
    void StartUnpackage();
    // Default, print usage info
    void PrintUsage();
    // Generate MC data, CEPCPixtpcTaskType: GenMCdata, 1 in json
    void StartGenMCdata();
    // Reco PixTPC hits, CEPCPixtpcTaskType: TPChitReco, 2 in json 
    void StartRecoPixTPCEvts();


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
