################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CBSTmain.cpp \
../LockFreeBST.cpp 

O_SRCS += \
../CBSTmain.o \
../LockFreeBST.o 

OBJS += \
./CBSTmain.o \
./LockFreeBST.o 

CPP_DEPS += \
./CBSTmain.d \
./LockFreeBST.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


