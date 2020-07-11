#include "Testing.h"
#include "CalibMotor.h"

extern TMC2209Stepper driver;
extern TMC2209Stepper driver2;

void info_motor1(void);
void info_motor2(void);
void mover(int,int,int);
void Testing(void);

CalibMotor haloCalibTest;

#define PIN 32
#define NUMPIXELS 36 // numero de pixels en la tira
Adafruit_NeoPixel pixels_test(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 50

void Testing::Test()
{
	haloCalibTest.init();
	File myFile;
	File root;
	SdFat SD;
	uint32_t cardSize;
	info_motor1();
	info_motor2();
	delay(2000);
	Serial.println("Pin config");
	Serial.println(analogRead(PIN_ProducType));

	//====Inicializar SD====
	while (!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
	{
		Serial.println("Card failed, or not present");
		delay(200);
	}
	Serial.println("Card present");
	root = SD.open("/");

	cardSize = SD.card()->cardSize();
	Serial.println("Card size");
	Serial.print(0.000512*cardSize);
	Serial.println("  MB");

	delay(1000);

	Serial.println("Prueba sensor Hall");
	delay(5000);
	//===============Sensor_Hall==================
	int dato_hall1 = 0;
	int dato_hall2 = 0;

	dato_hall1 = analogRead(hall1);
	dato_hall2 = analogRead(hall2);
	if (dato_hall1 > 1500 && dato_hall1 < 2200)
	{
		Serial.println("Sensor Hall 1 OK");
	}
	else
	{
		Serial.println("Sensor Hall 1 Fail");
	}
	if (dato_hall2 > 1500 && dato_hall2 < 2200)
	{
		Serial.println("Sensor Hall 2 OK");
	}
	else
	{
		Serial.println("Sensor Hall 2 Fail");
	}

	//==============================================

	digitalWrite(EN_PIN, LOW);
	digitalWrite(DIR_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(DIR_PIN2, LOW);
	//================Test===================
	//===============Motor2==================
	digitalWrite(DIR_PIN2, LOW);
	driver2.rms_current(500);
	driver2.microsteps(16);
	Serial.println("Currente_motor2");
	Serial.println(driver2.rms_current());
	Serial.println("Microsteps_motor2");
	Serial.println(driver2.microsteps());
	mover(1600, 2, 1000);
	delay(500);

	digitalWrite(DIR_PIN2, HIGH);
	driver2.rms_current(700);
	driver2.microsteps(32);
	Serial.println("Currente_motor2");
	Serial.println(driver2.rms_current());
	Serial.println("Microsteps_motor2");
	Serial.println(driver2.microsteps());
	mover(3200, 2, 1000);
	delay(500);

	digitalWrite(DIR_PIN2, LOW);
	driver2.rms_current(900);
	driver2.microsteps(64);
	Serial.println("Currente_motor2");
	Serial.println(driver2.rms_current());
	Serial.println("Microsteps_motor2");
	Serial.println(driver2.microsteps());
	mover(6400, 2, 1000);
	delay(500);
	//===============Motor1==================
	digitalWrite(DIR_PIN, LOW);
	driver.rms_current(500);
	driver.microsteps(16);
	Serial.println("Currente_motor1");
	Serial.println(driver.rms_current());
	Serial.println("Microsteps_motor1");
	Serial.println(driver.microsteps());
	mover(1600, 1, 1000);
	delay(500);

	digitalWrite(DIR_PIN, HIGH);
	driver.rms_current(700);
	driver.microsteps(32);
	Serial.println("Currente_motor1");
	Serial.println(driver.rms_current());
	Serial.println("Microsteps_motor1");
	Serial.println(driver.microsteps());
	mover(3200, 1, 1000);
	delay(500);

	digitalWrite(DIR_PIN, LOW);
	driver.rms_current(900);
	driver.microsteps(64);
	Serial.println("Currente_motor1");
	Serial.println(driver.rms_current());
	Serial.println("Microsteps_motor1");
	Serial.println(driver.microsteps());
	mover(6400, 1, 1000);
	delay(500);

	delay(1000);

	for (int i = 0; i < NUMPIXELS; i++)
	{
		pixels_test.setPixelColor(i, pixels_test.Color(255, 255, 255));

		pixels_test.show(); // Send the updated pixel colors to the hardware.

		delay(DELAYVAL); // Pause before next pass through loop
	}

	driver.rms_current(500);
	driver.microsteps(16);
	driver2.rms_current(500);
	driver2.microsteps(16);
	digitalWrite(EN_PIN, LOW);
	digitalWrite(DIR_PIN, LOW);
	digitalWrite(EN_PIN2, LOW);
	digitalWrite(DIR_PIN2, LOW);
	while (true)
	{
		mover(1, 1, 1000);
		mover(1, 2, 1000);
	}
}

void info_motor1()
{
	uint8_t result_1 = driver.test_connection();
	if (result_1)
	{
		Serial.println(F("failed motor 1 connection"));
		Serial.println(result_1);
	}
	if (result_1 == 0)
	{
		Serial.println(F("motor1 conectted"));
	}
}

void info_motor2()
{
	uint8_t result_2 = driver2.test_connection();
	if (result_2)
	{
		Serial.println(F("failed motor 2 connection"));
		Serial.println(result_2);
	}
	if (result_2 == 0)
	{
		Serial.println(F("motor 2 conectted"));
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