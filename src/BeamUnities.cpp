/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-09-04 20:01
 * Filename         : BeamUnities.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "BeamUnities.h"

void cepcPixTPClog(int flags, const std::string& file,int line,const std::string& message,bool islocated)
{
    std::string logflagStr;
    switch (flags)
    {
        case PIXtpcINFO:
            logflagStr = "INFO";
            break;
        case PIXtpcERROR:
            logflagStr = "ERROR";
            break;
        case PIXtpcWARNING:
            logflagStr = "WARNING";
            break;
        case PIXtpcDebug:
            logflagStr = "DEBUG";
            break;
    }
    if(islocated)
    {
        std::printf("[cepcPixTPC %s] File: %s, Line: %d: %s\n",logflagStr.c_str(),
                    file.c_str(),
                    line,
                    message.c_str());
    }
    else
    {
        std::printf("[cepcPixTPC %s]: %s\n",logflagStr.c_str(),
                    message.c_str());
    }
}

std::vector<std::pair<int,int>> BeamUnities::CreateChipChnToRowColMap(std::string ChipChnmapfile)
{
    std::vector<std::pair<int,int>> vMaps;

    std::ifstream fin(ChipChnmapfile.data(),std::ios::in);
    if(!fin.is_open())
    {
        //std::printf("########!!!!Error!!! Maps file doesn't exist\n");
        throw std::runtime_error("########!!!!Error!!! Maps file doesn't exist");
    }
    std::string lines;

    //skip header line
    std::getline(fin,lines);
    
    while(std::getline(fin,lines))
    {
        std::istringstream ss(lines);
        std::string token;
        std::vector<int> rows;
        
        int tmpcnts=0;
        while(std::getline(ss,token,','))
        {
            rows.push_back(std::stoi(token));
        }
        
        vMaps.push_back(std::make_pair(rows.at(1),rows.at(2)));
    }
    
    fin.close();

    return vMaps;
}

std::pair<int,int> BeamUnities::RowColIdx2ChipChn(int row, int col,const std::vector<std::pair<int,int>> vMaps)
{
    if(row<0 || row>=__ROW__ || col<0 || col>=__COL__) 
    {
        //std::printf(">>>>>>>>>>>>>>>>>>>>>> Error!!! row col in RowColIdx2ChipChn\n");
        PixTPCLog(PIXtpcERROR,">>> Error!!! row col in RowColIdx2ChipChn",true);
        //throw  std::out_of_range(">>>>>>>>>>>>>>>>>>>>>> Error!!! row col in RowColIdx2ChipChn");
        return std::make_pair(-1,-1);
    }
    
    int IdxGlobal = row*__COL__ + col;
    
    return vMaps.at(IdxGlobal);
}

std::pair<int,int> BeamUnities::RowColIdx2ChipChn(std::pair<int,int> rowcolpair,const std::vector<std::pair<int,int>> vMaps)
{
    int rowid = rowcolpair.first; 
    int colid = rowcolpair.second; 

    return BeamUnities::RowColIdx2ChipChn(rowid,colid,vMaps);
}

std::pair<int,int> BeamUnities::ChipChn2RowCol(int chipIdx, int chnIdx, const std::vector<std::pair<int,int>> vMaps)
{
    if(chipIdx <0 || chipIdx>=__NumChip__ || chnIdx<0 || chnIdx >= __NumChn__)
    {
        //std::printf(">>>>>>>>>>>>>>>>>>>>>> Error!!! chip chn Idx in ChipChn2RowCol\n");
        PixTPCLog(PIXtpcERROR,">>> Error!!! chip chn Idx in ChipChn2RowCol",true);
        return std::make_pair(-1,-1);
    }
    auto chipchnpair = std::make_pair(chipIdx,chnIdx);

    auto it = std::find(vMaps.cbegin(),vMaps.cend(),chipchnpair);

    //Add find check
    if(it!=vMaps.end())
    {
        auto Idxglobal = std::distance(vMaps.begin(),it);
        return std::make_pair(Idxglobal/__COL__,Idxglobal%__COL__);
    }
    else 
    {
        return std::make_pair(-1,-1);
    }
}

