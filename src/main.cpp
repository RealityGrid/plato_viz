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

// system includes
#include <iostream>
#include <cstring>

// vtk includes
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkMultiThreader.h"

// plato includes
#include "main.h"
#include "PlatoDataReader.h"
#include "PlatoRenderWindow.h"
#include "PlatoXYZPipeline.h"

int main(int argc, char** argv) {

  parseOptions(argc, argv);

  PlatoRenderWindow* prw = new PlatoRenderWindow("Plato Visualization System",
						 500, 500);

  if(xyzFilename) {
    PlatoXYZPipeline* xyz = new PlatoXYZPipeline(xyzFilename);
    prw->addActor(xyz->getAtomsActor());
    prw->addActor(xyz->getBondsActor());
  }

  if(rhoFilename) {
    PlatoDataReader* pdr = new PlatoDataReader(rhoFilename);
  }

  vtkMultiThreader* thread = vtkMultiThreader::New();
  int num = 10;
  thread->SpawnThread(test, &num);

  prw->start();
  
}

void* test(void* userData) {
  int count = *((int*) (((ThreadInfoStruct*) userData)->UserData));
  std::cout << "Start " << count << "...\n";
  for(int i = 0; i < count; i++) {
    sleep(1);
    std::cout << i << std::endl;
  }
  std::cout << "Stop...\n";
}

void parseOptions(int argc, char** argv) {
  bool rhoFile = false;
  bool xyzFile = false;

  for(int i = 1; i < argc; i++) {

    if(!strcmp("-h", argv[i]) || !strcmp("-help", argv[i])) {
      usage(argv[0]);
      exit(0);
    }

    if(!strcmp("-r", argv[i]) || !strcmp("--rho", argv[i])) {
      rhoFile = true;
      i++;
      rhoFilename = argv[i];
      continue;
    }

    if(!strcmp("-v", argv[i]) || !strcmp("--version", argv[i])) {
      std::cout << "Plato Visualisation System 0.01pre\n";
      std::cout << "Robert Haines for RealityGrid.\n";
      std::cout << "Copyright (C) 2005  University of Manchester, ";
      std::cout << "United Kingdom,\nall rights reserved.\n";
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
    std::cerr << "No rho or xyz file.\n";
    exit(1);
  }
}

void usage(char* progName) {
  std::cout << "Usage: " << progName << "  [options]\nOptions:\n";
  std::cout << "  -h, --help\t\tPrint this message and exit.\n";
  std::cout << "  -r, --rho\t\tInput rho file for viewing.\n";
  std::cout << "  -x, --xyz\t\tInput xyz file for viewing.\n";
  std::cout << "  -v, --version\t\tPrint the version number and exit.\n\n";
  std::cout << "Report bugs to <www.kato.mvc.mcc.ac.uk/bugzilla>\n";
}
