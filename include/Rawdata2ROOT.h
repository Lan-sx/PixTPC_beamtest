/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-06 19:46
 * Filename         : Rawdata2ROOT.h
 * Description      : Class to convert raw binary data to root 
 *                    Ref. Canwen Liu matlab code 
 * Update           : 
 * ******************************************************************/
#ifndef __Rawdata2ROOT_H__
#define __Rawdata2ROOT_H__ 1
//std
#include <cstring>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <bitset>
#include <exception>

//ROOT CERN
#include "TFile.h"
#include "TTree.h"

#include "PixelTPCdata.h"


#define __MB__ 1024*1024
#define __DEBUG__
using namespace std;

class Rawdata2ROOT 
{
public: 
    Rawdata2ROOT(const char* rawdatafilename);
    ~Rawdata2ROOT();
   
    // 2024-07-06: convert function, only one TEPIX Chip used, need to update when Det & Ele commissioning
    bool DoUnpackage();
    
    //----------------------------------------
    //@brief find position of header
    //@param ifstream *fin: file stream
    //@param unsigned char *tar: target  
    //@param int lengthtar: length of target
    //----------------------------------------
    vector<long long> find_header(ifstream* fin,unsigned char *tar,int lengthtar);

private:
    PixelTPCdata* fPixtpcdata;
    string fRootName;
    ifstream* f_file;

};

#endif
