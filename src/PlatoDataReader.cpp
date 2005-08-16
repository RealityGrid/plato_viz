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
#include <fstream>

// vtk includes
#include "vtkDelaunay3D.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPointSet.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

// plato includes
#include "PlatoDataReader.h"

PlatoDataReader::PlatoDataReader(char* filename) {
  rhoFilename = filename;
  uniformMesh = true;

  dataDims = new int[3];
  dataRange = new double[2];
  dataCentre = new double[3];
  dataBounds = new double[6];

  dataPoints = vtkPoints::New();
  dataValues = vtkFloatArray::New();
  dataSet = NULL;
  delaunay = NULL;

  readRhoFile();
  buildPipeline();
}

PlatoDataReader::~PlatoDataReader() {
  delete[] dataDims;
  delete[] dataRange;
  delete[] dataCentre;
  delete[] dataBounds;

  dataPoints->Delete();
  dataValues->Delete();
  if(dataSet)
    dataSet->Delete();
  if(delaunay)
    delaunay->Delete();
}

void PlatoDataReader::readRhoFile() {
  //int meshDims[] = {0, 0, 0};
  int numPoints = 0;
  float tmpData[] = {0.0f, 0.0f, 0.0f, 0.0f};
  float cellVec[9];
  float bohr = 0.529177f;

  ifstream fin(rhoFilename);
  if(!fin) {
    std::cerr << "Could not open file: " << rhoFilename << std::endl;
    exit(1);
  }

  // read in the cellVec data...
  for(int i = 0; i < 9; i++) {
    fin >> cellVec[i];
    std::cout << cellVec[i] << std::endl;
    cellVec[i] *= bohr;
  }

  // read two other numbers, second one gives mesh type:
  // 0: Uniform
  // 1: Atom centred
  fin >> numPoints;
  fin >> numPoints;
  if(numPoints == 1) {
    uniformMesh = false;
    std::cout << "Atom-centred mesh...\n";
  }
  else
    std::cout << "Uniform mesh...\n";

  // read number of points...
  if(uniformMesh) {
    for(int i = 0; i < 3; i++) {
      fin >> dataDims[i];
    }
    numPoints = dataDims[0] * dataDims[1] * dataDims[2];
  }
  else {
    fin >> numPoints;
  }
  std::cout << "Number of points: " << numPoints << "...\n";
  dataPoints->Allocate(numPoints, 1000);
  dataValues->SetNumberOfComponents(1);
  dataValues->SetNumberOfTuples(numPoints);

  // read points and data...
  int index;
  float len1, len2, len3;
  if(uniformMesh) {
    for(int k = 0; k < dataDims[2]; k++) {
      len3 = (float) k / (float) dataDims[2];
      for(int j = 0; j < dataDims[1]; j++) {
	len2 = (float) j / (float) dataDims[1];
	for(int i = 0; i < dataDims[0]; i++) {
	  len1 = (float) i / (float) dataDims[0];

	  index = i + (j * dataDims[0]) + (k * dataDims[0] * dataDims[1]);
    
	  fin >> tmpData[3];
	  dataValues->InsertValue(index, tmpData[3]);

	  tmpData[0] = len1 * cellVec[0] + len2 * cellVec[3] + len3 * cellVec[6];
	  tmpData[1] = len1 * cellVec[1] + len2 * cellVec[4] + len3 * cellVec[7];
	  tmpData[2] = len1 * cellVec[2] + len2 * cellVec[5] + len3 * cellVec[8];

	  dataPoints->InsertPoint(index, tmpData[0], tmpData[1], tmpData[2]);
	} // i
      } // j
    } // k
  }
  else {
    for(int i = 0; i < numPoints; i++) {
      for(int j = 0; j < 4; j++) {
	fin >> tmpData[j];
      }

      dataPoints->InsertPoint(i, tmpData[0], tmpData[1], tmpData[2]);
      dataValues->InsertValue(i, tmpData[3]);
    }
  }

  fin.close();
}

void PlatoDataReader::buildPipeline() {
  if(uniformMesh) {
    dataSet = vtkStructuredGrid::New();
    static_cast<vtkStructuredGrid*>(dataSet)->SetDimensions(dataDims);
    dataSet->SetPoints(dataPoints);
    dataSet->GetPointData()->SetScalars(dataValues);
  }
  else {
    dataSet = vtkUnstructuredGrid::New();
    dataSet->SetPoints(dataPoints);
    dataSet->GetPointData()->SetScalars(dataValues);

    // with an unstructured grid a delaunay triangulation is required...
    delaunay = vtkDelaunay3D::New();
    delaunay->SetInput(dataSet);
    delaunay->SetAlpha(0.0);
    delaunay->SetTolerance(0.0001);
    delaunay->SetOffset(2.5);
    delaunay->BoundingTriangulationOff();
  }

  dataSet->Update();
  dataSet->GetScalarRange(dataRange);
  dataSet->GetCenter(dataCentre);
  dataSet->GetBounds(dataBounds);
}

vtkPointSet* PlatoDataReader::getData() {
  if(uniformMesh) {
    return dataSet;
  }
  else {
    return delaunay->GetOutput();
  }
}

int* PlatoDataReader::getDataDimensions() {
  return dataDims;
}

double* PlatoDataReader::getDataRange() {
  return dataRange;
}

double* PlatoDataReader::getDataCentre() {
  return dataCentre;
}

double* PlatoDataReader::getDataBounds() {
  return dataBounds;
}

bool PlatoDataReader::isUniformMesh() {
  return uniformMesh;
}
