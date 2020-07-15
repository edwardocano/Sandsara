#include "CalibMotor.h"

hw_timer_t *timer1 = NULL;

MeanFilter<long> meanFilter(5);
MeanFilter<long> meanFilter2(40);
MeanFilter<long> meanFilter3(10);
MeanFilter<long> meanFilter4(10);
int flag = 0;
int avoid = 0;
int avoid2 = 0; 
int A = 0;
int B = 0;
byte L;
byte H;
int p = 0;
int motor_degrees = 0;
int Speed = 5000;
int value[100];
int value2[600];
int value2_r[600];
int vect_prom[5];
int vect_simi[5];
int vect_max[5];
int vect_min[5];
int vect_prom2[5];
int vect_simi2[5];
int vect_max2[5];
int vect_min2[5];
int maximum = 0;
int maximum2 = 0;
int minimum = 5000;
int minimum2 = 5000;
int max_hall1;
int min_hall1;
int level_zero1;
int level_zero2;
int max_hall2;
int min_hall2;
int read_hall2;
int read_hall1;
int Pole_sens1;
int Pole_sens2;
int Flag_adjust_ini = 0;
TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driver2(&SERIAL_PORT2, R_SENSE, DRIVER_ADDRESS2);

void normal_turn();
void move(int, int, int);
void slow_Calibration_hall2(void);
void slow_Calibration_hall1(void);
void slow_Calibration_hall2_negative(void);
void slow_Calibration_hall1_negative(void);
int zero_Hall1(void);
int zero_Hall2(void);
int Pole1(void);
int Pole2(void);
int Check_ini(void);

CalibMotor::CalibMotor()
{
}

/**
 * @brief Esta funcion activa y desactiva los pines de salida STEP mediante interrupciones. 
 * @param flag Es la variable para determinar que brazo es el que gira.
 * 0 representa el giro del brazo 1.
 * 1 representa que no se mueve ningun brazo.
 * 2 representa el giro de ambos brazos de manera simultanea.
 * 3 representa el giro del brazo 3.
 */

void IRAM_ATTR onTimer()
{

	if (flag == 0)
	{
		digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
	}
	if (flag == 2)
	{
		digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
		digitalWrite(STEP_PIN2, !digitalRead(STEP_PIN2));
	}
	if (flag == 3)
	{
		digitalWrite(STEP_PIN2, !digitalRead(STEP_PIN2));
	}
}

int CalibMotor::init()
{

	SERIAL_PORT.begin(115200);
	SERIAL_PORT2.begin(115200);

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

	driver.pdn_disable(true);         // Activa la comunicacion PDN/UART
	
	driver.toff(4);                   // Establece el tiempo de disminucion lenta (tiempo de apagado) [1 ... 15]
	                                  // Esta configuración también limita la frecuencia máxima de chopper. Para operar con StealthChop
									  // En caso de operar solo con StealthChop, cualquier configuración está bien.
	
	driver.blank_time(24);
	
	driver.rms_current(500);          // Fija el valor de la corriente

	driver.microsteps(MICROSTEPPING); // Se define el valor de microstepps

	driver.TCOOLTHRS(0xFFFFF);        // Velocidad umbral inferior para encender la energía inteligente CoolStep y StallGuard a la salida del DIAG
	
	driver.semin(0);                  // Umbral inferior CoolStep [0 ... 15].
                                      // Si SG_RESULT cae por debajo de este umbral, CoolStep aumenta la corriente a ambas bobinas.
                                      // 0: deshabilitar CoolStep
	//driver.semax(2);

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

	driver2.rms_current(500);          // Fija el valor de la corriente

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

	EEPROM.begin(EEPROM_SIZE);
}

