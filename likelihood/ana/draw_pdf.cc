// note that pdf is scaled in ×1000 order !!

#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <iostream>

void draw_pdf() // GeV/c
{
  // Set some drawing options
  gStyle->SetOptStat(0); // Disable statistics box

  int layers = 32;
  double true_mom = 0.310;
  int fixed_mom = (int)((true_mom - 0.3) * 1000.);

  TFile *file = new TFile(Form("/home/had/kohki/work/ML/2024/geant/rootfiles/hpdf_%dlayer.root", layers));
  if (!file || file->IsZombie())
  {
    std::cerr << "Error opening file!" << std::endl;
    return;
  }

  // Create a canvas
  TCanvas *canvas = new TCanvas("canvas", Form("momentum: %.3f (GeV/c)", true_mom), 1800, 600);
  canvas->Divide(3, 1);

  TH2D *histogram[3];
  for (int id = 0; id < 3; ++id)
  {
    // Get the histogram corresponding to the specified id and mom_bin
    histogram[id] = dynamic_cast<TH2D *>(file->Get(Form("hpdf%d_%d_layer%d", id, fixed_mom, layers)));
    if (!histogram[id])
    {
      std::cerr << "Error getting histogram for id=!" << id << std::endl;
      continue;
    }
    double entry = histogram[id]->GetEntries();
    // histogram[id]->Scale(1. / entry);
    histogram[id]->Scale(1000. / entry); // scale in 1000 order
    canvas->cd(id + 1);
    if (id == 0)
      histogram[0]->SetTitle("PDF_{p}");
    else if (id == 1)
      histogram[1]->SetTitle("PDF_{#pi}");
    else if (id == 2)
      histogram[2]->SetTitle("PDF_{K}");

    histogram[id]->GetXaxis()->SetTitle("ToF (ns)");
    histogram[id]->GetYaxis()->SetTitle("-dE/dx (keV)");
    histogram[id]->GetXaxis()->SetTitleSize(0.04);
    histogram[id]->GetXaxis()->CenterTitle();
    histogram[id]->GetYaxis()->SetTitleSize(0.04);
    histogram[id]->GetYaxis()->CenterTitle();
    // histogram[id]->GetXaxis()->SetRangeUser(1, 4);
    // histogram[id]->GetYaxis()->SetRangeUser(0, 8);
    // Set color scale manually if needed
    histogram[id]->Draw("colz");
    canvas->Update();
  }
  // Close the file
  // file->Close();
  canvas->SaveAs(Form("/home/had/kohki/work/ML/2024/likelihood/fig/pdf_%3f_%d.png", true_mom, layers));
}
