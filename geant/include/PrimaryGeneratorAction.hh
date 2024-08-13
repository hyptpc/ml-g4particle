// ====================================================================
//    PrimaryGeneratorAction.hh
//
// ====================================================================
#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"

class G4ParticleGun;

// ====================================================================
//
// class definition
//
// ====================================================================

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
private:
  // use G4 particle gun
  G4ParticleGun* particleGun;
  G4ParticleDefinition* particle;

public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event* anEvent);

};

#endif
