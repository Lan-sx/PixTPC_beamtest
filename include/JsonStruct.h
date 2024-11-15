/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-10-10 23:01
 * Filename         : JsonStruct.h
 * Description      : 
 * Update           : 
 * ******************************************************************/
#ifndef __JsonStruct_H__
#define __JsonStruct_H__ 1
#include <string>

//nlohmann json
#include <nlohmann/json.hpp>

//define namespace for json parser
namespace TaskConfigStruct
{
//structure for ChipChn Map, version 1
struct ChipChnMaps_V1
{
    bool isActived;          //
    int globalIdx;           // from 0-2999
    int globalrowcolIdx[2];  // 0-9,0-299
    int localIdx;            // from 1 to 125
    int chipchnIdx[2];       // 0-23, 0-127
    double pixelNoise;       // 
    std::string chipName;    // A1-D1,A3-D3,A5-D5
                             // D2-A2,D4-A4,D6-A6
    std::string IOidxArr[2]; //IO0-IO127 : [A,T] x [1,12]

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ChipChnMaps_V1,isActived,globalIdx,globalrowcolIdx,
                                   localIdx,chipchnIdx,pixelNoise,chipName,IOidxArr)
};


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

//structure for TPChitReco 
struct PixTPChitRecoParsList
{
    bool Isdebug;
    bool EquivalentPad;
    int Processorid;
    int NumOfColMerge;
    std::string Inputfile;
    std::string Inputbranch;
    std::string Outputfile;
    std::string Outputbranch;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PixTPChitRecoParsList,Isdebug,EquivalentPad,Processorid,NumOfColMerge,
                                   Inputfile,Inputbranch,Outputfile,Outputbranch)
};


//structure for Histograms (debug,plots etc) ,TH1 TH2
struct HistConfigList
{
    int NumberOfHist;
    int Dim;       
    std::string Histname;
    std::string Histtitle;
    int Histbins[2];
    double HistXYstart[2];
    double HistXYend[2]; 
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(HistConfigList,NumberOfHist,Dim,Histname,Histtitle,Histbins,HistXYstart,HistXYend)
};

//structure for PrintProcessor input
struct PrintProcessorParsList
{
    bool Isdebug;
    int Processorid;
    int PrintType;
    int NumberOfChips; 
    int PlotPattern[5]; // [0] -> entry_id
                        //        -1, read all entries
                        //        [0,Entries], read one entry, for 2D hist
                        // [1] -> chip_id
                        //        -1, read all chips data
                        //        [0,NumberOfChips-1], read i-th chip data
                        // [2] -> channel_id
                        //        -1, read all channels data
                        //        [0,127], read j-th channel data
                        // [3] -> overthreshold_id
                        //        -1, read 4 over threshold QT
                        //        [0,3], read k-th over threshold QT
                        // [4] -> Q/T flags
    std::string Inputfile;
    std::string Inputbranch;
    HistConfigList HistPars;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PrintProcessorParsList,Isdebug,Processorid,
                                   PrintType,NumberOfChips,PlotPattern,
                                   Inputfile,Inputbranch,HistPars)
};


}






#endif
