#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TMath.h"
#include "TColor.h"
#include "TAttMarker.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

void plotHistos(const char * fileList, const int chan, const char * outFile) {
  char pdfF[100];
  char pngF[100];
  char cF[100];

  // Get some things ready for plotting
  TH1::SetDefaultSumw2(kTRUE);//make sure errors are calculated correctly
  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0);

  TCanvas * c1 = new TCanvas("c1","");
  TH2F * psd_UnSat= 0;
  TH2F * psd_Sat = 0;
  TH2F * psd_All = 0;
  TH2F * zoom_psd_UnSat= 0;
  TH2F * zoom_psd_Sat = 0;
  TH2F * zoom_psd_All = 0;

  THStack * psdCompHS = new THStack("psdCompHS", "Pseudo F90;Integrated ADCs;Peak / (Peak + Tail)");
  THStack * zoom_psdCompHS = new THStack("psdCompHS", "Pseudo F90;Integrated ADCs;Peak / (Peak + Tail)");

  TPaveText *pt = new TPaveText(0.63, 0.70, 0.95, 0.85, "brNDC"); 
  pt->AddText(Form("Channel %i", chan));
  pt->AddText(Form("Peak Integral: %2.2f < t < %1.2f #mus", -0.05, 0.09));
  pt->AddText(Form("Tail Integral: %2.2f < t < %1.2f #mus", 0.40, 2.50));
  pt->AddText(Form("Total Integral: %2.2f < t < %1.2f #mus", -0.05, 2.50));
  pt->SetTextColor(kBlack);
  pt->SetFillColor(gStyle->GetCanvasColor());
  pt->SetBorderSize(1);

  TLegend *satLeg = new TLegend(0.6, 0.2, 0.8, 0.3);
  satLeg->AddEntry( (TObject*) 0, "Saturated", "");
  satLeg->SetTextColor(kRed);
  satLeg->SetLineColor(gStyle->GetCanvasColor());
  satLeg->SetFillColor(gStyle->GetCanvasColor());
  satLeg->SetShadowColor(gStyle->GetCanvasColor());

  TLegend *unsatLeg = new TLegend(0.25, 0.65, 0.50, 0.75);
  unsatLeg->AddEntry( (TObject*) 0, "Unsaturated", "");
  unsatLeg->SetTextColor(kBlack);
  unsatLeg->SetLineColor(gStyle->GetCanvasColor());
  unsatLeg->SetFillColor(gStyle->GetCanvasColor());
  unsatLeg->SetShadowColor(gStyle->GetCanvasColor());

  TLegend *compLeg = new TLegend(0.42, 0.79, 0.62, 0.89);
  compLeg->SetLineColor(gStyle->GetCanvasColor());
  compLeg->SetFillColor(gStyle->GetCanvasColor());
  compLeg->SetShadowColor(gStyle->GetCanvasColor());

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Loop Over File List >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  // Loop over the list of fileLists
  printf("\nLooking at the file list. \n");
  std::string name = "";
  std::string date = "";
  std::string type = "";
  std::vector<std::string> list;
  std::vector<std::string> dateList;
  std::ifstream ins(fileList);
  if(!ins.good() ) {
    std::cerr << "Error: file list " << fileList << " could not be opened!" << endl;
  }
  ins >> name >> date;
  while(!ins.eof())
  {
    list.push_back(name);
    dateList.push_back(date);
    ins >> name >> date;
  }//end while !ins.eof
  ins.close();

  int numFiles = list.size();
  int fileCount = 0;

  for (; fileCount<numFiles; ++fileCount) {
    const char *inFS = list[fileCount].c_str();
    printf(" File %i of %i \n  File: %s. \n", fileCount+1, numFiles, inFS);
    TFile * inFR = TFile::Open(inFS, "READ");
    inFR->GetObject(Form("PSD_unSat%i", chan+1), psd_UnSat);
    inFR->GetObject(Form("PSD_Sat%i", chan+1), psd_Sat);
    inFR->GetObject(Form("PSD%i", chan+1), psd_All);
    inFR->GetObject(Form("zoom_PSD_unSat%i", chan+1), zoom_psd_UnSat);
    inFR->GetObject(Form("zoom_PSD_Sat%i", chan+1), zoom_psd_Sat);
    inFR->GetObject(Form("zoom_PSD%i", chan+1), zoom_psd_All);

    psd_UnSat->Scale(psd_UnSat->Integral() / psd_All->Integral() );
    psd_Sat->Scale(psd_Sat->Integral() / psd_All->Integral() );
    zoom_psd_UnSat->Scale(psd_UnSat->Integral() / psd_All->Integral() );
    zoom_psd_Sat->Scale(psd_Sat->Integral() / psd_All->Integral() );

    THStack * psdStack = new THStack("psdStack", Form("Pseudo F90 (%s);Total Integrated ADCs;Peak / (Peak + Tail)", dateList[fileCount].c_str() ) );
    psdStack->Add(psd_UnSat);
    psdStack->Add(psd_Sat);
    psd_UnSat->SetMarkerColor(kBlack);
    psd_Sat->SetMarkerColor(kRed);

    THStack * psdZoomStack = new THStack("psdZoomStack", Form("Pseudo F90 (%s);Total Integrated ADCs;Peak / (Peak + Tail)", dateList[fileCount].c_str() ) );
    psdZoomStack->Add(zoom_psd_UnSat);
    psdZoomStack->Add(zoom_psd_Sat);
    zoom_psd_UnSat->SetMarkerColor(kBlack);
    zoom_psd_Sat->SetMarkerColor(kRed);

    psdStack->Draw("NOSTACK");
    pt->Draw("SAME");
    satLeg->Draw("SAME");
    unsatLeg->Draw("SAME");

    // Saving Histos here
    strcpy(pdfF, outFile);
    strcpy(pngF, outFile);
    strcpy(cF, outFile);

    strcat(pngF, Form("channel%i_%s_psd.png", chan, dateList[fileCount].c_str()) );
    strcat(  cF,   Form("channel%i_%s_psd.C", chan, dateList[fileCount].c_str()) );

    if (numFiles == 1) {
      strcat(pdfF,    Form("channel%i_psd.pdf", chan) );
    } else if (fileCount == 0) {
      strcat(pdfF,    Form("channel%i_psd.pdf(", chan) );
    } else if (fileCount == numFiles-1) {
      strcat(pdfF,    Form("channel%i_psd.pdf)", chan) );
    } else {
      strcat(pdfF,    Form("channel%i_psd.pdf", chan) );
    }
    c1->Print(pngF);
    c1->Print(cF);
    c1->Print(pdfF);
    c1->Clear();

    // Now draw the zoomed versions
    psdZoomStack->Draw("NOSTACK");
    pt->Draw("SAME");
    satLeg->Draw("SAME");
    unsatLeg->Draw("SAME");

    // Saving Histos here
    strcpy(pdfF, outFile);
    strcpy(pngF, outFile);
    strcpy(cF, outFile);

    strcat(pngF, Form("channel%i_%s_psd_zoom.png", chan, dateList[fileCount].c_str()) );
    strcat(  cF,   Form("channel%i_%s_psd_zoom.C", chan, dateList[fileCount].c_str()) );

    if (numFiles == 1) {
      strcat(pdfF,    Form("channel%i_psd_zoom.pdf", chan) );
    } else if (fileCount == 0) {
      strcat(pdfF,    Form("channel%i_psd_zoom.pdf(", chan) );
    } else if (fileCount == numFiles-1) {
      strcat(pdfF,    Form("channel%i_psd_zoom.pdf)", chan) );
    } else {
      strcat(pdfF,    Form("channel%i_psd_zoom.pdf", chan) );
    }
    c1->Print(pngF);
    c1->Print(cF);
    c1->Print(pdfF);
    c1->Clear();

    if (numFiles>1) {
      if (fileCount==0) {
        psd_All->SetMarkerColor(kBlue);
        psd_All->SetMarkerStyle(7);
        psdCompHS->Add(psd_All);
        zoom_psd_All->SetMarkerColor(kBlue);
        zoom_psd_All->SetMarkerStyle(7);
        zoom_psdCompHS->Add(zoom_psd_All);
        compLeg->AddEntry(psd_All, Form("%s", dateList[fileCount].c_str() ), "P" );
      } else if (fileCount == numFiles-1) {
        psd_All->SetMarkerColor(kGreen);
        psd_All->SetMarkerStyle(7);
        psdCompHS->Add(psd_All);
        zoom_psd_All->SetMarkerColor(kGreen);
        zoom_psd_All->SetMarkerStyle(7);
        zoom_psdCompHS->Add(zoom_psd_All);
        compLeg->AddEntry(psd_All, Form("%s", dateList[fileCount].c_str() ), "P" );

        psdCompHS->Draw("NOSTACK");
        pt->Draw("SAME");
        compLeg->Draw("SAME");

        // Saving Histos here
        strcpy(pdfF, outFile);
        strcpy(pngF, outFile);
        strcpy(cF, outFile);
        strcat(pngF, Form("channel%i_psd_comp.png", chan) );
        strcat(  cF,   Form("channel%i_psd_comp.C", chan) );
        strcat(pdfF,    Form("channel%i_psd_comp.pdf", chan) );
        c1->Print(pngF);
        c1->Print(cF);
        c1->Print(pdfF);
        c1->Clear();

        // Not plot zoomed version
        zoom_psdCompHS->Draw("NOSTACK");
        pt->Draw("SAME");
        compLeg->Draw("SAME");

        // Saving Histos here
        strcpy(pdfF, outFile);
        strcpy(pngF, outFile);
        strcpy(cF, outFile);
        strcat(pngF, Form("channel%i_psd_comp_zoom.png", chan) );
        strcat(  cF,   Form("channel%i_psd_comp_zoom.C", chan) );
        strcat(pdfF,    Form("channel%i_psd_comp_zoom.pdf", chan) );
        c1->Print(pngF);
        c1->Print(cF);
        c1->Print(pdfF);
        c1->Clear();
      }
    } // end if numFIles>1
  } // end for fileCount
}
