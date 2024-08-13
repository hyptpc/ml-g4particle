// *************** INFO *************************
// Run as root 'compare_likelihood(<layer>)'
// layer : 10 ~ 32
// Compare Efficiency / FoM value between likelihood and ML
//
// K. Amemiya 2024/07/23
// **********************************************

#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <iostream>
#include <TGraph.h>
#include <TLatex.h>
#include <cmath>

void compare_likelihood(int layer)
{
  gStyle->SetOptStat(0);
  TFile *fin1 = new TFile(Form("/home/had/kohki/work/ML/2024/geant/rootfiles/hpdf_%dlayer.root", layer));
  TFile *fin2 = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root");
  TFile *fin3 = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/output.root");

  TTree *t2 = (TTree *)fin2->Get(Form("tree_%dlayer", layer));
  TTree *t3 = (TTree *)fin3->Get(Form("tree_%dlayer", layer));

  TH2D *hpdf[3][500]; //[3]:particle, [500]:mom

  fin1->ReadAll();
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 500; ++j)
    {
      hpdf[i][j] = (TH2D *)fin1->FindObject(Form("hpdf%d_%d_layer%d", i, j, layer));
      double Norm_factor = 1. / hpdf[i][j]->GetEntries();
      hpdf[i][j]->Scale(Norm_factor);
    }
  }

  int pid;
  int pidML_in, pidML_out;
  double mom;
  double tof;
  double ene;

  t2->SetBranchAddress("pid", &pid);
  t2->SetBranchAddress("mom", &mom);
  t2->SetBranchAddress("tof", &tof);
  t2->SetBranchAddress("ene", &ene);

  int num_event = t2->GetEntries();

  TH1D *h_Likeli_S[3];
  TH1D *h_Likeli_B[3];
  for (int ip = 0; ip < 3; ++ip)
  {
    h_Likeli_S[ip] = new TH1D(Form("h_Likeli_S%d", ip),
                              Form("h_Likeli_S%d", ip),
                              1000, 0, 1);
    h_Likeli_B[ip] = new TH1D(Form("h_Likeli_B%d", ip),
                              Form("h_Likeli_B%d", ip),
                              1000, 0, 1);
    h_Likeli_B[ip]->SetLineColor(1);
    h_Likeli_S[ip]->SetLineColor(2);
  }

  const int numLcut = 100;
  double L_cut[numLcut];
  for (int i = 0; i < numLcut; i++)
  {
    L_cut[i] = (double)(i + 1) / (double)numLcut;
  }
  int N_particle_input[3][numLcut] = {0};
  int N_particleL_S[3][numLcut] = {0}; // true identification
  int N_particleL[3][numLcut] = {0};

  double FoM[3][numLcut]; // Figure of Merit (S/S+BG)
  double Eff[3][numLcut];

  for (int k = 0; k < num_event; k++)
  {
    if (k % 100000 == 0)
      std::cout << "event:" << k << std::endl;

    t2->GetEntry(k);
    int mom_id = (int)((mom - 0.3) * 1000.);
    int p_id = (int)pid;
    int tof_id = hpdf[0][mom_id]->GetXaxis()->FindBin(tof);
    int Eloss_id = hpdf[0][mom_id]->GetYaxis()->FindBin(ene);
    double pdf[3];
    double Likeli[3];
    for (int ip = 0; ip < 3; ++ip)
    {
      pdf[ip] = hpdf[ip][mom_id]->GetBinContent(tof_id, Eloss_id);
      // std::cout<<"pdf("<<ip<<"):"<<pdf[ip]<<std::endl;
    }
    double Sum_pdf = pdf[0] + pdf[1] + pdf[2];
    for (int ip = 0; ip < 3; ++ip)
    {
      Likeli[ip] = pdf[ip] / Sum_pdf;
      if ((int)pid == ip)
        h_Likeli_S[ip]->Fill(Likeli[ip]);
      else
        h_Likeli_B[ip]->Fill(Likeli[ip]);
    }

    for (int ip = 0; ip < 3; ++ip)
    {
      for (int icut = 0; icut < numLcut; ++icut)
      {
        if ((int)pid == ip)
        {
          ++N_particle_input[ip][icut];
          if (Likeli[ip] > L_cut[icut])
            ++N_particleL_S[ip][icut];
        }
        if (Likeli[ip] > L_cut[icut])
          ++N_particleL[ip][icut];
      }
    }
  }
  for (int ip = 0; ip < 3; ++ip)
  {
    for (int icut = 0; icut < numLcut; ++icut)
    {
      Eff[ip][icut] = (double)N_particleL_S[ip][icut] / (double)N_particle_input[ip][icut];
      FoM[ip][icut] = (double)N_particleL_S[ip][icut] / (double)N_particleL[ip][icut];
    }
  }

  double EffML[3];
  double FoMML[3];
  t3->SetBranchAddress("pid", &pidML_in);
  t3->SetBranchAddress("pid_ML", &pidML_out);
  for (int ip = 0; ip < 3; ++ip)
  {
    int count_particle = 0, count_particle_ML = 0;
    int count_match = 0;

    for (int k = 0; k < t3->GetEntries(); k++)
    {
      t3->GetEntry(k);
      if ((int)pidML_in == ip)
      {
        count_particle++;
        if ((int)pidML_out == ip)
        {
          count_match++;
        }
      }
      if ((int)pidML_out == ip)
      {
        count_particle_ML++;
      }
    }

    EffML[ip] = (double)count_match / (double)count_particle;
    FoMML[ip] = (double)count_match / (double)count_particle_ML;
    // std::cout << "count_particle : " << count_particle << " count_particle_ML : " << count_particle_ML << std::endl;
  }

  // +++++++++++++++ DRAW GRAPH +++++++++++++++++++++

  TCanvas *c = new TCanvas("c", "c", 1800, 600);
  c->Divide(3, 1);
  TGraph *gEff_SB[3];

  std::ofstream output_file(Form("/home/had/kohki/work/ML/2024/likelihood/ana/Lcut_%dlayer.csv", layer));
  output_file << "Particle,Best_Eff,Best_FoM,L_cut\n"; // CSV header

  for (int ip = 0; ip < 3; ++ip)
  {
    c->cd(ip + 1);
    gEff_SB[ip] = new TGraph(numLcut, Eff[ip], FoM[ip]);
    if (ip == 0)
      gEff_SB[ip]->SetTitle("ROC comparison on p");
    else if (ip == 1)
      gEff_SB[ip]->SetTitle("ROC comparison on #pi");
    else if (ip == 2)
      gEff_SB[ip]->SetTitle("ROC comparison on K");
    gEff_SB[ip]->SetName(Form("gEff_SB%d", ip));
    gEff_SB[ip]->GetYaxis()->SetTitle("Purity");
    gEff_SB[ip]->GetYaxis()->CenterTitle();
    gEff_SB[ip]->GetXaxis()->SetTitle("Efficiency");
    gEff_SB[ip]->GetXaxis()->CenterTitle();
    gEff_SB[ip]->GetXaxis()->SetTitleSize(0.05);
    gEff_SB[ip]->GetXaxis()->SetTitleOffset(0.8);
    gEff_SB[ip]->GetYaxis()->SetTitleSize(0.05);
    gEff_SB[ip]->GetYaxis()->SetTitleOffset(1.2);
    gEff_SB[ip]->SetMarkerColor(1);
    gEff_SB[ip]->SetMarkerStyle(7);
    gEff_SB[ip]->GetXaxis()->SetRangeUser(0.7, 1.01); // change here
    gEff_SB[ip]->GetYaxis()->SetRangeUser(0.7, 1.01); // change here
    gEff_SB[ip]->Draw("AP");
    TGraph *point = new TGraph(1);
    point->SetPoint(0, EffML[ip], FoMML[ip]);
    point->SetMarkerStyle(20);
    point->SetMarkerColor(kRed);
    point->Draw("P");

    // Find the best efficiency for the likelihood method
    double max_product = 0;
    double best_Eff = 0;
    double best_FoM = 0;
    double best_Lcut = 0;

    for (int i = 0; i < numLcut; ++i)
    {
      double product = Eff[ip][i] * FoM[ip][i];
      if (product > max_product)
      {
        max_product = product;
        best_Eff = Eff[ip][i];
        best_FoM = FoM[ip][i];
        best_Lcut = L_cut[i];
      }
    }

    output_file << ip << "," << best_Eff << "," << best_FoM << "," << best_Lcut << "\n"; // Write the best efficiency to the CSV file

    TLatex *tex_ml = new TLatex(0.25, 0.45, Form("Eff: %.3f, FoM: %.3f", EffML[ip], FoMML[ip]));
    tex_ml->SetTextColor(2);
    tex_ml->SetNDC();
    tex_ml->Draw();
    TLatex *tex_likeli = new TLatex(0.25, 0.5, Form("Eff: %.3f, FoM: %.3f", best_Eff, best_FoM)); // Display the best efficiency
    tex_likeli->SetNDC();
    tex_likeli->Draw();
    std::cout << "Eff : " << best_Eff << " FoM : " << best_FoM << std::endl;
    std::cout << "Eff_ML : " << EffML[ip] << " FoM_ML : " << FoMML[ip] << std::endl;
  }
  c->SaveAs(Form("/home/had/kohki/work/ML/2024/likelihood/fig/comparison_%dlayer.png", layer));
}
