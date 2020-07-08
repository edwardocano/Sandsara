#include "MoveSara.h"
#include "AccelStepper.h"
#include "MultiStepper.h"
#include "BlueSara.h"

//extern void ledsFunc();
double m[no_picos * 2], b[no_picos * 2];
extern bool productType;
extern bool pauseModeGlobal;

//====Prototipos de funcion====
double dkX(double , double );
double dkY(double , double );

/**
 * @brief es el contructor de la clase
 * @param microstepping es el microstepping que tienen los motores, por defecto es 16
 */
MoveSara::MoveSara()
{
    this->microstepping = MICROSTEPPING;
    degrees_per_step = (littlePulleySize / bigPulleySize) * (PI / 180.0) * (1.8 / microstepping);
    stepper1 = AccelStepper(1, STEP_PIN, DIR_PIN);
    stepper2 = AccelStepper(1, STEP_PIN2, DIR_PIN2);
}

/**
 * @brief Interpola los puntos necesarios entre el punto actual y el siguiente con el objetivo que
 * se mueva en coordenadas polares como lo hace sisyphus.
 * @param zNext valor en el eje z polar, medido en milimetros.
 * @param thetaNext valor en el eje theta polar, medido en radianes.
 * @note es muy importante que se hayan definido las variables zCurrent y thetaCurrent antes
 * de llamar a esta funcion porque es apartir de estas variables que se calcula cuanto se debe mover.
 */
