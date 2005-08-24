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

#ifndef __PLATOMAIN_H__

// macro definitions...
#define PVS_VERSION "0.05 pre"
#define PVS_MAX_ISOS 2

#ifndef PVS_BIN_NAME
#define PVS_BIN_NAME "pvs"
#endif

// system includes...
#include <semaphore.h>

// vtk forward references...
class vtkObject;
class vtkMutexLock;

// Plato forward references...
class PlatoDataReader;
class PlatoRenderWindow;
class PlatoVTKPipeline;

// struct for the results of parseOptions...
struct optionsData {
  char* rhoFilename;
  char* xyzFilename;
  bool useCutplane;
  bool useOrthoslice;
};

// struct to pass data to the thread...
struct threadData {
  PlatoRenderWindow* window;
  PlatoDataReader* dataReader;
  PlatoVTKPipeline* xyzPipeline;
  PlatoVTKPipeline* isoPipeline;
  PlatoVTKPipeline* orthoPipeline;
};

// global variables...
extern volatile bool reRender;
extern volatile bool regLoopDone;
extern vtkMutexLock* renderLock;
extern vtkMutexLock* loopLock;
extern sem_t regDone;

// prototypes...
void parseOptions(int, char*[], optionsData*);
void renderCallback(vtkObject*, unsigned long, void*, void*);
void usage();

#define __PLATOMAIN_H__
#endif // __PLATOMAIN_H__
