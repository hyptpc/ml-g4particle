#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TStyle.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

double CalculateTruncatedMean(int layer, int file_id, double mom[65])
{
    // -----------------------------------------------------------------
    // Function to calculate truncated energy losses and fill histograms
    // arguments: int layer: number of total layers
    //            int file_id: file ID (0 = proton, 1 = pion, 2 = kaon)
    //            double mom[65]: momentum array
    //
    // ------------------------------------------------------------------

    TDatabasePDG *pdg = TDatabasePDG::Instance();
    double mass;
    if (file_id == 0)
    {
        mass = pdg->GetParticle(2212)->Mass() * 1000;
    } // proton mass in MeV
    else if (file_id == 1)
    {
        mass = pdg->GetParticle(-211)->Mass() * 1000;
    } // pion mass in MeV
    else if (file_id == 2)
    {
        mass = pdg->GetParticle(-321)->Mass() * 1000;
    } // kaon mass in MeV

    double total_truncated_ene = 0.;
    int count = 0;

    std::vector<double> energylosses;

    for (int i = 0; i < layer; ++i) // for number of layers
    {
        // double energyloss = adc[i * 2 + 1] * 1000; // [keV]
        double energyloss = mom[i * 2] / sqrt(pow(mom[i * 2], 2) + pow(mass, 2)) * (mom[i * 2] - mom[(i + 1) * 2]) * 1000; // [keV]
        energylosses.push_back(energyloss);
        // std::cout << "energy loss: " << energyloss << std::endl;
    }

    // std::cout << "energylosses_size: " << energylosses.size() << std::endl; // debug

    // Sort the energy losses and remove the top 30%
    std::sort(energylosses.begin(), energylosses.end(), std::greater<double>());
    int threshold_index = energylosses.size() * 0.3;
    for (int i = threshold_index; i < energylosses.size(); ++i)
    {
        total_truncated_ene += energylosses[i];
        count++;
    }

    double truncated_mean_ene = (count > 0) ? total_truncated_ene / count : 0.0;
    return truncated_mean_ene;
}

void read_data(const std::vector<std::string> &filenames, const std::vector<int> &particle_ids, int layer, std::vector<int> &particle_id_vec, std::vector<double> &tof_vec, std::vector<double> &init_mom_vec, std::vector<double> &trunc_mean_ene_vec)
{
    // --------------------------------------------------------------------------------
    // Function to process multiple ROOT files and fill the output tree
    // arguments: filenames (std::vector<std::string>): list of ROOT files to process
    //            particle_ids (std::vector<int>): list of particle IDs
    //            layer (int): number of total layers
    //            outTree (TTree*): output tree to fill
    //            particle_id (int&): particle ID
    //            tof (double&): ToF at HTOF
    //            mom_init (double&): initialmomentum
    //            trunc_mean_ene (double&): truncated mean energy
    // ---------------------------------------------------------------------------------

    std::vector<double> trunc_mean_ene_arr;

    for (size_t file_id = 0; file_id < filenames.size(); ++file_id)
    {
        TFile *file = TFile::Open(filenames[file_id].c_str(), "READ");
        TTree *tree = (TTree *)file->Get("tree");

        int nEntries = tree->GetEntries();
        double resgtime[65]; // [ns]
        double mom[65];      // [MeV/c]

        TDatabasePDG *pdg = TDatabasePDG::Instance();
        double mass;
        if (file_id == 0)
        {
            mass = pdg->GetParticle(2212)->Mass() * 1000;
        } // proton mass in MeV
        else if (file_id == 1)
        {
            mass = pdg->GetParticle(-211)->Mass() * 1000;
        } // pion mass in MeV
        else if (file_id == 2)
        {
            mass = pdg->GetParticle(-321)->Mass() * 1000;
        } // kaon mass in MeV

        tree->SetBranchAddress("resgtime", resgtime);
        tree->SetBranchAddress("mom", mom);

        for (int event = 0; event < nEntries; ++event)
        {
            tree->GetEntry(event);

            if (resgtime[64] == 0) // only take events that hit HTOF
                continue;

            double dl = 0.0126 * (32 - layer);
            double c = 0.299792458; //[m/ns]
            double beta = mom[0] / sqrt(pow(mom[0], 2) + pow(mass, 2));
            double tof = resgtime[64] - (dl / (c * beta));

            particle_id_vec.push_back(particle_ids[file_id]);
            tof_vec.push_back(tof);
            init_mom_vec.push_back(mom[0]);
            trunc_mean_ene_vec.push_back(CalculateTruncatedMean(layer, file_id, mom));
        }

        file->Close();
    }
}

void mkinput(int layer)
{
    // --------------------------------------------------------------------------------
    // Main function to open the ROOT file and call processing and drawing functions
    // --------------------------------------------------------------------------------

    // List of ROOT files to process
    std::vector<std::string> filenames = {
        "/home/had/kohki/work/ML/2024/geant/rootfiles/proton_raw.root",
        "/home/had/kohki/work/ML/2024/geant/rootfiles/pion_raw.root",
        "/home/had/kohki/work/ML/2024/geant/rootfiles/kaon_raw.root"};

    std::vector<int> particle_ids = {0, 1, 2};

    // Create a new ROOT file to save the output
    TFile *outfile = new TFile(Form("/home/had/kohki/work/ML/2024/geant/rootfiles/input%dlayer.root", layer), "RECREATE");

    int particle_id;
    double tof;
    double init_mom;
    double trunc_mean_ene;

    // Create branches for each total layers in each tree
    TTree *outTree = new TTree(Form("tree_%dlayer", layer), Form("Output tree for %d layer", layer));
    outTree->Branch("pid", &particle_id, "pid/I");
    outTree->Branch("tof", &tof, "tof/D");
    outTree->Branch("mom", &init_mom, "mom/D");
    outTree->Branch("ene", &trunc_mean_ene, "ene/D");

    // Process files and fill the output tree
    std::vector<int> particle_id_vec;
    std::vector<double> tof_vec;
    std::vector<double> init_mom_vec;
    std::vector<double> trunc_mean_ene_vec;

    read_data(filenames, particle_ids, layer, particle_id_vec, tof_vec, init_mom_vec, trunc_mean_ene_vec);

    // Shuffle the data
    std::vector<size_t> indices(particle_id_vec.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    for (size_t i : indices)
    {
        particle_id = particle_id_vec[i];
        tof = tof_vec[i];                       // [ns]
        init_mom = init_mom_vec[i] * 0.001;     // [MeV/c] -> [GeV/c]
        trunc_mean_ene = trunc_mean_ene_vec[i]; // [keV]
        outTree->Fill();
    }

    outfile->cd();
    outTree->Write("", TObject::kOverwrite);
    outfile->Close();
}
