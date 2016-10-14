#******************************************************************************
#
# OGC Web Coverage Service (WCS) implementation source code
# ---------------------------------------------------------
# GMU CSISS WCS.
#
# Copyright (C) 2015 CSISS, GMU (http://csiss.gmu.edu) and Liping Di (ldi@gmu.edu)
#
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#******************************************************************************
################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AbstractDataset.cpp \
../src/BoundingBox.cpp \
../src/HE4_GRID_Dataset.cpp \
../src/HE4_SWATH_Dataset.cpp \
../src/HE5_GRID_Dataset.cpp \
../src/HE5_SWATH_Dataset.cpp \
../src/NC_GOES_Dataset.cpp \
../src/NITF_Dataset.cpp \
../src/TRMM_Dataset.cpp \
../src/wcsUtil.cpp \
../src/wcs_error.cpp 

OBJS += \
./src/AbstractDataset.o \
./src/BoundingBox.o \
./src/HE4_GRID_Dataset.o \
./src/HE4_SWATH_Dataset.o \
./src/HE5_GRID_Dataset.o \
./src/HE5_SWATH_Dataset.o \
./src/NC_GOES_Dataset.o \
./src/NITF_Dataset.o \
./src/TRMM_Dataset.o \
./src/wcsUtil.o \
./src/wcs_error.o 

CPP_DEPS += \
./src/AbstractDataset.d \
./src/BoundingBox.d \
./src/HE4_GRID_Dataset.d \
./src/HE4_SWATH_Dataset.d \
./src/HE5_GRID_Dataset.d \
./src/HE5_SWATH_Dataset.d \
./src/NC_GOES_Dataset.d \
./src/NITF_Dataset.d \
./src/TRMM_Dataset.d \
./src/wcsUtil.d \
./src/wcs_error.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/opt/local/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


