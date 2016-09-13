#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TFile.h"

#include <fstream>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <iostream>
#include <cstring>
#include <stdint.h>

void parseWaveForm(const char *inFile, const char *outFile, const int nPMTs) {
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
    TH1F * waveForm = new TH1F(Form("waveForm_tube%d_signal%d", pmt, signalCount), "R5912 Waveform", nSamples+1, signalStart, signalEnd);
    std::stringstream ss_waveForm(line);
    float counter = 0;
    while (ss_waveForm.good() ) {
      float sampleTime = signalStart + counter * sampleWidth;
      ss_waveForm >> adcCounts;
      waveForm->Fill(sampleTime, adcCounts);
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
    if (signalCount>100000) {
      break;
    }
  } // end while getline
  printf("Finished looping over waveforms\n");
  ins.close();
  printf("Scaling average forms\n");
  printf("Saving\n");
  outF->Write();
  outF->Close();

}
