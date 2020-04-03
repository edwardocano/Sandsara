#include <AccelStepper.h>
#include <MultiStepper.h>

//Speed
#define millimeterSpeed 10;
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
double m[no_picos * 2], b[no_picos * 2];
double theta = 0;
double radius_1 = 400;
double radius_2 = 500;

class MoveSara {
    public:
        int microstepping = 16;
        double degrees_per_step = (1.8 * PI * 2.0) / (6.0 * 180 * microstepping);
        double z_current;
        double theta_current;
        double couplingAngle;
        
    private:
        AccelStepper stepper1;
        AccelStepper stepper2;
        MultiStepper steppers;
        long maxSpeed;
        double q1_current;
        double q2_current;
        double x_current;
        double y_current;
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
    private:
        void moveInterpolateTo(double , double , double );
        void moveSteps(long, long, double);
        //mathematics methods
        long calculate_steps(double , double );
        void calculate_line_equations();
        double normalize_angle(double );
        double module(double , double , double , double );
        double z_polar(double , double );
        double thetaPolar(double , double );
        double dk_x(double , double ) ;
        double dk_y(double , double ) ;
        void ik(double , double , double* , double* );
};

MoveSara::MoveSara(int microstepping){
    this->microstepping = microstepping;
    degrees_per_step = (1.8 * PI * 2.0) / (6.0 * 180 * microstepping);
    stepper1 = AccelStepper(1, STEP, DIR);
    stepper2 = AccelStepper(1, STEP2, DIR2);
    
}

void MoveSara::movePolarTo(double z_next, double theta_next) { //z must be in milimeters and theta in radians
  long slices_factor = 1000;

  double theta_current_aux = thetaPolar(x_current, y_current);
  double z_current_aux = z_polar(x_current, y_current);

  double distance_points, delta_theta, delta_z;

  delta_theta = (theta_next - theta_current) / slices_factor;
  delta_z = (z_next - z_current) / slices_factor;

  double theta_auxiliar = theta_current_aux + delta_theta;
  double z_auxliar = z_current_aux + delta_z;

  double x_aux = z_auxliar * cos(theta_auxiliar);
  double y_aux = z_auxliar * sin(theta_auxiliar);

  distance_points = sqrt(pow(x_aux - x_current, 2) + pow(y_aux - y_current, 2));

  slices_factor *= distance_points; //in order to have a distance of 1 mm;
  if (slices_factor < 1) {
    slices_factor = 1;
  }

  delta_theta = (theta_next - theta_current) / slices_factor;
  delta_z = (z_next - z_current) / slices_factor;

  for (long i = 1; i <= slices_factor; i++) {
    theta_auxiliar = theta_current + delta_theta * double(i);
    z_auxliar = z_current + delta_z * double(i);
    if (i == slices_factor) {
      theta_auxiliar = theta_next;
      z_auxliar = z_next;
    }
    x_aux = z_auxliar * cos(theta_auxiliar + couplingAngle);
    y_aux = z_auxliar * sin(theta_auxiliar + couplingAngle);

    moveTo(x_aux, y_aux);
  }

  theta_current = theta_next;
  z_current = z_next;
}

void MoveSara::moveSteps(long q1_steps, long q2_steps, double distance) { //distance is in milimeters
    long positions[2];
    /*int valor_pot = analogRead(A0);
    if (valor_pot < 100)
        valor_pot = 100;*/
    if (abs(q1_steps) > abs(q2_steps + q1_steps)) //importante
        maxSpeed = abs(q1_steps) * 1L;
    else
        maxSpeed = abs(q2_steps + q1_steps) * 1L;
    maxSpeed = (maxSpeed/distance)*millimeterSpeed;// * valor_pot / 1024.0 * 10.0;
    if (maxSpeed > 150 * microstepping)
        maxSpeed = 150 * microstepping;
    if (constantMotorSpeed)
        maxSpeed = 50 * microstepping;
    //variar velocidad
    /*Serial.print(q1_steps);
    Serial.print(",");
    Serial.print(q2_steps + q1_steps);
    Serial.print(",");
    Serial.println(maxSpeed);*/
    stepper1.setMaxSpeed(maxSpeed);
    stepper2.setMaxSpeed(maxSpeed);
    positions[0] = stepper1.currentPosition() + q1_steps;
    positions[1] = stepper2.currentPosition() + q2_steps + q1_steps;

    steppers.moveTo(positions);
    steppers.runSpeedToPosition();

    q1_current += degrees_per_step * q1_steps;
    q2_current += degrees_per_step * q2_steps;
    q1_current = normalize_angle(q1_current);
    q2_current = normalize_angle(q2_current);
    x_current = dk_x(q1_current, q2_current);
    y_current = dk_y(q1_current, q2_current);

} //50

void MoveSara::moveTo(double x, double y) {
    double q1, q2, distance;
    long steps_of_q1, steps_of_q2;
    distance = module(x, y, x_current, y_current);
    if (distance > 1.1){
        moveInterpolateTo(x, y, distance);
    }
    else if (distance > 0.5){
        ik(x, y, &q1, &q2);
        steps_of_q1 = calculate_steps(q1_current, q1);
        steps_of_q2 = calculate_steps(q2_current, q2);
        moveSteps(steps_of_q1, steps_of_q2, distance);
    }

}

