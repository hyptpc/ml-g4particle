// ====================================================================
//   RCHit.hh
//
// ====================================================================
#ifndef RC_HIT_H
#define RC_HIT_H

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class RCHit : public G4VHit
{
private:
  G4int id;
  G4double edep;
  G4double mom;
  G4double momx;
  G4double momy;
  G4double momz;
  G4double gtime; // global time
  G4int particleID;

public:
  RCHit();
  RCHit(G4int aid, G4double aedep, G4double amom,
        G4double amomx, G4double amomy, G4double amomz, G4double atime, G4int aParticleID);
  virtual ~RCHit();

  // copy constructor & assignment operator
  RCHit(const RCHit &right);
  const RCHit &operator=(const RCHit &right);
  G4int operator==(const RCHit &right) const;

  // new/delete operators
  void *operator new(size_t);
  void operator delete(void *aHit);

  // set/get functions
  void SetID(G4int aid) { id = aid; }
  G4int GetID() const { return id; }

  void SetEdep(G4double aedep) { edep = aedep; }
  G4double GetEdep() const { return edep; }

  void SetMom(G4double amom) { mom = amom; }
  G4double GetMom() const { return mom; }

  void SetMomx(G4double amomx) { momx = amomx; }
  G4double GetMomx() const { return momx; }

  void SetMomy(G4double amomy) { momy = amomy; }
  G4double GetMomy() const { return momy; }

  void SetMomz(G4double amomz) { momz = amomz; }
  G4double GetMomz() const { return momz; }

  void SetTime(G4double atime) { gtime = atime; }
  G4double GetTime() const { return gtime; }

  void SetParticleID(G4int aParticleID) { particleID = aParticleID; }
  G4int GetParticleID() const { return particleID; }
  // methods
  virtual void Draw();
  virtual void Print();
};

// ====================================================================
// inline functions
// ====================================================================
inline RCHit::RCHit(const RCHit &right)
    : G4VHit()
{
  id = right.id;
  edep = right.edep;
  mom = right.mom;
  momx = right.momx;
  momy = right.momy;
  momz = right.momz;
  gtime = right.gtime;
  particleID = right.particleID;
}

inline const RCHit &RCHit::operator=(const RCHit &right)
{
  id = right.id;
  edep = right.edep;
  mom = right.mom;
  momx = right.momx;
  momy = right.momy;
  momz = right.momz;
  gtime = right.gtime;
  particleID = right.particleID;
  return *this;
}

inline G4int RCHit::operator==(const RCHit &right) const
{
  return (this == &right) ? 1 : 0;
}

typedef G4THitsCollection<RCHit> RCHitsCollection;
extern G4Allocator<RCHit> RCHitAllocator;

inline void *RCHit::operator new(size_t)
{
  void *aHit = (void *)RCHitAllocator.MallocSingle();
  return aHit;
}

inline void RCHit::operator delete(void *aHit)
{
  RCHitAllocator.FreeSingle((RCHit *)aHit);
}

#endif
