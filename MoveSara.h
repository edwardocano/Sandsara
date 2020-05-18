#ifndef _MOVESARA_H_
#define _MOVESARA_H_

#define PROCESSING_SIMULATOR
//#define DEBUGGING_DATA
//#define DEBUGGING_DETAIL
//#define DISABLE_MOTORS

//Speed
#define millimeterSpeed 25;
//variables of the geometry
#define l1 76
#define l2 76
#define bigPulleySize 6.0
#define littlePulleySize 2.0
#define maximun_radius 100.0
//variables of steppers pin
#define DIR 33
#define STEP 25
#define DIR2 26
#define STEP2 27
//variables of star equations
#define no_picos 6

#include "AccelStepper.h"
#include "MultiStepper.h"

/**
 * @class MoveSara
 * @param microstepping almacena el valor de microstepping del motor
 * @param x_current alamacena la coordenada x de la posicion actual del robot (en milimetros).
 * @param y_current alamacena la coordenada y de la posicion actual del robot (en milimetros).
 * @param q1_current almacena el valor actual del angulo del primer eslabon del robot (en radianes).
 * @param q2_current almacena el valor actual del angulo del segundo eslabon del robot (en radianes).
 * @param zCurrent se utiliza como referencia del componente  modulo para los archivos thr (en milimetros).
 * @param thetaCurrent se utiliza como referencia del componente del angulo para los archivos thr (en radianes).
 * @param maxSpeed se utiliza para controlar la velocidad de los motores (pasos/s).
 * @param constantMotorSpeed Si esta variable es true, los motores se moveran a una velocidad constante siempre, si es false la velocidad
 * dependera de la distancia recorrida de un punto a otro.
 */
class MoveSara {
    public:
        int microstepping;
        double degrees_per_step;
        double couplingAngle;
        double x_current;
        double y_current;
        double q1_current;
        double q2_current;
        
    private:
        AccelStepper stepper1;
        AccelStepper stepper2;
        MultiStepper steppers;
        double zCurrent;
        double thetaCurrent;
        long maxSpeed;        
        double x_home;
        double y_home;
        bool constantMotorSpeed = false;

    public:
        MoveSara(int = 16);
        void moveTo(double x, double y);
        void movePolarTo(double , double ); //(modulo,angulo)
        void init(double = 0,double = 0);
        void goHome();
        void setHomePosition();
        void setCouplingAngle(double );
        double getCurrentModule();
        double getCurrentAngle();
        void setZCurrent(double );
        void setThetaCurrent(double );
        double getZCurrent();
        double getThetaCurrent();
        static void rotate(double& ,double& ,double );
        static double zPolar(double , double );
        static double thetaPolar(double , double );
        static double normalizeAngle(double );
        static double arcLength(double ,double , double);
    private:
        void moveInterpolateTo(double , double , double );
        void moveSteps(long, long, double);
        //mathematics methods
        long calculate_steps(double , double );
        void calculate_line_equations();
        double module(double , double , double , double );
        double polarModule(double , double , double , double );
        double dk_x(double , double ) ;
        double dk_y(double , double ) ;
        void ik(double , double , double* , double* );
};

#endif