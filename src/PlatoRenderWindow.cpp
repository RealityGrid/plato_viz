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

// vtk includes
#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"

//plato includes
#include "main.h"
#include "PlatoRenderWindow.h"
#include "PlatoVTKPipeline.h"

PlatoRenderWindow::PlatoRenderWindow(const char* name, int width, int height) {
  callback = vtkCallbackCommand::New();
  callback->SetCallback(renderCallback);

  windowName = const_cast<char*>(name);
  windowWidth = width;
  windowHeight = height;

  renderer = vtkRenderer::New();

  window = vtkRenderWindow::New();
  window->AddRenderer(renderer);
  window->SetWindowName(windowName);
  window->SetSize(windowWidth, windowHeight);

  interactorStyle = vtkInteractorStyleTrackballCamera::New();

  interactor = vtkRenderWindowInteractor::New();
  interactor->SetRenderWindow(window);
  interactor->SetInteractorStyle(interactorStyle);
  interactor->Initialize();
  interactor->AddObserver(vtkCommand::TimerEvent, callback);
  interactor->CreateTimer(VTKI_TIMER_FIRST);
}

PlatoRenderWindow::~PlatoRenderWindow() {
  callback->Delete();
  renderer->Delete();
  window->Delete();
  if(interactor)
    interactor->Delete();
  interactorStyle->Delete();
}

void PlatoRenderWindow::addActor(vtkActor* actor) {
  renderer->AddActor(actor);
}

void PlatoRenderWindow::addActors(vtkActorCollection* collection) {
  vtkActor* tmpActor;
  for(int i = 0; i < collection->GetNumberOfItems(); i++) {
    tmpActor = (vtkActor*) collection->GetItemAsObject(i);
    renderer->AddActor(tmpActor);
  }
}

vtkActorCollection* PlatoRenderWindow::getActors() {
  return renderer->GetActors();
}

void PlatoRenderWindow::removeActor(vtkActor* actor) {
  renderer->RemoveActor(actor);
}

void PlatoRenderWindow::removeAllActors() {
  renderer->GetActors()->RemoveAllItems();
}

void PlatoRenderWindow::addPipeline(PlatoVTKPipeline* pipe) {
  addActors(pipe->getActors());
}

vtkRenderWindowInteractor* PlatoRenderWindow::getInteractor() {
  return interactor;
}

void PlatoRenderWindow::start() {
  interactor->Start();
}

void PlatoRenderWindow::exit() {
  interactor->ExitCallback();
}
