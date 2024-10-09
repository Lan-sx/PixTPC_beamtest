/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-10-09 11:22
 * Filename         : RawdataConverter.h
 * Description      : 
 * Update           : 
 * ******************************************************************/
#ifndef __RawdataConverter_H__
#define __RawdataConverter_H__ 1
//std
#include <cstring>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <bitset>
#include <iomanip>
#include <exception>

//ROOT CERN
#include "TFile.h"
#include "TTree.h"

//User
#include "PixelTPCdata.h"
#include "BeamUnities.h"


#define __MB__ 1024*1024
#define __DEBUG__
using namespace std;

class RawdataConverter
{
public: 
    RawdataConverter(std::string& rawdatafilename,std::string& rawrootfilename);
    RawdataConverter(const char* rawdatafilename);
    ~RawdataConverter();
   
    // 2024-07-06: convert function, only one TEPIX Chip used, need to update when Det & Ele commissioning
    bool DoUnpackage();
    // 2024-10-08: test ref Jianmeng Dong data
    bool DoUnpackageRawdata2ROOT();
    
    //----------------------------------------
    //@brief find position of header
    //@param ifstream *fin: file stream
    //@param unsigned char *tar: target  
    //@param int lengthtar: length of target
    //----------------------------------------
    vector<long long> find_header(ifstream* fin,unsigned char *tar,int lengthtar);

private:
    bool fIsdebug;
    string fRootName;
    ifstream* f_file;
    PixelTPCdata* fPixtpcdata;

};



#endif
