// ====================================================================
//   PrimaryGeneratorAction.cc
//
// ====================================================================
#include "PrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "Randomize.hh"

// ====================================================================
//
// class description
//
// ====================================================================

namespace
{
  using CLHEP::cm;
  using CLHEP::MeV;
}

////////////////////////////////////////////////
PrimaryGeneratorAction::PrimaryGeneratorAction()
////////////////////////////////////////////////
{
  particleGun = new G4ParticleGun;

  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  // G4ParticleDefinition *particle = particleTable->FindParticle(particleName = "proton");
  // G4ParticleDefinition *particle = particleTable->FindParticle(particleName = "pi-");
  G4ParticleDefinition *particle = particleTable->FindParticle(particleName = "kaon-");

  particleGun->SetParticleDefinition(particle);
  particleGun->SetParticleMomentumDirection(G4ThreeVector(1., 0., 0.));
  particleGun->SetParticleEnergy(50. * MeV); // dummy
  // G4double position = -0.5*(ExN03Detector->GetWorldSizeX());
  // particleGun->SetParticlePosition(G4ThreeVector(position,0.*cm,0.*cm));
}

/////////////////////////////////////////////////
PrimaryGeneratorAction::~PrimaryGeneratorAction()
/////////////////////////////////////////////////
{

  delete particleGun;
}

////////////////////////////////////////////////////////////////
void PrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent)
////////////////////////////////////////////////////////////////
{

  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  // Get particle mass
  double particle_mass = particle->GetPDGMass();

  // Generate a random momentum between 0.3 GeV/c and 0.8 GeV/c
  G4double mom_abs = 300. + G4UniformRand() * 500.;
  G4ThreeVector mom(mom_abs, 0., 0.);

  // Set the particle momentum and position
  G4double x0 = -3. * cm; // target position
  G4double y0 = 0. * cm;
  G4double z0 = 0. * cm;
  particleGun->SetParticleMomentum(mom * MeV);
  particleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
  particleGun->GeneratePrimaryVertex(anEvent);
}