int CalibMotor::start()
{
	//////////////////////////////////////////////////////////////////////
	////////////////////////////WITH STALLGUARD///////////////////////////
	int value_f;
	int mean;
	int dif_ref;
	int cont_turn = 0;
	int pas_hall1;
	int pas_hall2;
	int value2_f;
	flag = 0;

	Flag_adjust_ini = Check_ini();
	if (Flag_adjust_ini == 1)
	{
		int ref_sensor1 = 0;
		int ref_sensor2 = 0;
		while (ref_sensor2 != 1)
		{
			ref_sensor2 = zero_Hall2();
		}
		while (ref_sensor1 != 1)
		{
			ref_sensor1 = zero_Hall1();
		}
		max_hall1 = max_hall1 + 20;
		A = max_hall1;
		H = highByte(A);
		L = lowByte(A);
		EEPROM.write(407, H);
		EEPROM.commit();
		EEPROM.write(408, L);
		EEPROM.commit();
		max_hall2 = max_hall2 + 20;
		A = max_hall2;
		H = highByte(A);
		L = lowByte(A);
		EEPROM.write(409, H);
		EEPROM.commit();
		EEPROM.write(410, L);
		EEPROM.commit();
		min_hall1 = min_hall1 - 20;
		A = min_hall1;
		H = highByte(A);
		L = lowByte(A);
		EEPROM.write(411, H);
		EEPROM.commit();
		EEPROM.write(412, L);
		EEPROM.commit();
		min_hall2 = min_hall2 - 20;
		A = min_hall2;
		H = highByte(A);
		L = lowByte(A);
		EEPROM.write(413, H);
		EEPROM.commit();
		EEPROM.write(414, L);
		EEPROM.commit();
	}
	if (Flag_adjust_ini == 0)
	{
		L = EEPROM.read(403);
		H = EEPROM.read(404);
		level_zero1 = (L << 8) | H;

		L = EEPROM.read(405);
		H = EEPROM.read(406);
		level_zero2 = (L << 8) | H;

		L = EEPROM.read(407);
		H = EEPROM.read(408);
		max_hall1 = (L << 8) | H;

		L = EEPROM.read(409);
		H = EEPROM.read(410);
		max_hall2 = (L << 8) | H;

		L = EEPROM.read(411);
		H = EEPROM.read(412);
		min_hall1 = (L << 8) | H;

		L = EEPROM.read(413);
		H = EEPROM.read(414);
		min_hall2 = (L << 8) | H;
	}

	
	digitalWrite(DIR_PIN, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value_f = meanFilter2.AddValue(analogRead(hall1));
	}
	read_hall1 = value_f / 4;
	mean = read_hall1;
	if(mean > max_hall1 || mean < min_hall1)
	{
		move(200,1,6000);
	}
    
	flag = 0;
	digitalWrite(EN_PIN, LOW);
	digitalWrite(DIR_PIN, LOW);
	normal_turn();
	delay(100);
	while (true)
	{
		digitalWrite(EN_PIN2, HIGH);
		digitalWrite(DIR_PIN2, HIGH);

		if (analogRead(PIN_ProducType) > 4000)
		{
			if (digitalRead(DIAG_PIN) == 1)
			{
				flag = 1;
				//Back a little  first arm
				digitalWrite(DIR_PIN, HIGH);
				move(300, 1, 5000);

				//Move second arm 90 degrees.
				digitalWrite(EN_PIN, LOW);
				digitalWrite(EN_PIN2, LOW);
				digitalWrite(DIR_PIN2, LOW);
				move(2400, 2, 5000);
				//move both arms until encoder detects
				digitalWrite(DIR_PIN, LOW);
				digitalWrite(DIR_PIN2, LOW);

				for (int i = 0; i < 40; i++)
				{
					value_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value_f / 4;

				while (read_hall1 < max_hall1 && read_hall1 > min_hall1)
				{
					for (int i = 0; i < 40; i++)
					{
						value_f = meanFilter2.AddValue(analogRead(hall1));
					}
					read_hall1 = value_f / 4;

					flag = 2;
				}
				flag = 1;

				//move second arm until hall detects
				digitalWrite(DIR_PIN2, LOW);
				for (int i = 0; i < 40; i++)
				{
					value_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value_f / 4;
				mean = read_hall2;
				while (mean > max_hall2 || mean < min_hall2)
				{
					for (int i = 0; i < 40; i++)
					{
						value_f = meanFilter2.AddValue(analogRead(hall2));
					}
					read_hall2 = value_f / 4;
					mean = read_hall2;
					flag = 3;
				}
				flag = 1;
				digitalWrite(DIR_PIN2, HIGH);
				for (int i = 0; i < 40; i++)
				{
					value_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value_f / 4;
				mean = read_hall2;
				while (mean < max_hall2 && mean > min_hall2)
				{
					for (int i = 0; i < 40; i++)
					{
						value_f = meanFilter2.AddValue(analogRead(hall2));
					}
					read_hall2 = value_f / 4;
					mean = read_hall2;
					flag = 3;
					delay(10);
					cont_turn++;
					if (analogRead(PIN_ProducType) > 4000)
					{
						if ((digitalRead(DIAG_PIN2) == 1 and avoid2 == 1) || cont_turn == 1280)
						{
							cont_turn = 0;
							flag = 1;
							
							delay(250);
							digitalWrite(DIR_PIN, HIGH);
							move(540, 1, 5000);
							//second arm 90 degree
							digitalWrite(DIR_PIN2, LOW);
							move(3200, 2, 2000);

							//move both arms at same time.
							digitalWrite(DIR_PIN, LOW);
							digitalWrite(DIR_PIN2, LOW);

							for (int i = 0; i < 40; i++)
							{
								value_f = meanFilter2.AddValue(analogRead(hall1));
							}
							read_hall1 = value_f / 4;

							while (read_hall1 < max_hall1 && read_hall1 > min_hall1)
							{
								for (int i = 0; i < 40; i++)
								{
									value_f = meanFilter2.AddValue(analogRead(hall1));
								}
								read_hall1 = value_f / 4;
								flag = 2;
							}
							digitalWrite(DIR_PIN2, HIGH);
							//move second arm ultil hall
							while (mean < max_hall2 && mean > min_hall2)
							{
								for (int i = 0; i < 40; i++)
								{
									value_f = meanFilter2.AddValue(analogRead(hall2));
								}
								read_hall2 = value_f / 4;
								mean = read_hall2;
								flag = 3;
							}
							flag = 1;
							break;
						}

						if (digitalRead(DIAG_PIN2) == 1 and avoid2 == 0)
						{
							digitalWrite(DIR_PIN2, HIGH);
							avoid2 = 1;
							delay(100);
							
						}
					}
				}
				flag = 1;
				
				if (Flag_adjust_ini == 1)
				{
					Pole_sens1 = Pole1();
					EEPROM.write(401, Pole_sens1);
					EEPROM.commit();
					Pole_sens2 = Pole2();
					EEPROM.write(402, Pole_sens2);
					EEPROM.commit();
				}
				if (Flag_adjust_ini == 0)
				{
					Pole_sens1 = EEPROM.read(401);
					Pole_sens2 = EEPROM.read(402);
				}
				if (Pole_sens1 == 1)
				{
						slow_Calibration_hall1();
				}
				if (Pole_sens1 == 0)
				{
						slow_Calibration_hall1_negative();
				}
				if (Pole_sens2 == 1)
				{
						slow_Calibration_hall2();
				}
				if (Pole_sens2 == 0)
				{
						slow_Calibration_hall2_negative();
				}
				driver.rms_current(500);
				driver2.rms_current(500);
				digitalWrite(EN_PIN, LOW);
				digitalWrite(EN_PIN2, LOW);
				avoid = 1;
				return 0;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		////////////////////////////WITHOUT STALLGUARD////////////////////////////
		for (int i = 0; i < 40; i++)
		{
			value_f = meanFilter2.AddValue(analogRead(hall1));
		}
		read_hall1 = value_f / 4;
		if (read_hall1 > max_hall1 || read_hall1 < min_hall1) ///===================condicional para detener el primer brazo
		{
			if (avoid == 0)
			{
				flag = 1;
				digitalWrite(DIR_PIN2, LOW);
				for (int i = 0; i < 40; i++)
				{
					value_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value_f / 4;
				mean = read_hall2;
				digitalWrite(EN_PIN2, LOW); 
				while (mean > max_hall2 || mean < min_hall2)
				{
					for (int i = 0; i < 40; i++)
					{
						value_f = meanFilter2.AddValue(analogRead(hall2));
					}
					read_hall2 = value_f / 4;
					mean = read_hall2;
					flag = 3;
				}
				flag = 1;
				digitalWrite(DIR_PIN, LOW);
				digitalWrite(DIR_PIN2, HIGH);
				for (int i = 0; i < 40; i++)
				{
					value_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value_f / 4;
				mean = read_hall2;
				digitalWrite(EN_PIN2, LOW);
				digitalWrite(EN_PIN, LOW);
				driver.rms_current(1500);
				delay(500);
				while (mean < max_hall2 && mean > min_hall2)
				{
					for (int i = 0; i < 40; i++)
					{
						value_f = meanFilter2.AddValue(analogRead(hall2));
					}
					read_hall2 = value_f / 4;
					mean = read_hall2;
					flag = 3;
					delay(10);
					cont_turn++;
					if (analogRead(PIN_ProducType) > 4000)
					{

						if ((digitalRead(DIAG_PIN2) == 1 and avoid2 == 1) || cont_turn == 1280)
						{
							cont_turn = 0;
							flag = 1;
							//secuencia donde el brazo se encuentra entre ambos picos//regresa main 30 grados, el otro gira 90, y ambos se mueven juntos.
							//main arm 30 degree
							delay(250);
							digitalWrite(DIR_PIN, HIGH);
							move(540, 1, 5000);
							//second arm 90 degree
							digitalWrite(DIR_PIN2, LOW);
							move(3200, 2, 2000);

							//move both arms at same time.
							digitalWrite(DIR_PIN, LOW);
							digitalWrite(DIR_PIN2, LOW);
							//read_hall1 = (analogRead(hall1))/4;
							for (int i = 0; i < 40; i++)
							{
								value_f = meanFilter2.AddValue(analogRead(hall1));
							}
							read_hall1 = value_f / 4;
							while (read_hall1 < max_hall1 && read_hall1 > min_hall1)
							{
								for (int i = 0; i < 40; i++)
								{
									value_f = meanFilter2.AddValue(analogRead(hall1));
								}
								read_hall1 = value_f / 4;
								flag = 2;
							}
							digitalWrite(DIR_PIN2, HIGH);
							//move second arm ultil hall
							while (mean < max_hall2 && mean > min_hall2)
							{
								//mean = (analogRead(hall2))/4;
								for (int i = 0; i < 40; i++)
								{
									value_f = meanFilter2.AddValue(analogRead(hall2));
								}
								read_hall2 = value_f / 4;
								mean = read_hall2;
								flag = 3;
							}
							
							flag = 1;
							break;
						}

						if (digitalRead(DIAG_PIN2) == 1 and avoid2 == 0)
						{
							digitalWrite(DIR_PIN2, HIGH);
							avoid2 = 1;
							delay(100);
							
						}
					}
				}
				flag = 1;

				if (Flag_adjust_ini == 1)
				{
					Pole_sens1 = Pole1();
					EEPROM.write(401, Pole_sens1);
					EEPROM.commit();
					Pole_sens2 = Pole2();
					EEPROM.write(402, Pole_sens2);
					EEPROM.commit();
				}
				if (Flag_adjust_ini == 0)
				{
					Pole_sens1 = EEPROM.read(401);
					Pole_sens2 = EEPROM.read(402);
				}

				if (Pole_sens1 == 1)
				{
						slow_Calibration_hall1();
				}
				if (Pole_sens1 == 0)
				{	
						slow_Calibration_hall1_negative();
				}
				if (Pole_sens2 == 1)
				{
						slow_Calibration_hall2();
				}
				if (Pole_sens2 == 0)
				{
						slow_Calibration_hall2_negative();
				}
				driver.rms_current(500);
				driver2.rms_current(500);
				avoid = 1;
				return 0;
			}
		}
	}
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////END_LOOP////////////////////////////////
}

void normal_turn()
{
	{
		cli();										  //stop interrupts
		timer1 = timerBegin(3, 8, true);			  // Se configura el timer, en este caso uso el timer 4 de los 4 disponibles en el ESP(0,1,2,3)
													  // el prescaler es 8, y true es una Flagera que indica si la interrupcion se realiza en borde o en level
		timerAttachInterrupt(timer1, &onTimer, true); //Se vincula el timer con la funcion AttachInterrup
													  //la cual se ejecuta cuando se genera la interrupcion
		timerAlarmWrite(timer1, 10000, true);		  //En esta funcion se define el valor del contador en el cual se genera la interrupción del timer
		timerAlarmEnable(timer1);					  //Función para habilitar el temporizador.
		sei();										  //allow interrupts
	}
}

/**
 * @brief Esta funcion continua con el proceso de calibracion una vez que el brazo 1 esta cerca del sensor hall. 
 * @param value[] En este vector se alamacenan los valores obtenidos del sensor Hall.
 * @param limit En esta variable se define el limite del intervalo en el cual se buscara el punto medio de los datos almacenados en el vector value.
 * @param steps_ini Almacena el numero de pasos para llegar al inicio del intervalo de busqueda.
 * @param steps_fin Almacena el numero de pasos para llegar al fin del intervalo de busqueda.
 * @param pas Esta variable almacena los pasos necesarios para posicionar el brazo 1 al centro del sensor hall.
 * DESCRIPCION GENERAL 
 * Inicia moviendo el brazo uno 80 pasos en sentido antihorario para salir por completo del rango del sensor
 * Comienza a tomar mediciones del sensor en cada paso que avanza en sentido horario hasta llenar el vector value[]
 * Durante el llenado del vector tambien se determina el valor maximo entre todos los elementos del  vector
 * La grafica de los datos tiene forma gaussiana por lo cual se determina un limite del rango en cual se buscara el punto medio de la funcion,
 *   dicho rango se establecio como los valores mayores al 90% del valor maximo encontrado.
 * Se determinan los pasos correspondientes a el limite derecho y el limite izquierdo de la funcion
 * Se calculan los pasos necesarios para llegar al punto medio del sensor
 */

void slow_Calibration_hall1()
{
	int value_f;
	digitalWrite(EN_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);

	for (int i = 0; i < 100; i++)
	{
		value[i] = -1;
	}
	int k = 0;
	int val_sens;
	int val_sens_filt;
	int Flag_ini = 0;
	int Flag_max = 0;
	int Flag_fin = 0;
	int steps_ini;
	int steps_fin;
	int limit;
	int count_ini = 0;
	int count_fin = 0;
	int dif_ref;
	digitalWrite(DIR_PIN, LOW);

	for (int i = 0; i < 40; i++)
	{
		value_f = meanFilter2.AddValue(analogRead(hall1));
	}
	read_hall1 = value_f / 4;

	for (int t = 0; t < 80; t++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_f = meanFilter2.AddValue(analogRead(hall1));
		}
		read_hall1 = value_f / 4;
		move(1, 1, 5000);
	}

	digitalWrite(DIR_PIN, HIGH);
	while (k < 100)
	{
		move(1, 1, 5000);
		for (int y = 0; y < 40; y++)
		{
			read_hall1 = (analogRead(hall1)) / 4;
			val_sens = meanFilter2.AddValue(read_hall1);
		}
		delay(1);

		if (val_sens > maximum) 
		{
			maximum = val_sens; 
		}
		value[k] = val_sens; 

		delay(1);
		k++;
	}
	limit = maximum * 0.9;
	k = 99;
	while (k >= 0)
	{
		if (value[k] > limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini = k + 9;
				Flag_ini = 1;
			}
		}
		if (value[k] < limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin = k + 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	digitalWrite(DIR_PIN, LOW);
	int pas;
	int half;
	half = (steps_ini - steps_fin) / 2;
	pas = (99 - steps_ini) + half;
	move(pas, 1, 5000);
}

/**
 * @brief Esta funcion continua con el proceso de calibracion una vez que el brazo 2 esta cerca del sensor hall. 
 * @param value2[] En este vector se alamacenan los valores obtenidos del sensor Hall.
 * @param limit En esta variable se define el limite del intervalo en el cual se buscara el punto medio de los datos almacenados en el vector value.
 * @param steps_ini Almacena el numero de pasos para llegar al inicio del intervalo de busqueda.
 * @param steps_fin Almacena el numero de pasos para llegar al fin del intervalo de busqueda.
 * @param pas Esta variable almacena los pasos necesarios para posicionar el brazo 2 al centro del sensor hall.
 * DESCRIPCION GENERAL
 * Inicia moviendose 100 pasos en sentido antihorario
 * Regresa en sentido horario hasta que es detectado por el sensor
 * Continua moviendose en sentido horario hasta completar 600 pasos,  en cada paso toma la medicion del sensor y este dato se almacena en el vector value2_r[]
 * Regresa el brazo en sentido antihorario hasta que es detectado por el sensor
 * Continua moviendose en sentido antihorario hasta completar 600 pasos, en cada paso toma la medicion del sensor y este dato se almacena en el vector value2[]
 * La grafica de ambos vectores es de forma gaussiana por lo cual se determina un limite del rango de busqueda en ambas funciones,
 * dicho rango se establecio como los valores mayores al 90% del valor maximo encontrado.
 * Se determinan los pasos correspondientes a el limite derecho y el limite izquierdo de ambas funciones y se usa el limite derecho de la funcion dos
 *  y el limite izquierdo de la funcion uno para encontrar el punto medio. 
 * Se calculan los pasos necesarios para llegar al punto medio del sensor
 */

void slow_Calibration_hall2()
{
	int value2_f;
	digitalWrite(EN_PIN2, LOW);
	for (int i = 0; i < 600; i++)
	{
		value2[i] = -1;
	}
	for (int j = 0; j < 600; j++)
	{
		value2_r[j] = -1;
	}
	int k = 0;
	int val_sens;
	int val_sens_filt;
	int Flag_ini = 0;
	int Flag_max = 0;
	int Flag_fin = 0;
	int steps_ini;
	int steps_fin;
	int steps_ini2;
	int steps_fin2;
	int limit;
	int count_ini = 0;
	int count_fin = 0;
	int dif_ref;
	int maximum2_r = 0;
	int index_min1;
	int index_min2;
	int ind_2;
	int offset;
	int cont_t1 = 0;
	int cont_t2 = 0;

	digitalWrite(DIR_PIN2, LOW);
	move(100, 2, 8000);
	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	while (read_hall2 < max_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			read_hall2 = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);

	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	for (int t = 0; t < 600; t++)
	{
		for (int i = 0; i < 40; i++)
		{
			value2_f = meanFilter2.AddValue(analogRead(hall2));
		}
		read_hall2 = value2_f / 4;
		move(1, 2, 8000);

		ind_2 = 599 - t;
		value2_r[ind_2] = read_hall2;
		if (value2_r[ind_2] > max_hall2)
		{
			cont_t2++;
		}

		if (read_hall2 > maximum2_r)
		{
			maximum2_r = read_hall2;
			index_min1 = ind_2;
		}
	}

	digitalWrite(DIR_PIN2, LOW);

	for (int y = 0; y < 40; y++)
	{
		read_hall2 = (analogRead(hall2)) / 4;
		val_sens = meanFilter2.AddValue(read_hall2);
	}

	while (val_sens < max_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			val_sens = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);

	while (k < 600)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			val_sens = meanFilter2.AddValue(read_hall2);
		}
		delay(1);

		if (val_sens > maximum2)
		{
			maximum2 = val_sens;
			index_min2 = k;
		}
		value2[k] = val_sens;

		if (value2[k] > max_hall2)
		{
			cont_t1++;
		}
		delay(1);
		k++;
	}

	limit = maximum2 * 0.9;
	k = 599;
	while (k >= 0)
	{
		if (value2[k] > limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini = k + 9;
				Flag_ini = 1;
			}
		}
		if (value2[k] < limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin = k + 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	Flag_ini = 0;
	Flag_fin = 0;
	count_ini = 0;
	count_fin = 0;

	k = 599;
	while (k >= 0)
	{
		if (value2_r[k] > limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini2 = k + 9;
				Flag_ini = 1;
			}
		}
		if (value2_r[k] < limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin2 = k + 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	digitalWrite(DIR_PIN2, HIGH);
	int pas;
	int half;
	half = (cont_t1 + cont_t2) / 2;

	int half1;
	int half2;

	half1 = (half) / 2;

	offset = (steps_ini2 - steps_ini) / 2;

	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	while (read_hall2 < max_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			read_hall2 = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);
	pas = half1 + 15;
	move(pas, 2, 5000);
}

/**
 * @brief Esta funcion continua con el proceso de calibracion una vez que el brazo 1 esta cerca del sensor hall, esta version de la funcion contempla el polo negativo del iman 
 * @param value[] En este vector se alamacenan los valores obtenidos del sensor Hall.
 * @param limit En esta variable se define el limite del intervalo en el cual se buscara el punto medio de los datos almacenados en el vector value.
 * @param steps_ini Almacena el numero de pasos para llegar al inicio del intervalo de busqueda.
 * @param steps_fin Almacena el numero de pasos para llegar al fin del intervalo de busqueda.
 * @param pas Esta variable almacena los pasos necesarios para posicionar el brazo 1 al centro del sensor hall.
 * DESCRIPCION GENERAL 
 * Inicia moviendo el brazo uno 80 pasos en sentido antihorario para salir por completo del rango del sensor
 * Comienza a tomar mediciones del sensor en cada paso que avanza en sentido horario hasta llenar el vector value[]
 * Durante el llenado del vector tambien se determina el valor maximo entre todos los elementos del  vector
 * La grafica de los datos tiene forma gaussiana por lo cual se determina un limite del rango en cual se buscara el punto medio de la funcion,
 *   dicho rango se establecio como los valores mayores al 90% del valor maximo encontrado.
 * Se determinan los pasos correspondientes a el limite derecho y el limite izquierdo de la funcion
 * Se calculan los pasos necesarios para llegar al punto medio del sensor
 */

void slow_Calibration_hall1_negative()
{
	int value1_f;
	digitalWrite(EN_PIN, LOW);
	for (int i = 0; i < 100; i++)
	{
		value[i] = -1;
	}
	int k = 0;
	int val_sens;
	int val_sens_filt;
	int Flag_ini = 0;
	int Flag_max = 0;
	int Flag_fin = 0;
	int steps_ini;
	int steps_fin;
	int limit;
	int count_ini = 0;
	int count_fin = 0;
	int dif_ref;
	digitalWrite(DIR_PIN, LOW);
	for (int i = 0; i < 40; i++)
	{
		value1_f = meanFilter2.AddValue(analogRead(hall1));
	}
	read_hall1 = value1_f / 4;
	for (int t = 0; t < 80; t++)
	{
		for (int i = 0; i < 40; i++)
		{
			value1_f = meanFilter2.AddValue(analogRead(hall1));
		}
		read_hall1 = value1_f / 4;
		move(1, 1, 5000);
	}

	digitalWrite(DIR_PIN, HIGH);
	while (k < 100)
	{
		move(1, 1, 5000);
		for (int y = 0; y < 40; y++)
		{
			read_hall1 = (analogRead(hall1)) / 4;
			val_sens = meanFilter2.AddValue(read_hall1);
		}
		delay(1);

		if (val_sens < minimum)
		{
			minimum = val_sens;
		}
		value[k] = val_sens;

		delay(1);
		k++;
	}
	limit = minimum + ((level_zero1 - minimum) * 0.1);
	k = 99;
	while (k >= 0)
	{
		if (value[k] < limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini = k + 9;
				Flag_ini = 1;
			}
		}
		if (value[k] > limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin = k - 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	digitalWrite(DIR_PIN, LOW);
	int pas;
	int half;
	half = (steps_ini - steps_fin) / 2;
	pas = (99 - steps_ini) + half;
	move(pas, 1, 8000);
}

/**
 * @brief Esta funcion continua con el proceso de calibracion una vez que el brazo 2 esta cerca del sensor hall, esta version de la funcion contempla el polo negativo del iman 
 * @param value2[] En este vector se alamacenan los valores obtenidos del sensor Hall.
 * @param limit En esta variable se define el limite del intervalo en el cual se buscara el punto medio de los datos almacenados en el vector value.
 * @param steps_ini Almacena el numero de pasos para llegar al inicio del intervalo de busqueda.
 * @param steps_fin Almacena el numero de pasos para llegar al fin del intervalo de busqueda.
 * @param pas Esta variable almacena los pasos necesarios para posicionar el brazo 2 al centro del sensor hall.
 * DESCRIPCION GENERAL
 * Inicia moviendose 100 pasos en sentido antihorario
 * Regresa en sentido horario hasta que es detectado por el sensor
 * Continua moviendose en sentido horario hasta completar 600 pasos,  en cada paso toma la medicion del sensor y este dato se almacena en el vector value2_r[]
 * Regresa el brazo en sentido antihorario hasta que es detectado por el sensor
 * Continua moviendose en sentido antihorario hasta completar 600 pasos, en cada paso toma la medicion del sensor y este dato se almacena en el vector value2[]
 * La grafica de ambos vectores es de forma gaussiana por lo cual se determina un limite del rango de busqueda en ambas funciones,
 * dicho rango se establecio como los valores mayores al 90% del valor maximo encontrado.
 * Se determinan los pasos correspondientes a el limite derecho y el limite izquierdo de ambas funciones y se usa el limite derecho de la funcion dos
 *  y el limite izquierdo de la funcion uno para encontrar el punto medio. 
 * Se calculan los pasos necesarios para llegar al punto medio del sensor
 */

void slow_Calibration_hall2_negative()
{
	int value2_f;
	digitalWrite(EN_PIN2, LOW);
	for (int i = 0; i < 600; i++)
	{
		value2[i] = -1;
	}
	for (int j = 0; j < 600; j++)
	{
		value2_r[j] = -1;
	}
	int k = 0;
	int val_sens;
	int val_sens_filt;
	int Flag_ini = 0;
	int Flag_max = 0;
	int Flag_fin = 0;
	int steps_ini;
	int steps_fin;
	int steps_ini2;
	int steps_fin2;
	int limit;
	int count_ini = 0;
	int count_fin = 0;
	int dif_ref;
	int minimum2_r = 5000;
	int index_min1;
	int index_min2;
	int ind_2;
	int offset;
	int cont_t1 = 0;
	int cont_t2 = 0;

	digitalWrite(DIR_PIN2, LOW);
	move(100, 2, 8000);
	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	while (read_hall2 > min_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			read_hall2 = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);

	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	for (int t = 0; t < 600; t++)
	{
		for (int i = 0; i < 40; i++)
		{
			value2_f = meanFilter2.AddValue(analogRead(hall2));
		}
		read_hall2 = value2_f / 4;
		move(1, 2, 8000);

		ind_2 = 599 - t;
		value2_r[ind_2] = read_hall2;
		if (value2_r[ind_2] < min_hall2)
		{
			cont_t2++;
		}

		if (read_hall2 < minimum2_r)
		{
			minimum2_r = read_hall2;
			index_min1 = ind_2;
		}
	}

	digitalWrite(DIR_PIN2, LOW);

	for (int y = 0; y < 40; y++)
	{
		read_hall2 = (analogRead(hall2)) / 4;
		val_sens = meanFilter2.AddValue(read_hall2);
	}

	while (val_sens > min_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			val_sens = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);

	while (k < 600)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			val_sens = meanFilter2.AddValue(read_hall2);
		}
		delay(1);

		if (val_sens < minimum2)
		{
			minimum2 = val_sens;
			index_min2 = k;
		}
		value2[k] = val_sens;

		if (value2[k] < min_hall2)
		{
			cont_t1++;
		}

		delay(1);
		k++;
	}

	limit = minimum2 + ((level_zero2 - minimum2) * 0.1);
	k = 599;
	while (k >= 0)
	{
		if (value2[k] < limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini = k + 9;
				Flag_ini = 1;
			}
		}
		if (value2[k] > limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin = k + 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	Flag_ini = 0;
	Flag_fin = 0;
	count_ini = 0;
	count_fin = 0;

	k = 599;
	while (k >= 0)
	{
		if (value2_r[k] < limit && Flag_ini == 0)
		{
			count_ini++;
			if (count_ini == 9)
			{
				steps_ini2 = k + 9;
				Flag_ini = 1;
			}
		}
		if (value2_r[k] > limit && Flag_ini == 1 && Flag_fin == 0)
		{
			count_fin++;
			if (count_fin == 9)
			{
				steps_fin2 = k + 9;
				Flag_fin = 1;
			}
		}
		k--;
	}

	digitalWrite(DIR_PIN2, HIGH);
	int pas;
	int half;
	half = (cont_t1 + cont_t2) / 2;

	int half1;
	int half2;

	half1 = (half) / 2;

	offset = (steps_ini2 - steps_ini) / 2;

	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	while (read_hall2 > min_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			read_hall2 = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);
	pas = half1 + 15;
	move(pas, 2, 8000);
}

/**
 * @brief Esta funcion perimite mover los motores de manera controlada mediante numero de pasos. 
 * @param pasos Esta variable indica el numero de pasos que se desea avanzar.
 * @param motor_d Indica el motor que se quiere mover, opcion 1 o 2.
 * @param Speed Indica la velocidad del giro, si se disminuye el valor se aumenta la velocidad y si se aumenta este valor la velocidad disminuye .
 */

void move(int pasos, int motor_d, int Speed)
{
	if (motor_d == 1)
	{
		for (int i = 0; i < pasos; i++)
		{
			digitalWrite(STEP_PIN, HIGH);
			delayMicroseconds(Speed);
			digitalWrite(STEP_PIN, LOW);
			delayMicroseconds(Speed);
		}
	}

	if (motor_d == 2)
	{
		for (int j = 0; j < pasos; j++)
		{
			digitalWrite(STEP_PIN2, HIGH);
			delayMicroseconds(Speed);
			digitalWrite(STEP_PIN2, LOW);
			delayMicroseconds(Speed);
		}
	}
}

/**
 * @brief Esta funcion se utiliza entre programas para verificar que la siguiente secuencia iniciara en el punto cero.
 * en este caso la funcion contempla solo al brazo 1 con la polaridad del iman positiva. 
 * DESCRIPCION GENERAL
 * En la primera condicion de esta funcion se determina si el brazo se encuentra fuera del rango del sensor
 * En caso de entrar en esta condicion inicia una busqueda con rango maximo de 200 pasos en sentido horario
 * Si no se encuentra nada en ese rango inicia una busqueda en sentido antihorario con un rango maximo de 400 pasos
 * Si encuentra el sensor en la primera busqueda se activa una bandera para determinar que esta posicionado del lado izquierdo del sensor 
 * posteriormente se mueve 100 pasos en sentido horario y regresa en sentido antihorario para posicionarse al inicio del sensor pero del lado derecho.
 * Existe otro caso en el cual al iniciar la funcion el brazo ya se encuentra dentro del rango del sensor, en ese caso se mueve 100 pasos en sentido 
 * horario para garantizar que sale del sensor y posteriormente regresa en sentido antihorario hasta que es detectado por el sensor posicionandose asi
 * del lado derecho del sensor.
 * Finalmente cuando ya esta posicionado al inicio del lado derecho del sensor realiza el proceso de slow calibration para centrarse correctamente.
 */

void verif_cal_positiveb1(void)
{
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(EN_PIN, LOW);
	int value1_f;
	flag = 1;
	int busq = 0;
	int Flag_b = 0;
	int Flag_der = 0;
	int Flag_izq = 0;

	digitalWrite(DIR_PIN, HIGH);
	digitalWrite(DIR_PIN2, HIGH);

	for (int i = 0; i < 40; i++)
	{
		value1_f = meanFilter2.AddValue(analogRead(hall1));
	}
	read_hall1 = value1_f / 4;
	if (read_hall1 < max_hall1)
	{
		while (busq < 200)
		{
			move(1, 1, 5000);
			move(1, 2, 5000);
			busq++;

			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;

			if (read_hall1 > max_hall1)
			{
				busq = 200;
				Flag_b = 1;
				Flag_izq = 1;
			}
		}
		if (Flag_b == 0)
		{
			busq = 0;
			digitalWrite(DIR_PIN, LOW);
			digitalWrite(DIR_PIN2, LOW);
			while (busq < 400)
			{
				move(1, 1, 5000);
				move(1, 2, 5000);
				busq++;
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
				if (read_hall1 > max_hall1)
				{
					busq = 400;
					Flag_der = 1;
				}
			}
		}
		if (Flag_izq == 1)
		{
			move(100, 1, 5000);
			digitalWrite(DIR_PIN, LOW);
			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;
			while (read_hall1 < max_hall1)
			{
				move(1, 1, 5000);
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
			}
		}
	}
	if (read_hall1 > max_hall1)
	{
		if (Flag_der == 0 && Flag_izq == 0)
		{
			digitalWrite(DIR_PIN, HIGH);
			move(100, 1, 5000);
			digitalWrite(DIR_PIN, LOW);
			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;
			while (read_hall1 < max_hall1)
			{
				move(1, 1, 5000);
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
			}
		}
	}

	delay(5000);
	slow_Calibration_hall1();
	delay(5000);
}

/**
 * @brief Esta funcion se utiliza entre programas para verificar que la siguiente secuencia iniciara en el punto cero.
 * en este caso la funcion contempla solo al brazo 2 con la polaridad del iman positiva.
 * DESCRIPCION GENERAL
 * En la primera condicion de esta funcion se determina si el brazo se encuentra fuera del rango del sensor
 * En caso de entrar en esta condicion inicia una busqueda con rango maximo de 600 pasos en sentido antihorario
 * Si no se encuentra nada en ese rango inicia una busqueda en sentido horario con un rango maximo de 1200 pasos
 * Si encuentra el sensor en la primera busqueda se activa una bandera para determinar que esta posicionado del lado derecho del sensor 
 * posteriormente se mueve 600 pasos en sentido antihorario y regresa en sentido horario para posicionarse al inicio del sensor pero del lado izquierdo.
 * Existe otro caso en el cual al iniciar la funcion el brazo ya se encuentra dentro del rango del sensor, en ese caso se mueve 600 pasos en sentido 
 * antihorario para garantizar que sale del sensor y posteriormente regresa en sentido horario hasta que es detectado por el sensor posicionandose asi
 * del lado izquierdo del sensor.
 * Finalmente cuando ya esta posicionado al inicio del lado izquierdo del sensor realiza el proceso de slow calibration para centrarse correctamente.
 */

void verif_cal_positiveb2(void)
{
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(EN_PIN, LOW);
	int value2_f;
	flag = 1;
	int busq2 = 0;
	int Flag2_b = 0;
	int Flag2_der = 0;
	int Flag2_izq = 0;
	int value1_f;
	digitalWrite(DIR_PIN, LOW);
	digitalWrite(DIR_PIN2, LOW);

	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value1_f / 4;
	if (read_hall2 < max_hall2)
	{
		while (busq2 < 600)
		{
			move(1, 2, 5000);
			busq2++;

			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;

			if (read_hall2 > max_hall2)
			{
				busq2 = 600;
				Flag2_b = 1;
				Flag2_izq = 1;
			}
		}
		if (Flag2_b == 0)
		{
			busq2 = 0;
			digitalWrite(DIR_PIN2, HIGH);
			while (busq2 < 1200)
			{
				move(1, 2, 5000);
				busq2++;
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
				if (read_hall2 > max_hall2)
				{
					busq2 = 1200;
					Flag2_der = 1;
				}
			}
		}
		if (Flag2_izq == 1)
		{
			move(600, 2, 5000);
			digitalWrite(DIR_PIN2, HIGH);
			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;
			while (read_hall2 < max_hall2)
			{
				move(1, 2, 5000);
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
			}
		}
	}
	if (read_hall2 > max_hall2)
	{
		if (Flag2_der == 0 && Flag2_izq == 0)
		{
			digitalWrite(DIR_PIN2, LOW);
			move(600, 1, 5000);
			digitalWrite(DIR_PIN2, HIGH);
			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;
			while (read_hall2 < max_hall2)
			{
				move(1, 2, 5000);
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
			}
		}
	}

	delay(5000);
	slow_Calibration_hall2();
	delay(5000);
}

/**
 * @brief Esta funcion se utiliza entre programas para verificar que la siguiente secuencia iniciara en el punto cero.
 * en este caso la funcion contempla solo al brazo 1 con la polaridad del iman negativa. 
 * DESCRIPCION GENERAL
 * En la primera condicion de esta funcion se determina si el brazo se encuentra fuera del rango del sensor
 * En caso de entrar en esta condicion inicia una busqueda con rango maximo de 200 pasos en sentido horario
 * Si no se encuentra nada en ese rango inicia una busqueda en sentido antihorario con un rango maximo de 400 pasos
 * Si encuentra el sensor en la primera busqueda se activa una bandera para determinar que esta posicionado del lado izquierdo del sensor 
 * posteriormente se mueve 100 pasos en sentido horario y regresa en sentido antihorario para posicionarse al inicio del sensor pero del lado derecho.
 * Existe otro caso en el cual al iniciar la funcion el brazo ya se encuentra dentro del rango del sensor, en ese caso se mueve 100 pasos en sentido 
 * horario para garantizar que sale del sensor y posteriormente regresa en sentido antihorario hasta que es detectado por el sensor posicionandose asi
 * del lado derecho del sensor.
 * Finalmente cuando ya esta posicionado al inicio del lado izquierdo del sensor realiza el proceso de slow calibration para centrarse correctamente.
 */

void verif_cal_negativeb1(void)
{
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(EN_PIN, LOW);
	int value1_f;
	flag = 1;
	int busq = 0;
	int Flag_b = 0;
	int Flag_der = 0;
	int Flag_izq = 0;
	digitalWrite(DIR_PIN, HIGH);
	digitalWrite(DIR_PIN2, HIGH);

	for (int i = 0; i < 40; i++)
	{
		value1_f = meanFilter2.AddValue(analogRead(hall1));
	}
	read_hall1 = value1_f / 4;
	if (read_hall1 > min_hall1)
	{
		while (busq < 200)
		{
			move(1, 1, 5000);
			move(1, 2, 5000);
			busq++;

			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;

			if (read_hall1 < min_hall1)
			{
				busq = 200;
				Flag_b = 1;
				Flag_izq = 1;
			}
		}
		if (Flag_b == 0)
		{
			busq = 0;
			digitalWrite(DIR_PIN, LOW);
			digitalWrite(DIR_PIN2, LOW);
			while (busq < 400)
			{
				move(1, 1, 5000);
				move(1, 2, 5000);
				busq++;
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
				if (read_hall1 < min_hall1)
				{
					busq = 400;
					Flag_der = 1;
				}
			}
		}
		if (Flag_izq == 1)
		{
			move(100, 1, 5000);
			digitalWrite(DIR_PIN, LOW);
			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;
			while (read_hall1 > min_hall1)
			{
				move(1, 1, 5000);
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
			}
		}
	}
	if (read_hall1 < min_hall1)
	{
		if (Flag_der == 0 && Flag_izq == 0)
		{
			digitalWrite(DIR_PIN, HIGH);
			move(100, 1, 5000);
			digitalWrite(DIR_PIN, LOW);
			for (int i = 0; i < 40; i++)
			{
				value1_f = meanFilter2.AddValue(analogRead(hall1));
			}
			read_hall1 = value1_f / 4;
			while (read_hall1 > min_hall1)
			{
				move(1, 1, 5000);
				for (int i = 0; i < 40; i++)
				{
					value1_f = meanFilter2.AddValue(analogRead(hall1));
				}
				read_hall1 = value1_f / 4;
			}
		}
	}

	delay(5000);
	slow_Calibration_hall1_negative();
	delay(5000);
}

/**
 * @brief Esta funcion se utiliza entre programas para verificar que la siguiente secuencia iniciara en el punto cero.
 * en este caso la funcion contempla solo al brazo 2 con la polaridad del iman negativa.
 * DESCRIPCION GENERAL
 * En la primera condicion de esta funcion se determina si el brazo se encuentra fuera del rango del sensor
 * En caso de entrar en esta condicion inicia una busqueda con rango maximo de 600 pasos en sentido antihorario
 * Si no se encuentra nada en ese rango inicia una busqueda en sentido horario con un rango maximo de 1200 pasos
 * Si encuentra el sensor en la primera busqueda se activa una bandera para determinar que esta posicionado del lado derecho del sensor 
 * posteriormente se mueve 600 pasos en sentido antihorario y regresa en sentido horario para posicionarse al inicio del sensor pero del lado izquierdo.
 * Existe otro caso en el cual al iniciar la funcion el brazo ya se encuentra dentro del rango del sensor, en ese caso se mueve 600 pasos en sentido 
 * antihorario para garantizar que sale del sensor y posteriormente regresa en sentido horario hasta que es detectado por el sensor posicionandose asi
 * del lado izquierdo del sensor.
 * Finalmente cuando ya esta posicionado al inicio del lado izquierdo del sensor realiza el proceso de slow calibration para centrarse correctamente. 
 */

void verif_cal_negativeb2(void)
{
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(EN_PIN, LOW);
	int value2_f;
	flag = 1;
	int busq2 = 0;
	int Flag2_b = 0;
	int Flag2_der = 0;
	int Flag2_izq = 0;
	int value1_f;
	digitalWrite(DIR_PIN, LOW);
	digitalWrite(DIR_PIN2, LOW);

	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value1_f / 4;
	if (read_hall2 > min_hall2)
	{
		while (busq2 < 600)
		{
			move(1, 2, 5000);
			busq2++;

			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;

			if (read_hall2 < min_hall2)
			{
				busq2 = 600;
				Flag2_b = 1;
				Flag2_izq = 1;
			}
		}
		if (Flag2_b == 0)
		{
			busq2 = 0;
			digitalWrite(DIR_PIN2, HIGH);
			while (busq2 < 1200)
			{
				move(1, 2, 5000);
				busq2++;
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
				if (read_hall2 < min_hall2)
				{
					busq2 = 1200;
					Flag2_der = 1;
				}
			}
		}
		if (Flag2_izq == 1)
		{
			move(600, 2, 5000);
			digitalWrite(DIR_PIN2, HIGH);
			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;
			while (read_hall2 > min_hall2)
			{
				move(1, 2, 5000);
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
			}
		}
	}
	if (read_hall2 < min_hall2)
	{
		if (Flag2_der == 0 && Flag2_izq == 0)
		{
			digitalWrite(DIR_PIN2, LOW);
			move(600, 2, 5000);
			digitalWrite(DIR_PIN2, HIGH);
			for (int i = 0; i < 40; i++)
			{
				value2_f = meanFilter2.AddValue(analogRead(hall2));
			}
			read_hall2 = value2_f / 4;
			while (read_hall2 > min_hall2)
			{
				move(1, 2, 5000);
				for (int i = 0; i < 40; i++)
				{
					value2_f = meanFilter2.AddValue(analogRead(hall2));
				}
				read_hall2 = value2_f / 4;
			}
		}
	}

	delay(5000);
	slow_Calibration_hall2_negative();
	delay(5000);
}

///////////////////////

/**
 * @brief Esta funcion se seleccionan las funciones necesarias para la verificacion de los brazos segun la polaridad correspondiente a cada iman.
 * 
 */

void CalibMotor::verificacion_cal()
{
	if (Pole_sens1 == 1)
	{
		verif_cal_positiveb1();
	}
	if (Pole_sens1 == 0)
	{
		verif_cal_negativeb1();
	}
	if (Pole_sens2 == 1)
	{
		verif_cal_positiveb2();
	}
	if (Pole_sens2 == 0)
	{
		verif_cal_negativeb2();
	}
}

///////////////////////

/**
 * @brief Esta funcion determina el nivel de referencia a partir del cual se considera la deteccion de campo magnetico.
 * @param level_zero1 esta variable global almacena el nivel de referencia para el sensor Hall 1.
 * @return Retorna un 1 si el calculo del nivel de referencia se obtuvo correctamente, en caso contrario retorna un 0 para repetir de proceso de busqueda del nivel de referencia.
 * DESCRIPCION GENERAL
 * Esta funcion inicia moviendo el brazo en sentido antihorario usando la funcion normal_turn y sensando el DIAG_PIN en caso de que el brazo colisione,
 * si colisiona el brazo 1 regresa 300 pasos y el brazo 2 baja a 90 grados para evitar la colision.
 * Si el brazo logra finalizar el primer movimiento sin colisionar regresa 800 pasos e inicia el algoritmo para determinar el nivel cero del sensor.
 * Se toman 5 muestras en angulos distintos, la primera muestra se toma desde donde quedo posicionado el brazo, posteriormente se mueve 10 grados en sentido horario para tomar
 * la segunda muestra, se mueve 10 grados en sentido horario para la tercera medicion, regresa 30 grados en sentido antihorario para la cuarta medicion y avanza 10 grados mas
 * para tomar la quita medicion.
 * En cada una de las 5 muestras se toman 5 mediciones filtradas de las cuales se obtienen el valor maximo, el valor minimo, el valor promedio y un valor al que se le determino 
 * como valor similitud ya que es comparado con cada promedio de las 5 muestras tomadas a diferentes algulos y se almacena el contador de cuantos valores son similares al de la 
 * muestra analizada.
 * Finalmente se determina que si 3 muestras o mas son similares se ha hallado el valor cero del sensor el cual se almacena en una variable global y se retorna un 1 para 
 * indicar que no es necesario repetir el procedimiento.
 */

int zero_Hall1(void)
{
	flag = 1;
	digitalWrite(EN_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);
	int value_hall1;
	int value_filt;
	int min = 5000;
	int max = 0;
	int add = 0;
	int average;
	int add_averages = 0;
	int values_similares = 0;
	int p = 0;
	int dif_ref;
	flag = 2;
	int flag_c = 0;
	digitalWrite(DIR_PIN, LOW);
	digitalWrite(DIR_PIN2, LOW);
	normal_turn();
	delay(500);
	while (p < 300)
	{
		if (digitalRead(DIAG_PIN) == 1)
		{
			flag = 1;
			flag_c = 1;
			//Back a little  first arm
			digitalWrite(DIR_PIN, HIGH);
			move(300, 1, 3000);

			//Move second arm 90 degrees.
			digitalWrite(EN_PIN, LOW);
			digitalWrite(EN_PIN2, LOW);
			digitalWrite(DIR_PIN2, LOW);
			move(1600, 2, 2000);
			p = 300;
		}
		p++;
		delay(10);
	}
	if (flag_c == 0)
	{
		flag = 1;
		digitalWrite(EN_PIN, LOW);
		digitalWrite(EN_PIN2, LOW);
		digitalWrite(DIR_PIN, HIGH);
		move(800, 1, 2000);
	}

	flag = 1;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall1));
		}
		value_hall1 = value_filt / 4;

		if (value_hall1 > max)
		{
			max = value_hall1;
		}
		if (value_hall1 < min)
		{
			min = value_hall1;
		}
		add = add + value_hall1;
		vect_max[0] = max;
		vect_min[0] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom[0] = average;
	add = 0;

	digitalWrite(DIR_PIN, HIGH);
	move(178, 1, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall1));
		}
		value_hall1 = value_filt / 4;

		if (value_hall1 > max)
		{
			max = value_hall1;
		}
		if (value_hall1 < min)
		{
			min = value_hall1;
		}
		add = add + value_hall1;
		vect_max[1] = max;
		vect_min[1] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom[1] = average;
	add = 0;

	digitalWrite(DIR_PIN, HIGH);
	move(178, 1, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall1));
		}
		value_hall1 = value_filt / 4;

		if (value_hall1 > max)
		{
			max = value_hall1;
		}
		if (value_hall1 < min)
		{
			min = value_hall1;
		}
		add = add + value_hall1;
		vect_max[2] = max;
		vect_min[2] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom[2] = average;
	add = 0;

	digitalWrite(DIR_PIN, LOW);
	move(533, 1, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall1));
		}
		value_hall1 = value_filt / 4;

		if (value_hall1 > max)
		{
			max = value_hall1;
		}
		if (value_hall1 < min)
		{
			min = value_hall1;
		}
		add = add + value_hall1;
		vect_max[3] = max;
		vect_min[3] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom[3] = average;
	add = 0;

	digitalWrite(DIR_PIN, LOW);
	move(178, 1, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall1));
		}
		value_hall1 = value_filt / 4;

		if (value_hall1 > max)
		{
			max = value_hall1;
		}
		if (value_hall1 < min)
		{
			min = value_hall1;
		}
		add = add + value_hall1;
		vect_max[4] = max;
		vect_min[4] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom[4] = average;
	add = 0;

	int similarity;
	int cont_simi = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			similarity = vect_prom[i] - vect_prom[j];
			if (similarity < 0)
			{
				similarity = similarity * (-1);
			}
			if (similarity <= 10)
			{
				cont_simi++;
			}
		}
		vect_simi[i] = cont_simi;
		cont_simi = 0;
	}
	int accum5 = 0;
	int accum4 = 0;
	int accum3 = 0;
	for (int i = 0; i < 5; i++)
	{
		if (vect_simi[i] == 5)
		{
			accum5++;
		}
		if (vect_simi[i] == 4)
		{
			accum4++;
		}
		if (vect_simi[i] == 3)
		{
			accum3++;
		}
	}

	min = 5000;
	max = 0;
	for (int i = 0; i < 5; i++)
	{
		if (vect_simi[i] >= 3)
		{
			if (vect_max[i] > max)
			{
				max = vect_max[i];
			}
			if (vect_min[i] < min)
			{
				min = vect_min[i];
			}
			add_averages = add_averages + vect_prom[i];
			values_similares++;
		}
	}
	level_zero1 = add_averages / values_similares;
	A = level_zero1;
	H = highByte(A);
	L = lowByte(A);
	EEPROM.write(403, H);
	EEPROM.commit();
	EEPROM.write(404, L);
	EEPROM.commit();
	max_hall1 = max;
	min_hall1 = min;

	if (accum5 != 0)
	{
		return 1;
	}
	if (accum4 != 0)
	{
		return 1;
	}
	if (accum3 != 0)
	{
		return 1;
	}
}

