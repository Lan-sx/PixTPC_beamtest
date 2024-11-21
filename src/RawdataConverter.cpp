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
    fAmpOrTime = true;
    fChipdebug = 0;
    fOverThdebug = 0;

    f_file = new ifstream;
    f_file->open(rawdatafilename,ios::in | ios::binary);
    if(!f_file->is_open())
        throw std::runtime_error("FILE DOES NOT EXIST!");
}

RawdataConverter::RawdataConverter(const TaskConfigStruct::RawdataConvParsList inputPars) : fIsdebug(true),fPixtpcdata(nullptr),f_file(nullptr)
{
    fRootName = "";
    fAmpOrTime = true;
    fChipdebug = 0;
    fOverThdebug = 0;

    f_InputPars = inputPars;
}

RawdataConverter::RawdataConverter(const char* rawdatafilename) : fIsdebug(true),fPixtpcdata(nullptr)
{
    fAmpOrTime = true;
    fChipdebug = 0;
    fOverThdebug = 0;

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
    
    auto hist_name = inputhistconfig.Histname.data();
    auto hist_title = inputhistconfig.Histtitle.data();
    fHistdebug = std::make_shared<TH1D>(hist_name,hist_title,inputhistconfig.Histbins[0],
                                                             inputhistconfig.HistXYstart[0],
                                                             inputhistconfig.HistXYend[0]);
    PixTPCLog(PIXtpcDebug,"RawdataConverter debug hist created! fill Q/T from i-th chip",false);
}

void  RawdataConverter::ConfigDebugHist(int converter_idx, int nbins, int x_start, int x_end)
{
     if(!fIsdebug)
    {
        PixTPCLog(PIXtpcWARNING,"Only used in debug mode",false);
        return;
    }
    
    TString hist_name = Form("hist_debug%d",converter_idx);

    fHistdebug = std::make_shared<TH1D>(hist_name.Data(),"", nbins, x_start, x_end);
                                                             
    PixTPCLog(PIXtpcINFO,"RawdataConverter debug hist created! fill Q/T from i-th chip",false);
}

