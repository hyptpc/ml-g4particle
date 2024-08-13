#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

void cal_acc(int layer)
{
    gStyle->SetOptStat(0);
    TFile *fin1 = new TFile(Form("/home/had/kohki/work/ML/2024/geant/rootfiles/hpdf_%dlayer.root", layer));
    TFile *fin2 = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root");

    TTree *t2 = (TTree *)fin2->Get(Form("tree_%dlayer", layer));

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

    t2->SetBranchAddress("pid", &pid);
    t2->SetBranchAddress("mom", &mom);
    t2->SetBranchAddress("tof", &tof);
    t2->SetBranchAddress("ene", &ene);

    std::ifstream infile(Form("/home/had/kohki/work/ML/2024/likelihood/ana/Lcut_%dlayer.csv", layer));
    std::string line;
    std::vector<double> Lcut(3);

    // Skip the header line
    std::getline(infile, line);

    while (std::getline(infile, line))
    {
        std::stringstream ss(line);
        std::string token;
        int particle;
        double best_Eff, best_FoM, best_Lcut;
        std::getline(ss, token, ',');
        particle = std::stoi(token);
        std::getline(ss, token, ',');
        best_Eff = std::stod(token);
        std::getline(ss, token, ',');
        best_FoM = std::stod(token);
        std::getline(ss, token, ',');
        best_Lcut = std::stod(token);
        Lcut[particle] = best_Lcut;
    }
    infile.close();

    int num_event = t2->GetEntries();
    int correct_predictions = 0;

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
        }
        double Sum_pdf = pdf[0] + pdf[1] + pdf[2];
        for (int ip = 0; ip < 3; ++ip)
        {
            Likeli[ip] = pdf[ip] / Sum_pdf;
        }

        int predicted_pid = -1;
        double max_likelihood = -1;

        for (int ip = 0; ip < 3; ++ip)
        {
            if (Likeli[ip] > Lcut[ip] && Likeli[ip] > max_likelihood)
            {
                max_likelihood = Likeli[ip];
                predicted_pid = ip;
            }
        }

        if (predicted_pid == pid)
        {
            correct_predictions++;
        }
    }

    double accuracy = (double)correct_predictions / num_event;
    std::cout << "Accuracy: " << accuracy << std::endl;

    // Append accuracy to the CSV file
    std::ofstream outfile(Form("/home/had/kohki/work/ML/2024/likelihood/ana/Lcut_%dlayer.csv", layer), std::ios_base::app);
    outfile << "Accuracy," << accuracy << std::endl;
    outfile.close();

    fin1->Close();
    fin2->Close();
}
