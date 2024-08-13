// ====================================================================
//   CalorimeterSD.hh
//
// ====================================================================
#ifndef CALORIMETER_SD_H
#define CALORIMETER_SD_H
 
#include "G4VSensitiveDetector.hh"
#include "CalHit.hh"

class G4HCofThisEvent;
class G4Step;
class G4TouchableHistory;

enum { NCHANNEL=50 };

class CalorimeterSD : public G4VSensitiveDetector {
private:
  CalHitsCollection* hitsCollection;
  G4double edepbuf[NCHANNEL]; // buffer for energy deposit

public:
  CalorimeterSD(const G4String& name);
  virtual ~CalorimeterSD();

  // virtual methods
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  virtual void Initialize(G4HCofThisEvent* HCTE);
  virtual void EndOfEvent(G4HCofThisEvent* HCTE);

  virtual void DrawAll();
  virtual void PrintAll(); 
 
};

#endif
