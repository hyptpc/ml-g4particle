// ====================================================================
//   RCHit.cc
//
// ====================================================================
#include "RCHit.hh"
#include "G4SDManager.hh"
#include "RCParameterisation.hh"


namespace
{
  using CLHEP::eplus;
  using CLHEP::GeV;
  using CLHEP::keV;
  using CLHEP::MeV;
  using CLHEP::ns;
  // const auto& gConf = ConfMan::GetInstance();
}

// allocator
G4Allocator<RCHit> RCHitAllocator;

////////////////////
RCHit::RCHit()
  : id(-1), edep(0.), mom(0.), momx(0.), momy(0.), momz(0.), gtime(0.), particleID(-1)
////////////////////
{
}

/////////////////////////////////////////
RCHit::RCHit(G4int aid, G4double aedep, G4double amom,
	     G4double amomx, G4double amomy, G4double amomz, G4double atime, G4int aparticleID)
  : id(aid), edep(aedep), mom(amom), momx(amomx),momy(amomy),momz(amomz),gtime(atime), particleID(aparticleID)
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
  // debug test
  // G4cout << "RCHit: id=" << id << " edep=" << edep/MeV << " MeV" << " mom=" << mom/MeV << " MeV" << " momx=" << momx/MeV << " MeV" << " momy=" << momy/MeV << " MeV" << " momz=" << momz/MeV << " MeV" << " gtime=" << gtime/ns << " ns" << " particleID=" << particleID << G4endl;
}

