#ifndef _CALIBMOTOR_H_
#define _CALIBMOTOR_H_

#include <Arduino.h>
#include <TMCStepper.h>
#include <AccelStepper.h>
#include "MeanFilterLib.h"
#include <HardwareSerial.h>
#include <EEPROM.h>
#include "config.h"

class CalibMotor{
    public:
        CalibMotor();
        void verificacion_cal();
    private:

    public:
        int init();
        int start();
};

#endif
