// this guy needs compiled
// .L Reader_v1.C++
//
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include "EventData.hh"
#include "ChannelData.hh"
#include "Pulse.hh"

const static int NumChannels = 2;

//void Analyze(const char *name, const int printSkip = 0)
void Reader_MRH(const char *name, const char * outFile, const int printSkip = 10000)
{
  //char *name = "DetA_1250V_BG_04092014";

  int ichan = 0;

  TFile *tf = new TFile(name, "READ");
  TTree *tree = (TTree *)tf->Get("Events");

  EventData *evt = new EventData();
  vector<ChannelData> chans;
  vector<Roi> regs;
  vector<double> regInt;
  vector<double> wave;
  vector<Pulse> pulses;

  tree->SetBranchAddress("event", &evt);

  // Let's make a simple container; each tree entry is a single pulse
  // Could make it per trigger and use variable length arrays... 
  // Maybe useful later

  TFile *tfout = new TFile(outFile, "RECREATE");
  TTree *outtree = new TTree("DataTree", "Tree of Pulse Information");

  // tree variable 
  Int_t iTrigger, TotPulses, pChannel, pSaturated;
  Float_t pTime, pIntegral, pF90;
  Int_t PulsesPerChannel[2];

  outtree->Branch("iTrigger", &iTrigger, "iTrigger/I");
  outtree->Branch("TotPulses", &TotPulses, "TotPulses/I");
  outtree->Branch("pChannel", &pChannel, "pChannel/I");
  outtree->Branch("PulsesPerChannel", PulsesPerChannel, "PulsesPerChannel[2]/I");
  outtree->Branch("pSaturated", &pSaturated, "pSaturated/I");
  outtree->Branch("pTime", &pTime, "pTime/F");
  outtree->Branch("pIntegral", &pIntegral, "pIntegral/F");
  outtree->Branch("pF90", &pF90, "pF90/F");

  Int_t numEvents = tree->GetEntries();

  cout << "Total number of events: " << numEvents << endl;

  for (int i=0; i<numEvents; ++i) {

    if (printSkip && i%printSkip == 0) cout << "At event: " << i << endl;

    // reset arrays to initial values for error checking
    for (int j=0; j<2; ++j) { PulsesPerChannel[j] = 0; }
    iTrigger = i; TotPulses = 0; pChannel = -9; pSaturated = -1;
    pTime = -999.;  pIntegral = -99999.; pF90 = -99.;

    tree->GetEntry(i);

    // get channels vector
    chans = evt->channels;
    const int numchans = static_cast<int>(evt->channels.size());
    if (printSkip && i%printSkip == 0) { cout << "\tChannels size: " << numchans << endl; }    
    
    // loop over active channels to count pulses
    for (int j=0; j<numchans; ++j) {

      ichan = chans[j].channel_num; // sum channel = -1
      if (ichan < 0) continue; // don't want sum channel

      pulses = chans[j].pulses;
      const int numpulses = static_cast<int>(pulses.size());

      PulsesPerChannel[ichan] = numpulses;
      TotPulses += numpulses;
    }
    
    // reloop over channels to get individual pulses, could be a better way
    for (int j=0; j<numchans; ++j) {

      ichan = chans[j].channel_num; // sum channel = -1
      if (ichan < 0) continue; // don't want sum channel

      pulses = chans[j].pulses;
      const int numpulses = static_cast<int>(pulses.size());
      pChannel = ichan;

      if (printSkip && i%printSkip == 0) { cout << "\tNumber Pulses: " << numpulses << endl; }
      
      for(int k=0; k<numpulses; ++k) {
	pF90 = pulses[k].f90;
	pIntegral = -pulses[k].integral;
	pTime = pulses[k].start_time;
	pSaturated = pulses[k].peak_saturated;

	if (printSkip && i%printSkip == 0) { 
	  cout << "\t\tpulse: " << k;
	  cout << " chan: " << pChannel;
	  cout << " f90: " << pF90;
	  cout << " integral: " << pIntegral;
	  cout << " start_time: " << pTime;
	  cout << " peak_sat: " << pSaturated;
	  cout << endl;
	} // end print

	outtree->Fill(); // fill tree pulse by pulse

      } // loop over pulses
      
    } // loop over channels

    if (TotPulses == 0) {
      pChannel = -9; // want to be sure we know there was no pulses
      outtree->Fill();
    }

  } // loop over events

  outtree->Write();

  delete outtree;
  delete tfout;

  delete evt;
  delete tree;
  delete tf;
}
