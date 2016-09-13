#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TFile.h"
#include "TGaxis.h"

#include <fstream>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <iostream>
#include <cstring>
#include <stdint.h>

void parseWaveForm_formAvg(const char *inFile, const char *outFile, const int nPMTs) {
  float nSamples = 0;
  float sampleRate = 0;
  float triggerSample = 0;

  float sampleWidth = 0;
  float signalStart = 0;
  float signalEnd = 0;

  std::string line;

  ifstream ins(inFile);
  if (!ins.good() ) {
    std::cerr << "Error: " << inFile << " could not be opened!" << endl;
  }
  getline(ins, line);
  std::stringstream ss(line);
  ss >> nSamples >> sampleRate >> triggerSample;
  sampleWidth = 1.0 / sampleRate;
  signalStart = -1.0 * sampleWidth * triggerSample;
  signalEnd = sampleWidth * (nSamples-triggerSample);

  TFile *outF = new TFile(outFile, "RECREATE");
  TH1F *avgForm0 = new TH1F("avgForm0", "Average Waveform1", nSamples+1, signalStart, signalEnd);
  TH1F *avgForm1 = new TH1F("avgForm1", "Average Waveform2", nSamples+1, signalStart, signalEnd);
  float adcCounts = 0;
  int signalCount = 1;
  int pmt = 1;
  while (getline(ins, line) ) {
    if (ins.eof() ) {
      break;
    }
    if (signalCount%10000 == 0 && pmt==1) {
      printf("Signal %d\n", signalCount);
    }
    std::stringstream ss_waveForm(line);
    float counter = 0;
    while (ss_waveForm.good() ) {
      float sampleTime = signalStart + counter * sampleWidth;
      ss_waveForm >> adcCounts;
      if (pmt==1) {
        avgForm0->Fill(sampleTime, adcCounts);
      } else if (pmt==2) {
        avgForm1->Fill(sampleTime, adcCounts);
      }
      ++counter;
    } // end while ss_waveForm
    //outF->Write();
    if (pmt==nPMTs) {
      ++signalCount;
      pmt = 1;
    } else {
      ++pmt;
    }
    if (signalCount % 20 == 0 && pmt == 1) {
      outF->Flush(); // clear what's sitting in memory
    }
    //if (signalCount>100) {
    //  break;
    //}
  } // end while getline
  printf("Finished looping over waveforms\n");
  ins.close();
  printf("Scaling average forms\n");
  avgForm0->Scale(1./signalCount);
  avgForm1->Scale(1./signalCount);
  printf("Saving\n");

  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);
  avgForm0->SetLineColor(kBlack);
  avgForm1->SetLineColor(kRed);
  TCanvas *c1 = new TCanvas("c1", "c1");
  c1->cd();
  THStack *hs = new THStack("hs", "Compare Average BKG Waveforms;Time to Trigger (#mus);ADCs");
  hs->Add(avgForm0);
  hs->Add(avgForm1);
  hs->Draw("NOSTACK HIST");
  hs->GetHistogram()->GetXaxis()->SetRangeUser(-0.5, 2.45);
  hs->SetMinimum(3000);

  TLegend *leg = new TLegend(0.5, 0.25, 0.8, 0.4);
  leg->AddEntry(avgForm0, "Channel 0", "l");
  leg->AddEntry(avgForm1, "Channel 1", "l");
  leg->Draw("SAME");

  c1->Print("cenns_09082016_LED_10V20nsPulse_avgWaveForms.pdf");

  outF->Write();
  outF->Close();

}
