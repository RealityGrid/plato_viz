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
#include <unistd.h>

// vtk includes...
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"

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

void regInit() {
  Steering_enable(REG_TRUE);
  int cmds[] = {REG_STR_STOP};
  Steering_initialize(PVS_BIN_NAME, 1, cmds);
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

  // params to be registered...
  int mVis;
  int bVis;
  double isoValue[PVS_MAX_ISOS];
  int isoVis[PVS_MAX_ISOS];
  int orthoslice;
  int cutplane;

  // thread data...
  threadData* td = (threadData*) ((ThreadInfoStruct*) userData)->UserData;

  // allocate memory...
  changedParamLabels = Alloc_string_array(REG_MAX_STRING_LENGTH,
					  REG_MAX_NUM_STR_PARAMS);
  recvdCmdParams = Alloc_string_array(REG_MAX_STRING_LENGTH,
				      REG_MAX_NUM_STR_CMDS);

  // initialise steering library...
  regInit();

  // register params...
  if(td->xyzPipeline) {
    ((PlatoXYZPipeline*) td->xyzPipeline)->isMoleculeVisible() ? mVis = 1 : mVis = 0;
    status = Register_param("Molecule visible?", REG_TRUE, (void*) &mVis,
			    REG_INT, "0", "1");
    ((PlatoXYZPipeline*) td->xyzPipeline)->isBondsVisible() ? bVis = 1 : bVis = 0;
    status = Register_param("Bonds visible?", REG_TRUE, (void*) &bVis,
			    REG_INT, "0", "1");
  }

  double* isoRange = ((PlatoDataReader*) td->dataReader)->getDataRange();
  char isoMin[10];
  char isoMax[10];
  char isoLabel[20];
  snprintf(isoMin, 10, "%f", isoRange[0]);
  snprintf(isoMax, 10, "%f", isoRange[1]);
  for(int i = 0; i < PVS_MAX_ISOS; i++) {
    snprintf(isoLabel, 20, "Iso %d visible?", i);
    ((PlatoIsoPipeline*) td->isoPipeline)->isIsoVisible(i) ? isoVis[i] = 1 : isoVis[i] = 0;
    status = Register_param(isoLabel, REG_TRUE, (void*) &isoVis[i],
			    REG_INT, "0", "1");
    snprintf(isoLabel, 20, "Iso %d value", i);
    isoValue[i] = ((PlatoIsoPipeline*) td->isoPipeline)->getIsoValue(i);
    status = Register_param(isoLabel, REG_TRUE, (void*) &isoValue[i],
			    REG_DBL, isoMin, isoMax);
  }

  ((PlatoOrthoPipeline*) td->orthoPipeline)->isOrthosliceOn() ? orthoslice = 1
    : orthoslice = 0;
  status = Register_param("Orthoslice?", REG_TRUE, (void*) &orthoslice,
			  REG_INT, "0", "1");

  ((PlatoIsoPipeline*) td->isoPipeline)->isIsoCutterOn() ? cutplane = 1 :
    cutplane = 0;
  status = Register_param("Cut-plane?", REG_TRUE, (void*) &cutplane,
			  REG_INT, "0", "1");

  loopLock->Lock();
  done = regLoopDone;
  loopLock->Unlock();

  // go into loop until told to finish...
  while(!done) {
    // sleep for 0.2 seconds...
    usleep(200000);

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
	//td->window->getInteractor()->InvokeEvent(vtkCommand::ExitEvent);
	td->window->exit();
	break;
      }
    }

    // deal with changed parameters...
    for(int i = 0; i < numParamsChanged; i++) {
      if(strstr(changedParamLabels[i], "Molecule") || 
	 strstr(changedParamLabels[i], "Bonds")) {
	moleculeVisibility((PlatoXYZPipeline*) td->xyzPipeline, mVis, bVis);
	needRefresh = true;
	continue;
      }

      if(!strncmp(changedParamLabels[i], "Iso", 3)) {
	isoChanged((PlatoIsoPipeline*) td->isoPipeline,
		   changedParamLabels[i], isoValue, isoVis);
	needRefresh = true;
	continue;
      }

      if(!strcmp(changedParamLabels[i], "Orthoslice?")) {
	toggleOrthoslice((PlatoOrthoPipeline*) td->orthoPipeline, orthoslice);
	needRefresh = true;
	continue;
      }

      if(!strcmp(changedParamLabels[i], "Cut-plane?")) {
	toggleCutplane((PlatoIsoPipeline*) td->isoPipeline, cutplane);
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

  // clean up steering library...
  regFinalise();

  // tell main thread that this one is done...
  sem_post(&regDone);
}

void moleculeVisibility(PlatoXYZPipeline* xyz, int mVis, int bVis) {
  std::cout << "Molecule state changed...\n";
  (mVis == 1) ? xyz->setMoleculeVisible(true) : xyz->setMoleculeVisible(false);
  (bVis == 1) ? xyz->setBondsVisible(true) : xyz->setBondsVisible(false);
}

void isoChanged(PlatoIsoPipeline* pip, const char* label,
		double* values, int* visible) {
  int iso = strtol(&label[4], NULL, 10);
  std::cout << "Iso " << iso << " changed: " << values[iso] << std::endl;
  if(visible[iso] == 1) {
    pip->setIsoVisible(iso, true);
    pip->setIsoValue(iso, values[iso]);
  }
  else {
    pip->setIsoVisible(iso, false);
  }
}

void toggleOrthoslice(PlatoOrthoPipeline* pop, int toggle) {
    std::cout << "Orthoslice state changed..." << std::endl;
    (toggle == 1) ? pop->setOrthoslice(true) : pop->setOrthoslice(false);
}

void toggleCutplane(PlatoIsoPipeline* pip, int toggle) {
  std::cout << "Cut-plane state changed..." << std::endl;
  (toggle == 1) ? pip->setIsoCutter(true) : pip->setIsoCutter(false);
}

void regFinalise() {
  Steering_finalize();
}
