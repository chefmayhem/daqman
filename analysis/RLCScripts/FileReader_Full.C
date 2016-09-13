#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include "EventData.hh"
#include "ChannelData.hh"

const static int NumChannels = 5;

//void Analyze(const char *name, const int printSkip = 0)
void FileReader_Full(const char *name, const int printSkip = 0)
{
  //char *name = "DetA_1250V_BG_04092014";

  int numchans = 0, ichan = 0;

  TFile *tf = new TFile(Form("data/root/%s.root",name), "READ");
  TTree *tree = (TTree *)tf->Get("Events");

  EventData *evt = new EventData();
  vector<ChannelData> chans;
  vector<Roi> regs;
  vector<double> wave;

  tree->SetBranchAddress("event", &evt);

  TFile *tfout = new TFile(Form("data/root/%s_v5.root", name), "RECREATE");
  TTree *outtree = new TTree("DataTree", "Tree of Region Integrals");

  // tree variables
  Float_t IntPeak[NumChannels], IntTail[NumChannels], IntFull[NumChannels];
  Int_t Saturated[NumChannels];

  outtree->Branch("Saturated", Saturated, "Saturated[5]/I");
  outtree->Branch(  "IntPeak",   IntPeak,   "IntPeak[5]/F");
  outtree->Branch(  "IntTail",   IntTail,   "IntTail[5]/F");
  outtree->Branch(  "IntFull",   IntFull,   "IntFull[5]/F");

  TH1F *ADCfull[NumChannels];
  TH1F *ADCpeak[NumChannels];
  TH1F *ADCtail[NumChannels];
  TH1F *ADC1PE[NumChannels];

  TH1F *ADCsat[NumChannels];
  TH1F *ADCall[NumChannels];

  TH2F *PSD[NumChannels];

  for (int i=0; i<NumChannels; ++i) {
    ADCfull[i] = new TH1F(Form("ADCfull%i",i), Form("Full Integral Spectrum (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADCpeak[i] = new TH1F(Form("ADCpeak%i",i), Form("Peak Integral Spectrum (< 90 ns) (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADCtail[i] = new TH1F(Form("ADCtail%i",i), Form("Tail Integral Spectrum (> 90 ns) (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADC1PE[i] =  new TH1F(Form("ADC1PE%i",i),  Form("Full Integral Spectrum (near 0) (Ch. %i);Integrated Pulse;Counts",i), 300, -10., 90.);

    ADCsat[i] =  new TH1F(Form("ADCsat%i",i),  Form("Saturated Integral Spectrum (Ch. %i);Integrated Pulse;Counts",i), 2000, 0., 50000.);
    ADCall[i] =  new TH1F(Form("ADCall%i",i),  Form("Integral Spectrum (All) (Ch. %i);Integrated Pulse;Counts",i), 6000, 0., 150000.);

    PSD[i] =     new TH2F(Form("PSD%i",i),     Form("F90 Pulse Shape Discrimination (Ch. %i);Integrated Pulse;F90",i), 500, 0., 25000, 550, 0., 1.1);
  }
  Int_t numEvents = tree->GetEntries();

  cout << "Total number of events: " << numEvents << endl;

  for (int i=0; i<numEvents; ++i) {
    // if (i > 100000) break;
    if (printSkip && i%printSkip == 0) {
      cout << "At event: " << i << endl;
    }

    // reset arrays to null values
    for (int j=0; j<NumChannels; ++j) {
      Saturated[j] = -1;
      IntPeak[j] = 0.; IntTail[j] = 0.; IntFull[j] = 0.;
    }

    tree->GetEntry(i);
    
    const int numchans = static_cast<int>(evt->channels.size());
    if (printSkip && i%printSkip == 0) {
      cout << "\tChannels size: " << numchans << endl;
    }    
    chans = evt->channels;

    for (int j=0; j<numchans; ++j) {

      ichan = chans[j].channel_num + 1; // sum channel = -1

      regs = chans[j].regions;

      Saturated[ichan] = (Int_t)chans[j].saturated;
      IntFull[ichan] = -regs[0].integral;
      IntPeak[ichan] = -regs[1].integral;
      IntTail[ichan] = -regs[2].integral;

      // everything (un)saturated
      ADCall[ichan]->Fill( -regs[0].integral );

      // only unsaturated events
      if (chans[j].saturated == 0) {
        ADCfull[ichan]->Fill( -regs[0].integral );
        ADCpeak[ichan]->Fill( -regs[1].integral );
        ADCtail[ichan]->Fill( -regs[2].integral );
        ADC1PE[ichan]->Fill( -regs[0].integral );
        
        PSD[ichan]->Fill( -regs[0].integral, (regs[1].integral / regs[0].integral) );
      } else { // only saturated events
        ADCsat[ichan]->Fill( -regs[0].integral );
      }
    }

    outtree->Fill();

  }
  outtree->Write();

  for (int i=0; i<NumChannels; i++) {
    ADCfull[i]->Write();
    ADCpeak[i]->Write();
    ADCtail[i]->Write();
    ADC1PE[i]->Write();
    ADCsat[i]->Write();
    ADCall[i]->Write();

    PSD[i]->Write();
  }

  delete outtree;

  for (int i=0; i<NumChannels; i++) {
    delete ADCfull[i];
    delete ADCall[i];
    delete ADCpeak[i];
    delete ADCtail[i];
    delete ADC1PE[i];
    delete ADCsat[i];
    delete PSD[i];
  }

  delete tfout;

  delete evt;
  delete tree;
  delete tf;
}
