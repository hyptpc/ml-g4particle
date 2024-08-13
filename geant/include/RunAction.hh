// ====================================================================
//   RunAction.hh
//
// ====================================================================
#ifndef RUN_ACTION_H
#define RUN_ACTION_H

#include "G4UserRunAction.hh"
#include "TH2D.h"
#include "TTree.h"
#include "globals.hh"

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

class G4Run;
class TH1D;
class TTree;
class TH2D;


class RunAction : public G4UserRunAction {
private:
  TH1D* hist_shower;

  TTree * tree;
  //  double beta1;




  TH2D *h2_layer_beta;
  TH2D *h2_layer_beta_had;
  TH2D *h2_layer_beta_nohad;




  
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run* aRun);
  virtual void EndOfRunAction(const G4Run* aRun);
};

#endif
