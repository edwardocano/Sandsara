#ifndef _CALIBMOTOR_H_
#define _CALIBMOTOR_H_

#include <Arduino.h>
#include <TMCStepper.h>
#include <AccelStepper.h>
#include "MeanFilterLib.h"
#include <HardwareSerial.h>

#define encoder           4
#define hall              15

#define DIAG_PIN            18 //STALL motor 1
#define EN_PIN              32 // Enable motor 1
#define DIR_PIN             22  // Direction
#define STEP_PIN            12 // Step
#define SERIAL_PORT         Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS      0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f 
#define STALL_VALUE         5 // [0..255]

#define DIAG_PIN2           19 //STALL motor 2
#define EN_PIN2             5 // Enable
#define DIR_PIN2            23 // Direction
#define STEP_PIN2           14 // Step
#define SERIAL_PORT2        Serial2 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS2     0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE2            0.11f 
#define STALL_VALUE2        2



class CalibMotor{
    public:
        CalibMotor();
    private:

    public:
        int init();
        int start();
};

#endif