/**
 * @brief Esta funcion determina el nivel de referencia a partir del cual se considera la deteccion de campo magnetico.
 * @param level_zero2 esta variable global almacena el nivel de referencia para el sensor Hall 2.
 * @return Retorna un 1 si el calculo del nivel de referencia se obtuvo correctamente, en caso contrario retorna un 0 para repetir de proceso de busqueda del nivel de referencia.
 * DESCRIPCION GENERAL
 * Esta funcion inicia moviendo el brazo en sentido antihorario usando la funcion normal_turn y sensando el DIAG_PIN en caso de que el brazo colisione,
 * si colisiona el brazo 1 regresa 300 pasos y el brazo 2 baja a 90 grados para evitar la colision.
 * Si el brazo logra finalizar el primer movimiento sin colisionar regresa 800 pasos e inicia el algoritmo para determinar el nivel cero del sensor.
 * Se toman 5 muestras en angulos distintos, la primera muestra se toma desde donde quedo posicionado el brazo, posteriormente se mueve 10 grados en sentido horario para tomar
 * la segunda muestra, se mueve 10 grados en sentido horario para la tercera medicion, regresa 30 grados en sentido antihorario para la cuarta medicion y avanza 10 grados mas
 * para tomar la quita medicion.
 * En cada una de las 5 muestras se toman 5 mediciones filtradas de las cuales se obtienen el valor maximo, el valor minimo, el valor promedio y un valor al que se le determino 
 * como valor similitud ya que es comparado con cada promedio de las 5 muestras tomadas a diferentes algulos y se almacena el contador de cuantos valores son similares al de la 
 * muestra analizada.
 * Finalmente se determina que si 3 muestras o mas son similares se ha hallado el valor cero del sensor el cual se almacena en una variable global y se retorna un 1 para 
 * indicar que no es necesario repetir el procedimiento.
 */

