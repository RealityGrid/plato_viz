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

#ifndef __PLATOVTKPIPELINE_H__

// vtk forward references...
class vtkActorCollection;
class vtkLookupTable;

class PlatoVTKPipeline {

 private:
  bool internalColourTable;

 protected:
  vtkLookupTable* colourTable;
  vtkActorCollection* actors;

 private:
  virtual void init();
  virtual void buildPipeline() = 0;

 public:
  PlatoVTKPipeline();
  PlatoVTKPipeline(vtkLookupTable*);
  ~PlatoVTKPipeline();
  vtkActorCollection* getActors();
  void setColourTable(vtkLookupTable*);
  vtkLookupTable* getColourTable();
};

#define __PLATOVTKPIPELINE_H__
#endif // __PLATOVTKPIPELINE_H__