std::pair<int,int> BeamUnities::ChipChn2RowCol(std::pair<int,int> chipchnpair,const std::vector<std::pair<int,int>> vMaps)
{
    int chipid = chipchnpair.first; 
    int chnid = chipchnpair.second; 
    
    return  BeamUnities::ChipChn2RowCol(chipid,chnid,vMaps);
}

std::pair<int,int> BeamUnities::Position2RowCol(double px, double py)
{

    if(px < Xlowboundary || px > X_upboundary || py < Ylowboundary || py > Yupboundary)
    {
        return std::make_pair(-1,-1);
    }

    int rowidx_ = int((px-Xlowboundary)/PixelSpace);
    int colidx_ = int((py-Ylowboundary)/PixelSpace);
    
    auto pixelcenter = BeamUnities::RowColIdx2Position(rowidx_,colidx_);
    // if (px,py) is in pixel
    if(fabs(px-pixelcenter.first)<PixelSize/2 && fabs(py-pixelcenter.second)<PixelSize/2.)
    {
        return std::make_pair(rowidx_,colidx_) ;
    }
    else
    {
        return std::make_pair(-1,-1);
    }

}

std::pair<int,int> BeamUnities::Position2RowCol(std::pair<double,double> positionpair)
{
    double px = positionpair.first; 
    double py = positionpair.second;

    return BeamUnities::Position2RowCol(px,py);
}

std::pair<double,double> BeamUnities::RowColIdx2Position(int row, int col)
{
    if(row >= __ROW__ || col >= __COL__)  
    {
        std::printf("=============> Error! row col index\n");
        std::make_pair(1.,16.);
    }

    return std::make_pair(row*PixelSpace,col*PixelSpace);
}

std::pair<double,double> BeamUnities::RowColIdx2Position(std::pair<int,int> rowcolpair)
{
    int rowid = rowcolpair.first; 
    int colid = rowcolpair.second;

    return BeamUnities::RowColIdx2Position(rowid,colid);
}

void BeamUnities::DFS_algo(int row, int col, PixelMatrix &matrix, std::set<std::pair<int, int>> &cluster, std::vector<std::vector<bool>> &visited,bool eightdirec) 
{
    visited[row][col] = true;
    cluster.insert({row, col});
    
    int Npoint = (!eightdirec) ? 4 : 8;
    int counter = 0;
    for (const auto &dir : directions8) 
    {
        if(counter<Npoint)
        {
            int newRow = row + dir[0];
            int newCol = col + dir[1];
            counter++;
            //check boundary conditions
            if (newRow >= 0 && newRow < matrix.GetNrows() && newCol >= 0 && newCol < matrix.GetNcols()) {
                if (!visited[newRow][newCol] && matrix[newRow][newCol] != 0) 
                {
                    DFS_algo(newRow, newCol, matrix, cluster, visited);
                }
            }
        }
        else 
            break;
    }
} 

std::vector<std::set<std::pair<int,int>>> BeamUnities::PixClusterFinder(PixelMatrix& matrix, bool usingEightdirec)
{
    std::vector<std::set<std::pair<int, int>>> clusters;
    int nRows = matrix.GetNrows();
    int nCols = matrix.GetNcols();

    std::vector<std::vector<bool>> visited(nRows, std::vector<bool>(nCols, false));

    for (int i = 0; i < nRows; ++i) 
    {
        for (int j = 0; j < nCols; ++j) 
        {
            if (matrix[i][j] != 0 && !visited[i][j]) 
            {
                std::set<std::pair<int, int>> cluster;
                DFS_algo(i, j, matrix, cluster, visited, usingEightdirec);
                clusters.push_back(cluster);
            }
        }
    }
    return clusters;
}



