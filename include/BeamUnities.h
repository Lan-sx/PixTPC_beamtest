/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-04 19:51
 * Filename         : BeamUnities.h
 * Description      : 
 * Update           : 
 * ******************************************************************/
#ifndef __BeamUnities_H__
#define __BeamUnities_H__ 1

//std 
#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <stdexcept>

//ROOT CERN
#include "PixelMatrix.h"

//global pars
constexpr int  __ROW__=10;
constexpr int  __COL__=300;
constexpr int  __NumChip__=24;
constexpr int __NumChn__=128;

constexpr double PixelSize = 0.04; // [cm]
constexpr double PixelPitch = 0.01; // [cm]
constexpr double PixelSpace = PixelSize+PixelPitch;
constexpr double Ylowboundary = -PixelSize/2;  // [cm]
constexpr double Yupboundary = (PixelSize+PixelPitch)*__COL__+Ylowboundary;  // [cm]
constexpr double Xlowboundary = -PixelSize/2.;
constexpr double X_upboundary = (PixelSize+PixelPitch)*__ROW__+Xlowboundary;

//Diffiusion pars, from Yue Chang 
constexpr double X_Cd = 8.66021e-3;  // [sqrt(cm)]
constexpr double Y_Cd = 8.71839e-3;  // [sqrt(cm)]
constexpr double Sigma_aval = 0.003; // [cm]
constexpr double Velo_e = 8.; // [cm/us] from Magboltz

//Simulation Pars for MC data
constexpr double MaxDriftLength = 50.; // [cm]
constexpr double MaxLengthY = 20.; // [cm]
constexpr double Mean_Pixbaseline = 0.; // default, mean baseline of all pixels are set to 0 e- 
constexpr double Sigma_Pix = 30.; // default, sigma of all pixels are set to 30 e- (fluctuation of baseline)
constexpr double Sigma_electronics = 0.035; // default, electronics noise (3.5%) for all pixels                
constexpr int NumOfe_cut = 300;   // The pixel is actived if NumOf e- > 300

//Define macro for Log Print
enum LogsFlag {
    PIXtpcINFO = 0,
    PIXtpcERROR
};

#define PixTPCLog(flags,message) cepcPixTPClog(flags,__FILE__,__LINE__,message)

void cepcPixTPClog(int flags, const std::string& file,int line,const std::string& message);

//Colors array for visualization
constexpr int ColorArray[9] = {kBlack,kBlue+1,kPink,
                               kViolet+7,kOrange+8,kGreen,
                               kRed,kRed+2,kYellow-3}; 

class PixelMatrix;

namespace BeamUnities
{
// Maps Chip,Channel index <-> Row,Col index
// @param std::string ChipChnmapfile, config csv file 
std::vector<std::pair<int,int>> CreateChipChnToRowColMap(std::string ChipChnmapfile);

// Row, Col index -> Chip, Channel index
// @param int row, row index, from 0 to __ROW__ -1
// @param int col, col index, from 0 to __COL__-1
// @parma vMaps, maps
std::pair<int,int> RowColIdx2ChipChn(int row, int col, const std::vector<std::pair<int,int>> vMaps);
std::pair<int,int> RowColIdx2ChipChn(std::pair<int,int> rowcolpair,const std::vector<std::pair<int,int>> vMaps);

// Chip, Channel index -> Row, Col index
// @param int chipIdx, chip index, from 0 to __NumChip__ -1
// @param int chnIdx, channel index, from 0 to __NumChn__ -1
std::pair<int,int> ChipChn2RowCol(int chipIdx, int chnIdx,const std::vector<std::pair<int,int>> vMaps);
std::pair<int,int> ChipChn2RowCol(std::pair<int,int> chipchnpair,const std::vector<std::pair<int,int>> vMaps);

// Position (x,y) -> pixel row col index
// @param double px, position x
// @param double py, position y
std::pair<int,int> Position2RowCol(double px, double py);
std::pair<int,int> Position2RowCol(std::pair<double,double> positionpair);

// Pixel row col index -> Pixel Center Position (x,y)
// @param int row, row index
// @param int col, col index
std::pair<double,double> RowColIdx2Position(int row, int col);
std::pair<double,double> RowColIdx2Position(std::pair<int,int> rowcolpair);

// functions and constexpr for PixelCluster Finding
// search directions
constexpr std::array<std::array<int,2>,8> directions8 = {
                                                         {{1, 0}, {-1, 0}, {0, 1}, {0, -1},
                                                          {1, 1}, {1, -1}, {-1,-1}, {-1,1}}
                                                        };
//DFS algo 
//@param int row, row index
//@param int col, col index
//@param PixelMatrix& matrix, input matrix
//@param std::set<std::pair<int,int>> cluster
//@param std::vector<bool>& visited, vector to flag if the pixel visited
//@param bool eightdirec, search 4/8 directions 
void DFS_algo(int row, int col, PixelMatrix &matrix, std::set<std::pair<int, int>> &cluster, std::vector<std::vector<bool>> &visited,bool eightdirec=false);

//PixClusterFinder, ref TimePix Reconstruction program, TimePixClusterFinderProcessor
//@param PixelMatrix& matrix, input matrix
//@param bool eightdirec, search 4/8 directions 
std::vector<std::set<std::pair<int,int>>> PixClusterFinder(PixelMatrix& matrix, bool usingEightdirec=false);

}



#endif
