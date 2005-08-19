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

// vtk includes
#include "vtkActorCollection.h"
#include "vtkGlyph3D.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkTubeFilter.h"
#include "vtkXYZMolReader.h"

//plato includes
#include "PlatoXYZPipeline.h"

PlatoXYZPipeline::PlatoXYZPipeline(char* filename) : PlatoVTKPipeline() {
  xyzFilename = filename;
  drawResolution = 12;
  sphereScale = 0.5f;
  numAtoms = 0;

  moleculeVisible = true;
  bondsVisible = true;

  init();
  buildPipeline();
}

PlatoXYZPipeline::~PlatoXYZPipeline() {
  // remove actor from collection...
  actors->RemoveAllItems();

  // delete all vtk objects...
  xyzReader->Delete();
  sphere->Delete();
  atoms->Delete();
  bonds->Delete();
  atomsMapper->Delete();
  bondsMapper->Delete();
  actorProperties->Delete();
  atomsActor->Delete();
  bondsActor->Delete();
}

void PlatoXYZPipeline::init() {
  // allocate memory for all the vtk objects...
  xyzReader = vtkXYZMolReader::New();
  sphere = vtkSphereSource::New();
  atoms = vtkGlyph3D::New();
  bonds = vtkTubeFilter::New();
  atomsMapper = vtkPolyDataMapper::New();
  bondsMapper = vtkPolyDataMapper::New();
  actorProperties = vtkProperty::New();
  atomsActor = vtkActor::New();
  bondsActor = vtkActor::New();

  // put the actors into the collection...
  actors->AddItem(atomsActor);
  actors->AddItem(bondsActor);
}

void PlatoXYZPipeline::buildPipeline() {
  // set up actor properties...
  actorProperties->SetRepresentationToSurface();
  actorProperties->SetInterpolationToGouraud();
  actorProperties->SetAmbient(0.15);
  actorProperties->SetDiffuse(0.85);
  actorProperties->SetSpecular(0.1);
  actorProperties->SetSpecularPower(100);
  actorProperties->SetSpecularColor(1.0, 1.0, 1.0);
  actorProperties->SetColor(1.0, 1.0, 1.0);

  // load model...
  xyzReader->SetFileName(xyzFilename);
  numAtoms = xyzReader->GetNumberOfAtoms();

  // create the atom glyph and the glyph itself...
  sphere->SetThetaResolution(drawResolution);
  sphere->SetPhiResolution(drawResolution);
  atoms->SetInput(xyzReader->GetOutput());
  atoms->SetOrient(1);
  atoms->SetColorMode(1);
  atoms->SetScaleMode(2);
  atoms->SetScaleFactor(sphereScale);
  atoms->SetSource(sphere->GetOutput());

  // colour the atoms...
  atomsMapper->SetInput(atoms->GetOutput());
  atomsMapper->SetImmediateModeRendering(1);
  atomsMapper->UseLookupTableScalarRangeOff();
  atomsMapper->SetScalarVisibility(1);
  atomsMapper->SetScalarModeToDefault();

  // put the atoms into an actor...
  atomsActor->SetMapper(atomsMapper);
  atomsActor->SetProperty(actorProperties);

  // create the tube glyph for the bonds...
  bonds->SetInput(xyzReader->GetOutput());
  bonds->SetNumberOfSides(drawResolution);
  bonds->SetCapping(0);
  bonds->SetRadius(0.2);
  bonds->SetVaryRadius(0);
  bonds->SetRadiusFactor(10);

  // colour the bonds...
  bondsMapper->SetInput(bonds->GetOutput());
  bondsMapper->SetImmediateModeRendering(1);
  bondsMapper->UseLookupTableScalarRangeOff();
  bondsMapper->SetScalarVisibility(1);
  bondsMapper->SetScalarModeToDefault();

  // put the bonds into an actor...
  bondsActor->SetMapper(bondsMapper);
  bondsActor->SetProperty(actorProperties);
}

vtkActor* PlatoXYZPipeline::getAtomsActor() {
  return atomsActor;
}

vtkActor* PlatoXYZPipeline::getBondsActor() {
  return bondsActor;
}

void PlatoXYZPipeline::setMoleculeVisible(bool toggle) {
  // toggle the atoms...
  moleculeVisible = toggle;
  toggle ? atomsActor->SetVisibility(1) : atomsActor->SetVisibility(0);

  // if needs be, toggle the bonds...
  if(bondsVisible) {
    toggle ? bondsActor->SetVisibility(1) : bondsActor->SetVisibility(0);
  }
}

void PlatoXYZPipeline::setBondsVisible(bool toggle) {
  // if we're not showing the molecule, the bonds don't count...
  if(!moleculeVisible)
    return;

  // toggle the bonds...
  bondsVisible = toggle;
  toggle ? bondsActor->SetVisibility(1) : bondsActor->SetVisibility(0);
}

bool PlatoXYZPipeline::isMoleculeVisible() {
  return moleculeVisible;
}

bool PlatoXYZPipeline::isBondsVisible() {
  // if we're not showing the molecule, we're not showing the bonds...
  if(!moleculeVisible)
    return false;

  return bondsVisible;
}