int zero_Hall2(void)
{
	flag = 1;
	digitalWrite(EN_PIN, HIGH);
	int value_hall2;
	int value_filt;
	int min = 5000;
	int max = 0;
	int add = 0;
	int average;
	int add_averages2 = 0;
	int values_similares2 = 0;
	int p = 0;
	int dif_ref;
	int flag_c = 0;
	flag = 3;
	normal_turn();
	delay(500);
	while (p < 300)
	{
		if (digitalRead(DIAG_PIN2) == 1)
		{
			flag = 1;
			flag_c = 1;
			//Move second arm 90 degrees.
			digitalWrite(EN_PIN, LOW);
			digitalWrite(EN_PIN2, LOW);
			digitalWrite(DIR_PIN2, HIGH);
			move(1600, 2, 2000);
			p = 300;
		}
		p++;
		delay(10);
	}
	if (flag_c == 0)
	{
		flag = 1;
		//Move second arm 90 degrees.
		digitalWrite(EN_PIN, LOW);
		digitalWrite(EN_PIN2, LOW);
		digitalWrite(DIR_PIN2, HIGH);
		move(800, 2, 2000);
	}
	flag = 1;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall2));
		}
		value_hall2 = value_filt / 4;

		if (value_hall2 > max)
		{
			max = value_hall2;
		}
		if (value_hall2 < min)
		{
			min = value_hall2;
		}
		add = add + value_hall2;
		vect_max2[0] = max;
		vect_min2[0] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom2[0] = average;
	add = 0;

	digitalWrite(DIR_PIN2, HIGH);
	move(178, 2, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall2));
		}
		value_hall2 = value_filt / 4;
		if (value_hall2 > max)
		{
			max = value_hall2;
		}
		if (value_hall2 < min)
		{
			min = value_hall2;
		}
		add = add + value_hall2;
		vect_max2[1] = max;
		vect_min2[1] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom2[1] = average;
	add = 0;

	digitalWrite(DIR_PIN2, HIGH);
	move(178, 2, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall2));
		}
		value_hall2 = value_filt / 4;

		if (value_hall2 > max)
		{
			max = value_hall2;
		}
		if (value_hall2 < min)
		{
			min = value_hall2;
		}
		add = add + value_hall2;
		vect_max2[2] = max;
		vect_min2[2] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom2[2] = average;
	add = 0;

	digitalWrite(DIR_PIN2, LOW);
	move(533, 2, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall2));
		}
		value_hall2 = value_filt / 4;

		if (value_hall2 > max)
		{
			max = value_hall2;
		}
		if (value_hall2 < min)
		{
			min = value_hall2;
		}
		add = add + value_hall2;
		vect_max2[3] = max;
		vect_min2[3] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom2[3] = average;
	add = 0;

	digitalWrite(DIR_PIN2, LOW);
	move(178, 2, 3000);

	min = 5000;
	max = 0;
	for (int x = 0; x < 5; x++)
	{
		for (int i = 0; i < 40; i++)
		{
			value_filt = meanFilter2.AddValue(analogRead(hall2));
		}
		value_hall2 = value_filt / 4;

		if (value_hall2 > max)
		{
			max = value_hall2;
		}
		if (value_hall2 < min)
		{
			min = value_hall2;
		}
		add = add + value_hall2;
		vect_max2[4] = max;
		vect_min2[4] = min;
		delay(100);
	}
	average = add / 5;
	vect_prom2[4] = average;
	add = 0;

	int similarity;
	int cont_simi = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			similarity = vect_prom2[i] - vect_prom2[j];
			if (similarity < 0)
			{
				similarity = similarity * (-1);
			}
			if (similarity <= 10)
			{
				cont_simi++;
			}
		}
		vect_simi2[i] = cont_simi;
		cont_simi = 0;
	}
	int accum5 = 0;
	int accum4 = 0;
	int accum3 = 0;
	for (int i = 0; i < 5; i++)
	{
		if (vect_simi2[i] == 5)
		{
			accum5++;
		}
		if (vect_simi2[i] == 4)
		{
			accum4++;
		}
		if (vect_simi2[i] == 3)
		{
			accum3++;
		}
	}

	min = 5000;
	max = 0;
	for (int i = 0; i < 5; i++)
	{
		if (vect_simi2[i] >= 3)
		{
			if (vect_max2[i] > max)
			{
				max = vect_max2[i];
			}
			if (vect_min2[i] < min)
			{
				min = vect_min2[i];
			}
			add_averages2 = add_averages2 + vect_prom2[i];
			values_similares2++;
		}
	}
	level_zero2 = add_averages2 / values_similares2;
	A = level_zero2;
	H = highByte(A);
	L = lowByte(A);
	EEPROM.write(405, H);
	EEPROM.commit();
	EEPROM.write(406, L);
	EEPROM.commit();
	max_hall2 = max;
	min_hall2 = min;

	if (accum5 != 0)
	{
		return 1;
	}
	if (accum4 != 0)
	{
		return 1;
	}
	if (accum3 != 0)
	{
		return 1;
	}
}

