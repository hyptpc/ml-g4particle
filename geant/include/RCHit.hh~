// ====================================================================
//   RCHit.hh
//
// ====================================================================
#ifndef RC_HIT_H
#define RC_HIT_H
 
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class RCHit : public G4VHit {
private:
  G4int id;
  G4double edep;
  G4double mom;

public:
  RCHit();
  RCHit(G4int aid, G4double aedep, G4double amom);
  virtual ~RCHit();

  // copy constructor & assignment operator
  RCHit(const RCHit& right);
  const RCHit& operator=(const RCHit& right);
  G4int operator==(const RCHit& right) const;
  
  // new/delete operators
  void* operator new(size_t);
  void operator delete(void* aHit);
  
  // set/get functions
  void SetID(G4int aid) { id = aid; }
  G4int GetID() const { return id; }

  void SetEdep(G4double aedep) { edep = aedep; }
  G4double GetEdep() const { return edep; }

  void SetMom(G4double amom) { mom = amom; }
  G4double GetMom() const { return mom; }
  // methods
  virtual void Draw();
  virtual void Print();
};

// ====================================================================
// inline functions
// ====================================================================
inline RCHit::RCHit(const RCHit& right)
  : G4VHit()
{
  id= right.id;
  edep= right.edep;
  mom= right.mom;
}

inline const RCHit& RCHit::operator=(const RCHit& right)
{
  id= right.id;
  edep= right.edep;
  mom= right.mom;
  return *this;
}

inline G4int RCHit::operator==(const RCHit& right) const 
{
   return (this==&right) ? 1 : 0; 
}

typedef G4THitsCollection<RCHit> RCHitsCollection;
extern G4Allocator<RCHit> RCHitAllocator; 

inline void* RCHit::operator new(size_t)
{
  void* aHit= (void*)RCHitAllocator.MallocSingle();
  return aHit;
}

inline void RCHit::operator delete(void* aHit)
{
  RCHitAllocator.FreeSingle((RCHit*) aHit);
}

#endif
