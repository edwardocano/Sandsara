#include "Testing.h"
#include "Calibration.h"
#include <Adafruit_NeoPixel.h>

TaskHandle_t Task3;

extern TMC2209Stepper driver;
extern TMC2209Stepper driver2;

void info_motor1(void);
void info_motor2(void);
void mover(int,int,int);
void Testing(void);
void Task3code( void *);

Calibration haloCalibTest;

#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN 32
#define NUMPIXELS 60 // numero de pixels en la tira
Adafruit_NeoPixel pixels_test(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 100
unsigned long t0, t1;
uint16_t waitTime = 10000.0;



//====================TESTING=====================
// - La funcion info_motor1 verifica la conexion con el driver del motor 1.
//   * Si se conecto correctamente devuelve:                                               "M1 OK"
//   * Si no se conecto correctamente devuelve:                                            "M1 Fail"

// - La funcion info_motor2 verifica la conexion con el driver del motor 2.
//   * Si se conecto correctamente devuelve:                                               "M2 OK"
//   * Si no se conecto correctamente devuelve:                                            "M2 Fail"

// - Se lee el valor del pin de configuracion.
//   * Devuelve el mensaje                                                                  "Pin config"
//   * Despues devuelve el valor analogico que se lee en el pin de configuracion:           [0 ...4095]

// - Se inicializa la comunicacion con la microSD y muestra el tamaño de la memoria en MB.
//   * Si no logra leer la microSD deviuelve el mensaje:                                    "Card Fail"
//   * Si logra leer la microSD devuelve el mensaje:                                        "Card OK"
//   * Posteriormente de leer la microSD devuelve el mensaje:                               "Card size"
//     y el tamaño de memoria en MB.

// - Se realiza la lectura analogica de los sensores Hall.
//   * Inicia enviando el mensaje :                                                         "Test Hall"
//   * Si el valor del sensor 1 se encuentra dentro del rango correcto imprime:             "Hall 1 OK"
//   * Si el valor del sensor 1 no se encuentra dentro del rango correcto imprime:          "Hall 1 Fail"
//   * Si el valor del sensor 2 se encuentra dentro del rango correcto imprime:             "Hall 2 OK"
//   * Si el valor del sensor 2 no se encuentra dentro del rango correcto imprime:          "Hall 2 Fail"

// - Se configura el motor dos a 500mA , 16 microsteeps, y se mueve 1600 pasos. 
//   * Muestra el mensaje:                                                                   "Currente M2"
//     y posteriormente un valor cercano a la configuracion de 500mA
//   * Muestra el mensaje:                                                                   "Microsteps M2"
//     y posteriormente el valor 16 correspondiente a los microsteeps.

// - Se configura el motor dos a 700mA , 32 microsteeps, y se mueve 3200 pasos.
//   * Muestra el mensaje:                                                                   "Currente M2"
//     y posteriormente un valor cercano a la configuracion de 700mA
//   * Muestra el mensaje:                                                                   "Microsteps M2"
//     y posteriormente el valor 32 correspondiente a los microsteeps.

// - Se configura el motor dos a 900mA , 64 microsteeps, y se mueve 6400 pasos.
//   * Muestra el mensaje:                                                                   "Currente M2"
//     y posteriormente un valor cercano a la configuracion de 900mA
//   * Muestra el mensaje:                                                                   "Microsteps M2"
//     y posteriormente el valor 64 correspondiente a los microsteeps.

// - Se configura el motor uno a 500mA , 16 microsteeps, y se mueve 1600 pasos. 
//   * Muestra el mensaje:                                                                   "Currente M1"
//     y posteriormente un valor cercano a la configuracion de 500mA
//   * Muestra el mensaje:                                                                   "Microsteps M1"
//     y posteriormente el valor 16 correspondiente a los microsteeps.
// - Se configura el motor uno a 700mA , 32 microsteeps, y se mueve 3200 pasos.
//   * Muestra el mensaje:                                                                   "Currente M1"
//     y posteriormente un valor cercano a la configuracion de 700mA
//   * Muestra el mensaje:                                                                   "Microsteps M1"
//     y posteriormente el valor 32 correspondiente a los microsteeps.
// - Se configura el motor uno a 900mA , 64 microsteeps, y se mueve 6400 pasos.
//   * Muestra el mensaje:                                                                   "Currente M1"
//     y posteriormente un valor cercano a la configuracion de 900mA
//   * Muestra el mensaje:                                                                   "Microsteps M1"
//     y posteriormente el valor 64 correspondiente a los microsteeps.

// - Se enciende la tira de de leds con su configuracion de mayor consumo energetico.

// - Se configuran ambos motores a 500 mA y a 16 microsteeps y giran en el mismo sentido.

void Testing::Test()
{
	#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif
	//haloCalibTest.init();
    SERIAL_PORT.begin(115200);
	SERIAL_PORT2.begin(115200);

	pixels_test.begin();

	xTaskCreatePinnedToCore(
                    Task3code,   /* Task function. */
                    "Task3",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &Task3,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
    delay(500);	

	pinMode(DIAG_PIN, INPUT);
	pinMode(EN_PIN, OUTPUT);
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);

	pinMode(DIAG_PIN2, INPUT);
	pinMode(EN_PIN2, OUTPUT);
	pinMode(STEP_PIN2, OUTPUT);
	pinMode(DIR_PIN2, OUTPUT);

	digitalWrite(EN_PIN, LOW);
	digitalWrite(DIR_PIN, LOW);

	digitalWrite(EN_PIN2, LOW);
	digitalWrite(DIR_PIN2, LOW);

	driver.begin();                   

	driver.pdn_disable(true);         // enables the  PDN/UART comunication.
	
	driver.toff(4);                   // Establece el tiempo de disminucion lenta (tiempo de apagado) [1 ... 15]
	                                  // Esta configuración también limita la frecuencia máxima de chopper. Para operar con StealthChop
									  // En caso de operar solo con StealthChop, cualquier configuración está bien.
	
	driver.blank_time(24);
	
	driver.rms_current(CURRENT_IN_CALIBRATION);          // Fija el valor de la corriente

	driver.microsteps(MICROSTEPPING); // Se define el valor de microstepps

	driver.TCOOLTHRS(0xFFFFF);        // Velocidad umbral inferior para encender la energía inteligente CoolStep y StallGuard a la salida del DIAG
	
	driver.semin(0);                  // Umbral inferior CoolStep [0 ... 15].
                                      // Si SG_RESULT cae por debajo de este umbral, CoolStep aumenta la corriente a ambas bobinas.
                                      // 0: deshabilitar CoolStep

	driver.shaft(false);              //Establece el sentido de giro del motor mediante la comunicacion UART

	driver.sedn(0b01);                // Establece el número de lecturas de StallGuard2 por encima del umbral superior necesario
                                      // por cada disminución de corriente de la corriente del motor.
	
	driver.SGTHRS(STALL_VALUE);       // Nivel de umbral StallGuard4 [0 ... 255] para la detección de bloqueo. Compensa
  									  // características específicas del motor y controla la sensibilidad. Un valor más alto da un valor más alto
  									  // sensibilidad. Un valor más alto hace que StallGuard4 sea más sensible y requiere menos torque para
  									  // indica una oposicion al movimiento. 

	
	
	driver2.begin();

	driver2.pdn_disable(true);         // Activa la comunicacion PDN/UART

	driver2.toff(4);                   // Establece el tiempo de disminucion lenta (tiempo de apagado) [1 ... 15]
	                                   // Esta configuración también limita la frecuencia máxima de chopper. Para operar con StealthChop
									   // En caso de operar solo con StealthChop, cualquier configuración está bien.

	driver2.blank_time(24);

	driver2.rms_current(CURRENT_IN_CALIBRATION);          // Fija el valor de la corriente

	driver2.microsteps(MICROSTEPPING); // Se define el valor de microstepps

	driver2.TCOOLTHRS(0xFFFFF);        // Velocidad umbral inferior para encender la energía inteligente CoolStep y StallGuard a la salida del DIAG

	driver2.semin(0);                  // Umbral inferior CoolStep [0 ... 15].
                                       // Si SG_RESULT cae por debajo de este umbral, CoolStep aumenta la corriente a ambas bobinas.
                                       // 0: deshabilitar CoolStep

	//driver.semax(2);

	driver2.shaft(false);              // Establece el sentido de giro del motor mediante la comunicacion UART

	driver2.sedn(0b01);                // Establece el número de lecturas de StallGuard2 por encima del umbral superior necesario
                                       // por cada disminución de corriente de la corriente del motor.

	driver2.SGTHRS(STALL_VALUE2);      // Nivel de umbral StallGuard4 [0 ... 255] para la detección de bloqueo. Compensa
  									   // características específicas del motor y controla la sensibilidad. Un valor más alto da un valor más alto
  									   // sensibilidad. Un valor más alto hace que StallGuard4 sea más sensible y requiere menos torque para
  									   // indica una oposicion al movimiento. 								 

    File myFile;
	File root;
	SdFat SD;
	uint32_t cardSize;
	
	while (!Serial) {
    // wait for serial port to connect. Needed for native USB
	;
    }
	delay(1000);
	while(true)
	{
		if(Serial) {

		int cont_m1 = 0;
		int cont_m2 = 0;
		int band_sd = 0;
		//===============Sensor_Hall==================
		int dato_hall1 = 0;
		int dato_hall2 = 0;

		dato_hall1 = analogRead(hall1);
		dato_hall2 = analogRead(hall2);
		if (dato_hall1 > 1500 && dato_hall1 < 2200)
		{
			Serial.print("Hall1_OK,");
		}
		else
		{
			Serial.print("Hall1_Fail,");
		}
		if (dato_hall2 > 1500 && dato_hall2 < 2200)
		{
			Serial.print("Hall2_OK,");
		}
		else
		{
			Serial.print("Hall2_Fail,");
		}

		//===============Info Motors===================
		info_motor1();
		info_motor2();
		digitalWrite(EN_PIN, LOW);
		digitalWrite(DIR_PIN, LOW);
		digitalWrite(EN_PIN2, LOW);
		digitalWrite(DIR_PIN2, LOW);
		//================Test===================
		//===============Motor2==================
		digitalWrite(DIR_PIN2, LOW);

		//Current m2
		cont_m2 = 0;
		driver2.microsteps(16);
		driver2.rms_current(500);

		if(driver2.rms_current() >=400 && driver2.rms_current() <=600)
		{
			cont_m2++;
		}
		delay(100);

		digitalWrite(DIR_PIN2, HIGH);

		driver2.microsteps(16);
		driver2.rms_current(700);

		if(driver2.rms_current() >=600 && driver2.rms_current() <=800)
		{
			cont_m2++;
		}
		delay(100);

		digitalWrite(DIR_PIN2, LOW);
		driver2.microsteps(64);
		driver2.rms_current(900);

		if(driver2.rms_current() >=800 && driver2.rms_current() <=1000)
		{
			cont_m2++;
		}
		delay(100);
		if(cont_m2 == 3)
		{
			Serial.print("CM2_OK,");
		}
		else
		{
			Serial.print("CM2_Fail,");
		}
		
		//Microsteps m2
		cont_m2 = 0;
		driver2.rms_current(500);
		driver2.microsteps(16);

		if(driver2.microsteps() == 16)
		{
			cont_m2++;
		}
		delay(100);

		digitalWrite(DIR_PIN2, HIGH);
		driver2.microsteps(32);
		if(driver2.microsteps() == 32)
		{
			cont_m2++;
		}
		delay(100);

		digitalWrite(DIR_PIN2, LOW);
		driver2.microsteps(64);

		if(driver2.microsteps() == 64)
		{
			cont_m2++;
		}
		delay(100);
		if(cont_m2 == 3)
		{
			Serial.print("SM2_OK,");
		}
		else
		{
			Serial.print("SM2_Fail,");
		}
		
		//===============Motor1==================
		digitalWrite(DIR_PIN, LOW);
		//Current m1
		cont_m1 = 0;
		driver.microsteps(16);
		driver.rms_current(500);

		if(driver.rms_current() >=400 && driver.rms_current() <=600)
		{
			cont_m1++;
		}
		delay(100);

		digitalWrite(DIR_PIN, HIGH);

		driver.microsteps(16);
		driver.rms_current(700);

		if(driver.rms_current() >=600 && driver.rms_current() <=800)
		{
			cont_m1++;
		}
		delay(100);

		digitalWrite(DIR_PIN, LOW);
		driver.microsteps(64);
		driver.rms_current(900);

		if(driver.rms_current() >=800 && driver.rms_current() <=1000)
		{
			cont_m1++;
		}
		delay(100);
		if(cont_m1 == 3)
		{
			Serial.print("CM1_OK,");
		}
		else
		{
			Serial.print("CM1_Fail,");
		}
		
		//MicroSteps m1
		cont_m1 = 0;
		driver.microsteps(16);
		driver.rms_current(500);

		if(driver.microsteps() == 16)
		{
			cont_m1++;
		}
		delay(100);

		digitalWrite(DIR_PIN, HIGH);

		driver.microsteps(32);

		if(driver.microsteps() == 32)
		{
			cont_m1++;
		}
		delay(100);

		digitalWrite(DIR_PIN, LOW);
		driver.microsteps(64);

		if(driver.microsteps() == 64)
		{
			cont_m1++;
		}
		delay(100);
		if(cont_m1 == 3)
		{
			Serial.print("SM1_OK,");
		}
		else
		{
			Serial.print("SM1_Fail,");
		}
		

		//====SD====
		t1 = millis();
		t0 = t1;
		band_sd = 0;
		while (!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
		{
			
			delay(200);
			t1 = millis();
			if (t1 - t0 > waitTime)
			{
				t0 = millis();
				break;
				
			}
		}
		if(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
		{
			Serial.print("Card_Fail");
			Serial.println("@@@@@");
		}
		else
		{
			Serial.print("Card_OK");
			Serial.println("@@@@@");
			band_sd = 1;
			
		}
        
		while(true)
		{
			if(band_sd == 0)
			{
				SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD);
				if(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
				{
					Serial.print("Card_Fail");
					Serial.println("@@@@@");
				}
				else
				{
					Serial.print("Card_OK");
					Serial.println("@@@@@");
					band_sd = 1;
				}
			}

			if (Serial.available())
			{
				char data = Serial.read();
			
				if (data >= '0' && data <= '9')
				{
					break;
				}
			}
		}
		}
	}
	
}

void info_motor1()
{
	uint8_t result_1 = driver.test_connection();
	if (result_1 == 0)
	{
		Serial.print(F("M1_OK,"));
	}
	else
	{
		

		Serial.print(F("M1_Fail,"));
	}
}

void info_motor2()
{
	uint8_t result_2 = driver2.test_connection();
	if (result_2 == 0)
	{
		Serial.print(F("M2_OK,"));
	}
	else
	{
		Serial.print(F("M2_Fail,"));
	}
}

void mover(int pasos, int motor_d, int Velocidad)
{
	if (motor_d == 1)
	{
		for (int i = 0; i < pasos; i++)
		{
			digitalWrite(STEP_PIN, HIGH);
			delayMicroseconds(Velocidad);
			digitalWrite(STEP_PIN, LOW);
			delayMicroseconds(Velocidad);
		}
	}

	if (motor_d == 2)
	{
		for (int j = 0; j < pasos; j++)
		{
			digitalWrite(STEP_PIN2, HIGH);
			delayMicroseconds(Velocidad);
			digitalWrite(STEP_PIN2, LOW);
			delayMicroseconds(Velocidad);
		}
	}
}

void Task3code( void * pvParameters ){
  
	for (int i = 0; i < NUMPIXELS; i++)
	{
		pixels_test.setPixelColor(i, pixels_test.Color(255, 255, 255));

		pixels_test.show(); // Send the updated pixel colors to the hardware.

		delay(DELAYVAL); // Pause before next pass through loop
	} 
	vTaskDelete(NULL);
}
