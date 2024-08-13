#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TStyle.h>
#include <algorithm>
#include <iostream>
#include <vector>

void drawHistograms(TH1D *hE, TH1D *hE_trunc)
// --------------------------------------------------------------------
// Function to draw the histograms and save the canvas
// arguments: hE (TH1D*): histogram of energy losses
//            hE_trunc (TH1D*): histogram of truncated energy losses
// --------------------------------------------------------------------
{
  // Create a canvas and set up the legend
  TCanvas *canvas = new TCanvas("canvas", "ene vs truncated ene", 800, 600);
  canvas->SetGrid();
  TLegend *legend = new TLegend(0.78, 0.6, 0.98, 0.77);

  // Set histogram style
  hE->SetLineWidth(2);
  hE->SetXTitle("energy [keV]");
  hE->GetXaxis()->CenterTitle();
  hE->SetYTitle("count");
  hE->GetYaxis()->CenterTitle();
  hE->Draw();

  hE_trunc->SetLineWidth(2);
  hE_trunc->SetLineColor(kRed);
  hE_trunc->Draw("SAME");

  // Add entry to the legend
  legend->AddEntry(hE, "raw data", "l");
  legend->AddEntry(hE_trunc, "truncated data", "l");
  legend->Draw();

  // Save the canvas
  // canvas->SaveAs("proton_energy.png");
  // canvas->SaveAs("pion_energy.png");
  canvas->SaveAs("kaon_energy.png");

  // Clean up
  delete legend;
  delete canvas;
}

void FillHistograms(TTree *tree, int file_id, TH1D *hE, TH1D *hE_trunc)
{
  // -----------------------------------------------------------------
  // Function to calculate truncated energy losses and fill histograms
  // arguments: tree (TTree*): input tree
  //            file_id (int): file ID (0 = proton, 1 = pion, 2 = kaon)
  // ------------------------------------------------------------------

  double resgtime[65]; // [ns]
  double mom[65];      // [MeV/c]
  // double adc[65];      // [MeV]

  tree->SetBranchAddress("resgtime", resgtime);
  // tree->SetBranchAddress("adc", adc);
  tree->SetBranchAddress("mom", mom);

  TDatabasePDG *pdg = TDatabasePDG::Instance();
  double mass;

  if (file_id == 0)
  {
    mass = pdg->GetParticle(2212)->Mass() * 1000; // proton mass in MeV
  }
  else if (file_id == 1)
  {
    mass = pdg->GetParticle(-211)->Mass() * 1000; // pion mass in MeV
  }
  else if (file_id == 2)
  {
    mass = pdg->GetParticle(-321)->Mass() * 1000; // kaon mass in MeV
  }

  // Loop over entries and fill the histogram
  int nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i)
  {
    tree->GetEntry(i);

    if (resgtime[64] == 0) // only take events that hit HTOF
      continue;

    if (mom[0] < 450 || mom[0] > 550) // only take events with momentum between 475 and 525 MeV/c
      continue;

    // std::cout << "mom[0]: " << mom[0] << std::endl;

    std::vector<double> energylosses;

    for (int j = 0; j < 32; ++j) // for 32 layers
    {
      // double energyloss = adc[j * 2 + 1] * 1000; // [keV]
      double energyloss = mom[j * 2] / sqrt(pow(mom[j * 2], 2) + pow(mass, 2)) * (mom[j * 2] - mom[(j + 1) * 2]) * 1000; // [keV]
      hE->Fill(energyloss);
      energylosses.push_back(energyloss);
      // std::cout << "energy loss: " << energyloss << std::endl;
    }

    // Sort the energy losses and remove the top 30%
    std::sort(energylosses.begin(), energylosses.end(), std::greater<double>());
    int threshold_index = energylosses.size() * 0.3;
    for (int j = threshold_index; j < energylosses.size(); ++j)
    {
      hE_trunc->Fill(energylosses[j]);
    }
  }
}

void DrawTruncatedEne()
{
  // --------------------------------------------------------------------------------
  // Main function to open the ROOT file and call processing and drawing functions
  // arguments: none
  // --------------------------------------------------------------------------------

  TH1D *hE = new TH1D("hE", "energy loss", 3000, 0, 30);                       // bin width=10eV
  TH1D *hE_trunc = new TH1D("hE_trunc", "truncated energy loss", 3000, 0, 30); // bin width=10eV

  // List of ROOT files to process
  // std::vector<std::string>
  //     filenames = {
  //         "/home/had/kohki/work/ML/2024/geant/rootfiles/test_proton_raw.root",
  //         "/home/had/kohki/work/ML/2024/geant/rootfiles/test_pion_raw.root",
  //         "/home/had/kohki/work/ML/2024/geant/rootfiles/test_kaon_raw.root"};

  // std::vector<std::string> filenames = {"/home/had/kohki/work/ML/2024/geant/rootfiles/test_proton_raw.root"};
  // std::vector<std::string> filenames = {"/home/had/kohki/work/ML/2024/geant/rootfiles/test_pion_raw.root"};
  std::vector<std::string> filenames = {"/home/had/kohki/work/ML/2024/geant/rootfiles/test_kaon_raw.root"};

  for (size_t file_id = 0; file_id < filenames.size(); ++file_id)
  {
    TFile *file = TFile::Open(filenames[file_id].c_str(), "READ");
    TTree *tree = (TTree *)file->Get("tree");
    FillHistograms(tree, file_id, hE, hE_trunc);
  }
  drawHistograms(hE, hE_trunc);
}
