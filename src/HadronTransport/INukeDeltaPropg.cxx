//____________________________________________________________________________
/*
 Copyright (c) 2003-2017, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         University of Liverpool & STFC Rutherford Appleton Lab

 For the class documentation see the corresponding header file.

 Important revisions after version 2.0.0 :
 @ Oct 01, 2009 - CA
   Was first added in v2.5.1

*/
//____________________________________________________________________________

#include <TLorentzVector.h>
#include <TVector3.h>

#include "Algorithm/AlgConfigPool.h"
#include "EVGCore/EVGThreadException.h"
#include "GHEP/GHepRecord.h"
#include "GHEP/GHepParticle.h"
#include "HadronTransport/INukeDeltaPropg.h"
#include "HadronTransport/INukeUtils.h"
#include "Messenger/Messenger.h"
#include "Numerical/RandomGen.h"
#include "PDG/PDGCodes.h"
#include "Utils/NuclearUtils.h"

using namespace genie;

//___________________________________________________________________________
INukeDeltaPropg::INukeDeltaPropg() :
EventRecordVisitorI("genie::INukeDeltaPropg")
{

}
//___________________________________________________________________________
INukeDeltaPropg::INukeDeltaPropg(string config) :
EventRecordVisitorI("genie::INukeDeltaPropg", config)
{

}
//___________________________________________________________________________
INukeDeltaPropg::~INukeDeltaPropg()
{

}
//___________________________________________________________________________
void INukeDeltaPropg::ProcessEventRecord(GHepRecord * event) const
{
  // Check that we have an interaction with a nuclear target. If not skip...
  GHepParticle * nucltgt = event->TargetNucleus();
  if (!nucltgt) {
    LOG("INukeDelta", pINFO)
       << "No nuclear target. Skipping....";
    return;
  }
  
  // mass number, nuclear radius, step size
  int A = nucltgt->A();
  double step_sz     = fHadStep;
  double nucl_radius = utils::nuclear::Radius(A,fR0);
  nucl_radius *= fNR; // track the particle further out

  RandomGen * rnd = RandomGen::Instance();

  // Loop over GHEP rescatter handled particles
  TObjArrayIter piter(event);
  GHepParticle * p = 0;
  int icurr = -1;
     
  while( (p = (GHepParticle *) piter.Next()) )
  {
    icurr++;

    // handle?     
    int pdgc = p->Pdg();
    bool delta = (pdgc == kPdgP33m1232_DeltaPP);
    if (!delta) return;
    GHepStatus_t ist = p->Status();
    bool in_nucleus = (ist == kIStHadronInTheNucleus);
    if (!in_nucleus) return;
     
    LOG("INukeDelta", pNOTICE)
        << " >> Stepping a " << p->Name()
        << " with kinetic E = " << p->KinE() << " GeV";
          
    // Rescatter a clone, not the original particle
    GHepParticle * sp = new GHepParticle(*p);
        
    // Set clone's mom to be the hadron that was cloned
    sp->SetFirstMother(icurr);

    // Start stepping particle out of the nucleus
    bool has_interacted = false;
    bool has_decayed    = false;
    while (1) {

      const TLorentzVector & p4 = *(sp->P4());
      const TLorentzVector & x4 = *(sp->X4());

      bool is_in = (x4.Vect().Mag() < nucl_radius + step_sz);
      if (!is_in) break;

      // step
      utils::intranuke::StepParticle(sp, step_sz, nucl_radius);

      // check whether it decayed at this step
      double Ldec =  0.; // calculate
      double ddec = -1. * Ldec * TMath::Log(rnd->RndFsi().Rndm()); 
      has_decayed = (ddec < step_sz);
      if(has_decayed) break;

      // check whether it interacted at this step
      double Lint = utils::intranuke::MeanFreePath_Delta(pdgc,x4,p4,A);
      double dint = -1. * Lint * TMath::Log(rnd->RndFsi().Rndm()); 
      has_interacted = (dint < step_sz);
      if(has_interacted) break;

    }//stepping

    if(has_decayed) {
       // the particle decays 

    }
    else
    if(has_interacted)  {
       // the particle interacts - simulate the hadronic interaction
       LOG("INukeDelta", pINFO)
          << "Particle has interacted at location:  "
          << sp->X4()->Vect().Mag() << " / nucl radius = " << nucl_radius;

       //
       // *** Temporary / Used in test mode only ***
       // For now, just change the Delta++ to a proton to prevent the 
       // Delta++ decay later on.
       // Causes energy and charge non-conservation
       //
       sp->SetPdgCode(kPdgProton);
       sp->SetStatus(kIStHadronInTheNucleus);
       event->AddParticle(*sp);
    }
    else {
       // the particle escapes the nucleus
       LOG("Intranuke", pNOTICE)
          << "*** Hadron escaped the nucleus! Done with it.";
      sp->SetStatus(kIStStableFinalState);
      event->AddParticle(*sp);
    }//decay? interacts? escapes?

  }//particle loop
}
//___________________________________________________________________________
void INukeDeltaPropg::Configure(const Registry & config)
{
  Algorithm::Configure(config);
  this->LoadConfig();
}
//___________________________________________________________________________
void INukeDeltaPropg::Configure(string config)
{
  Algorithm::Configure(config);
  this->LoadConfig();
}
//___________________________________________________________________________
void INukeDeltaPropg::LoadConfig (void)
{
  AlgConfigPool * confp = AlgConfigPool::Instance();
  const Registry * gc = confp->GlobalParameterList();

  fR0      = fConfig->GetDoubleDef ("R0",      gc->GetDouble("NUCL-R0"));       // fm
  fNR      = fConfig->GetDoubleDef ("NR",      gc->GetDouble("NUCL-NR"));
  fHadStep = fConfig->GetDoubleDef ("HadStep", gc->GetDouble("INUKE-HadStep")); // fm
}
//___________________________________________________________________________

