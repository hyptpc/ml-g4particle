// ====================================================================
//   CalorimeterSD.cc
//
// ====================================================================
#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "RCSD.hh"
#include "RCHit.hh"

//////////////////////////////////////////////////
RCSD::RCSD(const G4String &name)
    : G4VSensitiveDetector(name)
//////////////////////////////////////////////////
{
  collectionName.insert("rangecounter");
}

///////////////////////////////
RCSD::~RCSD()
///////////////////////////////
{
}

/////////////////////////////////////////////////////
void RCSD::Initialize(G4HCofThisEvent *HCTE)
/////////////////////////////////////////////////////
{
  // create hit collection(s)
  hitsCollection = new RCHitsCollection(SensitiveDetectorName,
                                        collectionName[0]);

  // push H.C. to "Hit Collection of This Event"
  G4int hcid = GetCollectionID(0);
  HCTE->AddHitsCollection(hcid, hitsCollection);

  // clear energy deposit buffer
  for (G4int i = 0; i < NCHANNEL; i++)
  {
    edepbuf[i] = 0.;
    mombuf[i] = 0.;
    momxbuf[i] = 0.;
    momybuf[i] = 0.;
    momzbuf[i] = 0.;
    timebuf[i] = 0.;
    particleIDbuf[i] = -1; // Initialize with -1
  }
}

///////////////////////////////////////////////////////
G4bool RCSD::ProcessHits(G4Step *astep,
                         G4TouchableHistory *)
///////////////////////////////////////////////////////
{
  // get step information from "PreStepPoint"
  const G4StepPoint *preStepPoint = astep->GetPreStepPoint();
  G4TouchableHistory *touchable =
      (G4TouchableHistory *)(preStepPoint->GetTouchable());
  G4Track *atrack = astep->GetTrack();
  G4ThreeVector momV = atrack->GetMomentum();
  G4double gtime = astep->GetPreStepPoint()->GetGlobalTime();
  G4int tid = astep->GetTrack()->GetTrackID();
  G4int pid = astep->GetTrack()->GetDefinition()->GetPDGEncoding();

  // accumulate energy deposit in each scintillator
  G4int id = touchable->GetReplicaNumber();
  G4int copyNo = preStepPoint->GetPhysicalVolume()->GetCopyNo();
  // std::cout<<"id="<<id<<", copyNo="<<copyNo<<", mom="<<momV.mag()
  // 	   <<", tid="<<tid<<", pid="<<pid<<std::endl;
  if (id != copyNo)
  {
    std::cout << "id=" << id << ", copyNo=" << copyNo << std::endl;
  }
  
  if (tid == 1) //collect only primary particle
    {
      mombuf[id] = momV.mag();
      momxbuf[id] = momV.x();
      momybuf[id] = momV.y();
      momzbuf[id] = momV.z();
      timebuf[id] = gtime;
      edepbuf[id] += astep->GetTotalEnergyDeposit();
      particleIDbuf[id] = pid;
    }

  // mombuf[id] = momV.mag();
  // momxbuf[id] = momV.x();
  // momybuf[id] = momV.y();
  // momzbuf[id] = momV.z();
  // timebuf[id] = gtime;
  // edepbuf[id] += astep->GetTotalEnergyDeposit();
  // particleIDbuf[id] = pid;

  return true;
}

/////////////////////////////////////////////////
void RCSD::EndOfEvent(G4HCofThisEvent *)
/////////////////////////////////////////////////
{
  // make hits and push them to "Hit Colection"
  for (G4int id = 0; id < NCHANNEL; id++)
  {
    if (edepbuf[id] > 0.)
    {
      RCHit *ahit = new RCHit(id, edepbuf[id], mombuf[id],
                              momxbuf[id], momybuf[id], momzbuf[id], timebuf[id], particleIDbuf[id]);
      hitsCollection->insert(ahit);
    }
  }
}

/////////////////////////////
void RCSD::DrawAll()
/////////////////////////////
{
}

//////////////////////////////
void RCSD::PrintAll()
//////////////////////////////
{
  hitsCollection->PrintAllHits();
}
