/*----------------------------------------------------------------------------
  This file is part of the Plato Visualization System.

  (C) Copyright 2005, University of Manchester, United Kingdom,
  all rights reserved.

  This software was developed by the RealityGrid project
  (http://www.realitygrid.org), funded by the EPSRC under grants
  GR/R67699/01 and GR/R67699/02.

  LICENCE TERMS

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS MATERIAL IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. THE ENTIRE RISK AS TO THE QUALITY
  AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
  DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
  CORRECTION.

  Author........: Robert Haines
---------------------------------------------------------------------------*/

// system includes...
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <semaphore.h>

// vtk includes...
#include "vtkCommand.h"
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"
#include "vtkRenderWindowInteractor.h"

// RealityGrid includes...
#include "ReG_Steer_Appside.h"

// plato includes...
#include "main.h"
#include "PlatoDataReader.h"
#include "PlatoIsoPipeline.h"
#include "PlatoOrthoPipeline.h"
#include "PlatoRenderWindow.h"
#include "PlatoXYZPipeline.h"
#include "realitygrid.h"

// program-wide variable definitions...
volatile bool reRender = false;
volatile bool regLoopDone = false;
vtkMutexLock* renderLock;
vtkMutexLock* loopLock;
sem_t regDone;

int main(int argc, char** argv) {
  vtkMultiThreader* thread;
  threadData* td;

  // parse options...
  OptionsData* options = new OptionsData();
  parseOptions(argc, argv, options);

  // create vtk window...
  char windowTitle[100];
  sprintf(windowTitle, "Plato Visualization System (%s)", PVS_BIN_NAME);
  PlatoRenderWindow* prw = new PlatoRenderWindow(options->useSteering, windowTitle);

  PlatoXYZPipeline* xyz = NULL;
  if(options->xyzFilename) {
    xyz = new PlatoXYZPipeline(options->xyzFilename);
    prw->addPipeline(xyz);
  }

  PlatoDataReader* pdr;
  PlatoIsoPipeline* pip;
  PlatoOrthoPipeline* pop;
  if(options->rhoFilename) {
    pdr = new PlatoDataReader(options->rhoFilename);
    pip = new PlatoIsoPipeline(pdr);
    for(int i = 0; i < options->numIsos; i++)
      pip->setIsoVisible(i, true);
    pip->setIsoCutter(options->useCutplane);
    pop = new PlatoOrthoPipeline(pdr);
    pop->setOrthoslice(options->useOrthoslice);
    prw->addPipeline(pip);
    prw->addPipeline(pop);
  }

  // if we're not using realitygrid stuff we can ignore all this...
  if(options->useSteering) {
    // initialise thread stuff...
    renderLock = vtkMutexLock::New();
    loopLock = vtkMutexLock::New();
    sem_init(&regDone, 0, 0);
    thread = vtkMultiThreader::New();

    // initialise and start the RealityGrid loop...
    td = new threadData;
    td->window = prw;
    td->dataReader = pdr;
    td->xyzPipeline = xyz;
    td->isoPipeline = pip;
    td->orthoPipeline = pop;
    thread->SpawnThread(regLoop, td);
  }

  // start the vtk interactor (this blocks the main thread)...
  prw->start();
  
  if(options->useSteering) {
    // the interactor is finished so tell the loop to finish...
    loopLock->Lock();
    regLoopDone = true;
    loopLock->Unlock();

    // wait for it to finish...
    sem_wait(&regDone);
  }

  std::cout << "Ready to cleanup..." << std::endl;

  // clean up everything...
  if(options->useSteering) {
    thread->Delete();
    sem_destroy(&regDone);
    renderLock->Delete();
    loopLock->Delete();
    delete td;
  }

  delete prw;
  delete pop;
  delete pip;
  delete xyz;
  delete pdr;
  delete options;

  std::cout << "All done, bye..." << std::endl;
  return 0;
}

void renderCallback(vtkObject* obj, unsigned long eid, void* cd, void* calld) {
  bool render;

  renderLock->Lock();
  render = reRender;
  renderLock->Unlock();

  if(render) {
    std::cout << "Render...\n";

    // render via the interactor...
    ((vtkRenderWindowInteractor*) obj)->Render();

    // reset reRender...
    renderLock->Lock();
    reRender = false;
    renderLock->Unlock();
  }

  // reset the timer on the interactor...
  ((vtkRenderWindowInteractor*) obj)->CreateTimer(VTKI_TIMER_UPDATE);
}

