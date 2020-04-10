#include "MoveSara.h"
double m[no_picos * 2], b[no_picos * 2];

/**
 * @brief es el contructor de la clase
 * @param microstepping es el microstepping que tienen los motores, por defecto es 16
 */
MoveSara::MoveSara(int microstepping){
    this->microstepping = microstepping;
    degrees_per_step = (littlePulleySize / bigPulleySize) * (PI / 180.0) * (1.8 / microstepping);
    stepper1 = AccelStepper(1, STEP, DIR);
    stepper2 = AccelStepper(1, STEP2, DIR2);
    
}

/**
 * @brief Interpola los puntos necesarios entre el punto actual y el siguiente con el objetivo que
 * se mueva en coordenadas polares como lo hace sisyphus.
 * @param zNext valor en el eje z polar, medido en milimetros.
 * @param thetaNext valor en el eje theta polar, medido en radianes.
 * @note es muy importante que se hayan definido las variables zCurrent y thetaCurrent antes
 * de llamar a esta funcion porque es apartir de estas variables que se calcula cuanto se debe mover.
 */
void MoveSara::movePolarTo(double zNext, double thetaNext) {
  double slicesFactor = 1000;
  long slices;
  double distancePoints = 100, deltaTheta, deltaZ;
  double thetaAuxiliar, zAuxliar, xAux, yAux;
  int overIterates = 0;
  while (!(distancePoints > 0.9 && distancePoints < 1.1)){
    deltaTheta = (thetaNext - thetaCurrent) / slicesFactor;
    deltaZ = (zNext - zCurrent) / slicesFactor;
    thetaAuxiliar = thetaCurrent + deltaTheta;
    zAuxliar = zCurrent + deltaZ;
    distancePoints = polarModule(zCurrent, thetaCurrent, zAuxliar, thetaAuxiliar);
    slicesFactor *= distancePoints;
    overIterates += 1;
    if (slicesFactor == 0){
      break;
    }
    if (overIterates > 10){
      break;
    }
  }
  slices = slicesFactor;
  if (slices < 1) {
    slices = 1;
  }

  deltaTheta = (thetaNext - thetaCurrent) / slices;
  deltaZ = (zNext - zCurrent) / slices;
  for (long i = 1; i < slices; i++) {
    thetaAuxiliar = thetaCurrent + deltaTheta * double(i);
    zAuxliar = zCurrent + deltaZ * double(i);
    xAux = zAuxliar * cos(thetaAuxiliar);
    yAux = zAuxliar * sin(thetaAuxiliar);
    #ifdef DEBUGGING_DETAIL
    Serial.println("Se ejecutara moveTo dentro de movePolarTo: ");
    #endif
    moveTo(xAux, yAux);
  }
  xAux = zNext * cos(thetaNext);
  yAux = zNext * sin(thetaNext);
  #ifdef DEBUGGING_DETAIL
  Serial.println("Se ejecutara ultimo moveTo dentro de movePolarTo: ");
  #endif
  moveTo(xAux, yAux);
  thetaCurrent = thetaNext;
  zCurrent = zNext;
}

/**
 * @brief mueve los motores 1 y 2.
 * 
 * Los movera a una velocidad que depende de la distancia que va a recorrer el punto final del robot.
 * @param q1_steps es el numero de pasos que va a girar el motor correspondiente al angulo q1.
 * @param q2_steps es el numero de pasos que va a girar el motor correspondiente al angulo q2.
 * @param distance es la distancia que va a recorrer entre el punto actual y el punto despues del momimiento.
 * La distancia se mide en milimetros 
 */
void MoveSara::moveSteps(long q1_steps, long q2_steps, double distance) { //distance is in milimeters
    long positions[2];
    if (abs(q1_steps) > abs(q2_steps + q1_steps)) //importante
        maxSpeed = abs(q1_steps) * 1L;
    else
        maxSpeed = abs(q2_steps + q1_steps) * 1L;
    maxSpeed = (maxSpeed/distance)*millimeterSpeed;
    if (maxSpeed > 150 * microstepping)
        maxSpeed = 150 * microstepping;
    if (constantMotorSpeed)
        maxSpeed = 50 * microstepping;
    //variar velocidad
    #ifdef PROCESSING_SIMULATOR
    Serial.print(q1_steps);
    Serial.print(",");
    Serial.print(q2_steps + q1_steps);
    Serial.print(",");
    Serial.println(maxSpeed);
    #endif
    stepper1.setMaxSpeed(maxSpeed);
    stepper2.setMaxSpeed(maxSpeed);
    positions[0] = stepper1.currentPosition() + q1_steps;
    positions[1] = stepper2.currentPosition() + q2_steps + q1_steps;
    #ifndef DISABLE_MOTORS
    steppers.moveTo(positions);
    steppers.runSpeedToPosition();
    #endif
    q1_current += degrees_per_step * q1_steps;
    q2_current += degrees_per_step * q2_steps;
    q1_current = normalizeAngle(q1_current);
    q2_current = normalizeAngle(q2_current);
    x_current = dk_x(q1_current, q2_current);
    y_current = dk_y(q1_current, q2_current);

}

