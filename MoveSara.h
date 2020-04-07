#ifndef _MOVESARA_H_
#define _MOVESARA_H_

//#define PROCESSING_SIMULATOR
#define DEBUGGING_DATA
#include <AccelStepper.h>
#include <MultiStepper.h>

//Speed
#define millimeterSpeed 25;
//variables of the geometry
#define l1 76
#define l2 76
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
        int microstepping = 16;
        double degrees_per_step = (1.8 * PI * 2.0) / (6.0 * 180 * microstepping);
        double z_current;
        double theta_current;
        double couplingAngle;
        double x_current;
        double y_current;
        
    private:
        AccelStepper stepper1;
        AccelStepper stepper2;
        MultiStepper steppers;
        long maxSpeed;
        double q1_current;
        double q2_current;
        double x_home;
        double y_home;
        bool constantMotorSpeed = false;

    public:
        MoveSara(int = 16);
        void moveTo(double x, double y);
        void movePolarTo(double , double ); //(modulo,angulo)
        void init(int = 50, long = 0, long = 0);
        void goHome();
        void setHomePosition();
        void setCouplingAngle();
        static double z_polar(double , double );
        static double thetaPolar(double , double );
    private:
        void moveInterpolateTo(double , double , double );
        void moveSteps(long, long, double);
        //mathematics methods
        long calculate_steps(double , double );
        void calculate_line_equations();
        double normalize_angle(double );
        double module(double , double , double , double );
        double dk_x(double , double ) ;
        double dk_y(double , double ) ;
        void ik(double , double , double* , double* );
};

#endif