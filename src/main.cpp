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
#include <cstring>
#include <iostream>
#include <semaphore.h>
#include <unistd.h>

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
#include "PlatoRenderWindow.h"
#include "PlatoXYZPipeline.h"

// global variables...
char* rhoFilename = NULL;
char* xyzFilename = NULL;
volatile bool reRender = false;
volatile bool regLoopDone = false;

// threading stuff...
vtkMutexLock* renderLock;
vtkMutexLock* loopLock;
sem_t regDone;

int main(int argc, char** argv) {

  parseOptions(argc, argv);

  PlatoRenderWindow* prw = new PlatoRenderWindow("Plato Visualization System (pvs)");
  PlatoXYZPipeline* xyz;
  if(xyzFilename) {
    xyz = new PlatoXYZPipeline(xyzFilename);
    prw->addPipeline(xyz);
  }

  PlatoDataReader* pdr;
  PlatoIsoPipeline* pip;
  if(rhoFilename) {
    pdr = new PlatoDataReader(rhoFilename);
    pip = new PlatoIsoPipeline(pdr);
    prw->addPipeline(pip);
  }

  // initialise thread stuff...
  renderLock = vtkMutexLock::New();
  loopLock = vtkMutexLock::New();
  sem_init(&regDone, 0, 0);
  vtkMultiThreader* thread = vtkMultiThreader::New();

  // initialise ReG stuff...
  Steering_enable(REG_TRUE);
  int cmds[] = {REG_STR_STOP};
  Steering_initialize(PVS_BIN_NAME, 1, cmds);

  // initialise and start the RealityGrid loop...
  threadData* td = new threadData;
  td->window = prw;
  td->dataReader = pdr;
  td->xyzPipeline = xyz;
  td->isoPipeline = pip;
  thread->SpawnThread(regLoop, td);

  // start the vtk interactor (this blocks the main thread)...
  prw->start();
  
  // the interactor is finished so tell the loop to finish...
  loopLock->Lock();
  regLoopDone = true;
  loopLock->Unlock();
  
  // wait for it to finish...
  sem_wait(&regDone);

  std::cout << "Ready to cleanup...\n";

  // clean up ReG...
  Steering_finalize();

  // clean up everything else...
  thread->Delete();
  sem_destroy(&regDone);
  delete prw;
  delete td;

  std::cout << "All done, bye...\n";
  return 0;
}

void* regLoop(void* userData) {
  int l = 0;
  int status;
  int numParamsChanged;
  int numRecvdCmds;
  int recvdCmds[REG_MAX_NUM_STR_CMDS];
  char** changedParamLabels;
  char** recvdCmdParams;
  bool done;
  bool needRefresh = false;

  int bVis = 1;
  double isoValue[PVS_MAX_ISOS];

  // thread data...
  threadData* td = (threadData*) ((ThreadInfoStruct*) userData)->UserData;

  // allocate memory...
  changedParamLabels = Alloc_string_array(REG_MAX_STRING_LENGTH,
					  REG_MAX_NUM_STR_PARAMS);
  recvdCmdParams = Alloc_string_array(REG_MAX_STRING_LENGTH,
				      REG_MAX_NUM_STR_CMDS);

  // register params...
  status = Register_param("Bonds visible?", REG_TRUE, (void*) (&bVis),
			  REG_INT, "0", "1");

  isoValue[0] = ((PlatoIsoPipeline*) td->isoPipeline)->getIsoValue(0);
  status = Register_param("Iso 0 Value", REG_TRUE, (void*) &isoValue[0],
			  REG_DBL, "", "");

  loopLock->Lock();
  done = regLoopDone;
  loopLock->Unlock();

  // go into loop until told to finish...
  while(!done) {
    // sleep for 0.1 seconds...
    usleep(100000);

    status = Steering_control(l, &numParamsChanged, changedParamLabels,
			      &numRecvdCmds, recvdCmds, recvdCmdParams);

    if(status != REG_SUCCESS) {
      std::cerr << "Call to Steering_control failed...\n";
      continue;
    }

    // deal with commands from steering library...
    for(int i = 0; i < numRecvdCmds; i++) {
      switch(recvdCmds[i]) {
      case REG_STR_STOP:
	std::cout << "Try to exit...\n";
	//td->window->getInteractor()->InvokeEvent(vtkCommand::ExitEvent);
	td->window->exit();
	break;
      }
    }

    // deal with changed parameters...
    for(int i = 0; i < numParamsChanged; i++) {
      if(!strcmp(changedParamLabels[i], "Bonds visible?")) {
	std::cout << "Bonds Changed...\n";
	if(bVis)
	  ((PlatoXYZPipeline*) td->xyzPipeline)->setBondsVisible(true);
	else
	  ((PlatoXYZPipeline*) td->xyzPipeline)->setBondsVisible(false);
	needRefresh = true;
	continue;
      }

      if(!strncmp(changedParamLabels[i], "Iso", 3)) {
	std::cout << "Iso value changed...\n";
	((PlatoIsoPipeline*) td->isoPipeline)->setIsoValue(0, isoValue[0]);
	needRefresh = true;
	continue;
      }
    }

    // tell the interactor to render if needs be...
    if(needRefresh) {
      renderLock->Lock();
      reRender = true;
      renderLock->Unlock();
      needRefresh = false;
    }

    // see if we're done yet...
    loopLock->Lock();
    done = regLoopDone;
    loopLock->Unlock();

    // update loop count for steering library...
    l++;
  }

  std::cout << "Loop done...\n";
  sem_post(&regDone);
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

void parseOptions(int argc, char** argv) {
  bool rhoFile = false;
  bool xyzFile = false;

  for(int i = 1; i < argc; i++) {

    if(!strcmp("-h", argv[i]) || !strcmp("-help", argv[i])) {
      usage();
      exit(0);
    }

    if(!strcmp("-r", argv[i]) || !strcmp("--rho", argv[i])) {
      rhoFile = true;
      i++;
      rhoFilename = argv[i];
      continue;
    }

    if(!strcmp("-v", argv[i]) || !strcmp("--version", argv[i])) {
      std::cout << "Plato Visualisation System (" << PVS_BIN_NAME;
      std::cout << ") " << PVS_VERSION << std::endl;
      std::cout << "Robert Haines for RealityGrid.\n";
      std::cout << "Copyright (C) 2005  University of Manchester, ";
      std::cout << "United Kingdom.\nAll rights reserved.\n";
      exit(0);
    }

    if(!strcmp("-x", argv[i]) || !strcmp("--xyz", argv[i])) {
      xyzFile = true;
      i++;
      xyzFilename = argv[i];
      continue;
    }
  }

  // check that all required options are present...
  if(!(rhoFile || xyzFile)) {
    usage();
    exit(1);
  }
}

void usage() {
  std::cout << "Usage: " << PVS_BIN_NAME << "  [options]\nOptions:\n";
  std::cout << "  -h, --help\t\tPrint this message and exit.\n";
  std::cout << "  -r, --rho\t\tInput rho file for viewing.\n";
  std::cout << "  -x, --xyz\t\tInput xyz file for viewing.\n";
  std::cout << "  -v, --version\t\tPrint the version number and exit.\n\n";
  std::cout << "Report bugs to <www.kato.mvc.mcc.ac.uk/bugzilla>\n";
}
