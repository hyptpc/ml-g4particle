#include <TFile.h>
#include <TTree.h>
#include <iostream>

void split_input()
{
    // Open the input file
    TFile *infile = TFile::Open("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root", "READ");

    // Create the output files
    // TFile *outfile_test = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input_test.root", "RECREATE");
    TFile *outfile_nn = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input_nn.root", "RECREATE");

    // Loop over the layers
    for (int layer = 10; layer <= 32; ++layer)
    {
        // Get the input tree
        TTree *inTree = (TTree *)infile->Get(Form("tree_%dlayer", layer));

        // Create the output trees
        // TTree *outTree_test = new TTree(Form("tree_%dlayer", layer), Form("Output test tree for %d layer", layer));
        TTree *outTree_nn = new TTree(Form("tree_%dlayer", layer), Form("Output nn tree for %d layer", layer));

        // Create branches to hold the data
        int pid;
        double tof, mom, ene;
        inTree->SetBranchAddress("pid", &pid);
        inTree->SetBranchAddress("tof", &tof);
        inTree->SetBranchAddress("mom", &mom);
        inTree->SetBranchAddress("ene", &ene);

        // outTree_test->Branch("pid", &pid, "pid/I");
        // outTree_test->Branch("tof", &tof, "tof/D");
        // outTree_test->Branch("mom", &mom, "mom/D");
        // outTree_test->Branch("ene", &ene, "ene/D");

        outTree_nn->Branch("pid", &pid, "pid/I");
        outTree_nn->Branch("tof", &tof, "tof/D");
        outTree_nn->Branch("mom", &mom, "mom/D");
        outTree_nn->Branch("ene", &ene, "ene/D");

        // Loop over the entries in the input tree
        int nEntries = inTree->GetEntries();
        int test_entries = std::min(nEntries, 500000); // limit test dataset to 500,000 entries
        int nn_entries = nEntries - test_entries;      // remaining entries for nn dataset

        for (int i = 0; i < nEntries; ++i)
        {
            inTree->GetEntry(i);
            // if (i < test_entries)
            // {
            //     outTree_test->Fill();
            // }
            if (i > test_entries - 1)
            {
                outTree_nn->Fill();
            }
        }

        // Write the output trees to the respective files
        // outfile_test->cd();
        // outTree_test->Write("", TObject::kOverwrite);

        outfile_nn->cd();
        outTree_nn->Write("", TObject::kOverwrite);

        // Cleanup
        // delete outTree_test;
        delete outTree_nn;
    }

    // Close the files
    infile->Close();
    // outfile_test->Close();
    outfile_nn->Close();
}
