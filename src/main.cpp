#include <Arduino.h>
#include "SdFiles.h"
#include "Motors.h"
#include "Bluetooth.h"
#include <Adafruit_NeoPixel.h>
#define FASTLED_ESP32_I2S true
#include <FastLED.h>
#include "Calibration.h"
#include "Testing.h"

#include "SPI.h"
#include "SdFat.h"
SdFat SD;
bool readingSDFile = false;
#include <math.h>

#include <EEPROM.h>






#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID1 "fd31a2be-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID2 "fd31a58e-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID3 "fd31a688-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID4 "fd31a778-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID5 "fd31a840-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID6 "fd31abc4-22e7-11eb-adc1-0242ac120002"


#define CHARACTERISTIC_UUID_LEDSPEED "1a9a7b7e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_CYCLEMODE "1a9a7dea-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_DIRECTION "1a9a8042-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_SELECTEDPALETTE "1a9a813c-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_AMOUNTCOLORS "1a9a820e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_POSITIONS "1a9a82d6-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_RED "1a9a83a8-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_GREEN "1a9a8466-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_BLUE "1a9a852e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_UPDATECPALETTE "1a9a87b8-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_MSGERRORLEDS "1a9a8880-2305-11eb-adc1-0242ac120002"
/*

1a9a8948-2305-11eb-adc1-0242ac120002
1a9a8a06-2305-11eb-adc1-0242ac120002
1a9a8ac4-2305-11eb-adc1-0242ac120002
1a9a8b8c-2305-11eb-adc1-0242ac120002*/

#define CHARACTERISTIC_UUID_1 "903cfcc2-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_2 "903cfede-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_3 "903cffe2-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_4 "903d00be-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_5 "903d0190-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_6 "903d024e-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_7 "903d0316-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_8 "903d0596-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_9 "903d065e-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_10 "903d071c-22eb-11eb-adc1-0242ac120002"





//====Extern Variables====
extern TMC2209Stepper driver;
extern TMC2209Stepper driver2;
extern bool sdExists(String );
extern bool sdRemove(String );
extern bool incrementGlobal;
extern double times[];
extern int pointerGlobal;
//====

//====ROM Varibales====
String  playListGlobal;
String  bluetoothNameGlobal;
int     orderModeGlobal;
int     speedMotorGlobal;
int     ledModeGlobal;
int     periodLedsGlobal;
bool    ceroZoneGlobal;
//====Global variables====
bool    ledsOffGlobal = false;
bool    rewindPlaylist = false;
bool    incrementIndexGlobal = true;
String  currentProgramGlobal;
String  nextProgramGlobal;
String  currentPlaylistGlobal;
int     currentPositionListGlobal;
int     delayLeds;
int     pListFileGlobal;
bool    changePositionList;
bool    changeProgram;
bool    stopProgramChangeGlobal = true;
bool    stop_boton;
bool    intermediateCalibration = false;
bool    firstExecution;
bool    availableDeceleration = false;
bool    turnOnLeds = false;
bool    ledsDirection;
bool    pauseModeGlobal = false;
bool    suspensionModeGlobal = false;
bool    startMovement = false;
long    q1StepsGlobal, q2StepsGlobal;
double  distanceGlobal;
bool speedChangedMain = false;
//====
//====Pallete Color Variables====
CRGBPalette16   NO_SD_PALLETE;
CRGBPalette16   UPTADATING_PALLETE;
CRGBPalette16   CALIBRATING_PALLETE;
CRGBPalette16   SDEMPTY_PALLETE;
CRGBPalette256  customPallete;
CRGBPalette256  pallette1, pallette2,pallette3,
pallette4,pallette5,pallette6,
pallette7,pallette8,pallette9,
pallette10,pallette11,pallette12,
pallette13,pallette14,pallette15;
//====
Motors Sandsara;

int errorCode;
//====function prototypes====
extern  int programming(String );
extern  void rebootWithMessage(String );
extern int stringToArray(String , uint8_t* , int );

int     moveInterpolateTo(double x, double y, double distance);
void    executeCode(int );
void    Neo_Pixel(int );
uint32_t rainbow();
void    FillLEDsFromPaletteColors(uint8_t );
void    changePalette(int );
void    SetupTotallyRandomPalette();
void    SetupBlackAndWhiteStripedPalette();
void    SetupPurpleAndGreenPalette();
void    ledsFunc( void * );
int     run_sandsara(String ,int );
int     movePolarTo(double ,double ,double, bool = false);

int     romSetPlaylist(String );
String  romGetPlaylist();
int     romSetOrderMode(int );
int     romGetOrderMode();
int     romSetPallete(int );
int     romGetPallete();
int     romSetSpeedMotor(int );
int     romGetSpeedMotor();
int     romSetPeriodLed(int );
int     romGetPeriodLed();
int     romSetCustomPallete(uint8_t* ,uint8_t* , uint8_t* ,uint8_t*, int);
int     romGetCustomPallete(CRGBPalette256 &);
int     romSetBluetoothName(String );
String  romGetBluetoothName();
int     romSetIntermediateCalibration(bool );
bool    romGetIntermediateCalibration();
int     romSetPositionList(int );
int     romGetPositionList();
int     romSetIncrementIndexPallete(bool );
bool    romGetIncrementIndexPallete();
int     romSetLedsDirection(bool );
bool    romGetLedsDirection();

void    findUpdate();
int     orderRandom(String ,int);
void    setFrom1(int [], int);
void    removeIndex(int [], int , int );
int     runFile(String );
void    goHomeSpiral();
void    bluetoothThread(void* );
double  linspace(double init,double stop, int amount,int index);
int     rgb2Interpolation(CRGBPalette256 &,uint8_t* );
void    goCenterSpiral(bool);
void    goEdgeSpiral(bool);
void    spiralGoTo(float , float );

void moveSteps(void* );

//====
//====Led Variables====
#define LED_PIN     32
int     NUM_LEDS;
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[MAX_NUMBERLEDS];
//====
#include <leds.h>

unsigned long timeLeds;
uint8_t startIndex = 0;
//====Product Type====
bool productType;
//====
//====Thred====
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t motorsTask;
//====
//====Calibration====
Calibration haloCalib;
//====Testing========
Testing haloTest;
//====


BLECharacteristic *characteristic_ledSpeed;
BLECharacteristic *characteristic_updateCustomPalette;
BLECharacteristic *characteristic_cycleMode;
BLECharacteristic *characteristic_direction;
BLECharacteristic *characteristic_amountOfColors;
BLECharacteristic *characteristic_positions;
BLECharacteristic *characteristic_red;
BLECharacteristic *characteristic_green;
BLECharacteristic *characteristic_blue;
BLECharacteristic *characteristic_msgErrorLeds;
BLECharacteristic *characteristic_selectedPaletteIndex;

BLECharacteristic *characteristic_1;
BLECharacteristic *characteristic_2;
BLECharacteristic *characteristic_3;
BLECharacteristic *characteristic_4;
BLECharacteristic *characteristic_5;
BLECharacteristic *characteristic_6;
BLECharacteristic *characteristic_7;
BLECharacteristic *characteristic_8;
BLECharacteristic *characteristic_9;


class speedLedCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int periodLed = value.toInt();
        if(periodLed < MIN_PERIOD_LED || periodLed > MAX_PERIOD_LED){
            characteristic_msgErrorLeds->setValue("error = -70");
            characteristic_msgErrorLeds->notify();
            return;
        }
        Serial.print("speed led was changed to: ");
        Serial.println(periodLed);
        periodLedsGlobal = periodLed;
        delayLeds = periodLed;
        romSetPeriodLed(periodLedsGlobal);
        characteristic_msgErrorLeds->setValue("ok");
        characteristic_msgErrorLeds->notify();
    } //onWrite
};

class cycleModeCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int cycleMode = value.toInt();
        if (cycleMode > 0){
            romSetIncrementIndexPallete(true);
        }
        else{
            romSetIncrementIndexPallete(false);
        }
        incrementIndexGlobal = romGetIncrementIndexPallete();
        characteristic_msgErrorLeds->setValue("ok");
        characteristic_msgErrorLeds->notify();
    } //onWrite
};

class directionCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        //retorna ponteiro para o registrador contendo o valor atual da caracteristica
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int direction = value.toInt();
        if (direction != 0){
            romSetLedsDirection(true);
        }
        else{
            romSetLedsDirection(false);
        }
        ledsDirection = romGetLedsDirection();
        characteristic_msgErrorLeds->setValue("ok");
        characteristic_msgErrorLeds->notify();
    } //onWrite
};

class selectedPaletteCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        //retorna ponteiro para o registrador contendo o valor atual da caracteristica
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int valueInt = value.toInt();
        if(valueInt < MIN_PALLETE || valueInt > MAX_PALLETE){
            characteristic_msgErrorLeds->setValue("error = -31");
            characteristic_msgErrorLeds->notify();
            return;
        }
        ledModeGlobal = valueInt;
        //verifica se existe dados (tamanho maior que zero)
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
        characteristic_msgErrorLeds->setValue("ok");
        characteristic_msgErrorLeds->notify();
    } //onWrite
};

