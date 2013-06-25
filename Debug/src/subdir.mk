################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Communications.cpp \
../src/Encryption.cpp \
../src/Graphics.cpp \
../src/Networking.cpp \
../src/Socket.cpp \
../src/TCPSocket.cpp \
../src/UDPSocket.cpp 

OBJS += \
./src/Communications.o \
./src/Encryption.o \
./src/Graphics.o \
./src/Networking.o \
./src/Socket.o \
./src/TCPSocket.o \
./src/UDPSocket.o 

CPP_DEPS += \
./src/Communications.d \
./src/Encryption.d \
./src/Graphics.d \
./src/Networking.d \
./src/Socket.d \
./src/TCPSocket.d \
./src/UDPSocket.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lcrypto `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


