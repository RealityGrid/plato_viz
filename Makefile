#  Makefile for the RealityGrid Plato Visualization System.
#
#  (C) Copyright 2005, University of Manchester, United Kingdom,
#  all rights reserved.
#
#  This software was developed by the RealityGrid project
#  (http://www.realitygrid.org), funded by the EPSRC under grants
#  GR/R67699/01 and GR/R67699/02.
#
#  LICENCE TERMS
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  THIS MATERIAL IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. THE ENTIRE RISK AS TO THE QUALITY
#  AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
#  DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
#  CORRECTION.
#
#  Author........: Robert Haines
#
#------------------------------------------------------------------------

TARGET=pvs

REG_INCLUDES=-I${REG_STEER_HOME}/include

REG_LINK=-L${REG_STEER_HOME}/lib32 -lReG_Steer -lxml2 #-lReG_Steer_Utils

VTK_INCLUDES=-I/opt/software/vtk/VTK -I/opt/software/vtk/VTK/Common -I/opt/software/vtk/VTK/Filtering -I/opt/software/vtk/VTK/Graphics -I/opt/software/vtk/VTK/Hybrid -I/opt/software/vtk/VTK/Imaging -I/opt/software/vtk/VTK/IO -I/opt/software/vtk/VTK/Rendering -I/opt/software/vtk/VTK/Utilities -I/opt/software/vtk/VTK/VolumeRendering -I/opt/software/vtk/VTK/Widgets

VTK_LINK=-L/opt/software/vtk/VTK/bin -lvtkCommon -lvtkFiltering -lvtkGraphics -lvtkHybrid -lvtkImaging -lvtkIO -lvtkRendering -lvtkVolumeRendering -lvtkWidgets

CXX=g++
CPPFLAGS=-DPVS_BIN_NAME=\"${TARGET}\" -Iinclude ${REG_INCLUDES} ${VTK_INCLUDES}
CXXFLAGS=-Wno-deprecated -O3 -pipe
LDFLAGS=${REG_LINK} ${VTK_LINK}

OBJECTS=src/main.o \
	src/PlatoDataReader.o \
	src/PlatoIsoPipeline.o \
	src/PlatoOrthoPipeline.o \
	src/PlatoRenderWindow.o \
	src/PlatoVTKPipeline.o \
	src/PlatoXYZPipeline.o \
	src/realitygrid.o

# build the objects and link into the executable...

${TARGET}:	${OBJECTS}
	${CXX} -o ${TARGET} ${LDFLAGS} ${OBJECTS}

.cpp.o:
	${CXX} -o $@ ${CPPFLAGS} ${CXXFLAGS} -c $<

# package things up...

tar:	distclean
	cd ..; tar cjf pvs.tar.bz2 reg_plato_vis

# clean up...

distclean:	clean
	rm -f src/*~
	rm -f include/*~
	rm -f *~

clean:
	rm -f ${OBJECTS}
	rm -f ${TARGET}
