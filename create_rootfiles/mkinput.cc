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

double CalculateEnergyLoss(int layer, int file_id, double mom[65], int current_layer)
{
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

    double energyloss = mom[current_layer * 2] / sqrt(pow(mom[current_layer * 2], 2) + pow(mass, 2)) * (mom[current_layer * 2] - mom[(current_layer + 1) * 2]) * 1000; // [keV]
    return energyloss;
}

void read_data(const std::vector<std::string> &filenames, const std::vector<int> &particle_ids, int layer, std::vector<int> &particle_id_vec, std::vector<double> &tof_vec, std::vector<double> &init_mom_vec, std::vector<std::vector<double>> &energy_loss_vec)
{
    for (size_t file_id = 0; file_id < filenames.size(); ++file_id)
    {
        TFile *file = TFile::Open(filenames[file_id].c_str(), "READ");
        TTree *tree = (TTree *)file->Get("tree");

        int nEntries = tree->GetEntries();
        double resgtime[65]; // [ns]
        double mom[65];      // [MeV/c]

        tree->SetBranchAddress("resgtime", resgtime);
        tree->SetBranchAddress("mom", mom);

        for (int event = 0; event < nEntries; ++event)
        {
            tree->GetEntry(event);

            if (resgtime[64] == 0) // only take events that hit HTOF
                continue;

            double dl = 0.0126 * (32 - layer);
            double c = 0.299792458; //[m/ns]
            TDatabasePDG *pdg = TDatabasePDG::Instance();
            double mass;
            if (file_id == 0)
                mass = pdg->GetParticle(2212)->Mass() * 1000;
            else if (file_id == 1)
                mass = pdg->GetParticle(-211)->Mass() * 1000;
            else
                mass = pdg->GetParticle(-321)->Mass() * 1000;

            double beta = mom[0] / sqrt(pow(mom[0], 2) + pow(mass, 2));
            double tof = resgtime[64] - (dl / (c * beta));

            particle_id_vec.push_back(particle_ids[file_id]);
            tof_vec.push_back(tof);
            init_mom_vec.push_back(mom[0]);

            std::vector<double> energy_losses(layer);
            for (int i = 0; i < layer; ++i)
            {
                energy_losses[i] = CalculateEnergyLoss(layer, file_id, mom, i);
            }
            energy_loss_vec.push_back(energy_losses);
        }

        file->Close();
    }
}

void mkinput(int layer)
{
    std::vector<std::string> filenames = {
        "../../geant/rootfiles/proton_raw.root",
        "../../geant/rootfiles/pion_raw.root",
        "../../geant/rootfiles/kaon_raw.root"};

    std::vector<int> particle_ids = {0, 1, 2};

    TFile *outfile = new TFile(Form("../../geant/rootfiles/input%dlayer.root", layer), "RECREATE");

    int particle_id;
    double tof;
    double init_mom;
    std::vector<double> ene_layer(layer);

    TTree *outTree = new TTree(Form("tree_%dlayer", layer), Form("Output tree for %d layer", layer));
    outTree->Branch("pid", &particle_id, "pid/I");
    outTree->Branch("tof", &tof, "tof/D");
    outTree->Branch("mom", &init_mom, "mom/D");
    
    for (int i = 0; i < layer; ++i)
    {
        outTree->Branch(Form("ene_layer%d", i), &ene_layer[i], Form("ene_layer%d/D", i));
    }

    std::vector<int> particle_id_vec;
    std::vector<double> tof_vec;
    std::vector<double> init_mom_vec;
    std::vector<std::vector<double>> energy_loss_vec;

    read_data(filenames, particle_ids, layer, particle_id_vec, tof_vec, init_mom_vec, energy_loss_vec);

    std::vector<size_t> indices(particle_id_vec.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    for (size_t i : indices)
    {
        particle_id = particle_id_vec[i];
        tof = tof_vec[i]; 
        init_mom = init_mom_vec[i] * 0.001;     

        for (int j = 0; j < layer; ++j)
        {
            ene_layer[j] = energy_loss_vec[i][j]; 
        }
        outTree->Fill();
    }

    outfile->cd();
    outTree->Write("", TObject::kOverwrite);
    outfile->Close();
}
