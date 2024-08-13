// Set detectors

#include <stdio.h>
#include <iostream>
#include <TString.h>
#include "DetectorConstruction.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4UniformMagField.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4PVParameterised.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "RCParameterisation.hh"
#include "G4SDManager.hh"
#include "RCSD.hh"
#include <fstream>

namespace
{
  using CLHEP::atmosphere;
  using CLHEP::bar;
  using CLHEP::cm;
  using CLHEP::cm3;
  using CLHEP::deg;
  using CLHEP::eV;
  using CLHEP::g;
  using CLHEP::kelvin;
  using CLHEP::m;
  using CLHEP::mg;
  using CLHEP::mm;
  using CLHEP::mole;
  using CLHEP::pascal;
  using CLHEP::perCent;
  using CLHEP::STP_Temperature;
  using CLHEP::universe_mean_density;

  // const auto& gConf = ConfMan::GetInstance();
  // const auto& gGeom = DCGeomMan::GetInstance();
  // const auto& gSize = DetSizeMan::GetInstance();
  // TPCField* myField = nullptr;
  // color
  const G4Colour AQUA(0.247, 0.8, 1.0);
  const G4Colour ORANGE(1.0, 0.55, 0.0);
  const G4Colour LAVENDER(0.901, 0.901, 0.98);
  const G4Colour MAROON(0.5, 0.0, 0.0);
  const G4Colour PINK(1.0, 0.753, 0.796);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
    : AbsorberMaterial(0), GapMaterial(0), defaultMaterial(0),
      solidWorld(0), logicWorld(0), physiWorld(0),
      solidCalor(0), logicCalor(0), physiCalor(0),
      solidLayer(0), logicLayer(0), physiLayer(0),
      solidAbsorber(0), logicAbsorber(0), physiAbsorber(0),
      solidGap(0), logicGap(0), physiGap(0),
      magField(0)
{

  // default parameter values of the calorimeter
  AbsorberThickness = 0. * mm; // %%%%%%%%%%%%%%%%%
  GapThickness = 0. * mm;      // %%%%%%%%%%%%%%%%%
  NbOfLayers = 10;             // %%%%%%%%%%%%%%%%%
  CalorSizeYZ = 10. * cm;
  ComputeCalorParameters();

  // materials
  DefineMaterials();
  SetAbsorberMaterial("P10");
  SetGapMaterial("Vaccum");
  SetRCMaterial("P10"); // %%%%%%%%%%%%%%
  // SetRCMaterial("Carbon"); // %%%%%%%%%%%%%%

  // create commands for interactive definition of the calorimeter
  // detectorMessenger = new DetectorMessenger(this);

  //  std::ifstream det_param_file;
  // det_param_file.open("DetPar/det_param1.par");
  // det_param_file >> NbOfLayers;

  //  det_param_file.close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::Construct()
{

  // Clean old geometry, if any
  //

  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // complete the Calor parameters definition
  ComputeCalorParameters();

  //
  // World
  //
  solidWorld = new G4Box("World", 100. / 2.0 * cm, 100. / 2.0 * cm, 100. / 2.0 * cm); // X/2, Y/2, Z/2

  logicWorld = new G4LogicalVolume(solidWorld,      // its solid
                                   defaultMaterial, // its material
                                   "World");        // its name

  physiWorld = new G4PVPlacement(0,               // no rotation
                                 G4ThreeVector(), // at (0,0,0)
                                 logicWorld,      // its logical volume
                                 "World",         // its name
                                 0,               // its mother  volume
                                 false,           // no boolean operation
                                 0);              // copy number

  G4Material *P10_Mat = G4Material::GetMaterial("P10");
  G4Material *Vacuum_Mat = G4Material::GetMaterial("Vacuum");

  // 32 layers in total (10 thin layer + 22 thick layer)
  G4double thick[32];
  for (int i = 0; i < 10; ++i)
  {
    thick[i] = ( 9. / 2.0 )* mm; // thin layers (9 mm)
  }
  for (int i = 10; i < 32; ++i)
  {
    thick[i] = ( 12.5 / 2.0 )* mm; // thick layers (12.5 mm)
  }
  const G4double thick_vp = 0.1 / 2.0 * mm; // virtual plane thickness (0.1 mm)

  G4double xpos[32];
  xpos[0] = (0.1 + 9.0 / 2.) * mm; // layer1 position
  for (int i = 1; i < 10; ++i)
  {
    xpos[i] = xpos[0] + (9.1 * i) * mm; // position of other thin layers
  }
  xpos[10] = xpos[9] + 10.85 * mm; // layer 11 position
  for (int i = 11; i < 32; ++i)
  {
    xpos[i] = xpos[10] + (12.6 * (i - 10)) * mm; // position of other thick layers
  }

  // -----------------------------------------
  // define thin pad layers
  // -----------------------------------------
  G4Box *solidPad1[10];
  G4LogicalVolume *logPad1[10];
  G4VPhysicalVolume *physPad1[10];
  for (int i = 0; i < 10; ++i)
  {
    solidPad1[i] = new G4Box(Form("pad1_%d", i), thick[i], 280. / 2.0 * mm, 550. / 2.0 * mm);
    logPad1[i] = new G4LogicalVolume(solidPad1[i], P10_Mat, Form("pad1_%d", i), 0, 0, 0);
    G4ThreeVector posPad1 = G4ThreeVector(xpos[i], 0., 0.);
    G4VPhysicalVolume *physPad1 =
        new G4PVPlacement(0, posPad1, logPad1[i], Form("pad1_%d", i), logicWorld, false, i * 2 + 1);
  }

  // -----------------------------------------
  // define thick pad layers
  // -----------------------------------------
  G4Box *solidPad2[22];
  G4LogicalVolume *logPad2[22];
  G4VPhysicalVolume *physPad2[22];
  for (int i = 0; i < 22; ++i)
  {
    solidPad2[i] = new G4Box(Form("pad2_%d", i), thick[10 + i], 280. / 2.0 * mm, 550. / 2.0 * mm);
    logPad2[i] = new G4LogicalVolume(solidPad2[i], P10_Mat, Form("pad2_%d", i), 0, 0, 0);
    G4ThreeVector posPad2 = G4ThreeVector(xpos[10 + i], 0., 0.);
    G4VPhysicalVolume *physPad2 =
        new G4PVPlacement(0, posPad2, logPad2[i], Form("pad2_%d", i), logicWorld, false, (i + 10) * 2 + 1);
  }

  // -----------------------------------------
  // define virtual planes
  // -----------------------------------------
  G4Box *solidVP[32]; // VP in front of pads
  G4LogicalVolume *logVP[32];

  for (int i = 0; i < 32; ++i)
  {
    solidVP[i] = new G4Box(Form("VP_%d", i),
                           thick_vp, 280. / 2.0 * mm, 550. / 2.0 * mm);
    logVP[i] = new G4LogicalVolume(solidVP[i], Vacuum_Mat,
                                   Form("VP_%d", i), 0, 0, 0);
    G4ThreeVector posVP = G4ThreeVector(xpos[i] - thick[i] - thick_vp, 0., 0.);
    new G4PVPlacement(0, posVP, logVP[i],
                      Form("VP_%d", i), logicWorld, false,
                      i * 2);
  }

  G4Box *solidVP_tof; // VP in front of TOF
  G4LogicalVolume *logVP_tof;

  solidVP_tof = new G4Box("VP_tof",
                          thick_vp, 280. / 2.0 * mm, 550. / 2.0 * mm);
  logVP_tof = new G4LogicalVolume(solidVP_tof, Vacuum_Mat,
                                  "VP_tof", 0, 0, 0);
  G4ThreeVector posVP_tof = G4ThreeVector(453 * mm, 0., 0.);
  new G4PVPlacement(0, posVP_tof, logVP_tof,
                    "VP_tof", logicWorld, false,
                    64);

  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // define senstive detector
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  G4SDManager *SDmanager = G4SDManager::GetSDMpointer();
  RCSD *rcsd = new RCSD("rangecounter");
  SDmanager->AddNewDetector(rcsd);
  for (int i = 0; i < 10; ++i)
  {
    logPad1[i]->SetSensitiveDetector(rcsd);
  }
  for (int i = 0; i < 22; ++i)
  {
    logPad2[i]->SetSensitiveDetector(rcsd);
  }
  for (int i = 0; i < 32; ++i)
  {
    logVP[i]->SetSensitiveDetector(rcsd);
  }
  logVP_tof->SetSensitiveDetector(rcsd);

  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // define visualization
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  logicWorld->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes *simpleBoxVisAtt = new G4VisAttributes(G4Colour(1.0, 1.5, 1.5));
  G4VisAttributes *VPBoxVisAtt = new G4VisAttributes(AQUA);

  simpleBoxVisAtt->SetVisibility(true);
  VPBoxVisAtt->SetVisibility(true);
  for (int i = 0; i < 10; ++i)
  {
    logPad1[i]->SetVisAttributes(simpleBoxVisAtt);
  }
  for (int i = 0; i < 22; ++i)
  {
    logPad2[i]->SetVisAttributes(simpleBoxVisAtt);
  }
  for (int i = 0; i < 32; ++i)
  {
    logVP[i]->SetVisAttributes(VPBoxVisAtt);
  }
  logVP_tof->SetVisAttributes(VPBoxVisAtt);
  return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{
  // This function illustrates the possible ways to define materials

  G4String symbol;        // a=mass of a mole;
  G4double a, z, density; // z=mean number of protons;
  G4int iz, n;            // iz=number of protons in an isotope;
  // n=number of nucleons in an isotope;

  G4int ncomponents, natoms;
  G4double abundance, fractionmass;

  // ---------------
  // define Elements
  // ---------------

  G4Element *H = new G4Element("Hydrogen", symbol = "H", z = 1., a = 1.01 * g / mole);
  G4Element *C = new G4Element("Carbon", symbol = "C", z = 6., a = 12.01 * g / mole);
  G4Element *N = new G4Element("Nitrogen", symbol = "N", z = 7., a = 14.01 * g / mole);
  G4Element *O = new G4Element("Oxygen", symbol = "O", z = 8., a = 16.00 * g / mole);

  // ----------------------
  // define simple materials
  // ----------------------
  const G4double dens_Ar = 1.782 * mg / cm3;
  const G4double dens_CH4 = 0.717 * mg / cm3;
  const G4double dens_P10 = dens_Ar * 0.9 + dens_CH4 * 0.1;
  const G4double temperature_TPC = 300. * kelvin;
  const G4double pressure_TPC = 1. * atmosphere;

  G4Material *ArgonGas =
      new G4Material("ArgonGas", z = 18, a = 39.948 * g / mole, dens_Ar, kStateGas, temperature_TPC, pressure_TPC);

  G4Material *CH4 =
      new G4Material("CH4", dens_CH4, ncomponents = 2, kStateGas, temperature_TPC, pressure_TPC);
  CH4->AddElement(C, natoms = 1);
  CH4->AddElement(H, natoms = 4);

  G4Material *P10 =
      new G4Material("P10", dens_P10, ncomponents = 2, kStateGas, temperature_TPC, pressure_TPC);
  P10->AddMaterial(ArgonGas, fractionmass = 90.0 * perCent);
  P10->AddMaterial(CH4, fractionmass = 10.0 * perCent);

  //
  // examples of vacuum
  //
  G4Material *Vacuum =
      new G4Material("Vacuum", density = universe_mean_density,
                     ncomponents = 2);
  Vacuum->AddElement(N, 0.7);
  Vacuum->AddElement(O, 0.3);

  G4Material *beam =
      new G4Material("Beam", density = 1.e-5 * g / cm3, ncomponents = 1,
                     kStateGas, STP_Temperature, 2.e-2 * bar);
  // beam->AddMaterial(Air, fractionmass = 1.);
  beam->AddMaterial(Vacuum, fractionmass = 1.);

  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  // default materials of the World
  // defaultMaterial = Air;
  defaultMaterial = Vacuum;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::ConstructCalorimeter()
{
  // void DetectorConstruction::ConstructCalorimeter(G4VPhysicalVolume *pMother){
  //
  //  Calorimeter
  //
  solidCalor = 0;
  logicCalor = 0;
  physiCalor = 0;
  solidLayer = 0;
  logicLayer = 0;
  physiLayer = 0;

  if (CalorThickness > 0.)
  {
    solidCalor = new G4Box("Calorimeter",                                         // its name
                           CalorThickness / 2, CalorSizeYZ / 2, CalorSizeYZ / 2); // size

    logicCalor = new G4LogicalVolume(solidCalor,      // its solid
                                     defaultMaterial, // its material
                                     "Calorimeter");  // its name

    physiCalor = new G4PVPlacement(0,               // no rotation
                                   G4ThreeVector(), // at (0,0,0)
                                   logicCalor,      // its logical volume
                                   "Calorimeter",   // its name
                                   logicWorld,      // its mother  volume
                                   false,           // no boolean operation
                                   0);              // copy number

    //
    // Layer
    //
    solidLayer = new G4Box("Layer",                                               // its name
                           LayerThickness / 2, CalorSizeYZ / 2, CalorSizeYZ / 2); // size

    logicLayer = new G4LogicalVolume(solidLayer,      // its solid
                                     defaultMaterial, // its material
                                     "Layer");        // its name
    if (NbOfLayers > 1)
      physiLayer = new G4PVReplica("Layer",         // its name
                                   logicLayer,      // its logical volume
                                   logicCalor,      // its mother
                                   kXAxis,          // axis of replication
                                   NbOfLayers,      // number of replica
                                   LayerThickness); // witdth of replica
    else
      physiLayer = new G4PVPlacement(0,               // no rotation
                                     G4ThreeVector(), // at (0,0,0)
                                     logicLayer,      // its logical volume
                                     "Layer",         // its name
                                     logicCalor,      // its mother  volume
                                     false,           // no boolean operation
                                     0);              // copy number
  }

  //
  // Absorber
  //
  solidAbsorber = 0;
  logicAbsorber = 0;
  physiAbsorber = 0;

  if (AbsorberThickness > 0.)
  {
    solidAbsorber = new G4Box("Absorber", // its name
                              AbsorberThickness / 2, CalorSizeYZ / 2, CalorSizeYZ / 2);

    logicAbsorber = new G4LogicalVolume(solidAbsorber,                // its solid
                                        AbsorberMaterial,             // its material
                                        AbsorberMaterial->GetName()); // name

    physiAbsorber = new G4PVPlacement(0,                                        // no rotation
                                      G4ThreeVector(-GapThickness / 2, 0., 0.), // its position
                                      logicAbsorber,                            // its logical volume
                                      AbsorberMaterial->GetName(),              // its name
                                      logicLayer,                               // its mother
                                      false,                                    // no boulean operat
                                      0);                                       // copy number
  }

  //
  // Gap
  //
  solidGap = 0;
  logicGap = 0;
  physiGap = 0;

  if (GapThickness > 0.)
  {
    solidGap = new G4Box("Gap",
                         GapThickness / 2, CalorSizeYZ / 2, CalorSizeYZ / 2);

    logicGap = new G4LogicalVolume(solidGap,
                                   GapMaterial,
                                   GapMaterial->GetName());

    physiGap = new G4PVPlacement(0,                                            // no rotation
                                 G4ThreeVector(AbsorberThickness / 2, 0., 0.), // its position
                                 logicGap,                                     // its logical volume
                                 GapMaterial->GetName(),                       // its name
                                 logicLayer,                                   // its mother
                                 false,                                        // no boulean operat
                                 0);                                           // copy number
  }

  PrintCalorParameters();

  //
  // Visualization attributes
  //

  /*
   // Below are vis attributes that permits someone to test / play
   // with the interactive expansion / contraction geometry system of the
   // vis/OpenInventor driver :
  {G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,0.0));
   simpleBoxVisAtt->SetVisibility(true);
   delete logicCalor->GetVisAttributes();
   logicCalor->SetVisAttributes(simpleBoxVisAtt);}

  {G4VisAttributes* atb= new G4VisAttributes(G4Colour(1.0,0.0,0.0));
   logicLayer->SetVisAttributes(atb);}

  {G4VisAttributes* atb= new G4VisAttributes(G4Colour(0.0,1.0,0.0));
   atb->SetForceSolid(true);
   logicAbsorber->SetVisAttributes(atb);}

  {//Set opacity = 0.2 then transparency = 1 - 0.2 = 0.8
   G4VisAttributes* atb= new G4VisAttributes(G4Colour(0.0,0.0,1.0,0.2));
   atb->SetForceSolid(true);
   logicGap->SetVisAttributes(atb);}
   */

  //
  // always return the physical World
  // 0.18 g/cm3
  //  return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::PrintCalorParameters()
{
  G4cout << "\n------------------------------------------------------------"
         << "\n---> The calorimeter is " << NbOfLayers << " layers of: [ "
         << AbsorberThickness / mm << "mm of " << AbsorberMaterial->GetName()
         << " + "
         << GapThickness / mm << "mm of " << GapMaterial->GetName() << " ] "
         << "\n------------------------------------------------------------\n";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material *pttoMaterial = G4Material::GetMaterial(materialChoice);
  if (pttoMaterial)
    AbsorberMaterial = pttoMaterial;
}

void DetectorConstruction::SetRCMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material *pttoMaterial = G4Material::GetMaterial(materialChoice);
  if (pttoMaterial)
    RCMaterial = pttoMaterial;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetGapMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material *pttoMaterial = G4Material::GetMaterial(materialChoice);
  if (pttoMaterial)
    GapMaterial = pttoMaterial;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberThickness(G4double val)
{
  // change Absorber thickness and recompute the calorimeter parameters
  AbsorberThickness = val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetGapThickness(G4double val)
{
  // change Gap thickness and recompute the calorimeter parameters
  GapThickness = val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetCalorSizeYZ(G4double val)
{
  // change the transverse size and recompute the calorimeter parameters
  CalorSizeYZ = val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetNbOfLayers(G4int val)
{
  NbOfLayers = val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

void DetectorConstruction::SetMagField(G4double fieldValue)
{
  // apply a global uniform magnetic field along Z axis
  G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();

  if (magField)
    delete magField; // delete the existing magn field

  if (fieldValue != 0.) // create a new one if non nul
  {
    magField = new G4UniformMagField(G4ThreeVector(0., 0., fieldValue));
    fieldMgr->SetDetectorField(magField);
    fieldMgr->CreateChordFinder(magField);
  }
  else
  {
    magField = 0;
    fieldMgr->SetDetectorField(magField);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4RunManager.hh"

void DetectorConstruction::UpdateGeometry()
{
  //  G4RunManager::GetRunManager()->DefineWorldVolume(physiWorld);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