class CallbacksToUpdate : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        int amountOfColors = String(characteristic_amountOfColors->getValue().c_str()).toInt();
        uint8_t positions[amountOfColors];
        uint8_t red[amountOfColors];
        uint8_t green[amountOfColors];
        uint8_t blue[amountOfColors];
        String positionsString = characteristic_positions->getValue().c_str();
        String redString = characteristic_red->getValue().c_str();
        String greenString = characteristic_green->getValue().c_str();
        String blueString = characteristic_blue->getValue().c_str();
        Serial.println(positionsString);
        Serial.println(redString);
        Serial.println(greenString);
        Serial.println(blueString);
        if (amountOfColors < 2 || amountOfColors > 16){
            characteristic_msgErrorLeds->setValue("error= -181");
            characteristic_msgErrorLeds->notify();
            return;}
        if (stringToArray(positionsString, positions, amountOfColors) < 0){
            characteristic_msgErrorLeds->setValue("error= -182");
            characteristic_msgErrorLeds->notify();
            return;}
        if (stringToArray(redString, red, amountOfColors) < 0){
            characteristic_msgErrorLeds->setValue("error= -183");
            characteristic_msgErrorLeds->notify();
            return;}
        if (stringToArray(greenString, green, amountOfColors) < 0){
            characteristic_msgErrorLeds->setValue("error= -184");
            characteristic_msgErrorLeds->notify();
            return;}
        if (stringToArray(blueString, blue, amountOfColors) < 0){
            characteristic_msgErrorLeds->setValue("error= -185");
            characteristic_msgErrorLeds->notify();
            return;}
        romSetCustomPallete(positions, red, green, blue, amountOfColors);
        characteristic_msgErrorLeds->setValue("ok");
        characteristic_msgErrorLeds->notify();
        Serial.println("Se actualizo custom Pallete");
    } //onWrite
};

class genericCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        //retorna ponteiro para o registrador contendo o valor atual da caracteristica
        std::string rxValue = characteristic->getValue();
        std::string uuid = characteristic->getUUID().toString();
        String value = rxValue.c_str();
        double valueDouble = value.toDouble();
        for (int i = 0; i < uuid.length(); i++)
        {
            Serial.print(uuid[i]);
        }
        //verifica se existe dados (tamanho maior que zero)
        Serial.print(" Value was changed to: ");
        for (int i = 0; i < rxValue.length(); i++)
        {
            Serial.print(rxValue[i]);
        }
        Serial.println();
    } //onWrite
};






