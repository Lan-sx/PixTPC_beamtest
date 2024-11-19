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
#include <memory>

//ROOT CERN
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

//User
#include "JsonStruct.h"
#include "PixelTPCdata.h"
#include "BeamUnities.h"


#define __MB__ 1024*1024

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
    bool DoUnpackageRawdata2ROOT(int numofChipUsed);
    // TODO, add a function to merge root data. 
    // There will be 3 CUBES boards with 3 IP addresses and each board will output a data file with 8 TEPIX chips

    //Switch debug 
    void EnableUnpackgeDebug() { fIsdebug = true; } 
    void DisableUnpackgeDebug() { fIsdebug = false; }
    void SetdebugHistIndex(int chips, int evtoverThreshold, bool amp=true) { 
        fChipdebug= chips < 8 ? chips : 0; 
        fOverThdebug= evtoverThreshold < 4 ? evtoverThreshold : 0; 
        fAmpOrTime=amp; 
    }
    decltype(auto) GetDebugHist() { return fHistdebug; }

    //Set/Config Debug histogram bins,start bins, end bins when EnableUnpackgeDebug
    void  ConfigDebugHist(TaskConfigStruct::HistConfigList inputhistconfig);

protected:
    //----------------------------------------
    //@brief find position of header
    //@param ifstream *fin: file stream
    //@param unsigned char *tar: target  
    //@param int lengthtar: length of target
    //----------------------------------------
    vector<long long> find_header(ifstream* fin,unsigned char *tar,int lengthtar);

    //----------------------------------------
    //@brief description: Read Time stamp in big endian
    //@param vector<unsigned char>: data package buffer
    //----------------------------------------
    unsigned long GetTimeStampinBigendian(const vector<unsigned char> vbuffer);
    
    //----------------------------------------
    //@brief fill fPixtpcdata table (3-D data table) from one data package. 
    //       There will be 24 data packages in a beam trigger event
    //@param vector<unsigned char> vbuffer: a data package buffer
    //@param int chipnumber: chip number of this data package, from 0-7 for one board
    //----------------------------------------
    void FillPixelTPCdataTable(const vector<unsigned char> vbuffer,int chipnumber);

    //----------------------------------------
    //@brief description: 
    //       some debug print functions
    //----------------------------------------
    inline void printHeaderTail(const vector<unsigned char> vecbuffer);
    inline void printChipNumber(const vector<unsigned char> vecbuffer);
    inline void printTimeStamp (const vector<unsigned char> vecbuffer);
    inline void printTriggerNum(const vector<unsigned char> vecbuffer);

private:
    bool fIsdebug;
    string fRootName;
    ifstream* f_file;
    PixelTPCdata* fPixtpcdata;
    
    //histogram config, used in debug
    //TaskConfigStruct::HistConfigList fHistconfig;
    //histogram for debug
    std::shared_ptr<TH1D> fHistdebug;
    //vars to control debug histogram 
    bool fAmpOrTime;
    int fChipdebug;
    int fOverThdebug;
};


inline void RawdataConverter::printHeaderTail(const vector<unsigned char> vecbuffer)
{

    cout<<"Head: ";
    cout<<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(0))
        <<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(1))<<" ";
    cout<<"Tail: ";
    cout<<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(vecbuffer.size()-2))
        <<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(vecbuffer.size()-1))<<endl;
}

inline void RawdataConverter::printChipNumber(const vector<unsigned char> vecbuffer)
{
    cout<<"Chip Number: ";
    cout<<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(2))
        <<std::hex<<std::setw(2)<<std::setfill('0')
        <<static_cast<int>(vecbuffer.at(3));
    cout<<" -> "<<std::dec<<static_cast<int>(vecbuffer.at(2))*4+static_cast<int>(vecbuffer.at(3))<<endl;
}

inline void RawdataConverter::printTimeStamp (const vector<unsigned char> vecbuffer)
{
    std::cout<<"TimeStamp: ";
    for(int ii=0; ii<8;++ii) 
    {
        cout<<std::hex<<std::setw(2)<<std::setfill('0')
            <<static_cast<int>(static_cast<unsigned char>(vecbuffer.at(ii+4)))<<" ";
    }
    cout<<endl;
}

inline void RawdataConverter::printTriggerNum(const vector<unsigned char> vecbuffer)
{
    std::cout<<"Trigger Number: ";
    for(int ii=0; ii<4;++ii) 
    {
        cout<<std::hex<<std::setw(2)<<std::setfill('0')
            <<static_cast<int>(static_cast<unsigned char>(vecbuffer.at(ii+12)))<<" ";
    }
    cout<<endl;
}

#endif
