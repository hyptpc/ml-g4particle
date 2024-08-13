#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <iostream>
#include <TGraph.h>

void likelihood(int layer)
{
  gROOT->SetStyle("ATLAS");

  TFile *fin1 = new TFile(Form("/home/had/kohki/work/ML/2024/geant/rootfiles/hpdf_%dlayer.root", layer));
  TFile *fin2 = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root");
  TTree *tree = (TTree *)fin2->Get(Form("tree_%dlayer", layer));

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
  double mom;
  double tof;
  double ene;

  tree->SetBranchAddress("pid", &pid);
  tree->SetBranchAddress("mom", &mom);
  tree->SetBranchAddress("tof", &tof);
  tree->SetBranchAddress("ene", &ene);

  int num_events = tree->GetEntries();

  TH1D *h_Likeli_S[3];
  TH1D *h_Likeli_B[3];
  for (int ip = 0; ip < 3; ++ip)
  {
    h_Likeli_S[ip] = new TH1D(Form("h_Likeli_S%d", ip),
                              Form("h_Likeli_S%d", ip),
                              500, 0.001, 1);
    h_Likeli_B[ip] = new TH1D(Form("h_Likeli_B%d", ip),
                              Form("h_Likeli_B%d", ip),
                              500, 0.001, 1);
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

  for (int k = 0; k < num_events; k++)
  {
    if (k % 100000 == 0)
      std::cout << "event:" << k << std::endl;

    tree->GetEntry(k);
    int mom_id = (int)((mom - 0.3) * 1000.);
    int p_id = (int)pid;
    int tof_id = hpdf[0][mom_id]->GetXaxis()->FindBin(tof);
    int ene_id = hpdf[0][mom_id]->GetYaxis()->FindBin(ene);
    double pdf[3];
    double Likeli[3];
    for (int ip = 0; ip < 3; ++ip)
    {
      pdf[ip] = hpdf[ip][mom_id]->GetBinContent(tof_id, ene_id);
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

  TCanvas *c1 = new TCanvas("c1", "c1", 1200, 800);
  c1->Divide(3, 3);
  TGraph *gEff[3];
  TGraph *gSB[3];
  TGraph *gEff_SB[3];

  for (int ip = 0; ip < 3; ++ip)
  {
    c1->cd(ip + 1)->SetLogy();
    if (ip == 0)
      h_Likeli_S[ip]->SetTitle("counts at each L_{P}");
    else if (ip == 1)
      h_Likeli_S[ip]->SetTitle("counts at each L_{#pi}");
    else if (ip == 2)
      h_Likeli_S[ip]->SetTitle("counts at each L_{K}");

    if (ip == 0)
      h_Likeli_S[ip]->GetXaxis()->SetTitle("L_{P}");
    else if (ip == 1)
      h_Likeli_S[ip]->GetXaxis()->SetTitle("L_{#pi}");
    else if (ip == 2)
      h_Likeli_S[ip]->GetXaxis()->SetTitle("L_{K}");
    h_Likeli_S[ip]->GetXaxis()->SetTitleSize(0.06);
    h_Likeli_S[ip]->GetXaxis()->CenterTitle();
    h_Likeli_S[ip]->GetXaxis()->SetTitleOffset(1.0);
    h_Likeli_S[ip]->GetYaxis()->SetTitle("Counts");
    h_Likeli_S[ip]->GetYaxis()->SetTitleSize(0.06);
    h_Likeli_S[ip]->GetYaxis()->CenterTitle();
    h_Likeli_S[ip]->GetYaxis()->SetTitleOffset(1.0);
    h_Likeli_S[ip]->Draw();
    h_Likeli_B[ip]->Draw("same");

    c1->cd(ip + 4);
    gEff[ip] = new TGraph(numLcut, L_cut, Eff[ip]);
    gEff[ip]->SetName(Form("gEff%d", ip));
    if (ip == 0)
      gEff[ip]->SetTitle("Lcut dependency on p");
    else if (ip == 1)
      gEff[ip]->SetTitle("Lcut dependency on #pi");
    else if (ip == 2)
      gEff[ip]->SetTitle("Lcut dependency on K");
    gEff[ip]->GetXaxis()->SetTitle("Lcut");
    gEff[ip]->GetXaxis()->SetTitleSize(0.06);
    gEff[ip]->GetXaxis()->SetTitleOffset(1.0);
    gEff[ip]->GetXaxis()->CenterTitle();
    gEff[ip]->SetMarkerColor(1);
    gEff[ip]->SetMarkerStyle(7);
    gEff[ip]->GetXaxis()->SetRangeUser(0., 1.01);
    gEff[ip]->GetYaxis()->SetRangeUser(0.7, 1.01);
    gEff[ip]->Draw("AP");

    gSB[ip] = new TGraph(numLcut, L_cut, FoM[ip]);
    gSB[ip]->SetName(Form("gSB%d", ip));
    gSB[ip]->SetMarkerColor(2);
    gSB[ip]->SetMarkerStyle(7);
    gSB[ip]->Draw("sameP");

    c1->cd(ip + 7);
    gEff_SB[ip] = new TGraph(numLcut, Eff[ip], FoM[ip]);
    if (ip == 0)
      gEff_SB[ip]->SetTitle("ROC for p");
    else if (ip == 1)
      gEff_SB[ip]->SetTitle("ROC for #pi");
    else if (ip == 2)
      gEff_SB[ip]->SetTitle("ROC for K");
    gEff_SB[ip]->SetName(Form("gEff_SB%d", ip));
    gEff_SB[ip]->GetYaxis()->SetTitle("Purity");
    gEff_SB[ip]->GetYaxis()->SetTitleSize(0.06);
    gEff_SB[ip]->GetYaxis()->SetTitleOffset(1.0);
    gEff_SB[ip]->GetYaxis()->CenterTitle();
    gEff_SB[ip]->GetXaxis()->SetTitle("Efficiency");
    gEff_SB[ip]->GetXaxis()->SetTitleSize(0.06);
    gEff_SB[ip]->GetXaxis()->CenterTitle();
    gEff_SB[ip]->GetXaxis()->SetTitleOffset(1.0);
    gEff_SB[ip]->SetMarkerColor(1);
    gEff_SB[ip]->SetMarkerStyle(7);
    gEff_SB[ip]->GetXaxis()->SetRangeUser(0.7, 1.01);
    gEff_SB[ip]->GetYaxis()->SetRangeUser(0.7, 1.01);
    gEff_SB[ip]->Draw("AP");
  }
  c1->SaveAs(Form("/home/had/kohki/work/ML/2024/likelihood/fig/likelihood_%dlayer.png", layer));
}