void setup()
{
    delay(1000); // power-up safety delay
    //====Serial configuration====
    Serial.begin(115200);
    //====
    //====SD initialization====
    while(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
    {
        //changePalette(CODE_NOSD_PALLETE);
        #ifdef DEBUGGING_DATA
            Serial.println("Card failed, or not present");
        #endif
        delay(200); 
    }
    //changePalette(ledModeGlobal);
    findUpdate();
    //====Palletes initialization====
    NO_SD_PALLETE= breathRed;
    UPTADATING_PALLETE = breathYellow;
    CALIBRATING_PALLETE = breathBlue;
    SDEMPTY_PALLETE = breathOrange;
    rgb2Interpolation(pallette1, palletteColors1);
    rgb2Interpolation(pallette2, palletteColors2);
    rgb2Interpolation(pallette3, palletteColors3);
    rgb2Interpolation(pallette4, palletteColors4);
    rgb2Interpolation(pallette5, palletteColors5);
    rgb2Interpolation(pallette6, palletteColors6);
    rgb2Interpolation(pallette7, palletteColors7);
    rgb2Interpolation(pallette8, palletteColors8);
    rgb2Interpolation(pallette9, palletteColors9);
    rgb2Interpolation(pallette10, palletteColors10);
    rgb2Interpolation(pallette11, palletteColors11);
    rgb2Interpolation(pallette12, palletteColors12);
    rgb2Interpolation(pallette13, palletteColors13);
    rgb2Interpolation(pallette14, palletteColors14);
    rgb2Interpolation(pallette15, palletteColors15);

    //====
    //====EEPROM Initialization====
    EEPROM.begin(EEPROM_SIZE);
    delay(500);
	//====Testing====
	int dat_pin;
	dat_pin = analogRead(PIN_ProducType);
	if(dat_pin > 682 && dat_pin < 2047)
	{
		haloTest.Test();	
	}
    if(dat_pin > 2047 && dat_pin < 3413)
	{
		for(int x = 0; x < 512; x++)
        {
            EEPROM.write(x,-1);
            EEPROM.commit();
            delay(20);
        }	
	}
    //====restore the value intermediateCalibration====
    intermediateCalibration = romGetIntermediateCalibration();
    //====
    //====Restore speedMotor====
    speedMotorGlobal = romGetSpeedMotor();
    if (speedMotorGlobal > MAX_SPEED_MOTOR || speedMotorGlobal < MIN_SPEED_MOTOR){
        speedMotorGlobal = SPEED_MOTOR_DEFAULT;
        romSetSpeedMotor(SPEED_MOTOR_DEFAULT);
    }
    Sandsara.setSpeed(speedMotorGlobal);
    //====
    //====Restoring of refreshing time of leds====
    periodLedsGlobal = romGetPeriodLed();
    if (periodLedsGlobal > MAX_PERIOD_LED || periodLedsGlobal < MIN_PERIOD_LED){
        periodLedsGlobal = PERIOD_LED_DEFAULT;
        romSetPeriodLed(PERIOD_LED_DEFAULT);
    }
    //====
    //====Restoring of the latest pallete choosed====
    ledModeGlobal = romGetPallete();
    if (ledModeGlobal > MAX_PALLETE || ledModeGlobal < MIN_PALLETE){
        ledModeGlobal = PALLETE_DEFAULT;
        romSetPallete(PALLETE_DEFAULT);
    }
    changePalette(ledModeGlobal);
    //====
    
    //====Restoring bluetooth name====
    bluetoothNameGlobal = romGetBluetoothName();
    //====
    //====Configure the halo and bluetooth====
    Sandsara.init();
    //====
    //====Select type of product====
    if (analogRead(PIN_ProducType) < 1000){
        productType = false;
        NUM_LEDS = LEDS_OF_HALO;
    }
    else{
        productType = true;
        NUM_LEDS = LEDS_OF_STELLE;
    }
    //====
    
    delay(1000);
    //====Restore leds direction====
    ledsDirection = romGetLedsDirection();
    //====new task for leds====
    xTaskCreatePinnedToCore(
                    ledsFunc,
                    "Task1",
                    5000,
                    NULL,
                    5,
                    &Task1,
                    1);
    delay(500); 
    //====
    BLEDevice::init("Sandsara BLE");
    BLEServer *pServer = BLEDevice::createServer();    
    BLEService *pServiceLedConfig = pServer->createService(BLEUUID(SERVICE_UUID1), 30);
    BLEService *pServicePath = pServer->createService(BLEUUID(SERVICE_UUID2), 30);
    BLEService *pServiceSphere = pServer->createService(BLEUUID(SERVICE_UUID3), 30);
    BLEService *pServicePlaylist = pServer->createService(BLEUUID(SERVICE_UUID4), 30);
    BLEService *pServiceGeneralConfig = pServer->createService(BLEUUID(SERVICE_UUID5), 30);
    BLEService *pService1 = pServer->createService(BLEUUID(SERVICE_UUID6), 30);

    //====Characteristics for LEDs configuration====
    
    characteristic_selectedPaletteIndex = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_SELECTEDPALETTE,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_ledSpeed = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_LEDSPEED,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_cycleMode = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_CYCLEMODE,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_direction = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_DIRECTION,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_amountOfColors = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_AMOUNTCOLORS,
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_positions = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_POSITIONS,
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_red = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_RED,
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_green = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_GREEN,
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_blue = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_BLUE,
            BLECharacteristic::PROPERTY_WRITE);
    characteristic_updateCustomPalette = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_UPDATECPALETTE,
        BLECharacteristic::PROPERTY_WRITE);
    characteristic_msgErrorLeds = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_MSGERRORLEDS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    characteristic_msgErrorLeds->addDescriptor(new BLE2902());

    //====Characteristics for path configuration====
    characteristic_1 = pServicePath->createCharacteristic(
        CHARACTERISTIC_UUID_1,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    //====Characteristics for Sphere configuration====
    characteristic_3 = pServiceSphere->createCharacteristic(
        CHARACTERISTIC_UUID_3,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    //====Characteristics for Playlist configuration====
    characteristic_4 = pServicePlaylist->createCharacteristic(
        CHARACTERISTIC_UUID_4,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    //====Characteristics for General configuration====
    characteristic_5 = pServiceGeneralConfig->createCharacteristic(
        CHARACTERISTIC_UUID_5,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    //====Characteristics for 1 configuration====
    characteristic_6 = pService1->createCharacteristic(
        CHARACTERISTIC_UUID_6,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);
            
    //characteristic_ledSpeed->addDescriptor(new BLE2902());

    characteristic_ledSpeed->setCallbacks(new speedLedCallbacks());
    characteristic_cycleMode->setCallbacks(new cycleModeCallbacks());
    characteristic_direction->setCallbacks(new directionCallbacks());
    characteristic_amountOfColors->setCallbacks(new genericCallbacks());
    characteristic_positions->setCallbacks(new genericCallbacks);
    characteristic_red->setCallbacks(new genericCallbacks);
    characteristic_green->setCallbacks(new genericCallbacks);
    characteristic_blue->setCallbacks(new genericCallbacks);
    characteristic_updateCustomPalette->setCallbacks(new CallbacksToUpdate());
    //characteristic_msgErrorLeds->setCallbacks(new msgErrorLedCallbacks());
    characteristic_selectedPaletteIndex->setCallbacks(new selectedPaletteCallbacks());

    characteristic_ledSpeed->setValue(String(periodLedsGlobal).c_str());
    if(romGetIncrementIndexPallete()){
        characteristic_cycleMode->setValue("1");}
    else{
        characteristic_cycleMode->setValue("0");}
    if(romGetLedsDirection()){
        characteristic_direction->setValue("1");}
    else{
        characteristic_direction->setValue("0");}
    characteristic_selectedPaletteIndex->setValue(String(romGetPallete()).c_str());

    pServiceLedConfig->start();
    pServicePath->start();
    pServiceSphere->start();
    pServicePlaylist->start();
    pServiceGeneralConfig->start();
    pService1->start();
    //pService3->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID1);
    pAdvertising->addServiceUUID(SERVICE_UUID2);
    pAdvertising->addServiceUUID(SERVICE_UUID3);
    pAdvertising->addServiceUUID(SERVICE_UUID4);
    pAdvertising->addServiceUUID(SERVICE_UUID5);
    pAdvertising->addServiceUUID(SERVICE_UUID6);
    //pAdvertising->addServiceUUID(SERVICE_UUID8);

    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
    //====new task for Bluetooth====
    /*xTaskCreatePinnedToCore(
                    bluetoothThread,   
                    "Task2",     
                    5000,
                    NULL,
                    4,
                    &Task2,
                    0);
    delay(500); */
    //====
    //====new task for Motors====
    xTaskCreatePinnedToCore(
                    moveSteps,   
                    "motorsTasks",     
                    5000,
                    NULL,
                    4,
                    &motorsTask,
                    0);
    delay(500); 
    //====
    //====Cablibrating====
    
    haloCalib.init();
    #ifdef DEBUGGING_DATA
        Serial.println("Calibrando...");
    #endif
    haloCalib.start();
    #ifdef DEBUGGING_DATA
        Serial.println("calibrado");
    #endif
    //====After calibration current has to be set up====
    driver.rms_current(NORMAL_CURRENT);
    driver2.rms_current(NORMAL_CURRENT);
    pinMode(EN_PIN, OUTPUT);
    pinMode(EN_PIN2, OUTPUT);
    digitalWrite(EN_PIN, LOW);
    digitalWrite(EN_PIN2, LOW);
    //====
    
    #ifdef DEBUGGING_DATA
        Serial.println("Card present");
    #endif
    //====
    //====Restore playlist name and orderMode====    
    playListGlobal = romGetPlaylist();
    orderModeGlobal = romGetOrderMode();
    if (orderModeGlobal < MIN_REPRODUCTION_MODE || orderModeGlobal > MAX_REPRODUCTION_MODE){
        orderModeGlobal = 3;
        romSetOrderMode(3);
    }
    changePalette(romGetPallete());
    if (playListGlobal.equals("/")){
        #ifdef DEBUGGING_DATA
            Serial.print("No hay una playlist guardada, se reproduciran todos los archivos en la sd");
        #endif
    }
    else
    {
        if (sdExists(playListGlobal)){
            delay(1);
            #ifdef DEBUGGING_DATA
                Serial.print("lista guardada: ");
                Serial.println(playListGlobal);
            #endif
        }
        else{
            #ifdef DEBUGGING_DATA
                Serial.println("La playlist no existe, se reproduciran todos los archivos en la sd");
            #endif
        }
    }
    #ifdef DEBUGGING_DATA
        Serial.print("orderMode guardado: ");
        Serial.println(orderModeGlobal);
    #endif
    //====
    //====Searching for an update====
    //findUpdate();
    //====
    #ifdef PROCESSING_SIMULATOR
        Serial.println("inicia");
    #endif
    
    
    //=====if the file playlist.playlist exits it will be executed====
    if (sdExists("/playlist.playlist")){
        playListGlobal = "/playlist.playlist";
        orderModeGlobal = 1;
    }
    //=====
    goEdgeSpiral(false);
    delay(1000);
    firstExecution = true;
    
}

void loop()
{
    #ifdef DEBUGGING_DATA
        Serial.println("Iniciara la funcion runSansara");
    #endif
    errorCode = run_sandsara(playListGlobal, orderModeGlobal);
    #ifdef DEBUGGING_DATA
        Serial.print("errorCode de run: ");
        Serial.println(errorCode);
    #endif
    if (errorCode == -10){
        Sandsara.completePath();
        while(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
        {
            changePalette(CODE_NOSD_PALLETE);
            #ifdef DEBUGGING_DATA
                Serial.println("Card failed, or not present");
            #endif
            delay(200); 
        }
        changePalette(ledModeGlobal);
        #ifdef DEBUGGING_DATA
            Serial.println("Card present");
        #endif
        

    }
    else if (errorCode == -4){
        changePalette(CODE_SDEMPTY_PALLETE);
        SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD);
    }
    firstExecution = false;
    delay(1000);
    
}

/**
 * @brief
 * @param playlist the name of playlist to be executed, e.g. "/animales.playlist"
 * @param orderMode the orden that files will be executed
 * 1, files will be reproduced in descending order according to playlist.
 * 2, files will be reproduced in random order according to playlist.
 * 3, All files in SD will be reproduced in a determined order.
 * 4, All files in SD will be reproduced in random order.
 * @return an error code that could be one below.
 *  1, playlist or orderMode were changed.
 *  0, ends without error.
 * -1, playlist was not found.
 * -2, playlist is a directory.
 * -3, an attempt to choose an incorrect position in playlist was performed.
 * -4, number of files in playlist or SD are zero.
 * -10, SD was not found.
 */
int run_sandsara(String playList, int orderMode)
{
    if (firstExecution){
        pListFileGlobal = romGetPositionList();
    }
    else{
        pListFileGlobal = 1;
    }
    int numberOfFiles;
    String fileName;
    
    if (orderMode == 1 || orderMode == 2){
        File file;
        file = SD.open(playList);
        if (file && !file.isDirectory()){
            file.close();
            numberOfFiles = SdFiles::numberOfLines(playList);
            if (numberOfFiles < 0){
                return -4;
            }
            if (orderMode == 2){
                orderRandom(playList,numberOfFiles);
                playList = "/RANDOM.playlist";
                pListFileGlobal = 1;
            }
        }
        else
        {
            file.close();
            orderMode = 3;
            numberOfFiles = SdFiles::creatListOfFiles("/DEFAULT.playlist");
            if (numberOfFiles < 0){
                return -4;
            }
            playList = "/DEFAULT.playlist";
        }
    }
    else if (orderMode == 4)
    {
        numberOfFiles = SdFiles::creatListOfFiles("/auxList32132123.playlist");
        if (numberOfFiles < 0){
            return -4;
        }
        playList = "/auxList32132123.playlist";
        orderRandom(playList,numberOfFiles);
        playList = "/RANDOM.playlist";
        pListFileGlobal = 1;
    }
    else
    {
        numberOfFiles = SdFiles::creatListOfFiles("/DEFAULT.playlist");
        if (numberOfFiles < 0){
            return -4;
        }
        playList = "/DEFAULT.playlist";
    }
    #ifdef DEBUGGING_DATA
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    #endif

    if (numberOfFiles == 0){
        return -4;
    }
    
    changePalette(romGetPallete());
    //====restore playlist====
    currentPlaylistGlobal = playList;
    //====
    while (true)
    {
        //====Save the current position in playlist====
        romSetPositionList(pListFileGlobal);
        //====
        #ifdef DEBUGGING_DATA
            Serial.println("Abrira el siguiente archivo disponible");
        #endif
        errorCode = SdFiles::getLineNumber(pListFileGlobal, playList, fileName);
        if (errorCode < 0)
        {
            //====playList is not valid====
            delay(1000);
            return errorCode;
        }
        errorCode = SdFiles::getLineNumber(pListFileGlobal + 1, playList, nextProgramGlobal);
        if (errorCode < 0)
        {
            nextProgramGlobal = "none";
        }
        if (errorCode == 2 || errorCode == 3)
        {
            delay(1000);
            break;
        }
        while (readingSDFile){
            delay(1);
        }
        readingSDFile = true;
        File current_file = SD.open("/" + fileName);
        if (!current_file)
        {
            delay(1000);
            pListFileGlobal += 1;
            continue;
        }
        if (current_file.isDirectory())
        {
            delay(1000);
            pListFileGlobal += 1;
            continue;
        }
        char nameF[NAME_LENGTH];
        current_file.getName(nameF,NAME_LENGTH);
        fileName = nameF;
        current_file.close();
        readingSDFile = false;
        //====run program====
        errorCode = runFile(fileName);
        //====
        if (errorCode == -70)   {continue;}
        else if (errorCode == -71)   {break;}
        else if (errorCode == 20)    {/*pListFileGlobal = SandsaraBt.getPositionList();*/ continue;}
        else if (errorCode == 30)    {
            while(errorCode == 30){
                //fileName = SandsaraBt.getProgram();
                errorCode = runFile(fileName);
            }
            if (errorCode == 20)    {/*pListFileGlobal = SandsaraBt.getPositionList();*/ continue;}
            pListFileGlobal += 1;
            continue;
        }
        else if (errorCode == 40){
            while (suspensionModeGlobal)
            {
                ledsOffGlobal = true;
                delay(500);
            }
            ledsOffGlobal = false;
            continue;
        }
        else if (errorCode != 10)    {return errorCode;}
        //====Increment pListFileGlobal====
        pListFileGlobal += 1;
        //====
    }
    return 0;
}
/**
 * @brief execute a path
 * @param dirFile, path direction in SD.
 * @return an errorCode
 * -70, instruction continue has to be performed.
 * -71, instruction break has to be performed.
 *   1, orderMode or playlist have been changed.
 *  10, the function finished without errors.
 *  20, an instruction for changing path by postion was recieved.
 *  30, an instruction for changing path by name was recieved.
 *  40, an instruction for suspention was recieved.
 */
int runFile(String fileName){
    double component_1, component_2, distance;
    int working_status = 0;
    //====restore the path name and its position in the playlist.
    currentProgramGlobal = fileName;
    currentPositionListGlobal = pListFileGlobal;
    //====
    double couplingAngle, startFileAngle, endFileAngle;
    int posisionCase;
    SdFiles file(fileName);
    if (file.fileType < 0){
        pListFileGlobal += 1;
        return -70;
    }

    double zInit = Sandsara.getCurrentModule();
    //====read of File mode is selected.
    file.autoSetMode(zInit);
    //====
    if (!file.isValid()){
        pListFileGlobal += 1;
        return -70;
    }
    #ifdef PROCESSING_SIMULATOR
        Serial.print("fileName: ");
        Serial.println(fileName);
    #endif
    /*Serial.print("fileName: ");
    Serial.println(fileName);*/
    startFileAngle = file.getStartAngle();
    startFileAngle = Motors::normalizeAngle(startFileAngle);
    endFileAngle = file.getFinalAngle();
    endFileAngle = Motors::normalizeAngle(endFileAngle);
    
    posisionCase = Sandsara.position();
    if (posisionCase == 2){
        couplingAngle = startFileAngle;
    }
    else if (posisionCase == 0){
        couplingAngle = endFileAngle;
    }
    else{
        couplingAngle = endFileAngle;
    }
    if (true){
        double zf, thetaf, zi, thetai, thetaFinal;
        thetaf = Motors::normalizeAngle(file.getStartAngle() - couplingAngle);
        zf = file.getStartModule();
        thetai = Sandsara.getCurrentAngle();
        zi = Sandsara.getCurrentModule();
        Sandsara.setZCurrent(zi);
        Sandsara.setThetaCurrent(thetai);
        if (thetai > thetaf){
            if (thetai - thetaf > PI){
                thetaFinal = thetai + (2*PI - (thetai - thetaf));
            }
            else{
                thetaFinal = thetaf;
            }
        }
        else{
            if (thetaf - thetai> PI){
                thetaFinal = thetai - (2*PI - (thetaf - thetai));
            }
            else{
                thetaFinal = thetaf;
            }
        }
        errorCode = movePolarTo(zf, thetaFinal, 0);
        if (errorCode != 0){
            return errorCode;
        }
    }
    //if file is thr, the start point is save in order to know where is located.
    if (file.fileType == 2)
    {
        working_status = file.getNextComponents(&component_1, &component_2);
        while (working_status == 3)
        {
            working_status = file.getNextComponents(&component_1, &component_2);
        }
        Sandsara.setZCurrent(component_1);
        Sandsara.setThetaCurrent(component_2 - couplingAngle);
    }
    
    //the next while is stopped until file is finished or is interrupted.
    while (true)
    {
        //Serial.println("ejecutara nueva posicion");
        //====check if speed change====
        
        //====check if you want to change the path====
        if (changePositionList){
            changePositionList = false;
            changeProgram = false;
            goHomeSpiral();
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            if (changeProgram){
                return 30;
            }
            else{
                return 20;
            }
        }
        if (changeProgram){
            changePositionList = false;
            changeProgram = false;
            goHomeSpiral();
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            if (changePositionList){
                return 20;
            }
            else{
                return 30;
            }
        }
        //====check for suspention or pause====
        if (pauseModeGlobal == true){
            while (pauseModeGlobal)
            {
                delay(200);
            }
        }
        if (suspensionModeGlobal == true){
            goHomeSpiral();
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            return 40;
        }
        //====Check if you want to change playlist or Orden mode====
        if (rewindPlaylist){
            goHomeSpiral();
            rewindPlaylist = false;
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            return 1;
        }
        //====
        //====Get new components of the next point to go====
        //Serial.println("antes de next components");
        working_status = file.getNextComponents(&component_1, &component_2);                
        if (working_status == 3)
        {
            continue;
        }
        if (working_status != 0)
        {
            break;
        }
        //====According to type of file the functions moveTo or movePolarTo will be executed====
        if (file.fileType == 1 || file.fileType == 3)
        {
            if (speedChangedMain){
                Sandsara.resetSpeeds();
                speedChangedMain = false;
            }
            Motors::rotate(component_1, component_2, -couplingAngle);
            distance = Sandsara.module(component_1, component_2, Sandsara.x_current, Sandsara.y_current);
            if (distance > 1.1)
            {
                errorCode = moveInterpolateTo(component_1, component_2, distance);
                if (errorCode != 0){
                    return errorCode;
                }
            }
            else
            {
                Sandsara.moveTo(component_1, component_2);
            }
        }
        else if (file.fileType == 2)
        {
            errorCode = movePolarTo(component_1, component_2, couplingAngle, true);
            if (errorCode != 0){
                return errorCode;
            }
        }
        else
        {
            break;
        }
        //Serial.println("saliendo de mover");
    }
    
    //====update z and theta current====
    Sandsara.setZCurrent(Sandsara.getCurrentModule());
    if (Sandsara.getCurrentAngle() > PI){
        Sandsara.setThetaCurrent(Sandsara.getCurrentAngle() - 2*PI);
    }
    else{
        Sandsara.setThetaCurrent(Sandsara.getCurrentAngle());
    }
    //====
    //====check if there was a SD problem====
    if (working_status == -10)
    {
        #ifdef DEBUGGING_DATA
            Serial.println("There were problems for reading SD");
            Serial.println("Se mandara a cero");
        #endif
        movePolarTo(0, 0, 0, true);
        #ifdef PROCESSING_SIMULATOR
            Serial.println("finished");
        #endif
        return -10;
    }
    //====
    Sandsara.completePath();
    //====
    posisionCase = Sandsara.position();
    if (posisionCase == 2){
        movePolarTo(DISTANCIA_MAX, 0, 0, true);
    }
    else if (posisionCase == 0){
        #ifdef DEBUGGING_DATA
            Serial.println("Se mandara a cero");
        #endif
        movePolarTo(0, 0, 0, true);
		if (intermediateCalibration == true)
		{
            #ifdef DEBUGGING_DATA
                Serial.println("se realizara la calibracion intermedia");
            #endif
			haloCalib.verificacion_cal();
		}
	}
    #ifdef PROCESSING_SIMULATOR
        Serial.println("finished");
    #endif
    delay(1000);
    return 10;
}

/**
 * @brief returns to home (0,0) on a spiral path.
 * 
 */
void goHomeSpiral(){
    //delay(200);
    Sandsara.stopAndResetPositions();
    float currentModule = Sandsara.getCurrentModule();
    availableDeceleration = true;
    if (currentModule < DISTANCIA_MAX / 2){//sqrt(2)){
        goCenterSpiral(false);
    }
    else
    {
        goEdgeSpiral(false);
    }
    availableDeceleration = false;
    delay(1000); 
}

/**
 * @brief returns to home (0,0) on a spiral path.
 * 
 */
void goCenterSpiral(bool stop){
    Sandsara.setSpeed(SPEED_TO_CENTER);
    stopProgramChangeGlobal = stop;
    spiralGoTo(0,PI/2);
    stopProgramChangeGlobal = true;
    Sandsara.setSpeed(romGetSpeedMotor());
    Sandsara.completePath();
}

/**
 * @brief returns to the outer end (DISTANCIA_MAX,0) on a spiral path.
 * 
 */
void goEdgeSpiral(bool stop){
    Sandsara.setSpeed(SPEED_TO_CENTER);
    stopProgramChangeGlobal = stop;
    spiralGoTo(DISTANCIA_MAX,0);
    stopProgramChangeGlobal = true;
    Sandsara.setSpeed(romGetSpeedMotor());
    Sandsara.completePath();
}

/**
 * @brief move to a certain position in spiral path.
 */
void spiralGoTo(float module, float angle){
    float degreesToRotate;
    Sandsara.setZCurrent(Sandsara.getCurrentModule());
    degreesToRotate = int((Sandsara.getCurrentModule() - module)/EVERY_MILIMITERS) * 2*PI;
    Sandsara.setThetaCurrent(Sandsara.getCurrentAngle() + degreesToRotate);
    movePolarTo(module, angle, 0, true);
}

//=========================================================
/**
 * @brief this function is used to go from one point to another in a straight line path formed by equidistant points of 1 mm.
 * @param x X axis coordinate of the target point, measured in milimeters.
 * @param y Y axis coordinate of the target point, measured in milimeters.
 * @param distance is the distance between the current point and the target point.
 * @return an error code relating to bluetooth, this could be.
 * 1, playlist has changed.
 * 2, orderMode has changed.
 * 0, finished.
 */
int moveInterpolateTo(double x, double y, double distance)
{
    double alpha = atan2(y - Sandsara.y_current, x - Sandsara.x_current);
    double delta_x, delta_y;
    double x_aux = Sandsara.x_current, y_aux = Sandsara.y_current;
    delta_x = cos(alpha);
    delta_y = sin(alpha);
    int intervals = distance;
	
    for (int i = 1; i <= intervals; i++)
    {
        if (speedChangedMain){
            Sandsara.resetSpeeds();
            speedChangedMain = false;
        }
        //====check if this function has to stop because of program has change, pause or stop. or playlist has changed====
        if ((changePositionList || changeProgram || suspensionModeGlobal || rewindPlaylist) && stopProgramChangeGlobal){
            return 0;
        }
        //====
        x_aux += delta_x;
        y_aux += delta_y;
        Sandsara.moveTo(x_aux, y_aux);
    }
    Sandsara.moveTo(x, y);
    return 0;
}

/**
 * @brief Interpola los puntos necesarios entre el punto actual y el siguiente con el objetivo que
 * se mueva en coordenadas polares como lo hace sisyphus.
 * @param component_1 valor en el eje z polar, medido en milimetros.
 * @param component_2 valor en el eje theta polar, medido en radianes.
 * @param couplingAngle es el angulo que se va a rotar el punto con coordenadas polares component_1, component_2 (modulo y angulo, respectivamente).
 * @note es muy importante que se hayan definido las variables zCurrent y thetaCurrent antes.
 * de llamar a esta funcion porque es apartir de estas variables que se calcula cuanto se debe mover.
 * @return un codigo de error:
 * 0, no se recibio nada por bluetooth
 * 1, se cambio la playlist
 * 2, se cambio el orderMode
 */
int movePolarTo(double component_1, double component_2, double couplingAngle, bool littleMovement){
    //Serial.println("dentro de movePolar");
    double zNext = component_1;
    double thetaNext = component_2 - couplingAngle;
    double thetaCurrent = Sandsara.getThetaCurrent();
    double zCurrent = Sandsara.getZCurrent();
    double slicesFactor, distance;
    long slices;
    double deltaTheta, deltaZ;
    double thetaAuxiliar, zAuxliar, xAux, yAux;
    deltaTheta = thetaNext - thetaCurrent;
    deltaZ = zNext - zCurrent;
    slicesFactor = Sandsara.arcLength(deltaZ, deltaTheta, zCurrent);
    slices = slicesFactor;
    if (slices < 1)
    {
        slices = 1;
    }
    deltaTheta = (thetaNext - thetaCurrent) / slices;
    deltaZ = (zNext - zCurrent) / slices;
    for (long i = 0; i < slices; i++)
    {
        if (speedChangedMain){
            Sandsara.resetSpeeds();
            speedChangedMain = false;
        }
        ///====comprobar si se desea cambiar de archivo o suspender o cambiar playlist u orden====
        if ((changePositionList || changeProgram || suspensionModeGlobal || rewindPlaylist) && stopProgramChangeGlobal){
            return 0;
        }
        thetaAuxiliar = thetaCurrent + deltaTheta * double(i);
        zAuxliar = zCurrent + deltaZ * double(i);
        xAux = zAuxliar * cos(thetaAuxiliar);
        yAux = zAuxliar * sin(thetaAuxiliar);
        distance = Sandsara.module(xAux, yAux, Sandsara.x_current, Sandsara.y_current);
        if (distance > 1.1)
        {
            errorCode = moveInterpolateTo(xAux, yAux, distance);
            if (errorCode != 0){
                return errorCode;
            }
        }
        else{
            Sandsara.moveTo(xAux, yAux, littleMovement);
        }
    }
    xAux = zNext * cos(thetaNext);
    yAux = zNext * sin(thetaNext);

    Sandsara.moveTo(xAux, yAux, littleMovement);
    Sandsara.setThetaCurrent(thetaNext);
    Sandsara.setZCurrent(zNext);
    return 0;
}
/**
 * @brief Este es una tarea en paralelo que revisa si hay algun mensaje por bluetooth.
 */
/*void bluetoothThread(void * pvParameters ){
    for(;;){
        errorCode = SandsaraBt.checkBlueTooth();
        executeCode(errorCode);
        vTaskDelay(100);
    }
}*/

/**
 * @brief Ejecuta los codigos que regresa la funcion checkBluetooth()
 */
/*void executeCode(int errorCode){
    if (errorCode == 10){
        playListGlobal = "/" + SandsaraBt.getPlaylist();
        romSetPlaylist(playListGlobal);
    }
    else if (errorCode == 20){
        orderModeGlobal = SandsaraBt.getOrderMode();
        romSetOrderMode(orderModeGlobal);
    }
    else if (errorCode == 30){
        ledModeGlobal = SandsaraBt.getLedMode();
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
    }
    else if (errorCode == 50){
        int speed = SandsaraBt.getSpeed();
        Sandsara.setSpeed(speed);
        romSetSpeedMotor(speed);
        speedChangedMain = true;
    }
    else if (errorCode == 60){
        int periodLed = SandsaraBt.getPeriodLed();
        periodLedsGlobal = periodLed;
        delayLeds = periodLed;
        romSetPeriodLed(periodLedsGlobal);
    }
    else if (errorCode == 70){
        String blueName = SandsaraBt.getBluetoothName();
        bluetoothNameGlobal = blueName;
        romSetBluetoothName(bluetoothNameGlobal);
    }
    else if (errorCode == 80){
        suspensionModeGlobal = true;
        pauseModeGlobal = false;
        #ifdef DEBUGGING_DATA
            Serial.println("Entro a modo suspencion");
        #endif
    }
    else if (errorCode == 90){
        pauseModeGlobal = true;
        #ifdef DEBUGGING_DATA
            Serial.println("Entro en modo pausa");
        #endif
    }
    else if (errorCode == 100){
        pauseModeGlobal = false;
        suspensionModeGlobal = false;
        #ifdef DEBUGGING_DATA
            Serial.println("Reanudado");
        #endif
    }
    else if (errorCode == 130){
        SandsaraBt.writeBt("currentProgram= ");
        SandsaraBt.writeBtln(currentProgramGlobal);
    }
    else if (errorCode == 140){
        SandsaraBt.writeBt("nextProgram= ");
        SandsaraBt.writeBtln(nextProgramGlobal);
    }
    else if (errorCode == 150){
        String fileName;
        int i = 1, errorCode;
        SandsaraBt.writeBt("playlist: ");
        SandsaraBt.writeBtln(currentPlaylistGlobal);
        for (;;){
            errorCode = SdFiles::getLineNumber(i,currentPlaylistGlobal, fileName);
            if (errorCode < 0){
                SandsaraBt.writeBtln("error");
                break;
            }
            if (errorCode > 0){
                if (!fileName.equals("")){
                    SandsaraBt.writeBt(String(i) + ": ");
                    SandsaraBt.writeBtln(fileName);
                }
                break;
            }
            SandsaraBt.writeBt(String(i) + ": ");
            SandsaraBt.writeBtln(fileName);
            i++;
        }
        SandsaraBt.writeBtln("ok");
    }
    else if (errorCode == 160){
        changePositionList = true;
        changeProgram = false;
        #ifdef DEBUGGING_DATA
            Serial.println("cambio de programa por su posicion en la lista");
        #endif
    }
    else if (errorCode == 170){
        changeProgram = true;
        changePositionList = false;
        #ifdef DEBUGGING_DATA
            Serial.println("cambio de programa por su nombre en la memoria SD");
        #endif
    }
    else if (errorCode == 190){
        incrementIndexGlobal = romGetIncrementIndexPallete();
    }
    else if (errorCode == 200){
        intermediateCalibration = romGetIntermediateCalibration();
    }
    else if(errorCode == 210){
        rewindPlaylist = true;
    }
    else if(errorCode == 220){
        ledsDirection = romGetLedsDirection();
    }
    else if (errorCode == 970){
        for (int i = 0; i < 512; i++){
            EEPROM.write(i, -1);
        }
        EEPROM.commit();
        delay(1000);
        rebootWithMessage("Se hiso reset de fabrica, Reiniciando...");
    }
}*/

//====ROM functions Section====

/**
 * @brief actualiza el valor, en la ROM/FLASH, el nombre de la lista de reproducción.
 * @param str es la dirección en la SD de la lista de reproducción, ejemplo "/animales.playlist".
 * @note únicamente guarda el nombre, ignorando '/' y ".playlist", por ejemplo, si str es "/animales.playlist" solo guarda "animales".
 *pero la funcion romGetPlaylist() la devuelve como "/animales.playList" .
 */
int romSetPlaylist(String str){
    if (str.charAt(0) == '/')
    {
        str.remove(0,1);
    }
    if (str.indexOf(".playlist") >= 0)
    {
        str.remove(str.indexOf(".playlist"));
    }
    if (str.length() > MAX_CHARS_PLAYLIST){
        return -1;
    }
    int i = 0;
    for ( ; i < str.length(); i++){
        EEPROM.write(ADDRESSPLAYLIST + i, str.charAt(i));
    }
    EEPROM.write(ADDRESSPLAYLIST + i, '\0');
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera el nombre de la playlist guardada en ROM
 * @return la direccion del archivo, ejemplo "/animales.playlist"
 * @note regresa siempre un nombre con terminacion ".playlist"
 */
String romGetPlaylist(){
    String str = "/";
    char chr;
    for (int i = 0; i < MAX_CHARS_PLAYLIST; i++){
        chr = EEPROM.read(ADDRESSPLAYLIST + i);
        if (chr == '\0')
        {
            str.concat(".playlist");
            return str;
        }
        str.concat( chr );
    }
    return "/";
}

/**
 * @brief guarda el tipo de orden de reporduccion en la memoria ROM
 * @param orderMode el valor que corresponde al orden de reproduccion que va a ser almacenado en ROM.
 * @return 0
 */
int romSetOrderMode(int orderMode){
    uint8_t Mode = orderMode;
    EEPROM.write(ADDRESSORDERMODE, Mode);
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera el valor correspondiente al tipo de reporduccion guardado en la memoria ROM.
 * @return el valor correspondiente al tipo de orden de reporduccion guardado en la ROM.
 */
int romGetOrderMode(){
    uint8_t orderMode;
    orderMode = EEPROM.read(ADDRESSORDERMODE);
    return orderMode;
}

/**
 * @brief guarda la paleta de colores en rom
 * @param pallete es un numero entero que indica la paleta de colres actual
 * @return 0
 */
int romSetPallete(int pallete){
    uint8_t* p = (uint8_t* ) &pallete;
    for (int i = 0; i < sizeof(pallete); i++){
        EEPROM.write(ADDRESSPALLETE + i, *(p + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera, de la ROM, la ultima palleta de colores guardada.
 * @return un numero entero que indaca una paleta de colores. 
 */
int romGetPallete(){
    int pallete;
    uint8_t* p = (uint8_t* ) &pallete;
    for (int i = 0; i < sizeof(pallete); i++){
        *(p + i) = EEPROM.read(ADDRESSPALLETE + i);
    }
    return pallete;
}

/**
 * @brief guarda la velocidad de Sandsara en la ROM
 * @param speed es la velocidad en milimetros por segundo de Sandsara.
 * @return 0
 */
int romSetSpeedMotor(int speed){
    uint8_t* p = (uint8_t* ) &speed;
    for (int i = 0; i < sizeof(speed); i++){
        EEPROM.write(ADDRESSSPEEDMOTOR + i, *(p + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera, de la ROM, la velocidad de Sandsara.
 * @return un numero entero que indaca la velocidad de Sandsara, medida en milimetros por segundo. 
 */
int romGetSpeedMotor(){
    int speed;
    uint8_t* p = (uint8_t* ) &speed;
    for (int i = 0; i < sizeof(speed); i++){
        *(p + i) = EEPROM.read(ADDRESSSPEEDMOTOR + i);
    }
    return speed;
}

/**
 * @brief guarda el tiempo de refresco de los leds en ROM.
 * @param periodLed es el tiempo de refresco de los leds, medido en milisegundos.
 * @return 0
 */
int romSetPeriodLed(int periodLed){
    uint8_t* p = (uint8_t* ) &periodLed;
    for (int i = 0; i < sizeof(periodLed); i++){
        EEPROM.write(ADDRESSPERIODLED + i, *(p + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera, de la ROM, el tiempo de refresco de los leds.
 * @return el tiempo de refresco de los leds. 
 */
int romGetPeriodLed(){
    int periodLed;
    uint8_t* p = (uint8_t* ) &periodLed;
    for (int i = 0; i < sizeof(periodLed); i++){
        *(p + i) = EEPROM.read(ADDRESSPERIODLED + i);
    }
    return periodLed;
}

/**
 * @brief guarda el nombre del dispositivo bluetooth.
 * @param str corresponde al nombre del bluetooth.
 */
int romSetBluetoothName(String str){
    if (str.length() > MAX_CHARACTERS_BTNAME){
        return -1;
    }
    int i = 0;
    for ( ; i < str.length(); i++){
        EEPROM.write(ADDRESSBTNAME + i, str.charAt(i));
    }
    EEPROM.write(ADDRESSBTNAME + i, '\0');
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera el nombre del bluetooth guardado en ROM
 * @return el nombre del bluetooth guardado en ROM.
 * @note si no encuentra un nombre guardado, devuelve el nombre "Sandsara".
 */
String romGetBluetoothName(){
    String str = "";
    char chr;
    for (int i = 0; i < MAX_CHARACTERS_BTNAME; i++){
        chr = EEPROM.read(ADDRESSBTNAME + i);
        if (chr == '\0')
        {
            if (str.equals("")){
                return "Sandsara";
            }
            return str;
        }
        str.concat( chr );
    }
    return "Sandsara";
}

/**
 * @brief guarda la variable incrementIndexPallete.
 * @param incrementIndex es el valor que se va a guardar.
 * @return 0.
 */
int romSetIncrementIndexPallete(bool incrementIndex){
    if (incrementIndex){
        EEPROM.write(ADDRESSCUSTOMPALLETE_INCREMENTINDEX, 255);
    }
    else{
        EEPROM.write(ADDRESSCUSTOMPALLETE_INCREMENTINDEX, 0);
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera la variable incrementIndexPallete.
 * @return true o false dependiendo lo que haya guardado en la memoria.
 */
bool romGetIncrementIndexPallete(){
    uint8_t var = EEPROM.read(ADDRESSCUSTOMPALLETE_INCREMENTINDEX);
    if (var > 0){
        return true;
    }
    else{
        return false;
    }
}

/**
 * @brief guarda una custom pallete en la memoria ROM.
 * @param positions es la posicion del color en la paleta de colores va de 0 a 255.
 * @param red es un array que contiene los valores de Red de las posiciones en la paleta de colores.
 * @param green es un array que contiene los valores de green de las posiciones en la paleta de colores.
 * @param blue es un array que contiene los valores de blue de las posiciones en la paleta de colores.
 * @return un codigo de error que puede siginificar lo siguiente
 * 0, guardo la nueva pallete correctamente
 * -1, el numero de colores no es correcto.
 */
int romSetCustomPallete(uint8_t* positions ,uint8_t* red , uint8_t* green, uint8_t* blue, int numberOfColors){
    if (numberOfColors > 16 || numberOfColors < 1){
        return -1;
    }
    EEPROM.write(ADDRESSCUSTOMPALLETE_COLORS, numberOfColors);
    for (int i = 0; i < numberOfColors; i++){
        EEPROM.write(ADDRESSCUSTOMPALLETE_POSITIONS + i, *(positions + i));
        EEPROM.write(ADDRESSCUSTOMPALLETE_RED + i, *(red + i));
        EEPROM.write(ADDRESSCUSTOMPALLETE_GREEN + i, *(green + i));
        EEPROM.write(ADDRESSCUSTOMPALLETE_BLUE + i, *(blue + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera una paleta de colores personalizada de la memoria ROM.
 * @param pallete es la paleta que se recupera de la rom.
 */
int romGetCustomPallete(CRGBPalette256 &pallete){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    CRGBPalette256 palleteAuxiliar;
    if (numberOfColors > 16 || numberOfColors < 1){
        uint8_t bytes[8];
        bytes[0] = 0;
        bytes[1] = 255;
        bytes[2] = 0;
        bytes[3] = 0;
        bytes[4] = 255;
        bytes[5] = 0;
        bytes[6] = 255;
        bytes[7] = 0;
        palleteAuxiliar.loadDynamicGradientPalette(bytes);
        pallete = palleteAuxiliar;
        return -1;
    }
    uint8_t newPallete[4*numberOfColors];
    for (int i = 0; i < numberOfColors; i++){
        newPallete[i*4 + 0] = EEPROM.read(ADDRESSCUSTOMPALLETE_POSITIONS + i);
        newPallete[i*4 + 1] = EEPROM.read(ADDRESSCUSTOMPALLETE_RED + i);
        newPallete[i*4 + 2] = EEPROM.read(ADDRESSCUSTOMPALLETE_GREEN + i);
        newPallete[i*4 + 3] = EEPROM.read(ADDRESSCUSTOMPALLETE_BLUE + i);
    }
    rgb2Interpolation(palleteAuxiliar,newPallete);
    pallete = palleteAuxiliar;
    return 0;
}

/**
 * @brief save the variable IntermediateCalibration in ROM.
 * @param state is the value to be saved in ROM.
 * @return a code of error.
 * @note if state is true 0 will be saved in ROM and if it's false 255 will be saved instead.
 */
int romSetIntermediateCalibration(bool state){
    if (state){
        EEPROM.write(ADDRESSINTERMEDIATECALIBRATION,0);
    }
    else
    {
        EEPROM.write(ADDRESSINTERMEDIATECALIBRATION,255);
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief get the variable IntermediateCalibration from ROM.
 * @return true or false depending on what is stored in ROM.
 * @note if the stored value in ROM is greater than 0 false will be returned if it's not, true will be returned instead.
 */
bool romGetIntermediateCalibration(){
    if (EEPROM.read(ADDRESSINTERMEDIATECALIBRATION) > 0){
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * @brief This function save the current playlist position in ROM.
 * @param pList is the currente position to be saved.
 * @return 0
 */
int romSetPositionList(int pList){
    uint8_t* p = (uint8_t* ) &pList;
    for (int i = 0; i < sizeof(pList); i++){
        EEPROM.write(ADDRESSPOSITIONLIST + i, *(p + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief restore the current position in the playlist in ROM.
 * @return the postion list saved in ROM.
 */
int romGetPositionList(){
    int pList;
    uint8_t* p = (uint8_t* ) &pList;
    for (int i = 0; i < sizeof(pList); i++){
        *(p + i) = EEPROM.read(ADDRESSPOSITIONLIST + i);
    }
    if (pList > MAX_POSITIONLIST){
        pList = 1;
    }
    return pList;
}

/**
 * @brief stored the direction of leds in ROM.
 * @param direction is the value to be stored.
 * @return an error code.
 */
int romSetLedsDirection(bool direction){
    if (direction){
        EEPROM.write(ADRESSLEDSDIRECTION, 255);
    }
    else{
        EEPROM.write(ADRESSLEDSDIRECTION, 0);
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief restored the direction of leds saved in ROM.
 * @return true or false depending on the stored value.
 */
bool romGetLedsDirection(){
    uint8_t var = EEPROM.read(ADRESSLEDSDIRECTION);
    if (var > 0){
        return true;
    }
    else{
        return false;
    }
}

//====Funciones de Leds====

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    startIndex = colorIndex;
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        if (incrementIndexGlobal){
            colorIndex =startIndex + float(i+1)*(255.0/float(NUM_LEDS));
        }
    }
}

/**
 * @brief Change the pallette of leds.
 * @param pallet indicates the index pallete to be choosed.
 */
void changePalette(int pallet)
{
    incrementIndexGlobal = romGetIncrementIndexPallete();
    delayLeds = romGetPeriodLed();
    if     ( pallet == 0)   { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND;}
    else if( pallet == 1)   { currentPalette = pallette1;               currentBlending = LINEARBLEND;}
    else if( pallet == 2)   { currentPalette = pallette2;               currentBlending = LINEARBLEND;}
    else if( pallet == 3)   { currentPalette = pallette3;               currentBlending = LINEARBLEND;}
    else if( pallet == 4)   { currentPalette = pallette4;               currentBlending = LINEARBLEND;}
    else if( pallet == 5)   { currentPalette = pallette5;               currentBlending = LINEARBLEND;}
    else if( pallet == 6)   { currentPalette = pallette6;               currentBlending = LINEARBLEND;}
    else if( pallet == 7)   { currentPalette = pallette7;               currentBlending = LINEARBLEND;}
    else if( pallet == 8)   { currentPalette = pallette8;               currentBlending = LINEARBLEND;}
    else if( pallet == 9)   { currentPalette = pallette9;               currentBlending = LINEARBLEND;}
    else if( pallet == 10)  { currentPalette = pallette10;               currentBlending = LINEARBLEND;}
    else if( pallet == 11)  { currentPalette = pallette11;               currentBlending = LINEARBLEND;}
    else if( pallet == 12)  { currentPalette = pallette12;               currentBlending = LINEARBLEND;}
    else if( pallet == 13)  { currentPalette = pallette13;               currentBlending = LINEARBLEND;}
    else if( pallet == 14)  { currentPalette = pallette14;               currentBlending = LINEARBLEND;}
    else if( pallet == 15)  { currentPalette = pallette15;               currentBlending = LINEARBLEND;}
    else if( pallet == 16)  { romGetCustomPallete(currentPalette);      currentBlending = LINEARBLEND;}
    else if( pallet == CODE_NOSD_PALLETE    )     { currentPalette = NO_SD_PALLETE;           incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_UPDATING_PALLETE)     { currentPalette = UPTADATING_PALLETE;      incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_CALIBRATING_PALLETE)  { currentPalette = CALIBRATING_PALLETE;     incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_SDEMPTY_PALLETE)      { currentPalette = SDEMPTY_PALLETE;         incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else   { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND;}
}

/**
 * @brief ledsFunc es una tarea que se corre en paralelo y que se encarga de encender los leds.
 */
void ledsFunc( void * pvParameters ){
    //====Configurar fastled====
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );
    //====
    for(;;){
        if (ledsOffGlobal){
            FastLED.clear();
            FastLED.show();
            while(ledsOffGlobal){
                vTaskDelay(100);
            }
        }
        FillLEDsFromPaletteColors(startIndex);
        for (int i = 0; i < NUM_LEDS; i++){
            leds[i].red = gamma8[leds[i].red];
            leds[i].green = gamma8[leds[i].green];
            leds[i].blue = gamma8[leds[i].blue];
        }      
        FastLED.show();
        if (ledsDirection){
            startIndex += 1;
        }
        else{
            startIndex -= 1;
        }
        //vTaskDelay(delayLeds);
        FastLED.delay(delayLeds);
    } 
}

/**
 * @brief this function find if there is a file update in SD, and if there is one the firmware is updated.
 */
void findUpdate(){
    File file;
    File root = SD.open("/");
    String fileName;
    while(true){
        file = root.openNextFile();
        if (!file){
            root.close();
            return;
        }
        char nameF[NAME_LENGTH];
        file.getName(nameF,NAME_LENGTH);
        fileName = nameF;
        if (fileName.indexOf("firmware-") == 0){
            int indexDash1 = fileName.indexOf("-");
            int indexDash2 = fileName.indexOf("-", indexDash1 + 1);
            int indexDot1 = fileName.indexOf(".");
            int indexDot2 = fileName.indexOf(".", indexDot1 + 1);
            int indexDot3 = fileName.indexOf(".", indexDot2 + 1);
            if (indexDash1 == -1 || indexDash2 == -1 || indexDot1 == -1 || indexDot2 == -1 || indexDot3 == -1){
                #ifdef DEBUGGING_DATA
                    Serial.println("No se encontraron 3 puntos y 2 dash");
                #endif
                continue;
            }
            if (!fileName.substring(indexDot3).equals(".bin")){
                #ifdef DEBUGGING_DATA
                    Serial.println("no es un .bin");
                #endif
                continue;
            }
            int v1 = fileName.substring(indexDash1 + 1, indexDot1).toInt();
            int v2 = fileName.substring(indexDot1 + 1, indexDot2).toInt();
            int v3 = fileName.substring(indexDot2 + 1, indexDash2).toInt();
            int hash = fileName.substring(indexDash2 + 1, indexDot3).toInt();
            if (hash != v1 + v2 + v3 + 1){
                #ifdef DEBUGGING_DATA
                    Serial.println("El hash no coincide");
                #endif
                continue;
            }
            if (v1 > v1Current){
                changePalette(CODE_UPDATING_PALLETE);
                int errorCode = programming(fileName);
                if (errorCode == 1){
                    rebootWithMessage("Reiniciando");
                }
                continue;
            }
            else if(v1 == v1Current){
                if (v2 > v2Current){
                    changePalette(CODE_UPDATING_PALLETE);
                    int errorCode = programming(fileName);
                    if (errorCode == 1){
                        rebootWithMessage("Reiniciando");
                    }
                    continue;
                }
                else if(v2 == v2Current){
                    if(v3 > v3Current){
                        changePalette(CODE_UPDATING_PALLETE);
                        int errorCode = programming(fileName);
                        if (errorCode == 1){
                            rebootWithMessage("Reiniciando");
                        }
                        continue;
                    }
                    else{
                        continue;
                    }
                }
                else{
                    continue;
                }
            }
            else{
                continue;
            }
        }
    }
}

/**
 * @brief reordena las lineas del archivo dirFile de forma random en un archivo llamado RANDOM.playlist y borrar el archivo dirFile
 * @param dirFile es la direccion del archivo a ordenar ej. "/lista.txt"
 * @param numberLines es el numero de lineas que contiene el archivo
 * @return 1 si se ordeno de forma correcta
 * -1, no se pudo crear el archivo auxiliar.
 * -2, hubo problemas al leer el archivo dirFile.
 */
int orderRandom(String dirFile, int numberLines){
    File file;
    String fileName;
    int randomNumber, errorCode, limit = numberLines;
    sdRemove("/RANDOM.playlist");
    file = SD.open("/RANDOM.playlist", FILE_WRITE);
    if (!file){
        return -1;
    }
    int availableNumber[numberLines];
    setFrom1(availableNumber, numberLines);
    for(int i = 0; i < limit; i++){
        randomNumber = random(0, numberLines);
        errorCode = SdFiles::getLineNumber(availableNumber[randomNumber], dirFile, fileName);
        if (errorCode < 0){
            file.close();
            sdRemove(dirFile);
            return -2;
        }
        file.print(fileName + "\r\n");
        removeIndex(availableNumber, randomNumber, numberLines);
        numberLines -= 1;
    }
    file.close();
    sdRemove(dirFile);
    return 1;
}

/**
 * @brief inicializa los valores del vector en orden ascendente empezando en 1.
 * @param list es el array que se desea inicializar.
 * @param elements es la cantidad de elementos que va a contener el array.
 * @note ejemplo, el array resultante sera de forma general como este list[0] = 1, list[1] = 2, list[2] = 3... list[elements - 1] = elements.
 */
void setFrom1(int list[], int elements){
    for (int i = 1; i <= elements; i++){
        list[i-1] = i;
    }
}

/**
 * @brief elimina el elemento en la posicion index del array list
 * @param list es el array que se va a modificar.
 * @param index la posicion del elemento que se desea quitar.
 * @note ejemplo un array con los elementos 1,2,3,4,5,6,7,8,9 se le quita su elemento en la posicion 5 quedaria 1,2,3,4,5,7,8,9
 */
void removeIndex(int list[], int index, int elements){
    for(int i=index; i < elements; i++){
        list[i] = list[i+1];
    }
}

/**
 * @brief realiza una interpolacion lineal entre el valor init y stop dividio en tantas partes como lo indica la variable amount.
 * @param init es el valor inicial de la interpolacion.
 * @param stop es el valor final de la interpolacion.
 * @param amount es la cantidad de divisiones que va a tener la linea.
 * @param index es la posicion correspondiente a cada division en la linea, el index 0 representa el valor init.
 * @return el valor en la posicion index de la linea.
 */
double linspace(double init,double stop, int amount,int index){
    return init + (stop - init)/(amount - 1) * (index);
}

/**
 * @brief performs an interpolation with the method of RGB^2
 * @returns an error code.
 * -1 means the first value of matrix is not 0 what is not allowed.
 *  0 means everything is ok.
 */
int rgb2Interpolation(CRGBPalette256& pallete,uint8_t* matrix){
    if (*(matrix) != 0){
        return -1;
    }
    int i = 0;
    for (i = 0; i < 16; i++){
        int index = 0;
        for (int j = *(matrix + i*4); j < *(matrix + i*4 + 4); j++){
            int amount = *(matrix + i*4 + 4) - *(matrix + i*4);
            pallete[j].red = pow(linspace(pow(*(matrix + i*4 + 1),2.2),pow(*(matrix + i*4 + 1 + 4),2.2), amount, index), 1/2.2);
            pallete[j].green = pow(linspace(pow(*(matrix + i*4 + 2),2.2),pow(*(matrix + i*4 + 2 + 4),2.2), amount, index), 1/2.2);
            pallete[j].blue = pow(linspace(pow(*(matrix + i*4 + 3),2.2),pow(*(matrix + i*4 + 3 + 4),2.2), amount, index), 1/2.2);
            index ++;
        }
        if (*(matrix + i*4 + 4) == 255){
            break;
        }
    }
    pallete[255].red = *(matrix + i*4 + 4 + 1);
    pallete[255].green = *(matrix + i*4 + 4 + 2);
    pallete[255].blue = *(matrix + i*4 + 4 + 3);
    return 0;
}

extern double   maxPathSpeedGlobal;
extern long     maxStepsGlobal;
/**
 * @brief mueve los motores 1 y 2.
 * 
 * La velocidad de los motores se ajusta dependiendo de la distancia a recorrer del punto inicial al final, con la intencion
 * de que la velocidad a la que viaje la esfera sea una velocidad constante. Si la velocidad de los motores es constante, la velocidad
 * a la que se mueve la esfera no lo va a ser debido a la geometria del mecanismo.
 * Cada vez que se mueve la posicion a la que se movio se guarda como la posicion actual del robot.
 * @param q1_steps es el numero de pasos que va a girar el motor correspondiente al angulo q1.
 * @param q2_steps es el numero de pasos que va a girar el motor correspondiente al angulo q2.
 * @param distance es la distancia que va a recorrer entre el punto actual y el punto despues del movimiento.
 * @note La distancia se mide en milimetros 
 */
void moveSteps(void* pvParameters)
{ 
    long positions[2];
    float maxSpeed;
    long q1Steps, q2Steps;
    double distance;
    double pathSpeed = romGetSpeedMotor();
    long maxSteps;
    //this initialization has to be performed after Sandsara.init() 
    double q1Current = Sandsara.q1_current, q2Current = Sandsara.q2_current;
    Sandsara.setRealQ1(q1Current);
    Sandsara.setRealQ2(q2Current);
    //double milimiterSpeed = romGetSpeedMotor();
    for (;;){
        if (startMovement){
            q1Steps = q1StepsGlobal;
            q2Steps = q2StepsGlobal;
            distance = distanceGlobal;
            #ifdef IMPLEMENT_ACCELERATION
                pathSpeed = maxPathSpeedGlobal;
                maxSteps = maxStepsGlobal;
            #endif
            startMovement = false;
            
            //Serial.println(pathSpeed);
            #ifdef IMPLEMENT_ACCELERATION
                //pathSpeed
            #endif
            #ifndef IMPLEMENT_ACCELERATION
                if (abs(q1Steps) > abs(q2Steps + q1Steps)){
                    maxSpeed = abs(q1Steps);
                }
                else{
                    maxSpeed = abs(q2Steps + q1Steps);
                }
                maxSpeed = (maxSpeed / distance) * Sandsara.millimeterSpeed;
            #endif
             #ifdef IMPLEMENT_ACCELERATION
                maxSpeed = (maxSteps / distance) * pathSpeed;
            #endif
            if (maxSpeed > MAX_STEPS_PER_SECOND * MICROSTEPPING)
                maxSpeed = MAX_STEPS_PER_SECOND * MICROSTEPPING;

            #ifdef PROCESSING_SIMULATOR
                Serial.print(q1Steps);
                Serial.print(",");
                Serial.print(q2Steps + q1Steps);
                Serial.print(",");
                Serial.println(int(maxSpeed));
            #endif
            
            Sandsara.stepper1.setMaxSpeed(maxSpeed);
            Sandsara.stepper2.setMaxSpeed(maxSpeed);
            positions[0] = Sandsara.stepper1.currentPosition() + q1Steps;
            positions[1] = Sandsara.stepper2.currentPosition() + q2Steps + q1Steps;
            #ifndef DISABLE_MOTORS
                Sandsara.steppers.moveTo(positions);
                /*String info1;
                String info2;
                info1 = "1:" + String(int(Sandsara.stepper1.speed())) + "," + String(q1Steps) + ",1";
                info2 = "2:" + String(int(Sandsara.stepper2.speed())) + "," + String(q2Steps) + "," + String(pathSpeed);
                Serial.println(info1);
                Serial.println(info2);*/
                Sandsara.setRealSpeed1(Sandsara.stepper1.speed());
                Sandsara.setRealSpeed2(Sandsara.stepper2.speed());
                Sandsara.steppers.runSpeedToPosition();
            #endif
            q1Current += Sandsara.degrees_per_step * q1Steps;
            q2Current += Sandsara.degrees_per_step * q2Steps;
            q1Current = Motors::normalizeAngle(q1Current);
            q2Current = Motors::normalizeAngle(q2Current);
            Sandsara.setRealQ1(q1Current);
            Sandsara.setRealQ2(q2Current);
        }
        vTaskSuspend(motorsTask);
    }
}