void parseOptions(int argc, char* argv[], OptionsData* options) {

  // not enough arguments? error...
  if(argc < 2) {
    usage();
    exit(1);
  }

  using std::cout;
  using std::cerr;
  using std::endl;

  bool showHelp = false;
  bool showVersion = false;

  char* argStr;
  char* nextArgStr;
  char shortOpt;
  bool shortOptDone;
  int shortOptStrLen;
  int isLongOpt;

  for(int argNum = 1; argNum < argc; argNum++) {

    argStr = argv[argNum];
    shortOptStrLen = strlen(argStr);
    if(shortOptStrLen > 1 && argStr[0] == '-') {
      for(int j = 1; j < shortOptStrLen; j++) {
	shortOpt = ((argStr[1] == '-') ? '\0' : argStr[j]);
	shortOptDone = (argStr[j+1] == '\0');
	nextArgStr = (((argNum + 1) < argc) ? argv[argNum + 1] : NULL);

	if(shortOpt == 'c' || (isLongOpt = strcmp("--cut", argv[argNum])) == 0)
	  options->useCutplane = true;
	else if(shortOpt == 'h' || (isLongOpt = strcmp("--help", argv[argNum])) == 0)
	  showHelp = true;
	else if(shortOpt == 'i' || (isLongOpt = strcmp("--isosurfaces", argv[argNum])) == 0) {
	  if(nextArgStr) {
	    options->numIsos = atoi(nextArgStr);
	    if(options->numIsos > PVS_MAX_ISOS)
	      options->numIsos = PVS_MAX_ISOS;
	    argNum++;
	    break;
	  }
	  else {
	    cerr << "Number of initial isosurfaces not specified.\n\n";
	    usage();
	    exit(1);
	  }
	}
	else if(shortOpt == 'o' || (isLongOpt = strcmp("--ortho", argv[argNum])) == 0)
	  options->useOrthoslice = true;
	else if((shortOpt == 'r' && shortOptDone) || (isLongOpt = strcmp("--rho", argv[argNum])) == 0) {
	  if(nextArgStr) {
	    options->rhoFilename = nextArgStr;
	    argNum++;
	    break;
	  }
	  else {
	    cerr << "No filename supplied for RHOFILE.\n\n";
	    usage();
	    exit(1);
	  }
	}
	else if(shortOpt == 'R' || (isLongOpt = strcmp("--reg-io", argv[argNum])) == 0)
	  options->useReGIO = true;
	else if((isLongOpt = strcmp("--view-only", argv[argNum])) == 0 || (isLongOpt = strcmp("--no-steering", argv[argNum])) == 0)
	  options->useSteering = false;
	else if(shortOpt == 'v' || (isLongOpt = strcmp("--version", argv[argNum])) == 0)
	  showVersion = true;
	else if((shortOpt == 'x' && shortOptDone) || (isLongOpt = strcmp("--xyz", argv[argNum])) == 0) {
	  if(nextArgStr) {
	    options->xyzFilename = nextArgStr;
	    argNum++;
	    break;
	  }
	  else {
	    cerr << "No filename supplied for XYZFILE.\n\n";
	    usage();
	    exit(1);
	  }
	}
	else {
	  cerr << "Unknown option " << argStr << endl << endl;
	  usage();
	  exit(1);
	}

	// if we've just parsed a long option, kill the inner loop...
	if(isLongOpt == 0)
	  break;
      }
    }
  }

  // show version and/or help if needed and exit...
  if(showVersion) {
    cout << "Plato Visualisation System (" << PVS_BIN_NAME;
    cout << ") " << PVS_VERSION << endl;
    cout << "Robert Haines for RealityGrid.\n";
    cout << "Copyright (C) 2005  University of Manchester, ";
    cout << "United Kingdom.\nAll rights reserved.\n";    
  }
  if(showHelp)
    usage();
  if(showVersion || showHelp)
    exit(0);

  // check that all required options are present...
  if(!options->rhoFilename) {
    usage();
    exit(1);
  }

  if(!options->useOrthoslice && (options->numIsos < 1)) {
    cerr << "You must specify at least one isosurface or an orthoslice.\n\n";
    usage();
    exit(1);
  }
}

void usage() {
  using std::cout;

  cout << "Usage: " << PVS_BIN_NAME << " [options]\nOptions:\n";
  cout << "  -c, --cut\t\tEnable a cut plane through the data.\n";
  cout << "  -h, --help\t\tPrint this message and exit.\n";
  cout << "  -i N, --isosurfaces N\n\t\t\tThe number of visible isosurfaces";
  cout << " on startup.\n";
  cout << "  -o, --ortho\t\tEnable an orthoslice through the data.\n";
  cout << "  -r RHOFILE, --rho RHOFILE\n\t\t\tInput rho file for viewing.\n";
  cout << "  -R, --reg-io\t\tGet data from a RealityGrid socket.\n";
  cout << "  -v, --version\t\tPrint the version number and exit.\n";
  cout << "      --view-only, --no-steering\n\t\t\tUse " << PVS_BIN_NAME;
  cout << " as a viewer only - no interface control.\n";
  cout << "  -x XYZFILE, --xyz XYZFILE\n\t\t\tInput xyz file for viewing.\n";
  cout << "\nReport bugs to <www.kato.mvc.mcc.ac.uk/bugzilla>\n";
}
