
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
	g++ -I/opt/local/include -I/home/yshao/ows9/wcs/source/WCS20lib/src -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


