//____________________________________________________________________________
/*
 Copyright (c) 2003-2017, GENIE Neutrino MC Generator Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         University of Liverpool & STFC Rutherford Appleton Lab 

 For the class documentation see the corresponding header file.

 Important revisions after version 2.0.0 :

*/
//____________________________________________________________________________

#include "Base/XSecIntegratorI.h"

using namespace genie;

//___________________________________________________________________________
XSecIntegratorI::XSecIntegratorI() :
Algorithm()
{

}
//___________________________________________________________________________
XSecIntegratorI::XSecIntegratorI(string name) :
Algorithm(name)
{

}
//___________________________________________________________________________
XSecIntegratorI::XSecIntegratorI(string name, string config) :
Algorithm(name, config)
{

}
//___________________________________________________________________________
XSecIntegratorI::~XSecIntegratorI()
{

}
//___________________________________________________________________________
