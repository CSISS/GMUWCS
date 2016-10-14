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
../src/WCS_Configure.cpp \
../src/WCS_DescribeCoverage.cpp \
../src/WCS_GetCapabilities.cpp \
../src/WCS_GetCoverage.cpp \
../src/WCS_T.cpp \
../src/wcst.cpp 

OBJS += \
./src/WCS_Configure.o \
./src/WCS_DescribeCoverage.o \
./src/WCS_GetCapabilities.o \
./src/WCS_GetCoverage.o \
./src/WCS_T.o \
./src/wcst.o 

CPP_DEPS += \
./src/WCS_Configure.d \
./src/WCS_DescribeCoverage.d \
./src/WCS_GetCapabilities.d \
./src/WCS_GetCoverage.d \
./src/WCS_T.d \
./src/wcst.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../WCS20lib/src -I/opt/local/include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


