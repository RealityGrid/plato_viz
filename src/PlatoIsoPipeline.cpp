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
#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkClipPolyData.h"
#include "vtkLookupTable.h"
#include "vtkMarchingContourFilter.h"
#include "vtkPlane.h"
#include "vtkPointSet.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

// plato includes...
#include "main.h"
#include "PlatoDataReader.h"
#include "PlatoIsoPipeline.h"
#include "PlatoVTKPipeline.h"

PlatoIsoPipeline::PlatoIsoPipeline(PlatoDataReader* dr) : PlatoVTKPipeline() {
  data = dr;

  init();
  buildPipeline();
}

PlatoIsoPipeline::PlatoIsoPipeline(PlatoDataReader* dr, vtkLookupTable* clut)
  : PlatoVTKPipeline(clut) {
  data = dr;

  init();
  buildPipeline();
}

PlatoIsoPipeline::~PlatoIsoPipeline() {
  delete[] isoValues;
  delete[] isoVisible;
  delete[] cutPlaneNormals;

  // remove actor from collection...
  actors->RemoveAllItems();

  // delete all vtk objects...
  cutPlane->Delete();
  actorProperties->Delete();
  isoSurface->Delete();
  isoNormals->Delete();
  isoCutter->Delete();
  isoMapper->Delete();
  isoActor->Delete();
}

void PlatoIsoPipeline::init() {
  dataRange = data->getDataRange();
  cutPlaneCentre = data->getDataCentre();
  isoValues = new double[PVS_MAX_ISOS];
  isoVisible = new bool[PVS_MAX_ISOS];
  cutPlaneNormals = new double[3];
  cutPlaneNormals[0] = 0.0;
  cutPlaneNormals[1] = 1.0;
  cutPlaneNormals[2] = 0.0;

  cutPlaneOn = false;

  // keep track of isosurface values and visibilities...
  for(int i = 0; i < PVS_MAX_ISOS; i++) {
    isoValues[i] = dataRange[0] + ((dataRange[1] - dataRange[0]) / 2.0);
    isoVisible[i] = false;
  }

  // allocate memory for all the vtk objects...
  cutPlane = vtkPlane::New();
  actorProperties = vtkProperty::New();
  isoSurface = vtkMarchingContourFilter::New();
  isoNormals = vtkPolyDataNormals::New();
  isoCutter = vtkClipPolyData::New();
  isoMapper = vtkPolyDataMapper::New();
  isoActor = vtkActor::New();

  // add actor to the collection...
  actors->AddItem(isoActor);
}

void PlatoIsoPipeline::buildPipeline() {
  // set up the cut plane to cut the isosurfaces...
  cutPlane->SetOrigin(cutPlaneCentre);
  cutPlane->SetNormal(cutPlaneNormals);

  // set up actor properties...
  actorProperties->SetInterpolationToGouraud();
  actorProperties->SetAmbient(0.1);
  actorProperties->SetDiffuse(0.8);
  actorProperties->SetSpecular(0.1);
  actorProperties->SetSpecularPower(30);

  // create isosurfaces...
  isoSurface->SetInput(data->getData());
  isoSurface->UseScalarTreeOn();

  // set up cut-plane...
  isoCutter->SetInput(isoSurface->GetOutput());
  isoCutter->SetClipFunction(cutPlane);

  // calculate normals of isosurface...
  isoNormals->SetInput(isoSurface->GetOutput());
  isoNormals->ComputeCellNormalsOn();
  isoNormals->AutoOrientNormalsOff();
  isoNormals->FlipNormalsOn();

  // apply colour map...
  isoMapper->SetInput(isoNormals->GetOutput());
  isoMapper->SetScalarRange(dataRange);
  isoMapper->SetLookupTable(colourTable);

  // put it all into an actor and apply properties...
  isoActor->SetMapper(isoMapper);
  isoActor->SetProperty(actorProperties);

  // make a surface visible...
  setIsoVisible(0, true);
}

void PlatoIsoPipeline::setIsoValue(int iso, double value) {
  if(isoVisible[iso]) {
    isoValues[iso] = value;
    isoSurface->SetValue(iso, value);
  }
}

double PlatoIsoPipeline::getIsoValue(int iso) {
  if((iso < 0) || (iso >= PVS_MAX_ISOS))
    return 0.0;

  return isoValues[iso];
}

void PlatoIsoPipeline::setIsoVisible(int iso, bool toggle) {
  if((iso < 0) || (iso >= PVS_MAX_ISOS) || (isoVisible[iso] == toggle))
    return;

  isoVisible[iso] = toggle;

  // turn them all off...
  isoSurface->SetNumberOfContours(0);

  // turn on the ones we want...
  int j = 0;
  for(int i = 0; i < PVS_MAX_ISOS; i++) {
    if(isoVisible[i]) {
      isoSurface->SetValue(j, isoValues[i]);
      j++;
    }
  }
}

bool PlatoIsoPipeline::isIsoVisible(int iso) {
  if(iso > PVS_MAX_ISOS)
    return false;

  return isoVisible[iso];
}

void PlatoIsoPipeline::setIsoCutter(bool toggle) {
  cutPlaneOn = toggle;

  if(cutPlaneOn)
    isoNormals->SetInput(isoCutter->GetOutput());
  else
    isoNormals->SetInput(isoSurface->GetOutput());
}

bool PlatoIsoPipeline::isIsoCutterOn() {
  return cutPlaneOn;
}