/**
 * @brief Esta funcion determina el polo del campo magnetico correspondiente al iman que interacciona con el brazo 1.
 * @param vect_Pole1 Se almacenan un conjunto de datos correspondientes a un barrido del brazo1 por debajo del sensor de efecto Hall.
 * @return Retorna un 1 si se determina que el polo positivo y un 0 si el polo es negativo.
 * DESCRIPCION GFENERAL
 * Antes de entrar a esta funcion el brazo ya se encuentra posicionado correctamente al inicio del sensor
 * Para determinar el polo se avanzan 100 pasos para que al retornar el movimiento el brazo termine en la mis ma posicion en la que inicio.
 * Al retornar el sentido horario almacena el valor maximo y el valor minimo que haya obtenidode las mediciones tomadas en cada paso, este valor 
 * se obtiene como un valor absoluto respecto del nivel cero, el cual para este punto del programa ya se conoce.
 * finalmente se compara el valor maximo absoluto obtenido y el valor minimo absoluto obtenido, si el maximo absoluto es mayor que el minimo
 * absoluto se determina que le polo es positivo, en cambio si sucede de forma contraria se determina que el polo es negativo.
 */

int Pole1(void)
{
	int vect_Pole1[100];
	int maximum_Pole1 = 0;
	int minimum_Pole1 = 5000;
	int dif_max_abs1;
	int dif_min_abs1;
	digitalWrite(EN_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);

	for (int i = 0; i < 100; i++)
	{
		vect_Pole1[i] = -1;
	}
	int k = 0;
	int val_sens;
	digitalWrite(DIR_PIN, LOW);

	for (int t = 0; t < 100; t++)
	{
		move(1, 1, 5000);
	}

	digitalWrite(DIR_PIN, HIGH);
	while (k < 100)
	{
		move(1, 1, 5000);
		for (int y = 0; y < 40; y++)
		{
			read_hall1 = (analogRead(hall1)) / 4;
			val_sens = meanFilter2.AddValue(read_hall1);
		}
		delay(1);
		if (val_sens > maximum_Pole1) 
		{
			maximum_Pole1 = val_sens; 
		}
		if (val_sens < minimum_Pole1) 
		{
			minimum_Pole1 = val_sens; 
		}
		vect_Pole1[k] = val_sens; 

		delay(1);
		k++;
	}
	dif_max_abs1 = maximum_Pole1 - level_zero1;
	dif_min_abs1 = level_zero1 - minimum_Pole1;
	if (dif_max_abs1 > dif_min_abs1)
	{
		return 1;
	}
	if (dif_max_abs1 < dif_min_abs1)
	{
		return 0;
	}
}

