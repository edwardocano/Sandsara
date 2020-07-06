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
        void verif_cal_Positivob1();
        void verif_cal_Positivob2();
        void verif_cal_Negativob1();
        void verif_cal_Negativob2();
    private:

    public:
        int init();
        int start();
};

#endif