/**
 * @brief El robot se mueve de su posicion actual hacia una nueva posicion x,y.
 * 
 * Si las coordenadas x,y se encuentran fuera del espacio de trabajo, la funcion se encarga de limitarlas.
 * La posicion x,y se mide en milimetros.
 * Si la distancia que tiene que recorrer es menor a 0.5 milimetros entonces no avanza.
 * Si la distancia a recorrer es mayor a 1.1 milimetros, entonces se avanza en linea recta del origen al final a traves
 * puntos separados por 1 mm.
 * @param x es la coordenada en el eje x, medida en milimetros, hacia donde se desea avanzar.
 * @param y es la coordenada en el eje y, medida en milimetros, hacia donde se desea avanzar.
 */
void MoveSara::moveTo(double x, double y) {
    double q1, q2, distance;
    long steps_of_q1, steps_of_q2;
    distance = module(x, y, x_current, y_current);
    if (distance > 1.1){
        #ifdef DEBUGGING_DETAIL
        Serial.println("Se ejecutara moveInterpolateTo: ");
        #endif
        moveInterpolateTo(x, y, distance);
    }
    else if (distance > 0.5){
        if (!(x == 0 && y == 0)){
          ik(x, y, &q1, &q2);
          steps_of_q1 = calculate_steps(q1_current, q1);
          steps_of_q2 = calculate_steps(q2_current, q2);
          #ifdef DEBUGGING_DETAIL
          Serial.println("Se ejecutara moveSteps (Esta deshabilitado) ");
          #endif  
          moveSteps(steps_of_q1, steps_of_q2, distance);
        }
    }

}

/**
 * @brief Se usa esta funcion para avanzar de la posicion actual a un punto nuevo en linea recta por medio de puntos equidistantes a un 1 mm.
 * @param x coordenada en el eje x, medida en milimetros, a la que se desea avanzar.
 * @param y coordenada en el eje y, medida en milimetros, a la que se desea avanzar.
 * @param distance es la distancia, medida en milimetros, entre el punto actual y el punto al que se desea avanzar.
 */
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
        #ifdef DEBUGGING_DETAIL
        Serial.println("Se ejecutara moveTo dentro de moveInterpolateTo: ");
        #endif
        moveTo(x_aux, y_aux);
    }
    #ifdef DEBUGGING_DETAIL
    Serial.println("Se ejecutara ultimo moveTo dentro de moveInterpolateTo: ");
    #endif
    moveTo(x,y);
}
//properties----------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @return La distancia entre el centro y la posicion actual del robot.
 */
double MoveSara::getCurrentModule(){
  return zPolar(x_current, y_current);
}

/**
 * @return El angulo, medido desde la horizontal, de la posicion actual del robot.
 * @note el angulo se encuentra en el rango de [0 , 2*PI)
 */
double MoveSara::getCurrentAngle(){
  return thetaPolar(x_current, y_current);
}

//Configuration Methods-----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief inicializa el objeto MoveSara
 * @param xInit es la coordenada en el eje x, medida en milimetros, que se desea como posicion inicial.
 * @param yInit es la coordenada en el eje y, medida en milimetros, que se desea como posicion inicial.
 * @note por defecto el punto inicial es 0,0.
 */
void MoveSara::init(double xInit, double yInit) {
  double q1, q2;
  stepper1.setMaxSpeed(50 * microstepping);
  stepper2.setMaxSpeed(50 * microstepping);
  stepper1.setCurrentPosition(0);
  stepper2.setCurrentPosition(0);
  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2);
  ik(xInit, yInit, &q1, &q2);
  q1_current = q1;
  q2_current = q2;
  x_current = dk_x(q1_current, q2_current);
  y_current = dk_y(q1_current, q2_current);
  calculate_line_equations();
}

/**
 * @brief define el angulo de acoplamiento.
 * @param angle el angulo que se desea como angulo de acoplamiento.
 * @note no retorna nada pero moficica la variable couplingAngle.
 */
void MoveSara::setCouplingAngle(double angle){
    couplingAngle = normalizeAngle(angle);
}

/**
 * @brief moficica el miembro z_current a uno nuevo.
 * @param z es el valor que se le va a asignar a la variable miembro zCurrent.
 */
void MoveSara::setZCurrent(double z){
    zCurrent = z;
}

/**
 * @brief moficica el miembro theta_current a uno nuevo.
 * @param theta es el valor que se le va a asignar a la variable miembro thetaCurrent.
 * @note  Esto es importante para que los archivos .thr tengan una referencia de donde empezar a moverse.
 */
void MoveSara::setThetaCurrent(double theta){
    thetaCurrent = theta;
}
//----------------------------mathematics---------------------------------------------------

