#pragma once

//Speed
//#define millimeterSpeed 15;

#include "AccelStepper.h"
#include "MultiStepper.h"
#include "Bluetooth.h"
#include "config.h"

/**
 * @class Motors
 * @param microstepping almacena el valor de microstepping del motor
 * @param x_current alamacena la coordenada x de la posicion actual de Sandsara (en milimetros).
 * @param y_current alamacena la coordenada y de la posicion actual de Sandsara (en milimetros).
 * @param q1_current almacena el valor actual del angulo del primer eslabon de Sandsara (en radianes).
 * @param q2_current almacena el valor actual del angulo del segundo eslabon de Sandsara (en radianes).
 * @param zCurrent se utiliza como referencia del componente  modulo para los archivos thr (en milimetros).
 * @param thetaCurrent se utiliza como referencia del componente del angulo para los archivos thr (en radianes).
 * @param couplingAngle es un angulo que se utiliza para saber cuanto rotar un archivo.
 * @param constantMotorSpeed Si esta variable es true, los motores se moveran a una velocidad constante siempre, si es false la velocidad
 * dependera de la distancia recorrida de un punto a otro.
 */
class Motors {
    public:
        int microstepping;
        double degrees_per_step;
        double couplingAngle;
        double x_current;
        double y_current;
        double q1_current;
        double q2_current;
        AccelStepper stepper1;
        AccelStepper stepper2;
        AccelStepper stepper1Aux;
        AccelStepper stepper2Aux;
        MultiStepper steppers;
        MultiStepper stepps;
        int millimeterSpeed = 15;

    private:
        double zCurrent;
        double thetaCurrent;
        long maxSpeed;        
        double x_home;
        double y_home;
        bool constantMotorSpeed = false;
        
    public:
        Motors();
        void moveTo(double x, double y, bool = false);
        void movePolarTo(double , double ); //(modulo,angulo)
        void init(double = 0,double = 0);
        void goHome();
        void setHomePosition();
        void setCouplingAngle(double );
        double getCurrentModule();
        double getCurrentAngle();
        void setZCurrent(double );
        void setThetaCurrent(double );
        void setSpeed(int );
        int getSpeed();
        double getZCurrent();
        double getThetaCurrent();
        static void rotate(double& ,double& ,double );
        static double zPolar(double , double );
        static double thetaPolar(double , double );
        static double normalizeAngle(double );
        static double arcLength(double ,double , double);
        double module(double , double , double , double );
        int position();
        void completePath();
        
    private:
        void moveInterpolateTo(double ,double ,double ,bool );
        void moveSteps(long, long, double);
        //mathematics methods
        long calculate_steps(double , double );
        void calculate_line_equations();
        double polarModule(double , double , double , double );

        void ik(double , double , double* , double* );
};