void MoveSara::moveInterpolateTo(double x, double y, double distance){
    double alpha = atan2(y - y_current, x - x_current);
    double delta_x, delta_y;
    double x_aux = x_current, y_aux = y_current;
    delta_x = cos(alpha);
    delta_y = sin(alpha);
    int intervals = distance;
    for (int i = 1; i <= intervals; i++){
        x_aux += delta_x;
        y_aux += delta_y;
        moveTo(x_aux, y_aux);
    }
    moveTo(x,y);
}
//Configuration Methods---------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void MoveSara::init(int speedMax, long positionMotor1, long positionMotor2) { //40
  double q1, q2;
  stepper1.setMaxSpeed(speedMax * microstepping);
  stepper2.setMaxSpeed(speedMax * microstepping);
  stepper1.setCurrentPosition(positionMotor1);
  stepper2.setCurrentPosition(positionMotor2);
  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2);
  ik(0.0, 0.0, &q1, &q2);
  q1_current = q1;
  q2_current = q2;
  x_current = dk_x(q1_current, q2_current);
  y_current = dk_y(q1_current, q2_current);
  calculate_line_equations();
}

void MoveSara::setCouplingAngle(){
    couplingAngle = thetaPolar(x_current,y_current) - theta_current;
}

//----------------------------mathematics---------------------------------------------------

//Kinematics--------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double MoveSara::dk_x(double q1, double q2) {
  return l1 * cos(q1) + l2 * cos(q1 + q2);
}

double MoveSara::dk_y(double q1, double q2) {
  return l1 * sin(q1) + l2 * sin(q1 + q2);
}

void MoveSara::ik(double x, double y, double *q1, double *q2) {
  double z, z_max, theta;
  int auxiliar, i;
  //calculus of z
  z = sqrt(pow(x, 2) + pow(y, 2));
  //calculus of theta
  if (x > 0)
    theta = atan(y / x);
  else if (x < 0)
    theta = atan(y / x) + PI;
  else {
    if (y >= 0)
      theta = PI / 2;
    else
      theta = 3 / 2 * PI;
  }
  //if x,y is out of range, z will be the maximun radius possible
  if (z > l1 + l2 )
    z = l1 + l2;


  //theta always possitive
  if (theta < 0)
    theta = theta + 2 * PI;
  //Delimiter module z
  i = theta / (2 * PI / (2 * no_picos));
  z_max = abs(b[i] / (tan(theta) - m[i]) * sqrt(1 + pow(tan(theta), 2)));
  if (z > z_max)
    z = z_max;
  //calculus of q1 that is always possitive
  *q1 = theta - acos(z / (2 * l1));
  if (*q1 < 0)
    *q1 = *q1 + 2 * PI;
  //calculus of q2
  *q2 = 2 * (theta + 2 * PI - *q1);
  auxiliar = *q2 / (2 * PI);
  //in case q2 is greater than 2pi
  *q2 = *q2 - 2 * PI * auxiliar;
}

//Operations with components -------------------------------------------------------
//----------------------------------------------------------------------------------
double MoveSara::module(double x1, double y1, double x2, double y2){
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

double MoveSara::z_polar(double x, double y) {
  return sqrt(pow(x, 2) + pow(y, 2));
}

double MoveSara::thetaPolar(double x, double y) {
  //calculus of theta
  double theta;
  if (x > 0)
    theta = atan(y / x);
  else if (x < 0)
    theta = atan(y / x) + PI;
  else {
    if (y >= 0)
      theta = PI / 2;
    else
      theta = 3 / 2 * PI;
  }
  return theta;
}

/**
 * normalize an angle to be between 0 and 2*pi.
 *@param angle Is the angle to be limited, which is in radians.
 *@return a double value between [0 and 2*pi).
 */
double MoveSara::normalize_angle(double angle) {
  long aux = angle / (2 * PI);
  angle -= aux * 2 * PI;
  if (angle < 0) {
    return angle + 2 * PI;
  }
  else {
    return angle;
  }
}
//Workspace mathematics---------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void MoveSara::calculate_line_equations() {
  double z = radius_2;
  double theta;
  for (int i = 0; i < 2 * no_picos; i++) {
    double x1, x2, y1, y2;
    theta = 2 * PI / (2 * no_picos) * i;
    if (i % 2 == 0)
      z = radius_1;
    else
      z = radius_2;
    x1 = z * cos(theta);
    y1 = z * sin(theta);
    if (i % 2 != 0)
      z = radius_1;
    else
      z = radius_2;
    theta = 2 * PI / (2 * no_picos) * (i + 1);
    x2 = z * cos(theta);
    y2 = z * sin(theta);
    m[i] = (y2 - y1) / (x2 - x1);
    b[i] = y2 - m[i] * x2;
  }
}

//Motor maths--------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
/**
 * Calculate the steps to go form one angle to another one accoording to the stepping
 * configuration of the motor. As there are two option to achive the position, the shortest one
 * is chosen. 
 * @param q1 is the current angle
 * @param q2 is the next angle to be achived
 * @return the necessary steps to go from q1 to q2
 */
long MoveSara::calculate_steps(double q1, double q2) {
  double steps_option_1, steps_option_2;
  int aux1, aux2;
  steps_option_1 = 2 * PI - q2 + q1;
  steps_option_2 = 2 * PI - q1 + q2;
  aux1 = steps_option_1 / (2 * PI);
  aux2 = steps_option_2 / (2 * PI);
  steps_option_1 = steps_option_1 - aux1 * 2 * PI;
  steps_option_2 = steps_option_2 - aux2 * 2 * PI;
  if (steps_option_1 <= steps_option_2)
    return -steps_option_1 / degrees_per_step;
  else
    return steps_option_2 / degrees_per_step;
}
