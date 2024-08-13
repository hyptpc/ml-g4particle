// ====================================================================
//   RCSD.hh
//
// ====================================================================
#ifndef RC_SD_H
#define RC_SD_H
 
#include "G4VSensitiveDetector.hh"
#include "RCHit.hh"

class G4HCofThisEvent;
class G4Step;
class G4TouchableHistory;

enum { NCHANNEL=100 };

class RCSD : public G4VSensitiveDetector {
private:
  RCHitsCollection* hitsCollection;
  G4double edepbuf[NCHANNEL]; // buffer for energy deposit
  G4double mombuf[NCHANNEL]; // buffer for energy deposit
  G4double momxbuf[NCHANNEL]; // buffer for energy deposit
  G4double momybuf[NCHANNEL]; // buffer for energy deposit
  G4double momzbuf[NCHANNEL]; // buffer for energy deposit
  G4double timebuf[NCHANNEL];
  G4int particleIDbuf[NCHANNEL];

public:
  RCSD(const G4String& name);
  virtual ~RCSD();

  // virtual methods
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
  virtual void Initialize(G4HCofThisEvent* HCTE);
  virtual void EndOfEvent(G4HCofThisEvent* HCTE);
  
  virtual void DrawAll();
  virtual void PrintAll(); 
  
};

#endif
