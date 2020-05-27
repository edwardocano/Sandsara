#ifndef _CALIBMOTOR_H_
#define _CALIBMOTOR_H_

#include <Arduino.h>
#include <TMCStepper.h>
#include <AccelStepper.h>
#include "MeanFilterLib.h"
#include <HardwareSerial.h>

#define encoder           39
#define hall              36

#define DIAG_PIN            34 //STALL motor 1
#define EN_PIN              14 // Enable motor 1
#define DIR_PIN             33  // Direction
#define STEP_PIN            25 // Step
#define SERIAL_PORT         Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS      0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f 
#define STALL_VALUE         5 // [0..255]

#define DIAG_PIN2           22 //STALL motor 2
#define EN_PIN2             15 // Enable
#define DIR_PIN2            4 // Direction
#define STEP_PIN2           21 // Step
#define SERIAL_PORT2        Serial2 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS2     0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE2            0.11f 
#define STALL_VALUE2        2




class CalibMotor{
    public:
        CalibMotor();
        void verificacion_cal(void);
    private:

    public:
        int init();
        int start();
};

#endif
