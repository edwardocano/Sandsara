#ifndef _MOVESARA_H_
#define _MOVESARA_H_

//#define PROCESSING_SIMULATOR
#define DEBUGGING_DATA
//#define DEBUGGING_DETAIL
//#define DISABLE_MOTORS

#include <AccelStepper.h>
#include <MultiStepper.h>

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
        static void rotate(double& ,double& ,double );
        static double zPolar(double , double );
        static double thetaPolar(double , double );
        static double normalizeAngle(double );
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