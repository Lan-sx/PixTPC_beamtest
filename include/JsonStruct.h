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

//nlohmann json
#include <nlohmann/json.hpp>

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
    int Histbins[2];
    double HistXYstart[2];
    double HistXYend[2]; 
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(HistConfigList,NumberOfHist,Dim,Histbins,HistXYstart,HistXYend)
};

}






#endif
