# demo python script to read reco hits data
import ROOT

#set style 
ROOT.gROOT.SetStyle("BELLE2")

file = ROOT.TFile.Open("../task/Recodata_electron_5GeV_X0.21cmZ20cm_pixelQcenter.root")
t1 = file.Get("PixTPCdata")

hdEdx =ROOT.TH1D("hdEdx",";dEdx [a.u.];Evts",500,2000,17000);

recohits = ROOT.MCTrackdata()

t1.SetBranchAddress("recoHitsdata",recohits)

for entry in range(t1.GetEntries()):
    t1.GetEntry(entry)
    sumdEdx = 0.
    v_recohits = recohits.GetClusterVec()
    for hits in v_recohits: 
        sumdEdx = sumdEdx + hits.e()

    avedEdx = sumdEdx/v_recohits.size()
    hdEdx.Fill(avedEdx)

myc = ROOT.TCanvas("myc","myc",800,600)
myc.SetGrid()
hdEdx.SetLineColor(ROOT.kBlue)
hdEdx.Draw()
myc.Print("./myc.png")
