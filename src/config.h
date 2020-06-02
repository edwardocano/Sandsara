#pragma once

//Es el valor de la resistencia colocada en los drivers, por ejemplo 0.11f representa 0.11 ohms.
#define R_SENSE             0.11f

//STALL_VALUE y STALL_VALUE2 es la sensibilidad para la deteccion de colision la cual tambien depende de la corriente.
#define STALL_VALUE         5 // [0..255]
#define STALL_VALUE2        2

//MICROSTEPPING es para configurar las distintas configuraciones de de microstepping que tiene el motor
// entre las opciones estan 1,2,4,8,16,32,64.
#define MICROSTEPPING 16

/** 
 * l1 y l2 representan la longitud, en milimetros, de los eslabones 1 y 2, respectivamente.
 * el eslabon 1 es el eslabon que se encuentra acoplado al eje central del mecanismo
 * el eslabon 2 es el eslabon que se encuentra acoplado la eslabon 1.
 */
#define l1 76
#define l2 76

/**
 * bigPulleySize y littlePulleySize se utilizan para saber la relacion de engranes
 * bigPulleySize es la polea acoplada a los motores.
 * littlePulleySize es la polea acoplada al eslabon 2.
 * la relacion se consigue con haciendo littlePulleySize / bigPulleySize por lo que se recomienda utilizar numeros flotantes
 * por ejemplo si bigPulleySize es de 40 dientes y littlePulleySize 20 dientes
 * se puede hacer bigPulleySize = 4.0 y littlePulleySize = 2.0
 */
#define bigPulleySize 4.0
#define littlePulleySize 2.0

/**
 * DIR_PIN, STEP_PIN, DIR_PIN2 y STEP_PIN2 se utilizan en la clase MoveSara para representar los pines de conexion de los motores
 * DIR_PIN es el pin de direccion del driver del motor 1
 * STEP_PIN es el pin step del driver para el motor 1
 * EN_PIN es el pin enable del driver del motor 1
 * DIR_PIN2 es el pin de direccion del driver del motor 2
 * STEP_PIN2 es el pin step del driver para el motor 2
 * EN_PIN2 es el pin enable del driver del motor 2
 */
#define DIR_PIN 33
#define STEP_PIN 25
#define EN_PIN 14
#define DIR_PIN2 4
#define STEP_PIN2 21
#define EN_PIN2 15

//no_picos representa el numero de picos de Stelle
#define no_picos 6

/**
 * si PROCESSING_SIMULATOR se define entonces se enviaran mensajes por el serial para conocer los pasos que dan los motores.
 * si DEBUGGING_DATA se define se mostrara mensajes por serial que sirven para debugear
 * si DEBUGGING_DETAIL se define se mostraran mensajes por serial que ayudan a debuguear el codigo
 * si DISABLE_MOTORS se define, los motores no se moveran a la hora de dibujar los archivos.
 */
#define PROCESSING_SIMULATOR
//#define DEBUGGING_DATA
//#define DEBUGGING_DETAIL
#define DISABLE_MOTORS

/**
 * DISTANCIA_MAX es el radio maximo, desde el centro, que va a dibujar el robot
 * RESOLUCION_MAX es el numero que representa la DISTANCIA_MAX
 */
#define DISTANCIA_MAX (l1 + l2)
#define RESOLUCION_MAX 32768.0

/**
 * encoder es el pin para la senal del encoder
 * hall es el pin para la senal del sensor de efecto hall
 */
#define encoder           39
#define hall              36

/**
 * DIAG_PIN es el pin diag para detectar colisiones del motor 1
 * DIAG_PIN2 es el pin diag para detectar colisiones del motor 2
 */
#define DIAG_PIN            34 
#define DIAG_PIN2           22 

#define SERIAL_PORT         Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS      0b00 // TMC2209 Driver address according to MS1 and MS2


#define SERIAL_PORT2        Serial2 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS2     0b00 // TMC2209 Driver address according to MS1 and MS2

#define radius_1 152.0
#define radius_2 100.0

#define PIN_ProducType 35