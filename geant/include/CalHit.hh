// ====================================================================
//   CalHit.hh
//
// ====================================================================
#ifndef CAL_HIT_H
#define CAL_HIT_H
 
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class CalHit : public G4VHit {
private:
  G4int id;
  G4double edep;

public:
  CalHit();
  CalHit(G4int aid, G4double aedep);
  virtual ~CalHit();

  // copy constructor & assignment operator
  CalHit(const CalHit& right);
  const CalHit& operator=(const CalHit& right);
  G4int operator==(const CalHit& right) const;
  
  // new/delete operators
  void* operator new(size_t);
  void operator delete(void* aHit);
  
  // set/get functions
  void SetID(G4int aid) { id = aid; }
  G4int GetID() const { return id; }

  void SetEdep(G4double aedep) { edep = aedep; }
  G4double GetEdep() const { return edep; }

  // methods
  virtual void Draw();
  virtual void Print();
};

// ====================================================================
// inline functions
// ====================================================================
inline CalHit::CalHit(const CalHit& right)
  : G4VHit()
{
  id= right.id;
  edep= right.edep;
}

inline const CalHit& CalHit::operator=(const CalHit& right)
{
  id= right.id;
  edep= right.edep;
  return *this;
}

inline G4int CalHit::operator==(const CalHit& right) const 
{
   return (this==&right) ? 1 : 0; 
}

typedef G4THitsCollection<CalHit> CalHitsCollection;
extern G4Allocator<CalHit> CalHitAllocator; 

inline void* CalHit::operator new(size_t)
{
  void* aHit= (void*)CalHitAllocator.MallocSingle();
  return aHit;
}

inline void CalHit::operator delete(void* aHit)
{
  CalHitAllocator.FreeSingle((CalHit*) aHit);
}

#endif
