/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-10-09 11:22
 * Filename         : RawdataConverter.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/
#include "RawdataConverter.h"

RawdataConverter::RawdataConverter(std::string& rawdatafilename,std::string& rawrootfilename) : 
    fIsdebug(false),fRootName(rawrootfilename),fPixtpcdata(nullptr)
{
    f_file = new ifstream;
    f_file->open(rawdatafilename,ios::in | ios::binary);
    if(!f_file->is_open())
        throw std::runtime_error("FILE DOES NOT EXIST!");
}

RawdataConverter::RawdataConverter(const char* rawdatafilename) : fIsdebug(false),fPixtpcdata(nullptr)
{
    f_file = new ifstream;
    f_file->open(rawdatafilename,ios::in | ios::binary);
    if(!f_file->is_open())
        throw std::runtime_error("FILE DOES NOT EXIST!");
    fRootName = "./Test_lg_300ns.root";
}

RawdataConverter::~RawdataConverter()
{
    delete f_file;
    if(fPixtpcdata) 
    {
        //PixTPCLog(PIXtpcDebug,"fPixtpcdata  Dtors",true);
        delete fPixtpcdata;
    }
}

void  RawdataConverter::ConfigDebugHist(TaskConfigStruct::HistConfigList inputhistconfig)
{
    if(!fIsdebug)
    {
        PixTPCLog(PIXtpcWARNING,"Only used in debug mode",false);
        return;
    }
    fHistconfig.Dim = inputhistconfig.Dim;
    fHistconfig.Histbins[0]=inputhistconfig.Histbins[0];
    fHistconfig.Histbins[1]=inputhistconfig.Histbins[1];
    fHistconfig.HistXYstart[0]=inputhistconfig.HistXYstart[0];
    fHistconfig.HistXYstart[1]=inputhistconfig.HistXYstart[1];
    fHistconfig.HistXYend[0]=inputhistconfig.HistXYend[0];
    fHistconfig.HistXYend[1]=inputhistconfig.HistXYend[1];
}

bool RawdataConverter::DoUnpackageRawdata2ROOT()
{
    // header, ref Jianmeng Dong and Canwen Liu 
    unsigned char header_r[] = {0x55,0xaa};
    // finder header position 
    auto vheaderPos = find_header(f_file,header_r,2);
    //buffer vector
    vector<unsigned char> vBuffer;

    if(fIsdebug)
    {
        //fHistdebug = std::make_shared<TH1D>("hChips",";chip idx;Cnts",4,0,4);
        fHistdebug = std::make_shared<TH1D>("hChips",";Amp. [LSB];Cnts",fHistconfig.Histbins[0],
                                                                        fHistconfig.HistXYstart[0],
                                                                        fHistconfig.HistXYend[0]);
    }

    bool ChippackageLengthWarning = false;
    //loop all packages 
    for(size_t ii=1; ii<vheaderPos.size();++ii)
    {
        if(ii%5000==0)
            std::printf("### %zu packages Done!\n",ii);
        auto bufferlength = vheaderPos.at(ii)-vheaderPos.at(ii-1);
        if(bufferlength!=1858)
        {
            //if(fIsdebug)
            //    PixTPCLog(PIXtpcERROR,"There are Package length errors",true);
            ChippackageLengthWarning = true;
            continue;
        }
        vBuffer.resize(bufferlength);
        f_file->read(reinterpret_cast<char*>(vBuffer.data()),bufferlength);

        if(fIsdebug && ii<2)
        {
            printHeaderTail(vBuffer);    
            printChipNumber(vBuffer);
            printTimeStamp(vBuffer);
            printTriggerNum(vBuffer);
        }

        int chipNumber = static_cast<int>(vBuffer.at(3));
        if(fIsdebug)
            fHistdebug->Fill(chipNumber);

        vector<bool> binarySeq;

        // Convert bufferlength unsigned char to BinSequeue size = (bufferlength-16)*8 
        int kk=0;
        for(const auto &byte : vBuffer) 
        {
            //skip head,chipnumber,timestamp,triggernum = 16byte
            if(kk<16)
            {
                kk++;
                continue;
            }

            for(int mm=7; mm >= 0; --mm)
            {
                unsigned char bit = (byte >> mm) & 1; 
                binarySeq.push_back(bit);
            }
        }

#if 0
        if(fIsdebug && ii<4)
        {
            cout<<"Package"<<ii<<":";
            for(int ibits=0; ibits<31; ++ibits)
            {
                cout<<std::dec<<DataBinSeq.at(ibits);
            }
            cout<<endl;
        }
#endif

        bool evt_flag = true;
        int evt_num = 4;
        bitset<2> evtBits;
        bitset<14> timeBits,ampBits;
        size_t INDEX = 0;
        // Read T/Q info from 128 chips 
        for(int chn_id=0; chn_id < __NumChn__; ++chn_id)
        {
            if(INDEX >= binarySeq.size()-2)
            {
                PixTPCLog(PIXtpcERROR,"------Package Error! out range of binarySeq size",true);
            }
            else
            {
                evt_flag = static_cast<bool>(binarySeq.at(INDEX));
                //!!! For std::bitset<N> bs, index from right to left
                evtBits[1]=binarySeq.at(INDEX+1);
                evtBits[0]=binarySeq.at(INDEX+2);
                if(evt_flag)
                    evt_num = evtBits.to_ulong()+1;
                else 
                    evt_num=0;

                for(int i_evt=0;i_evt<evt_num;++i_evt)
                {
                    //fill raw Gray code
                    for(int i_bits=0;i_bits<14;++i_bits)
                    {
                        auto timeIdxOffset = INDEX +3 ;
                        auto ampIdxOffset = INDEX +3 +14;
                        //!!! For bitset<N> bs, index from right to left
                        //!!! So start from index 13
                        timeBits[13-i_bits] = binarySeq.at(i_bits+timeIdxOffset+28*i_evt);
                        ampBits[13-i_bits] = binarySeq.at(i_bits + ampIdxOffset+28*i_evt);
                    }
                    //convert raw Gray code to Binary code and decimal  
                    for(int ll=13;ll>=0;--ll)
                    {
                        timeBits[ll] = (ll==13) ? timeBits[ll] : (timeBits[ll+1] ^ timeBits[ll]); 
                        ampBits[ll] = (ll==13) ? ampBits[ll] : (ampBits[ll+1] ^ ampBits[ll]); 
                    }
                    
                    if(fIsdebug && i_evt==0 && chipNumber==0)
                    {
                        fHistdebug->Fill(double(ampBits.to_ulong()));
                    }

                }//end of channel evts 
            }
        }//end of this package / end of 128 channels   
        binarySeq.clear();
    }//end of data


    f_file->close();
    if(ChippackageLengthWarning)
        PixTPCLog(PIXtpcWARNING,"Chips data length error !!!",false);
    return true;
}