bool RawdataConverter::DoUnpackageRawdata2ROOT_withMultiIP()
{
    //===================================================================
    //Convert each board data to sub .root files
    //===================================================================
    //Get number of IP from inputPars
    int numofIpaddress = f_InputPars.NumOfIpAddress;
    //Get Isdebug flag
    bool isdebugflag = f_InputPars.Isdebug;
    // Create a TCanvas to plot debug hist if isdebug==true
    TCanvas *mycDebug = nullptr;
    if(isdebugflag)
    {
        mycDebug = new TCanvas("mycDebug","mycDebug",1200,400);
        mycDebug->Divide(numofIpaddress,1);
    }

    int NumOfChipsToMerge =0;
    //Create numofIpaddress RawdataConverters 
    for(int converter_i=0; converter_i < numofIpaddress; ++converter_i)
    {
        //create i-th data converter
        auto myConverter_i = new RawdataConverter(f_InputPars.InputfileArray[converter_i],
                                                  f_InputPars.OutputfileArray[converter_i]);
        int numofchipsused_i = f_InputPars.NumberOfChipsUsed[converter_i];
        // add numofChipUsed
        NumOfChipsToMerge += numofchipsused_i;

        if(isdebugflag)
        {
            myConverter_i->EnableUnpackgeDebug();
            myConverter_i->SetdebugHistIndex(f_InputPars.ChidIdxdebugArray[converter_i],
                                             f_InputPars.OverThreshArray[converter_i],
                                             f_InputPars.AmpOrTimeArray[converter_i]);
            //config debug historgrams
            myConverter_i->ConfigDebugHist(converter_i,
                                           f_InputPars.HistdebugConfig[3*converter_i+0],
                                           f_InputPars.HistdebugConfig[3*converter_i+1],
                                           f_InputPars.HistdebugConfig[3*converter_i+2]);
        }
        else
            myConverter_i->DisableUnpackgeDebug();

        //Start to Unpackage for this input file
        //TODO, multi thread implementation???
        auto flag_i = myConverter_i->DoUnpackageRawdata2ROOT(numofchipsused_i);
        
        if(isdebugflag)
        {
            auto histdebug_i = myConverter_i->GetDebugHist();
            histdebug_i->SetTitle(Form("HistDebug_package%d",converter_i));
            mycDebug->cd(converter_i+1);
            gPad->SetGrid();
            histdebug_i->DrawCopy();
            PixTPCLog(PIXtpcDebug,
                      Form("Hist Package_%d, UnderFlow = %f , OverFlow = %f",converter_i,histdebug_i->GetBinContent(0),histdebug_i->GetBinContent(histdebug_i->GetNbinsX()+1)),false);
        }
        delete myConverter_i;
    }
    
    PixTPCLog(PIXtpcINFO,Form(":> %d input binary files convert to root files",numofIpaddress),false);

    //===================================================================
    //Merge all sub .root files from different IP(readout board)
    //===================================================================
    //Open all sub .root files
    TFile** Arrfile = new TFile*[numofIpaddress];
    TTree** Arrtree = new TTree*[numofIpaddress];
    PixelTPCdata** ArrPixTPCdata = new PixelTPCdata*[numofIpaddress];
    std::set<long> entry_set;
    for(int file_i=0; file_i<numofIpaddress; ++file_i)
    {
        Arrfile[file_i] = TFile::Open(f_InputPars.OutputfileArray[file_i].c_str());
        Arrtree[file_i] = dynamic_cast<TTree*>(Arrfile[file_i]->Get("PixTPCdata"));

        if(isdebugflag)
            std::printf("[cepcPixTPC INFO]: %lld entries in %d-th sub .root\n",Arrtree[file_i]->GetEntries(),file_i);

        entry_set.insert(Arrtree[file_i]->GetEntries());
        ArrPixTPCdata[file_i] = new PixelTPCdata(f_InputPars.NumberOfChipsUsed[file_i]);
        Arrtree[file_i]->SetBranchAddress("pixelTPCdata",&ArrPixTPCdata[file_i]);
    }
    //Check data length (entries) of each sub .root file
    if(entry_set.size()>1)
    {
        PixTPCLog(PIXtpcERROR,"Data length of sub .root file error",true);
        PixTPCLog(PIXtpcINFO,"Merge processing break!!!",false);
        
        //Free 
        for(int file_i=0; file_i<numofIpaddress; ++file_i)
        {
            delete  ArrPixTPCdata[file_i];
            delete  Arrtree[file_i];
            delete  Arrfile[file_i];
        }
        delete  []ArrPixTPCdata;
        delete  []Arrtree;
        delete  []Arrfile;
        PixTPCLog(PIXtpcINFO,"Delete all sub .root pointers!!!",false);
        return false;
    }
    else
    {
        PixTPCLog(PIXtpcINFO,"Start Merging all sub .root!!!",false);
        //Create merge output .root file
        auto mergefile = std::make_unique<TFile>(f_InputPars.OutputfileMerge.c_str(),"RECREATE");
        mergefile->cd();
        auto mergetree = new TTree("PixTPCdata","raw data (merged)");
        auto dataTableMerge = new PixelTPCdata(NumOfChipsToMerge);
        mergetree->Branch("pixelTPCdata",&dataTableMerge);
        //Loop all Entries
        //for(long long entry_i=0; entry_i<100; ++entry_i)
        for(long long entry_i=0; entry_i<Arrtree[0]->GetEntries(); ++entry_i)
        {
            //Get i-th Entry
            for(int file_i=0; file_i<numofIpaddress; ++file_i)
                Arrtree[file_i]->GetEntry(entry_i);
            //Print progress
            if(entry_i%2000==0)
                std::printf("[cepcPixTPC INFO]: ### %lld*3 entries Filled!\n",entry_i);
            //Check TimeStamp
            //if abs(Timestamp_i - Timestamp_k)>1, continue
            unsigned long Timestamp_i = ArrPixTPCdata[0]->GetTriggleID();
            unsigned long Timestamp_k = 0;
            for(int file_i=1; file_i<numofIpaddress; ++file_i)
            {
                Timestamp_k = ArrPixTPCdata[file_i] ->GetTriggleID();
                if(std::fabs(Timestamp_k-Timestamp_i)>1) // 1, max difference between each IP
                {
                    if(isdebugflag)
                        std::cout<<"[cepcPixTPC INFO]: "<<std::hex<<Timestamp_i<<","<<Timestamp_k<<std::endl;
                    continue;
                }
                else 
                    Timestamp_i = Timestamp_k;
            }

            //Fill merge PixTPCdata table
            for(int file_i=0; file_i<numofIpaddress; ++file_i)
            {
                for(int chip_index=0; chip_index < f_InputPars.NumberOfChipsUsed[file_i]; ++chip_index)
                {
                    for(int chn_index=0; chn_index < __NumChn__; ++chn_index)
                    {
                        auto size_chip_chn = (*ArrPixTPCdata[file_i])(chip_index,chn_index).size();
                        int globalChip_index = chip_index + 8*file_i;
                        for(size_t overthresh=0; overthresh < size_chip_chn; ++overthresh)
                        {
                            auto QTpair_tmp = (*ArrPixTPCdata[file_i])(chip_index,chn_index).at(overthresh);
                            (*dataTableMerge)(globalChip_index,chn_index).push_back(QTpair_tmp);
                        }
                    }
                }
            }
            dataTableMerge->SetTiggleID(Timestamp_i);
            //Fill mergetree
            mergetree->Fill();
            //Clear current dataTable
            dataTableMerge->ClearPixelTPCdata(NumOfChipsToMerge);
        }
        
        //Write file
        mergetree->Write();
        delete mergetree;
        //Close merged root file
        mergefile->Close();
        //Free
        for(int file_i=0; file_i<numofIpaddress; ++file_i)
        {
            delete  ArrPixTPCdata[file_i];
            delete  Arrtree[file_i];
            delete  Arrfile[file_i];
        }
        delete  []ArrPixTPCdata;
        delete  []Arrtree;
        delete  []Arrfile;
        PixTPCLog(PIXtpcINFO,"Delete all sub .root pointers!!!",false);
        PixTPCLog(PIXtpcINFO,"Merge processing successfully!!!",false);
        return true;
    }
}

