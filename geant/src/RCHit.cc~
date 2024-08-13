// ====================================================================
//   RCHit.cc
//
// ====================================================================
#include "RCHit.hh"

// allocator
G4Allocator<RCHit> RCHitAllocator;

////////////////////
RCHit::RCHit()
  : id(-1), edep(0.), mom(0.), momx(0.), momy(0.), momz(0.)
////////////////////
{
}

/////////////////////////////////////////
RCHit::RCHit(G4int aid, G4double aedep, G4double amom,
	     G4double amomx, G4double amomy, G4double amomz)
  : id(aid), edep(aedep), mom(amom), momx(amomx),momy(amomy),momz(amomz)
/////////////////////////////////////////
{
}

/////////////////
RCHit::~RCHit()
/////////////////
{
}


///////////////////
void RCHit::Draw()
///////////////////
{
}

////////////////////
void RCHit::Print()
////////////////////
{
  G4cout << "RC Hit:" << id << "::" 
	 << edep/MeV << " MeV, Mom::" << mom/MeV<<G4endl;
}