bool RawdataConverter::DoUnpackage()
{
    // !!! Need to Update
    // create a root file and tree, link PixelTPCdata
    auto fileroot = new TFile(fRootName.c_str(),"RECREATE");
    fPixtpcdata = new PixelTPCdata(1);
    auto tree = new TTree("PixTPCdata","tpc channel data");
    tree->Branch("pixelTPCdata",&fPixtpcdata);

    // data buffer vector 
    vector<unsigned char> vBuffer;
    int vld_frame =0;

    // header
    // need to update
    unsigned char header_r[] = {0x57,0x46,0x19,0x23};
    // finder header position 
    auto vheaderPos = find_header(f_file,header_r,4);

    // sub collections for fill PixelTPCdata
    PairQT tmpPairqt;
    VecSingleChn tmpVecSingleChn;
    Array128Chns tmpArr128Chns;
    //tmpArr128Chns.resize(__NumChn__);

    for(size_t ii=1; ii<vheaderPos.size();++ii)
    {
        auto bufferlength = vheaderPos.at(ii) - vheaderPos.at(ii-1);
        vBuffer.resize(bufferlength);
        f_file->read(reinterpret_cast<char*>(vBuffer.data()),bufferlength);

        //=============================================================
        //Evt level: Triggle ID, TOP LEVEL, fill std::vector<TYPE1>
        //=============================================================
        if(bufferlength==1856)
        {
            fPixtpcdata->SetTiggleID(vld_frame);

            // convert buffer to a binary sequence, length == bufferlength*8; 
            vector<bool> binarySeq;
            for(size_t idx=8; idx < vBuffer.size();++idx)
            {
                for(int m=7; m>=0; --m)
                {
                    unsigned char bit = (vBuffer.at(idx) >> m) & 1;
                    binarySeq.push_back(bit);
                }
            }

            // get evt_num amp time info from 128 channnel
            bool evt_flag = true;
            int evt_num = 4;
            bitset<2> evtBits;
            bitset<14> timeBits,ampBits;
            size_t INDEX = 0;

            //=============================================================
            //Channel level: fill std::array<TYPE2,128>
            //=============================================================
            for(int chn=0; chn<128; ++chn)
            {
                if(INDEX >= binarySeq.size()-2)
                {
                    std::printf("-------------- Package Error! \n");
                    break;
                }
                else
                {
                    evt_flag = static_cast<bool>(binarySeq.at(INDEX));
                    //!!! For std::bitset<N> bs, index from right to left
                    evtBits[1]=binarySeq.at(INDEX+1);
                    evtBits[0]=binarySeq.at(INDEX+2);

                    if(evt_flag)
                        evt_num = evtBits.to_ulong()+1;
                    else
                        evt_num = 0;

                    //=============================================================
                    //electonical events level: fill std::vector<TYPE3>
                    //=============================================================

                    //calc time and amp for each events
                    for(int i_evt=0;i_evt<evt_num;++i_evt)
                    {
                        //fill raw Gray code
                        for(int i_bits=0;i_bits<14;++i_bits)
                        {
                            auto timeIdxOffset = INDEX +3 ;
                            auto ampIdxOffset = INDEX +3 +14;
                            //!!! For bitset<N> bs, index from right to left
                            //!!! So start from index 13
                            timeBits[13-i_bits] = binarySeq.at(i_bits+timeIdxOffset+28*i_evt);
                            ampBits[13-i_bits] = binarySeq.at(i_bits + ampIdxOffset+28*i_evt);
                        }
                        //convert raw Gray code to Binary code and decimal  
                        for(int ll=13;ll>=0;--ll)
                        {
                            timeBits[ll] = (ll==13) ? timeBits[ll] : (timeBits[ll+1] ^ timeBits[ll]); 
                            ampBits[ll] = (ll==13) ? ampBits[ll] : (ampBits[ll+1] ^ ampBits[ll]); 
                        }

                        //=============================================================
                        //QT level: fill std::pair<double,double>
                        //=============================================================

#if 1
                        //debug print
                        if(chn<2 && i_evt<4 && vld_frame<2)
                            cout<<"========> vld_cnt: "<<vld_frame<<" chn: "<<chn<<"\t"<<timeBits.to_ulong()
                                <<"\t"<<ampBits.to_ulong()<<endl;
#endif
                        //fill std::vector<TYPE3>
                        tmpVecSingleChn.push_back(std::make_pair(double(timeBits.to_ulong()),double(ampBits.to_ulong())));
                    }

                    INDEX = INDEX+3+28*evt_num;
                    // fill std::array<TYPE2,128>
                    //std::cout<<"================> Degbug Print "<<tmpArr128Chns.size()<<std::endl;
                    //tmpArr128Chns.at(chn) = tmpVecSingleChn;
                    tmpArr128Chns.push_back(tmpVecSingleChn);
                    tmpVecSingleChn.clear();
                }
                //cout<<evt_flag<<"\t"<<evt_num<<endl;
            }
            binarySeq.clear();

            // fill std::vector<TYPE1>
            fPixtpcdata->FillPixelTPCdata(tmpArr128Chns,0);
            tmpArr128Chns.clear();
            tree->Fill();
            fPixtpcdata->ClearPixelTPCdata(1);
            vld_frame++;
        }

        std::fill_n(vBuffer.data(),bufferlength,0);
    }

    //write root file 
    tree->Write();
    fileroot->Close();

    cout<<"===========> "<<vld_frame<<endl;
    cout<<"Write Done!"<<endl;
    return true;
}

vector<long long> RawdataConverter::find_header(ifstream* fin,unsigned char *tar,int lengthtar)
{
    vector<long long> vheaderPos;
    vector<char> tempbuffer(__MB__+lengthtar-1); // 1MB buffer plus (headersize - 1)
    streampos filepos =0;
    while(*fin)
    {
        fin->read(tempbuffer.data()+lengthtar-1,__MB__);
        streamsize bytesRead = fin->gcount();

        for(streamsize i=0; i <= bytesRead; ++i)
        {
            //cmp with tar
            if(std::memcmp(tempbuffer.data()+i,tar,lengthtar)==0)
            {
                vheaderPos.push_back(filepos+i);
            }
        }
        //update filepos
        filepos += bytesRead;
        //move tail of tempbuffer to the head
        std::memmove(tempbuffer.data(),tempbuffer.data()+__MB__,lengthtar-1);
    }

    //clear fin error and come back to file head
    fin->clear();
    fin->seekg(0,ios::beg);

    for(auto &it : vheaderPos)
        it -= (lengthtar-1);

    return vheaderPos;
}

