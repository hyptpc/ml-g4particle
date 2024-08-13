// ====================================================================
//    RCSim.cc
//
// ====================================================================
#include "G4RunManager.hh"
#include "G4UIterminal.hh"
#ifdef G4UI_USE_TCSH
#include "G4UItcsh.hh"
#endif
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
//#include "QGSP.hh"
#include "QGSP_BERT.hh"
#include "RunAction.hh"
#include "EventAction.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

// ====================================================================
//     main
// ====================================================================
int main(int argc, char** argv) 
{
  // run manager
  G4RunManager* runManager= new G4RunManager;  
  G4cout << G4endl;

  // set mandatory user initialization classes...
  // detector setup
  runManager-> SetUserInitialization(new DetectorConstruction);
  // particles and physics processes
//  runManager-> SetUserInitialization(new QGSP);
  runManager-> SetUserInitialization(new QGSP_BERT);
  // primary generator
  runManager-> SetUserAction(new PrimaryGeneratorAction);

  // set user acttions
 runManager-> SetUserAction(new RunAction);
 runManager-> SetUserAction(new EventAction);

#ifdef G4VIS_USE
  // initialize visualization package
  G4VisManager* visManager= new G4VisExecutive;
  visManager-> Initialize();
  G4cout << G4endl;
#endif

  // user session...
  runManager-> Initialize();
    
  if(argc==1) { // interactive session, if no arguments given
#ifdef G4UI_USE_TCSH
    // tcsh-like
    G4UItcsh* tcsh= new 
      G4UItcsh("RCSim(%s)[%/]:");
    G4UIterminal* session= new G4UIterminal(tcsh);
#else
    // csh-like
    G4UIterminal* session= new G4UIterminal();
    session-> SetPrompt("RCSim_csh(%s)[%/]:");
#endif
    session-> SessionStart();
    delete session;
  } else { // batch mode
    G4UImanager* UImanager= G4UImanager::GetUIpointer();
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager-> ApplyCommand(command+fileName);
  }

  // terminating...
#ifdef G4VIS_USE
  delete visManager;
#endif

  delete runManager;  G4cout << G4endl;
  return 0;
}
