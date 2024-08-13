#include "RCParameterisation.hh"

#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RCParameterisation::RCParameterisation()
{
  /*
  fNoLayer = 15; 
  fStartZ     =   0. *cm; 
  
  for(int i=0;i<15;i++){
    sci_width[i]=1.0 *cm;
    fSpacing[i]=0. *cm;
  }
  */
  // fNoLayer = 8;
  // fStartZ = 0. *cm;

  // sci_width[0]=0.3 *cm;
  // sci_width[1]=1.0 *cm;
  // sci_width[2]=1.0 *cm;
  // sci_width[3]=1.5 *cm;
  // sci_width[4]=2.0 *cm;
  // sci_width[5]=2.0 *cm;
  // sci_width[6]=2.0 *cm;
  // sci_width[7]=0.5 *cm;

  // fSpacing[0]=0. *cm;
  // fSpacing[1]=0.33 *cm;
  // fSpacing[2]=0.45 *cm;
  // fSpacing[3]=0.41 *cm;
  // fSpacing[4]=0.46 *cm;
  // fSpacing[5]=0.39 *cm;
  // fSpacing[6]=0. *cm;
  // fSpacing[7]=0. *cm;

  // sci_y=100. *cm;
  // sci_z=10. *cm;


  fNoLayer = 2;
  fStartZ = 0. *cm;

  //for CH2
  sci_width[0]= 10.002 *cm;
  fSpacing[0]=0.5 *cm;
  sci_width[1]= 0.001 *cm;
  fSpacing[1]=0. *cm;

  //for Carbon
  // sci_width[0]= 5.4206 *cm;
  // fSpacing[0]=0. *cm;
  
  sci_y=5. *cm;
  sci_z=15. *cm;


  // fHalfLengthLast = lengthFinal;
  
  
  //   if( NoChambers > 0 ){
  //  fHalfLengthIncr =  0.5 * (lengthFinal-lengthInitial)/NoChambers;
  //   if (spacingZ < widthChamber) {
  //       G4Exception("RCParameterisation construction: Width>Spacing");
  //  }
  // }
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RCParameterisation::~RCParameterisation()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RCParameterisation::ComputeTransformation
(const G4int copyNo, G4VPhysicalVolume* physVol) const
{

  G4double Zposition = fStartZ + sci_width[0]/2.;
  for(int i=0; i< copyNo; ++i){
    // G4cout<<"*";
    Zposition = Zposition + sci_width[i]/2. + fSpacing[i]/2. + sci_width[i+1]/2.;
    // G4cout << "i = "<<i<<"fspace is "<<fSpacing[copyNo-1] /cm<<"Zpos = " <<Zposition /cm<<G4endl;
  }

  //  G4cout << "copy NO is " <<copyNo <<" Zpos = "<<Zposition /cm<<G4endl;
  G4ThreeVector origin(0,0,Zposition);
  physVol->SetTranslation(origin);
  physVol->SetRotation(0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RCParameterisation::ComputeDimensions
(G4Box& trackerChamber, const G4int copyNo, const G4VPhysicalVolume*) const
{
  //  G4double  halfLength= fHalfLengthFirst + copyNo * fHalfLengthIncr;
  
  G4double HalfWidth = sci_width[copyNo]/2.;
  trackerChamber.SetXHalfLength(sci_y/2.);
  trackerChamber.SetYHalfLength(sci_z/2.);
  trackerChamber.SetZHalfLength(HalfWidth);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