bool RawdataConverter::DoUnpackageRawdata2ROOT(int numofChipUsed)
{
    // header, ref Jianmeng Dong and Canwen Liu 
    unsigned char header_r[] = {0x55,0xaa};
    // finder header position 
    auto vheaderPos = find_header(f_file,header_r,2);
    //buffer vector
    vector<unsigned char> vBuffer;

#if 1
    //Create TFile/TTree 
    auto outfile = std::make_unique<TFile>(fRootName.c_str(),"RECREATE");
    outfile->cd();
    auto tr_out = new TTree("PixTPCdata","raw tpc channel data");
    fPixtpcdata = new PixelTPCdata(numofChipUsed);//  TEPIX chips test ret
    tr_out->Branch("pixelTPCdata",&fPixtpcdata);
#endif

    unsigned long preTimestamp=0;
    unsigned long posTimestamp=0;
    int globaltriggleId =0;
    bool ChippackageLengthWarning = false;

    //loop all packages 
    for(size_t ii=1; ii<vheaderPos.size(); ++ii)
    {
        if(ii%5000==0)
            std::printf("[cepcPixTPC INFO]: ### %zu packages Done!\n",ii);

        auto bufferlength = vheaderPos.at(ii)-vheaderPos.at(ii-1);
        
        vBuffer.resize(bufferlength);
        f_file->read(reinterpret_cast<char*>(vBuffer.data()),bufferlength);

#if 1
        if(bufferlength < __MindataLength__ || !check_tail(vBuffer))
        {
            ChippackageLengthWarning = true;
            //printf("[AAAAAAAAA]: bufferlength=%lld, Global Index=%d",bufferlength,globaltriggleId);
            //if(bufferlength>4)
            //    std::cout<<" "<<static_cast<int>(vBuffer.at(2))<<"\t"<<static_cast<int>(vBuffer.at(3))<<"\t"<<static_cast<int>(vBuffer.at(2))*4+static_cast<int>(vBuffer.at(3));
            //printf("\n");
            continue;
        }

        if(!check_chipindex(vBuffer))
            continue;

        int chipNumber = static_cast<int>(vBuffer.at(2))*4+static_cast<int>(vBuffer.at(3));

        if(fIsdebug && ii<40)
        {
            //printHeaderTail(vBuffer);    
            //printChipNumber(vBuffer);
            //printTimeStamp(vBuffer);
            //printTriggerNum(vBuffer);
        }

        //init post timestamp 
        //std::memcpy(&posTimestamp,&vBuffer.at(4),sizeof(long));// little-endian 
        posTimestamp = this->GetTimeStampinBigendian(vBuffer);
        if(ii==1)
            preTimestamp = posTimestamp;

        if(fIsdebug && ii<10)
            std::cout<<std::hex<<"[cepcPixTPC DEBUG]: pre: "<<preTimestamp<<" pos:"<<posTimestamp<<"\t"<<chipNumber<<std::endl;

        if(preTimestamp == posTimestamp)
        {
            this->FillPixelTPCdataTable(vBuffer,chipNumber);
            //update pre time stamp
            preTimestamp = posTimestamp;
        }
        else
        {
            //set globaltriggleId and Fill tree
            //fPixtpcdata->SetTiggleID(globaltriggleId);
            fPixtpcdata->SetTiggleID(preTimestamp);
            //if(fIsdebug && ii<30)
            //    std::cout<<std::hex<<":> Trigger ID:"<<fPixtpcdata->GetTriggleID()<<std::endl;
            
            tr_out->Fill();
            globaltriggleId++;
            //resize fPixtpcdata
            fPixtpcdata->ClearPixelTPCdata(numofChipUsed);
            
            //fill this step data
            this->FillPixelTPCdataTable(vBuffer,chipNumber);
            //update pre time stamp
            preTimestamp = posTimestamp;
        }
#endif
    }//end of data

    //close file
    f_file->close();

    if(ChippackageLengthWarning)
        PixTPCLog(PIXtpcWARNING,"Packet loss, no data of some chips in one entry",false);
#if 1
    tr_out->Write();
    delete tr_out;
    outfile->Close();
#endif

    PixTPCLog(PIXtpcINFO,Form("Raw binary data convert to ROOT data done!"),false);

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


void RawdataConverter::FillPixelTPCdataTable(const vector<unsigned char> vbuffer, int chipnumber)
{
    // Convert bufferlength unsigned char to BinSequeue size = (bufferlength-16)*8 
    vector<bool> binarySeq;
    int kk=0;

    for(const auto &byte : vbuffer) 
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

    bool evt_flag = true;
    int evt_num = 4;
    bitset<2> evtBits;
    bitset<14> timeBits,ampBits;
    size_t INDEX = 0;

    // Read T/Q info from 128 channel of a chip
    for(int chn_id=0; chn_id < __NumChn__; ++chn_id)
    {
        if(INDEX >= binarySeq.size()-2)
        {
            PixTPCLog(PIXtpcERROR,"------Package Error! out range of binarySeq size",true);
            break;
        }
        else
        {
            (*fPixtpcdata)(chipnumber,chn_id).resize(4);
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

                if(fIsdebug && i_evt==fOverThdebug && chipnumber==fChipdebug)
                {
                    if(fAmpOrTime)
                        fHistdebug->Fill(double(ampBits.to_ulong()));
                    else
                        fHistdebug->Fill(double(timeBits.to_ulong()));
                    //std::cout<<std::dec<<"## "<<"chip "<<chipnumber<<" chn "<<chn_id<<","
                    //         <<ampBits.to_ulong()<<","<<timeBits.to_ulong()<<std::endl;
                }
                //TODO, need update to 24
                //4 for testing
                //20241111: Ref Jianmeng Dong, Max. chip is 8 for each readout board  
                if(chipnumber<8)
                {
                    //(*fPixtpcdata)(chipnumber,chn_id).push_back(std::make_pair(timeBits.to_ulong(),
                    //                                                           ampBits.to_ulong()));
                    auto QT_pair_thisOverThresh = std::make_pair(timeBits.to_ulong(),ampBits.to_ulong());
                    (*fPixtpcdata)(chipnumber,chn_id).at(i_evt) = QT_pair_thisOverThresh;
                }

            }//end of channel evts 
            
            //update INDEX
            INDEX = INDEX+3+28*evt_num;
        }
    }//end of this package / end of 128 channels   

    binarySeq.clear();
}

unsigned long RawdataConverter::GetTimeStampinBigendian(const vector<unsigned char> vbuffer)
{
    unsigned long timestamp_bigendian=0;
    
    if(vbuffer.size()<11)
        throw std::out_of_range("Input package buffer length too short! GetTimeStampinBigendian()");
    
    for(int m=0; m < 8; ++m)
    {
        timestamp_bigendian |= static_cast<unsigned long>(vbuffer[4+m]) << (56-8*m);
    }

    return  timestamp_bigendian;
}