void MoveSara::movePolarTo(double zNext, double thetaNext)
{
    double slicesFactor = 1000;
    long slices;
    double distancePoints = 100, deltaTheta, deltaZ;
    double thetaAuxiliar, zAuxliar, xAux, yAux;
    int overIterates = 0;
    deltaTheta = thetaNext - thetaCurrent;
    deltaZ = zNext - zCurrent;
    slicesFactor = arcLength(deltaZ, deltaTheta, zCurrent);
    slices = slicesFactor;
    if (slices < 1)
    {
        slices = 1;
    }
    deltaTheta = (thetaNext - thetaCurrent) / slices;
    deltaZ = (zNext - zCurrent) / slices;
    for (long i = 1; i < slices; i++)
    {
        thetaAuxiliar = thetaCurrent + deltaTheta * double(i);
        zAuxliar = zCurrent + deltaZ * double(i);
        xAux = zAuxliar * cos(thetaAuxiliar);
        yAux = zAuxliar * sin(thetaAuxiliar);
        moveTo(xAux, yAux);
    }
    xAux = zNext * cos(thetaNext);
    yAux = zNext * sin(thetaNext);
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
void MoveSara::moveSteps(long q1_steps, long q2_steps, double distance)
{ //distance is in milimeters
    long positions[2];
    if (abs(q1_steps) > abs(q2_steps + q1_steps)) //importante
        maxSpeed = abs(q1_steps) * 1L;
    else
        maxSpeed = abs(q2_steps + q1_steps) * 1L;
    maxSpeed = (maxSpeed / distance) * millimeterSpeed;
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
    //========
    double factor, p[2];
    long pL[2];
    double delta1 = q1_steps;
    double delta2 = q2_steps + q1_steps;
    p[0] = stepper1.currentPosition();
    p[1] = stepper2.currentPosition();
    if (abs(delta1) > abs(delta2)){
        factor = delta1/1;
    }
    else{
        factor = delta2/1;
    }
    if (factor < 1){
        factor = 1;
    }
    delta1 = delta1/factor;
    delta2 = delta2/factor;
#ifndef DISABLE_MOTORS
    /*steppers.moveTo(positions);
    steppers.runSpeedToPosition();*/
    for(int i=0; i < int(factor); i++){
        p[0] += delta1;
        p[1] += delta2;
        pL[0] = p[0];
        pL[1] = p[1];
        //ledsFunc();
        steppers.moveTo(pL);
        steppers.runSpeedToPosition();
    }
    steppers.moveTo(positions);
    steppers.runSpeedToPosition();
#endif
    q1_current += degrees_per_step * q1_steps;
    q2_current += degrees_per_step * q2_steps;
    q1_current = normalizeAngle(q1_current);
    q2_current = normalizeAngle(q2_current);
    x_current = dkX(q1_current, q2_current);
    y_current = dkY(q1_current, q2_current);
}

/**
 * @brief Esta funcion hace que el robot se mueve de su posicion actual hacia una nueva posicion x,y.
 * 
 * Si las coordenadas x,y se encuentran fuera del espacio de trabajo, la funcion se encarga de limitarlas.
 * La posicion x,y se mide en milimetros.
 * Si la distancia que tiene que recorrer es menor a 0.5 milimetros entonces no avanza.
 * Si la distancia a recorrer es mayor a 1.1 milimetros, entonces se avanza en linea recta del origen al final a traves
 * puntos separados por 1 mm.
 * @param x es la coordenada en el eje x, medida en milimetros, hacia donde se desea avanzar.
 * @param y es la coordenada en el eje y, medida en milimetros, hacia donde se desea avanzar.
 */
void MoveSara::moveTo(double x, double y, bool littleMovement)
{
    double q1, q2, distance;
    long steps_of_q1, steps_of_q2;
    if (pauseModeGlobal){
        while (pauseModeGlobal)
        {
            delay(200);
        }
    }
    ik(x, y, &q1, &q2);
    if (x == 0 && y == 0){
        Serial.print("q1: ");
        Serial.print(q1);
        Serial.print("\tq2: ");
        Serial.println(q2);
    }
    x = dkX(q1, q2);
    y = dkY(q1, q2);
    distance = module(x, y, x_current, y_current);
    if (distance > 1.1)
    {
        moveInterpolateTo(x, y, distance, littleMovement);
    }
    else if (distance > 0.5 || littleMovement)
    {
        if (true)//!(x == 0 && y == 0) || littleMovement)
        {
            ik(x, y, &q1, &q2);
            steps_of_q1 = calculate_steps(q1_current, q1);
            steps_of_q2 = calculate_steps(q2_current, q2);
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
void MoveSara::moveInterpolateTo(double x, double y, double distance, bool littlemovement)
{
    double alpha = atan2(y - y_current, x - x_current);
    double delta_x, delta_y;
    double x_aux = x_current, y_aux = y_current;
    delta_x = cos(alpha);
    delta_y = sin(alpha);
    int intervals = distance;
    for (int i = 1; i <= intervals; i++)
    {
        x_aux += delta_x;
        y_aux += delta_y;
        moveTo(x_aux, y_aux, littlemovement);
    }
    moveTo(x, y, littlemovement);
}
//properties----------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @return La distancia entre el centro y la posicion actual del robot.
 */
double MoveSara::getCurrentModule()
{
    return zPolar(x_current, y_current);
}

/**
 * @return El angulo, medido desde la horizontal, de la posicion actual del robot.
 * @note el angulo se encuentra en el rango de [0 , 2*PI)
 */
double MoveSara::getCurrentAngle()
{
    return thetaPolar(x_current, y_current);
}

/**
 * @brief devuelve un valor para saber en donde se encuentra la bola.
 * @return un entero que puede significar lo siguiente.
 * 0, se encuentra en el centro o a 2 mm.
 * 1, no se encuentra ni en el centro ni en la orilla.
 * 2, se cuentra
 */
int MoveSara::position(){
    double robotModule = getCurrentModule();
    Serial.print("robotModule: ");
    Serial.println(robotModule);
    int pos;
    if (robotModule >= l1 + l2 - 2){
        pos = 2;
    }
    else if (robotModule <= 2)
    {
        pos = 0;
    }
    else{
        pos = 1;
    }
    return pos;
}
//==================funciones Get y Set======================

/**
 * @brief regresa el miembro privado zCurrent
 * @return zCurrent
 */
double MoveSara::getZCurrent(){
    return zCurrent;
}

/**
 * @brief regresa el miembro privado thetaCurrent
 * @return thetaCurrent
 */
double MoveSara::getThetaCurrent(){
    return thetaCurrent;
}

/**
 * @brief obtiene el valor de la velocidad actual del robor en milimetros por segundo
 * @return la variable millimeterSpeed.
 */
int MoveSara::getSpeed(){
    return millimeterSpeed;
}

/**
 * @brief cambia la velocidad del robot
 * @param speed es la nueva valocidad, en milimetros por segundo, que va a tener el robot
 */
void MoveSara::setSpeed(int speed){
    millimeterSpeed = speed;
}
/**
 * @brief moficica el miembro z_current a uno nuevo.
 * @param z es el valor que se le va a asignar a la variable miembro zCurrent.
 * @note  Esto es importante para que los archivos .thr tengan una referencia de donde empezar a moverse.
 */
void MoveSara::setZCurrent(double z)
{
    zCurrent = z;
}

/**
 * @brief moficica el miembro theta_current a uno nuevo.
 * @param theta es el valor que se le va a asignar a la variable miembro thetaCurrent.
 * @note  Esto es importante para que los archivos .thr tengan una referencia de donde empezar a moverse.
 */
void MoveSara::setThetaCurrent(double theta)
{
    thetaCurrent = theta;
}

//Configuration Methods-----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief inicializa el objeto MoveSara
 * @param xInit es la coordenada en el eje x, medida en milimetros, que se desea como posicion inicial.
 * @param yInit es la coordenada en el eje y, medida en milimetros, que se desea como posicion inicial.
 * @note por defecto el punto inicial es 0,0.
 */
void MoveSara::init(double xInit, double yInit)
{
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
    x_current = dkX(q1_current, q2_current);
    y_current = dkY(q1_current, q2_current);
    calculate_line_equations();
}

/**
 * @brief define el angulo de acoplamiento.
 * @param angle el angulo que se desea como angulo de acoplamiento.
 * @note no retorna nada pero moficica la variable couplingAngle.
 */
void MoveSara::setCouplingAngle(double angle)
{
    couplingAngle = normalizeAngle(angle);
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
double dkX(double q1, double q2)
{
    return l1 * cos(q1) + l2 * cos(q1 + q2);
}

/**
 * @brief Calcula la cinematica directa del robot para la coordenada en y.
 * @param q1 es el angulo del motor que mueve el primer eslabon del robot (motor 1).
 * @param q2 es el angulo del motor que mueve el segundo eslabon del robot (motor 2).
 * @return la coordenada en el eje y, medida en milimetros, correspondiente a los angulos q1,q2 del robot.
 */
double dkY(double q1, double q2)
{
    return l1 * sin(q1) + l2 * sin(q1 + q2);
}

/**
 * @brief Calcula la cinematica inversa del robot.
 * @param x coordenada en el eje x, medida en milimetros.
 * @param y coordenada en el eje y, medida en milimetros.
 * @param q1 el angulo correspondiente al motor 1, en la posicion segun los parametros x,y.
 * @param q1 el angulo correspondiente al motor 2, en la posicion segun los parametros x,y.
 */
void MoveSara::ik(double x, double y, double *q1, double *q2)
{
    double z, z_max, theta;
    int auxiliar, i;
    //calculus of z
    z = zPolar(x, y);
    //calculus of theta
    theta = thetaPolar(x, y);
    //if x,y is out of range, z will be the maximun radius possible
    if (z > l1 + l2)
        z = l1 + l2;
    //Delimiter module z
    if (!productType){
        i = theta / (2 * PI / (2 * no_picos));
        z_max = abs(b[i] / (tan(theta) - m[i]) * sqrt(1 + pow(tan(theta), 2)));
        if (z > z_max){
            z = z_max;
        }
    }
    
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
void MoveSara::rotate(double &x, double &y, double angle)
{
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
double MoveSara::module(double x1, double y1, double x2, double y2)
{
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
double MoveSara::polarModule(double z1, double t1, double z2, double t2)
{
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
double MoveSara::zPolar(double x, double y)
{
    return sqrt(pow(x, 2) + pow(y, 2));
}

/**
 * @brief calcula el angulo que se forma con la horizontal, de un punto x,y.
 * @param x es el valor de la coordenada en el eje x, medido en milimetros.
 * @param y es el valor de la coordenada en el eje y, medido en milimetros.
 * @return el angulo, medido desde la horizontal, del punto x,y.
 * @note el angulo se encuentra en el rango de [0 , 2*PI)
 */
double MoveSara::thetaPolar(double x, double y)
{
    //calculus of theta
    double theta;
    if (x > 0)
        theta = atan(y / x);
    else if (x < 0)
        theta = atan(y / x) + PI;
    else
    {
        if (y >= 0)
            theta = PI / 2.0;
        else
            theta = 3.0 / 2.0 * PI;
    }
    return normalizeAngle(theta);
}

/**
 *@brief normaliza un angulo para estar en el rango de [0,2*PI).
 *@param angle es el angulo, medido en radianes, que se desea normalizar.
 *@return un valor con el rango de [0 , 2*PI).
 */
double MoveSara::normalizeAngle(double angle)
{
    long aux = angle / (2 * PI);
    angle -= aux * 2 * PI;
    if (angle < 0)
    {
        return angle + 2 * PI;
    }
    else
    {
        return angle;
    }
}

/**
 * @brief calcula la longitud de arco que se forma al girar de un punto A a un punto B en coordenadas polares.
 * @param deltaZ es la diferencia entre la componente modulo del punto final menos la componente modulo del punto inicial.
 * @param deltaTheta es la diferencia entre la componente angulo del punto final menos la componente angulo del punto inicial.
 * @param zInit es la componente modulo del punto inicial.
 */
double MoveSara::arcLength(double deltaZ, double deltaTheta, double zInit)
{
    double k, k2, a, a2, z, z2, inSqrt, inSqrt2;
    deltaTheta = abs(deltaTheta);
    if (deltaZ < 0)
    {
        zInit = zInit + deltaZ;
        deltaZ = abs(deltaZ);
    }
    if (deltaZ < 1.5)
    {
        return deltaTheta * (zInit + deltaZ);
    }
    if (deltaTheta < 10.0 * PI / 180.0)
    {
        return deltaZ;
    }
    k = deltaZ;
    k2 = deltaZ * deltaZ;
    a = deltaTheta;
    a2 = deltaTheta * deltaTheta;
    z = zInit;
    z2 = zInit * zInit;
    inSqrt2 = a2 * z2 + k2;
    inSqrt = a2 * k2 + 2 * a2 * k * z + inSqrt2;
    return (k + z) / (2 * k) * sqrt(inSqrt) + k / (2 * a) * log(a * (sqrt(inSqrt) + a * k + a * z)) - z / (2 * k) * sqrt(inSqrt2) - k / (2 * a) * log(a * (sqrt(inSqrt2) + a * z));
}
//Workspace mathematics---------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/**
 * @brief Calcula las escuaciones que describen la geometria de Stelle.
 * @return no retorna ningun valor pero modifica las variables miembro m y b.
 * @note m contiene las pendientes de las rectas que describen la geometria de Stelle.
 *       b contiene informacion de las rectas que describen la geometria de Stelle.
 */
void MoveSara::calculate_line_equations()
{
    double z = radius_2;
    double theta;
    for (int i = 0; i < 2 * no_picos; i++)
    {
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
long MoveSara::calculate_steps(double q1, double q2)
{
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
