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

// vtk includes...

// vtk includes...
#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkCutter.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"
#include "vtkPolyDataMapper.h"

// plato includes...
#include "main.h"
#include "PlatoDataReader.h"
#include "PlatoOrthoPipeline.h"
#include "PlatoVTKPipeline.h"

PlatoOrthoPipeline::PlatoOrthoPipeline(PlatoDataReader* dr)
  : PlatoVTKPipeline() {
  data = dr;

  init();
  buildPipeline();
}

PlatoOrthoPipeline::PlatoOrthoPipeline(PlatoDataReader* dr, vtkLookupTable* clut)
  : PlatoVTKPipeline(clut) {
  data = dr;

  init();
  buildPipeline();
}

PlatoOrthoPipeline::~PlatoOrthoPipeline() {
  delete[] orthoPlaneNormals;

  // remove actor from collection...
  actors->RemoveAllItems();

  // delete all vtk objects...
  orthoPlane->Delete();
  orthoSlice->Delete();
  orthoMapper->Delete();
  orthoActor->Delete();
}

void PlatoOrthoPipeline::init() {
  dataRange = data->getDataRange();
  orthoPlaneCentre = data->getDataCentre();
  orthoPlaneNormals = new double[3];
  orthoPlaneNormals[0] = 0.0;
  orthoPlaneNormals[1] = 0.0;
  orthoPlaneNormals[2] = 1.0;

  orthoPlane = vtkPlane::New();
  orthoSlice = vtkCutter::New();
  orthoMapper = vtkPolyDataMapper::New();
  orthoActor = vtkActor::New();

  // add actor to the collection...
  actors->AddItem(orthoActor);
}

void PlatoOrthoPipeline::buildPipeline() {
  // set up plane for the orthoslice...
  orthoPlane->SetOrigin(orthoPlaneCentre);
  orthoPlane->SetNormal(orthoPlaneNormals);

  // set up ortho slice...
  orthoSlice->SetInput(data->getData());
  orthoSlice->SetCutFunction(orthoPlane);

  // apply colour map...
  orthoMapper->SetInput(orthoSlice->GetOutput());
  orthoMapper->SetScalarRange(dataRange);
  orthoMapper->SetLookupTable(colourTable);
  //orthoMapper->UseLookupTableScalarRangeOn();
  //orthoMapper->SetColorModeToMapScalars();

  // put it into an actor...
  orthoActor->SetMapper(orthoMapper);
}
