// macro to check if fitting in mktrain.cc is correct

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TColor.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

double fit_mom = 0.310;

std::pair<double, double> fit_gaussian(int id, TH1D *h, TF1 *f, const std::vector<double> &data, TCanvas *c, TLegend *legend, int pid, double sigma_max)
{
    // Fill histogram with data
    for (auto val : data)
        h->Fill(val);

    // Set histogram and fit function colors based on particle_id
    int hist_colors[] = {1, 1, 1};
    int fit_colors[] = {46, 9, 8};
    // Set limits for mean
    double mean_min = h->GetMean() - sigma_max;
    double mean_max = h->GetMean() + sigma_max;
    f->SetParLimits(1, mean_min, mean_max);
    f->SetParLimits(2, 0, sigma_max);
    f->SetLineColor(fit_colors[pid]);
    h->SetLineColor(hist_colors[pid]);
    h->Fit(f);

    double mean = f->GetParameter(1);
    double sigma = f->GetParameter(2);

    // Ensure mean and sigma are within specified ranges
    if (mean < mean_min || mean > mean_max)
    {
        mean = h->GetMean();
        std::cout << "Mean out of range " << std::endl;
    }
    if (sigma > sigma_max)
    {
        sigma = sigma_max;
        std::cout << "Sigma out of range " << std::endl;
    }
    // Update the fit function with the corrected parameters
    f->SetParameter(1, mean);
    f->SetParameter(2, sigma);

    if (id == 0)
        h->GetXaxis()->SetTitle("Energy [MeV]");
    else
        h->GetXaxis()->SetTitle("TOF [ns]");
    h->GetYaxis()->SetTitle("Counts");
    h->GetXaxis()->CenterTitle(true);
    h->GetYaxis()->CenterTitle(true);

    h->Draw("same");
    f->Draw("same");

    c->Update();

    legend->AddEntry(h, Form("Particle %d", pid), "l");

    return std::make_pair(mean, sigma);
}

void checkfit()
{
    TFile *infile = TFile::Open("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root", "READ");
    TTree *tree = (TTree *)infile->Get("tree_32layer");
    int particle;
    double mom, ene, tof;
    tree->SetBranchAddress("pid", &particle);
    tree->SetBranchAddress("mom", &mom);
    tree->SetBranchAddress("ene", &ene);
    tree->SetBranchAddress("tof", &tof);

    std::map<int, std::map<int, std::vector<double>>> ene_data;
    std::map<int, std::map<int, std::vector<double>>> tof_data;

    int nEntries = tree->GetEntries();
    for (int i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        int momentum_bin = static_cast<int>((mom - 0.3) * 1000);
        int particle_type = static_cast<int>(particle);
        ene_data[particle_type][momentum_bin].push_back(ene);
        tof_data[particle_type][momentum_bin].push_back(tof);
    }

    int target_mom_bin = static_cast<int>((fit_mom - 0.3) * 1000);

    TCanvas *c1 = new TCanvas("c1", "Energy Distribution", 1200, 400);
    TCanvas *c2 = new TCanvas("c2", "TOF Distribution", 1200, 400);

    c1->Divide(3, 1);
    c2->Divide(3, 1);

    TLegend *legends1[3];
    TLegend *legends2[3];
    for (int i = 0; i < 3; ++i)
    {
        legends1[i] = new TLegend(0.78, 0.6, 0.98, 0.77);
        legends2[i] = new TLegend(0.78, 0.6, 0.98, 0.77);
    }

    // Energy Distribution
    TH1D *hene[3]; // Array to hold histograms
    TF1 *fene[3];  // Array to hold fit functions

    for (int pid = 0; pid < 3; ++pid)
    {
        if (ene_data[pid].find(target_mom_bin) != ene_data[pid].end()) // Check if target momentum bin exists
        {
            const std::vector<double> &ene_dist = ene_data[pid][target_mom_bin];

            if (!ene_dist.empty())
            {
                // Create histogram and fit function names dynamically
                TString hist_name = Form("hene%d", pid);
                TString func_name = Form("fene%d", pid);

                c1->cd(pid + 1);
                gPad->SetLogy(0); // Ensure not in log scale
                hene[pid] = new TH1D(hist_name, "", 100, 0, 18);
                fene[pid] = new TF1(func_name, "gaus");

                fit_gaussian(0, hene[pid], fene[pid], ene_dist, c1, legends1[pid], pid, 0.3);
                legends1[pid]->Draw();
            }
        }
    }

    c1->Update();

    // TOF Distribution
    TH1D *htof[3]; // Array to hold histograms
    TF1 *ftof[3];  // Array to hold fit functions

    for (int pid = 0; pid < 3; ++pid)
    {
        if (tof_data[pid].find(target_mom_bin) != tof_data[pid].end())
        {
            const std::vector<double> &tof_dist = tof_data[pid][target_mom_bin];

            if (!tof_dist.empty())
            {
                // Create histogram and fit function names dynamically
                TString hist_name = Form("htof%d", pid);
                TString func_name = Form("ftof%d", pid);

                c2->cd(pid + 1);
                gPad->SetLogy(0); // Ensure not in log scale
                htof[pid] = new TH1D(hist_name, "", 100, 0, 6);
                ftof[pid] = new TF1(func_name, "gaus");

                fit_gaussian(1, htof[pid], ftof[pid], tof_dist, c2, legends2[pid], pid, 0.15);
                legends2[pid]->Draw();
            }
        }
    }

    c2->Update();

    c1->SaveAs("ene_fit.png");
    c2->SaveAs("tof_fit.png");
}
