#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TRandom3.h>
#include <TString.h>
#include <TROOT.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// Read mom, ene, tof data from tree and store them in to std::map as "energy_data" and "tof_data" for each "momentum_bin"
void extract_input_data(TTree *tree, std::map<int, std::map<int, std::vector<double>>> &energy_data, std::map<int, std::map<int, std::vector<double>>> &tof_data)
{
    int particle;
    double mom, ene, tof;
    tree->SetBranchAddress("pid", &particle);
    tree->SetBranchAddress("mom", &mom);
    tree->SetBranchAddress("ene", &ene);
    tree->SetBranchAddress("tof", &tof);

    int nEntries = tree->GetEntries();
    for (int i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);

        int momentum_bin = static_cast<int>((mom - 0.3) * 1000);
        int particle_type = static_cast<int>(particle);
        energy_data[particle_type][momentum_bin].push_back(ene);
        tof_data[particle_type][momentum_bin].push_back(tof);
    }
}

// fit gaussian to the tof/ene data and return the mean and sigma
std::pair<double, double> fit_gaussian(const std::vector<double> &data, const std::string &hist_name, double min, double max)
{
    TH1D h(hist_name.c_str(), "", 100, min, max);
    for (auto val : data)
        h.Fill(val);

    double hist_std_dev = h.GetStdDev(); // Get the standard deviation from the histogram

    TF1 f("f", "gaus(0)", min, max);
    double mean_min = h.GetMean() - hist_std_dev;
    double mean_max = h.GetMean() + hist_std_dev;
    f.SetParLimits(1, mean_min, mean_max); // Set limits for mean
    f.SetParLimits(2, 0, hist_std_dev);    // Set upper limit for sigma based on the histogram's std dev

    h.Fit(&f, "Q");

    double mean = f.GetParameter(1);
    double sigma = f.GetParameter(2);
    // Ensure mean and sigma are within specified ranges
    if (mean < mean_min || mean > mean_max)
        mean = h.GetMean();
    if (sigma > hist_std_dev + 0.5)
        sigma = hist_std_dev;

    return std::make_pair(mean, sigma);
}

// Read input file and generate data
void generate_data(const char *input_file, const char *output_hpdf_file, int layer, int n_generated)
{
    TFile *infile = TFile::Open(input_file, "READ");
    TFile *hpdf_outfile = new TFile(output_hpdf_file, "RECREATE");

    TRandom3 rand_gen;
    int particle;
    double mom_gen;
    double tof_gen;
    double ene_gen;

    // layerに対応するツリーを取得
    TTree *tree = (TTree *)infile->Get(Form("tree_%dlayer", layer));

    // エネルギーとTOFのデータを格納するためのマップ
    std::map<int, std::map<int, std::vector<double>>> energy_data;
    std::map<int, std::map<int, std::vector<double>>> tof_data;

    extract_input_data(tree, energy_data, tof_data);

    // Initialize histograms
    TH2D *hpdf[3][500]; // [3]: particle, [500]: mom
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 500; ++j)
        {
            hpdf[i][j] = new TH2D(Form("hpdf%d_%d_layer%d", i, j, layer), Form("hpdf%d_%d_layer%d", i, j, layer), 500, 0, 6, 500, 0, 15);
        }
    }

    // Loop over particle IDs and momentum bins to perform Gaussian fitting
    std::map<int, std::map<int, std::pair<double, double>>> energy_params_map;
    std::map<int, std::map<int, std::pair<double, double>>> tof_params_map;

    for (const auto &particle_entry : energy_data)
    {
        int id = particle_entry.first; // particle ID
        for (const auto &momentum_entry : particle_entry.second)
        {
            int mom_bin = momentum_entry.first; // momentum bin

            const std::vector<double> &energy_dist = energy_data[id][mom_bin];
            const std::vector<double> &tof_dist = tof_data[id][mom_bin];

            if (!energy_dist.empty() && !tof_dist.empty())
            {
                energy_params_map[id][mom_bin] = fit_gaussian(energy_dist, "energy_dist", 0, 18);
                tof_params_map[id][mom_bin] = fit_gaussian(tof_dist, "tof_dist", 0, 6);
            }
        }
    }

    for (int i = 0; i < n_generated; ++i)
    {
        if (i % 10000 == 0)
            std::cout << "layer: " << layer << " event: " << i << std::endl;

        double rand = rand_gen.Uniform(0, 1);
        int id = (rand < 0.33) ? 0 : (rand < 0.66) ? 1
                                                   : 2;

        mom_gen = rand_gen.Uniform(0.3, 0.8); // GeV/c
        int mom_bin = static_cast<int>((mom_gen - 0.3) * 1000);

        if (energy_params_map[id].find(mom_bin) != energy_params_map[id].end() && tof_params_map[id].find(mom_bin) != tof_params_map[id].end())
        {
            auto energy_params = energy_params_map[id][mom_bin];
            auto tof_params = tof_params_map[id][mom_bin];

            ene_gen = rand_gen.Gaus(energy_params.first, energy_params.second);
            tof_gen = rand_gen.Gaus(tof_params.first, tof_params.second);

            hpdf[id][mom_bin]->Fill(tof_gen, ene_gen);
        }
    }

    // Write histograms to file
    hpdf_outfile->cd();
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 500; ++j)
        {
            hpdf[i][j]->Write();
            delete hpdf[i][j];
        }
    }
    hpdf_outfile->Close();

    infile->Close();

    std::cout << "Data generation finished for layer " << layer << std::endl;
}

int mkpdf(int layer)
{
    gROOT->SetBatch(kTRUE);
    const char *input_file = "/home/had/kohki/work/ML/2024/geant/rootfiles/input.root";
    const char *output_hpdf_file = Form("/home/had/kohki/work/ML/2024/geant/rootfiles/hpdf_%dlayer.root", layer);
    int n_generated = 1000000000;
    generate_data(input_file, output_hpdf_file, layer, n_generated);
    return 0;
}
