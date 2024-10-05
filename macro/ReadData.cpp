/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-10-05 14:51
 * Filename         : ReadData.cpp
 * Description      : 
 * Update           : 
 * ******************************************************************/

#include <iostream>
#include <numeric>

#include "TFile.h"
#include "TTree.h"
#include "Lansxlogon.h"

#include "MCTrackdata.h"

using namespace std;

void ReadData()
{
    LansxFormat::myStyle();
    auto file = TFile::Open("../task/Recodata_electron_5GeV_X0.21cmZ20cm_pixelQcenter.root");
    //auto file = TFile::Open("../task/Recodata_electron_5GeV_X0.21cmZ20cm_eqpad.root");
    auto t1 = dynamic_cast<TTree*>(file->Get("PixTPCdata"));
    
    auto recodata = new MCTrackdata;
    t1->SetBranchAddress("recoHitsdata",&recodata);

    auto hdEdx = new TH1D("hdEdx",";dEdx [a.u.];Evts",500,2000,17000);

    for(int ii=0; ii<t1->GetEntries();++ii)
    {
        t1->GetEntry(ii);

        auto Vecrecohits = recodata->GetClusterVec();

        auto sumdEdx = std::accumulate(Vecrecohits.begin(),Vecrecohits.end(),0., [](double acc, const FourVector& hit){
                                       return acc+hit.e();
                                       });
        hdEdx->Fill(sumdEdx/Vecrecohits.size());
    }

    hdEdx->Draw();
}
