/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-06 20:10
 * Filename         : Rawdata2ROOT.cpp
 * Description      : Class to convert raw binary data to root  
 * Update           : 
 * ******************************************************************/
#include "Rawdata2ROOT.h"

Rawdata2ROOT::Rawdata2ROOT(const char* rawdatafilename) : fPixtpcdata(nullptr)
{
    f_file = new ifstream;
    f_file->open(rawdatafilename,ios::in | ios::binary);
    if(!f_file->is_open())
        throw std::runtime_error("FILE DOES NOT EXIST!");
    fRootName = "./Test_lg_300ns.root";
}

Rawdata2ROOT::~Rawdata2ROOT()
{
    delete f_file;
    if(fPixtpcdata) delete fPixtpcdata;
}

bool Rawdata2ROOT::DoUnpackage()
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

#ifdef __DEBUG__
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
                    tmpArr128Chns.at(chn) = tmpVecSingleChn;
                    tmpVecSingleChn.clear();
                }
                //cout<<evt_flag<<"\t"<<evt_num<<endl;
            }
            binarySeq.clear();

            // fill std::vector<TYPE1>
            fPixtpcdata->FillPixelTPCdata(tmpArr128Chns,0);
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

vector<long long> Rawdata2ROOT::find_header(ifstream* fin,unsigned char *tar,int lengthtar)
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