//Kinematics--------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/**
 * @brief Calcula la cinematica directa del robot para la coordenada en x.
 * @param q1 es el angulo del motor que mueve el primer eslabon del robot.
 * @param q2 es el angulo del motor que mueve el segundo eslabon del robot.
 * @return la coordenada en el eje x, medida en milimetros, correspondiente a los angulos q1,q2 del robot.
 */
double MoveSara::dk_x(double q1, double q2) {
  return l1 * cos(q1) + l2 * cos(q1 + q2);
}

/**
 * @brief Calcula la cinematica directa del robot para la coordenada en y.
 * @param q1 es el angulo del motor que mueve el primer eslabon del robot (motor 1).
 * @param q2 es el angulo del motor que mueve el segundo eslabon del robot (motor 2).
 * @return la coordenada en el eje y, medida en milimetros, correspondiente a los angulos q1,q2 del robot.
 */
double MoveSara::dk_y(double q1, double q2) {
  return l1 * sin(q1) + l2 * sin(q1 + q2);
}

/**
 * @brief Calcula la cinematica inversa del robot.
 * @param x coordenada en el eje x, medida en milimetros.
 * @param y coordenada en el eje y, medida en milimetros.
 * @param q1 el angulo correspondiente al motor 1, en la posicion segun los parametros x,y.
 * @param q1 el angulo correspondiente al motor 2, en la posicion segun los parametros x,y.
 */
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
/**
 * @brief rota la posicion x,y con centro en 0,0 tantos grados como se desee.
 * @param x coordenada en el eje x.
 * @param y coordenada en el eje y.
 * @param angle es el angulo que se desea rotar los puntos x,y.
 * @note los valores x,y se pasan por referencia.
 */
void MoveSara::rotate(double& x,double& y,double angle){
    double z = zPolar(x, y);
    double theta = thetaPolar(x, y);
    theta += angle;
    x = z * cos(theta);
    y = z * sin(theta);
}

/**
 * @brief Calcula la distancia entre 2 puntos.
 * @param x1 es el valor en el eje x del primer punto.
 * @param y1 es el valor en el eje y del primer punto.
 * @param x2 es el valor en el eje x del segundo punto.
 * @param y2 es el valor en el eje y del segundo punto.
 * @return la distancia entre ambos puntos.
 */
double MoveSara::module(double x1, double y1, double x2, double y2){
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/**
 * @brief Calcula la distancia entre 2 puntos Polares.
 * @param z1 es el valor en el eje z del primer punto.
 * @param t1 es el valor en el eje theta del primer punto.
 * @param z2 es el valor en el eje z del segundo punto.
 * @param t2 es el valor en el eje theta del segundo punto.
 * @return la distancia entre ambos puntos.
 */
double MoveSara::polarModule(double z1, double t1, double z2, double t2){
    double x1, y1, x2, y2;
    x1 = z1 * cos(t1);
    y1 = z1 * sin(t1);
    x2 = z2 * cos(t2);
    y2 = z2 * sin(t2);
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/**
 * @brief calcula el valor del modulo del punto en coordenadas polares
 * @param x es el valor en el eje x del punto.
 * @param y es el valor en el eje y del punto.
 * @return el modulo del punto en coordenadas polares.
 */
double MoveSara::zPolar(double x, double y) {
  return sqrt(pow(x, 2) + pow(y, 2));
}

/**
 * @brief calcula el angulo que se forma con la horizontal, de un punto x,y.
 * @param x es el valor de la coordenada en el eje x, medido en milimetros.
 * @param y es el valor de la coordenada en el eje y, medido en milimetros.
 * @return el angulo, medido desde la horizontal, del punto x,y.
 * @note el angulo se encuentra en el rango de [0 , 2*PI)
 */
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
  return normalizeAngle(theta);
}

/**
 *@brief normaliza un angulo para estar en el rango de [0,2*PI).
 *@param angle es el angulo, medido en radianes, que se desea normalizar.
 *@return un valor con el rango de [0 , 2*PI).
 */
double MoveSara::normalizeAngle(double angle) {
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
/**
 * @brief Calcula las escuaciones que describen la geometria de Stelle.
 * @return no retorna ningun valor pero modifica las variables miembro m y b.
 * @note m contiene las pendientes de las rectas que describen la geometria de Stelle.
 *       b contiene informacion de las rectas que describen la geometria de Stelle.
 */
void MoveSara::calculate_line_equations() {
  double radius_1 = 400;
  double radius_2 = 500;
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
 * @brief Calcula los pasos que se debe mover el motor para ir de un angulo a otro.
 * @note existen 2 caminos para llegar a un cierto angulo, pero siempre se escoge el mas corto.
 * @param q1 es el angulo inicial.
 * @param q2 es el angulo al que se quiere llegar.
 * @return los angulos necesarios para ir de q1 a q2.
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