/**
 * @brief Esta funcion determina el polo del campo magnetico correspondiente al iman que interacciona con el brazo 2.
 * @param vect_Pole2 Se almacenan un conjunto de datos correspondientes a un barrido del brazo2 por debajo del sensor de efecto Hall.
 * @return Retorna un 1 si se determina que el polo positivo y un 0 si el polo es negativo.
 * DESCRIPCION GFENERAL
 * Antes de entrar a esta funcion el brazo ya se encuentra posicionado correctamente al inicio del sensor
 * Para determinar el polo se avanzan 800 pasos para que al retornar el movimiento el brazo termine en la mis ma posicion en la que inicio.
 * Al retornar el sentido horario almacena el valor maximo y el valor minimo que haya obtenidode las mediciones tomadas en cada paso, este valor 
 * se obtiene como un valor absoluto respecto del nivel cero, el cual para este punto del programa ya se conoce.
 * finalmente se compara el valor maximo absoluto obtenido y el valor minimo absoluto obtenido, si el maximo absoluto es mayor que el minimo
 * absoluto se determina que le polo es positivo, en cambio si sucede de forma contraria se determina que el polo es negativo.
 */

int Pole2(void)
{
	int value2_f;
	int vect_Pole2[800];
	int maximum_Pole2 = 0;
	int minimum_Pole2 = 5000;
	int dif_max_abs2;
	int dif_min_abs2;
	digitalWrite(EN_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);

	for (int i = 0; i < 800; i++)
	{
		vect_Pole2[i] = -1;
	}
	int k = 0;
	int val_sens;
	digitalWrite(DIR_PIN2, HIGH);

	for (int t = 0; t < 800; t++)
	{
		move(1, 2, 5000);
	}
	digitalWrite(DIR_PIN2, LOW);
	while (k < 800)
	{
		move(1, 2, 5000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			val_sens = meanFilter2.AddValue(read_hall2);
		}
		delay(1);
		if (val_sens > maximum_Pole2) 
		{
			maximum_Pole2 = val_sens; 
		}
		if (val_sens < minimum_Pole2) 
		{
			minimum_Pole2 = val_sens; 
		}
		vect_Pole2[k] = val_sens; 

		delay(1);
		k++;
	}
	digitalWrite(DIR_PIN2, LOW);
	move(100, 2, 8000);
	digitalWrite(DIR_PIN2, HIGH);
	for (int i = 0; i < 40; i++)
	{
		value2_f = meanFilter2.AddValue(analogRead(hall2));
	}
	read_hall2 = value2_f / 4;
	while (read_hall2 < max_hall2)
	{
		move(1, 2, 8000);
		for (int y = 0; y < 40; y++)
		{
			read_hall2 = (analogRead(hall2)) / 4;
			read_hall2 = meanFilter2.AddValue(read_hall2);
		}
	}
	delay(1000);
	dif_max_abs2 = maximum_Pole2 - level_zero2;
	dif_min_abs2 = level_zero2 - minimum_Pole2;
	if (dif_max_abs2 > dif_min_abs2)
	{
		return 1;
	}
	if (dif_max_abs2 < dif_min_abs2)
	{
		return 0;
	}
}

/**
 * @brief Esta funcion verifica si ya se han almacenado en la EEPROM las variables iniciales necesarias para en funcionamiento del sistema,
 * lo cual solo es posible cuando se programa por primera vez la ESP32 o despues de un reset completo del sistema.
 * @return Retorna un 1 si se determina que la EEPROM eseta vacia o un 0 en caso de que ya existan los valores almacenados. 
 */

int Check_ini(void)
{
	EEPROM.begin(EEPROM_SIZE);
	int value_eeprom;
	int cont_eeprom = 0;
	for (int i = 401; i < 413; i++)
	{
		value_eeprom = EEPROM.read(i);
		if (value_eeprom == 255)
		{
			cont_eeprom++;
		}
	}
	if (cont_eeprom == 12)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}   