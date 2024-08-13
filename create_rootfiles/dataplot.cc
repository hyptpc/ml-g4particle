// *****************************************
//
// Plot ToF vs mom & Energyloss vs mom data
//
// *****************************************

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraph.h>

void dataplot()
{
    // Open the ROOT file containing the generated data
    TFile *file = new TFile("/home/had/kohki/work/ML/2024/geant/rootfiles/input.root", "READ");
    TTree *tree = (TTree *)file->Get("tree_32layer"); // data of all-layer-hit

    // Declare variables to store data
    double mom, tof, ene;
    int particle_id;

    // Set the branch addresses
    tree->SetBranchAddress("mom", &mom);
    tree->SetBranchAddress("tof", &tof);
    tree->SetBranchAddress("ene", &ene);
    tree->SetBranchAddress("pid", &particle_id);

    //==========================================
    // Draw ToF vs momentum graph
    //==========================================

    TCanvas *canvas = new TCanvas("canvas", "ToF data", 800, 600);
    TMultiGraph *mg = new TMultiGraph();
    TLegend *legend = new TLegend(0.65, 0.7, 0.85, 0.88);

    // Create separate TGraphs for each particle
    TGraph *graphProton = new TGraph();
    TGraph *graphPion = new TGraph();
    TGraph *graphKaon = new TGraph();

    // Loop over the events in the tree and fill the graphs
    int nEntries = tree->GetEntries();
    for (int i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        double xValue = mom;
        double yValue = tof;

        // Fill the appropriate TGraph based on particle id
        if (particle_id == 0)
        {
            graphProton->SetPoint(graphProton->GetN(), xValue, yValue);
        }
        else if (particle_id == 1)
        {
            graphPion->SetPoint(graphPion->GetN(), xValue, yValue);
        }
        else if (particle_id == 2)
        {
            graphKaon->SetPoint(graphKaon->GetN(), xValue, yValue);
        }
    }

    // Set the point color and add to the multigraph and legend
    graphProton->SetMarkerColor(kRed);
    graphProton->SetMarkerStyle(1);
    graphPion->SetMarkerColor(kBlue);
    graphPion->SetMarkerStyle(1);
    graphKaon->SetMarkerColor(kGreen);
    graphKaon->SetMarkerStyle(1);

    mg->Add(graphProton);
    mg->Add(graphPion);
    mg->Add(graphKaon);

    legend->AddEntry(graphProton, "Proton", "P");
    legend->AddEntry(graphPion, "Pion", "P");
    legend->AddEntry(graphKaon, "Kaon", "P");

    mg->SetTitle("ToF & momentum; momentum (GeV/c); ToF (ns)");
    mg->GetYaxis()->CenterTitle(true);
    mg->GetXaxis()->CenterTitle(true);
    mg->Draw("AP");
    mg->GetXaxis()->SetRangeUser(0.25, 0.85);
    legend->Draw();
    canvas->SetGrid();
    canvas->Draw();
    canvas->Update();

    //=====================================
    // Draw Energy-Loss vs momentum graph
    //======================================

    TCanvas *canvas2 = new TCanvas("canvas2", "EnergyLoss data", 800, 600);
    TMultiGraph *mg2 = new TMultiGraph();
    TLegend *legend2 = new TLegend(0.65, 0.7, 0.85, 0.88);

    // Create separate TGraphs for each particle
    TGraph *graphProton2 = new TGraph();
    TGraph *graphPion2 = new TGraph();
    TGraph *graphKaon2 = new TGraph();

    // Loop over the events in the tree and fill the graphs
    for (Long64_t i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        double xValue2 = mom;
        double yValue2 = ene;

        // Fill the appropriate TGraph based on particle id
        if (particle_id == 0)
        {
            graphProton2->SetPoint(graphProton2->GetN(), xValue2, yValue2);
        }
        else if (particle_id == 1)
        {
            graphPion2->SetPoint(graphPion2->GetN(), xValue2, yValue2);
        }
        else if (particle_id == 2)
        {
            graphKaon2->SetPoint(graphKaon2->GetN(), xValue2, yValue2);
        }
    }

    // Set the point color and add to the multigraph and legend
    graphProton2->SetMarkerColor(kRed);
    graphProton2->SetMarkerStyle(1);
    graphPion2->SetMarkerColor(kBlue);
    graphPion2->SetMarkerStyle(1);
    graphKaon2->SetMarkerColor(kGreen);
    graphKaon2->SetMarkerStyle(1);

    mg2->Add(graphProton2);
    mg2->Add(graphPion2);
    mg2->Add(graphKaon2);

    legend2->AddEntry(graphProton2, "Proton", "P");
    legend2->AddEntry(graphPion2, "Pion", "P");
    legend2->AddEntry(graphKaon2, "Kaon", "P");

    mg2->SetTitle("Energy-Loss and momentum; momentum (GeV/c); Energy-Loss (keV)");
    mg2->GetYaxis()->CenterTitle(true);
    mg2->GetXaxis()->CenterTitle(true);
    mg2->Draw("AP");
    mg2->GetXaxis()->SetRangeUser(0.25, 0.85);

    // Change legend text colors
    TList *legendEntries = legend->GetListOfPrimitives();
    for (int i = 0; i < legendEntries->GetSize(); ++i)
    {
        TLegendEntry *entry = (TLegendEntry *)legendEntries->At(i);
        if (entry->GetLabel() == std::string("Proton"))
        {
            entry->SetTextColor(kRed);
        }
        else if (entry->GetLabel() == std::string("Pion"))
        {
            entry->SetTextColor(kBlue);
        }
        else if (entry->GetLabel() == std::string("Kaon"))
        {
            entry->SetTextColor(kGreen);
        }
    }

    // Change legend text colors
    TList *legendEntries2 = legend2->GetListOfPrimitives();
    for (int i = 0; i < legendEntries2->GetSize(); ++i)
    {
        TLegendEntry *entry2 = (TLegendEntry *)legendEntries2->At(i);
        if (entry2->GetLabel() == std::string("Proton"))
        {
            entry2->SetTextColor(kRed);
        }
        else if (entry2->GetLabel() == std::string("Pion"))
        {
            entry2->SetTextColor(kBlue);
        }
        else if (entry2->GetLabel() == std::string("Kaon"))
        {
            entry2->SetTextColor(kGreen);
        }
    }

    legend2->Draw();
    canvas2->SetGrid();
    canvas2->Draw();
    canvas2->Update();
    canvas->SaveAs("tof_mom_32.png");
    canvas2->SaveAs("ene_mom_32.png");
}
