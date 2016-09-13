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
#include "TF1.h"

#include <fstream>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <iostream>
#include <cstring>
#include <stdint.h>

// parameters
// 0 = baseline
// 1 = A
// 2 = t0
// 3 = tau1
// 4 = omega
// 5 = C
// 6 = t2
// 7 = tau2
double myFitFun(double * x, double * par) {
  if (x[0] < par[2]) {
    return par[0];
  }

  double one = std::exp((x[0] - par[2])/par[3]);
  double two = std::sin(par[4]*(x[0] - par[2]));
  double three = std::exp((x[0] - par[6])/par[7]);
  double four = std::exp(-TMath::Power(x[0]-par[8],2)/par[9]);

  return par[0] - par[1]*one*two + par[5]*three - par[10]*four;
}

void parseWaveForm_formAvg_Fit(const char *inFile) {
  TFile *inF = new TFile(inFile, "READ");
  TH1F * avgForm0 = (TH1F*) inF->Get("avgForm0");
  TH1F * avgForm1 = (TH1F*) inF->Get("avgForm1");

  double baselineAvg = 0.0;
  int bin = 0;
  while (avgForm0->GetXaxis()->GetBinCenter(bin)<-0.1) {
    baselineAvg+=avgForm0->GetBinContent(bin);
    ++bin;
  }
  
  baselineAvg = baselineAvg/static_cast<double>(bin-1);

  TF1 * chanFit0 = new TF1("chanFit0", myFitFun, -0.05, 2.5,11);
  chanFit0->FixParameter(0, baselineAvg);
  chanFit0->SetParameter(1,  1.00);
  chanFit0->SetParameter(2,    0.05);
  chanFit0->SetParameter(3,   1.00);
  chanFit0->SetParameter(4,    1.00);
  chanFit0->SetParameter(5,   1.00);
  chanFit0->SetParameter(6,    0.05);
  chanFit0->SetParameter(7,    1.60);
  chanFit0->SetParameter(8,    0.05);
  chanFit0->SetParameter(9,    1.60);
  chanFit0->SetParameter(10,   100.60);

/*  TF1 * chanFit1 = new TF1("chanFit1", myFitFun, -0.5, 2.5,8);
  chanFit1->SetParameter(0, 4000.00);
  chanFit1->SetParameter(1,  350.00);
  chanFit1->SetParameter(2,    1.00);
  chanFit1->SetParameter(3,   10.00);
  chanFit1->SetParameter(4,    2.00);
  chanFit1->SetParameter(5,   10.00);
  chanFit1->SetParameter(6,    1.60);
  chanFit1->SetParameter(7,    1.60);
*/
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

  c1->Print("cenns_09062016_avgWaveForms_Fit.pdf(");

  avgForm0->Fit(chanFit0);
  c1->Clear();
  avgForm0->Draw();
  avgForm0->GetYaxis()->SetRangeUser(3500, 4100);
  chanFit0->SetLineColor(kBlack);
  chanFit0->Draw("SAME");

  c1->Print("cenns_09062016_avgWaveForms_Fit.pdf)");

}

