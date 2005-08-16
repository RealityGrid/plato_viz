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

#ifndef __PLATOXYZPIPELINE_H__

// plato includes...
#include "PlatoVTKPipeline.h"

// vtk forward references...
class vtkXYZMolReader;
class vtkSphereSource;
class vtkGlyph3D;
class vtkTubeFilter;
class vtkPolyDataMapper;
class vtkProperty;
class vtkLODActor;

class PlatoXYZPipeline : public PlatoVTKPipeline {

 private:
  char* xyzFilename;
  int drawResolution;
  float sphereScale;
  int numAtoms;
  bool moleculeVisible;
  bool bondsVisible;

  vtkXYZMolReader* xyzReader;
  vtkSphereSource* sphere;
  vtkGlyph3D* atoms;
  vtkTubeFilter* bonds;
  vtkPolyDataMapper* atomsMapper;
  vtkPolyDataMapper* bondsMapper;
  vtkProperty* actorProperties;
  vtkLODActor* atomsActor;
  vtkLODActor* bondsActor;

 private:
  void init();
  void buildPipeline();

 public:
  PlatoXYZPipeline(char*);
  ~PlatoXYZPipeline();
  vtkActor* getAtomsActor();
  vtkActor* getBondsActor();
  void setMoleculeVisible(bool);
  void setBondsVisible(bool);
  bool isMoleculeVisible();
  bool isBondsVisible();
};

#define __PLATOXYZPIPELINE_H__
#endif // __PLATOXYZPIPELINE_H__
