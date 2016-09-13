#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"

#include "EventData.hh"
#include "ChannelData.hh"

const static int NumChannels = 3;

//void Analyze(const char *name, const int printSkip = 0)
void FileReader_Full_MRH(const char *fName, const char * outFile, const int printSkip = 50000)
{
  //char *name = "DetA_1250V_BG_04092014";

  int numchans = 0, ichan = 0;

  TFile *tf = new TFile(fName, "READ");
  TTree *tree = (TTree *)tf->Get("Events");

  EventData *evt = new EventData();
  vector<ChannelData> chans;
  vector<Roi> regs;
  vector<double> wave;

  tree->SetBranchAddress("event", &evt);

  TFile *tfout = new TFile(outFile, "RECREATE");
  TTree *outtree = new TTree("DataTree", "Tree of Region Integrals");

  // tree variables
  Float_t IntPeak[NumChannels], IntTail[NumChannels], IntFull[NumChannels];
  Int_t Saturated[NumChannels];

  outtree->Branch("Saturated", Saturated, "Saturated[3]/I");
  outtree->Branch(  "IntPeak",   IntPeak,   "IntPeak[3]/F");
  outtree->Branch(  "IntTail",   IntTail,   "IntTail[3]/F");
  outtree->Branch(  "IntFull",   IntFull,   "IntFull[3]/F");

  TH1F *ADCfull[NumChannels];
  TH1F *ADCpeak[NumChannels];
  TH1F *ADCtail[NumChannels];
  TH1F *ADC1PE[NumChannels];

  TH1F *ADCsat[NumChannels];
  TH1F *ADCall[NumChannels];

  TH2F *PSD[NumChannels];
  TH2F *PSD_unSat[NumChannels];
  TH2F *PSD_Sat[NumChannels];

  TH2F *zoom_PSD[NumChannels];
  TH2F *zoom_PSD_unSat[NumChannels];
  TH2F *zoom_PSD_Sat[NumChannels];

  for (int i=0; i<NumChannels; ++i) {
    ADCfull[i] = new TH1F(Form("ADCfull%i",i), Form("Full Integral Spectrum (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADCpeak[i] = new TH1F(Form("ADCpeak%i",i), Form("Peak Integral Spectrum (< 90 ns) (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADCtail[i] = new TH1F(Form("ADCtail%i",i), Form("Tail Integral Spectrum (> 400 ns) (Ch. %i);Integrated Pulse;Counts",i), 1000, 0., 25000.);
    ADC1PE[i] =  new TH1F(Form("ADC1PE%i",i),  Form("Full Integral Spectrum (near 0) (Ch. %i);Integrated Pulse;Counts",i), 300, -10., 90.);

    ADCsat[i] =  new TH1F(Form("ADCsat%i",i),  Form("Saturated Integral Spectrum (Ch. %i);Integrated Pulse;Counts",i), 2000, 0., 50000.);
    ADCall[i] =  new TH1F(Form("ADCall%i",i),  Form("Integral Spectrum (All) (Ch. %i);Integrated Pulse;Counts",i), 6000, 0., 150000.);

    PSD[i] =      new TH2F(Form("PSD%i",i),            Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) All Events;Integrated Pulse;Pseudo F90",i),  300, 0., 200000, 200, -0.2, 1.5);
    PSD_unSat[i] =     new TH2F(Form("PSD_unSat%i",i), Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) UnSaturated;Integrated Pulse;Pseudo F90",i), 300, 0., 200000, 200, -0.2, 1.5);
    PSD_Sat[i] =     new TH2F(Form("PSD_Sat%i",i),     Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) Saturated;Integrated Pulse;Pseudo F90",i),   300, 0., 200000, 200, -0.2, 1.5);

    zoom_PSD[i] =     new TH2F(Form("zoom_PSD%i",i),             Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) All Events;Integrated Pulse;Pseudo F90",i),  150, 0., 20000, 100, -0.2, 1.5);
    zoom_PSD_unSat[i] =     new TH2F(Form("zoom_PSD_unSat%i",i), Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) UnSaturated;Integrated Pulse;Pseudo F90",i), 150, 0., 20000, 100, -0.2, 1.5);
    zoom_PSD_Sat[i] =     new TH2F(Form("zoom_PSD_Sat%i",i),     Form("Pseudo F90 Pulse Shape Discrimination (Ch. %i) Saturated;Integrated Pulse;Pseudo F90",i),   150, 0., 20000, 100, -0.2, 1.5);
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
      if (printSkip && i%printSkip == 0) {
        cout << "\tChannel: " << j << endl;
      }    

      ichan = chans[j].channel_num + 1; // sum channel = -1

      regs = chans[j].regions;

      Saturated[ichan] = (Int_t)chans[j].saturated;
      IntFull[ichan] = -regs[2].integral;
      IntPeak[ichan] = -regs[0].integral;
      IntTail[ichan] = -regs[1].integral;

      // everything (un)saturated
      ADCall[ichan]->Fill( -regs[2].integral );

      // only unsaturated events
      if (chans[j].saturated == 0) {
        ADCfull[ichan]->Fill( -regs[2].integral );
        ADCpeak[ichan]->Fill( -regs[0].integral );
        ADCtail[ichan]->Fill( -regs[1].integral );
        ADC1PE[ichan]->Fill( -regs[0].integral );

        PSD_unSat[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
        zoom_PSD_unSat[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
      } else { // only saturated events
        ADCsat[ichan]->Fill( -regs[2].integral );
        PSD_Sat[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
        zoom_PSD_Sat[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
      }
      PSD[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
      zoom_PSD[ichan]->Fill( -regs[2].integral, regs[0].integral / (regs[1].integral + regs[0].integral) );
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
    PSD_unSat[i]->Write();
    PSD_Sat[i]->Write();

    zoom_PSD[i]->Write();
    zoom_PSD_unSat[i]->Write();
    zoom_PSD_Sat[i]->Write();
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
    delete PSD_unSat[i];
    delete PSD_Sat[i];

    delete zoom_PSD[i];
    delete zoom_PSD_unSat[i];
    delete zoom_PSD_Sat[i];
  }

  delete tfout;

  delete evt;
  delete tree;
  delete tf;
}
