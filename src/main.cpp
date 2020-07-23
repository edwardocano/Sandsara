#include <Arduino.h>
#include "FileSara.h"
#include "MoveSara.h"
#include "BlueSara.h"
#include <Adafruit_NeoPixel.h>
#define FASTLED_ESP32_I2S true
#include <FastLED.h>
#include "CalibMotor.h"
#include "Testing.h"

//#define FS_NO_GLOBALS
//#include <FS.h>
#include "SPI.h"
#include "SdFat.h"
SdFat SD;

#include <math.h>

#include <EEPROM.h>


//====Variables externas====
extern TMC2209Stepper driver;
extern TMC2209Stepper driver2;
extern bool sdExists(String );
extern bool sdRemove(String );
//====

File myFile;
File root;
//====variables globales en rom====
String playListGlobal;
String bluetoothNameGlobal;
int ordenModeGlobal;
int speedMotorGlobal;
int ledModeGlobal;
int periodLedsGlobal;
bool ceroZoneGlobal;
//====variables gloabales====
bool ledsOffGlobal = false;
bool playlistChanged = false;
bool ordenModeChanged = false;
bool incrementIndexGlobal = true;
String currentProgramGlobal;
String nextProgramGlobal;
String currentPlaylistGlobal;
int currentPositionListGlobal;
int delayLeds;
int pListFileGlobal;
bool changePositionList;
bool changeProgram;
bool stopProgramChangeGlobal = true;
bool stop_boton;
bool intermediateCalibration = false;
//====variables de estado====
bool pauseModeGlobal = false;
bool suspensionModeGlobal = false;
//====
//====Variables de paletas de colores====
CRGBPalette16 NO_SD_PALLETE;
CRGBPalette16 UPTADATING_PALLETE;
CRGBPalette16 CALIBRATING_PALLETE;
CRGBPalette16 SDEMPTY_PALLETE;
CRGBPalette256 customPallete;
//====
MoveSara halo;
BlueSara haloBt;

int errorCode;

//====prototipos de funciones====
int moveInterpolateTo(double x, double y, double distance);
void executeCode(int );
int romSetPlaylist(String );
String romGetPlaylist();
int romSetOrdenMode(int );
int romGetOrdenMode();
int romSetPosition(int );
int romGetPosition();
void Neo_Pixel(int );
uint32_t rainbow();
void FillLEDsFromPaletteColors(uint8_t );
void changePalette(int );
void SetupTotallyRandomPalette();
void SetupBlackAndWhiteStripedPalette();
void SetupPurpleAndGreenPalette();
void ledsFunc( void * );
int run_sandsara(String ,int );
int movePolarTo(double ,double ,double, bool = false);
int romSetPallete(int );
int romGetPallete();
int romSetSpeedMotor(int );
int romGetSpeedMotor();
int romSetPeriodLed(int );
int romGetPeriodLed();
int romSetBluetoothName(String );
int romGetCustomPallete(CRGBPalette256 &);
int romSetCustomPallete(uint8_t* ,uint8_t* , uint8_t* ,uint8_t*, int);
String romGetBluetoothName();
void findUpdate();
extern int programming(String );
extern void rebootEspWithReason(String );
int orderRandom(String ,int);
void setFrom1(int [], int);
void removeIndex(int [], int , int );
int runFile(String );
void goHomeSpiral(bool = true);
void bluetoothThread(void* );
int romSetIncrementIndexPallete(bool );
bool romGetIncrementIndexPallete();
double linspace(double init,double stop, int amount,int index);
int rgb2Interpolation(CRGBPalette256 &,uint8_t* );
int romSetIntermediateCalibration(bool );
bool romGetIntermediateCalibration();
//====
//====Variable leds====
#define LED_PIN     32
#define NUM_LEDS    36
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
//====

uint8_t pruebapaleta[64] = {0,68,3,86,
17,72,26,108,
34,70,48,126,
51,65,68,135,
68,57,86,140,
85,49,104,142,
102,42,120,142,
120,36,135,142,
136,31,152,139,
153,34,167,133,
170,52,182,121,
187,82,197,105,
204,119,209,83,
221,162,218,55,
238,207,225,28,
255,250,231,34};
/*uint8_t pruebapaleta[64] = {0,0,0,0,
127,127,127,127,
255,255,255,255};*/

CRGBPalette256 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette256 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

unsigned long timeLeds;
uint8_t startIndex = 0;
//====Product Type====
bool productType;
//====
//====Thred====
TaskHandle_t Task1;
TaskHandle_t Task2;
//====
//====Calibracion====
CalibMotor haloCalib;
//====Testing========
Testing haloTest;
//====
//====Paletas para codigos de colores
DEFINE_GRADIENT_PALETTE( breathRed ) {
        0,     0,   0,  0,   //black
        128,   255, 0,  0,   //red
        255,   0,   0,  0};
        
DEFINE_GRADIENT_PALETTE( breathBlue ) {
        0,     0,   0,  0,   //black
        128,   0, 0,  255,   //azul
        255,   0,   0,  0};

DEFINE_GRADIENT_PALETTE( breathYellow ) {
        0,     0,   0,  0,   //black
        128,   255, 255,  0,   //yellow
        255,   0,   0,  0};

DEFINE_GRADIENT_PALETTE( breathOrange ) {
        0,     0,   0,  0,   //black
        128,   255, 60,  0,   //orange
        255,   0,   0,  0};                    
//====
//====Paleta dinamica====
byte bytes[12];
//====
const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

void setup()
{
    delay(3000); // power-up safety delay
    //====Configurar Serial====
    Serial.begin(115200);
    //====
    //====Inicializar Paletas====
    NO_SD_PALLETE= breathRed;
    UPTADATING_PALLETE = breathYellow;
    CALIBRATING_PALLETE = breathBlue;
    SDEMPTY_PALLETE = breathOrange;
    bytes[0] = 0;
    bytes[1] = 255;
    bytes[2] = 0;
    bytes[3] = 0;
    bytes[4] = 125;
    bytes[5] = 0;
    bytes[6] = 255;
    bytes[7] = 0;
    bytes[8] = 255;
    bytes[9] = 0;
    bytes[10] = 0;
    bytes[11] = 255;
    
    //customPallete.loadDynamicGradientPalette(pruebapaleta);
    /*rgb2Interpolation(customPallete,pruebapaleta);
    for (int i = 0; i < 256; i++){
        customPallete[i].red = gamma8[customPallete[i].red];
        customPallete[i].green = gamma8[customPallete[i].green];
        customPallete[i].blue = gamma8[customPallete[i].blue];
    }*/

    /*for (int i = 0; i < 256; i++){
        customPallete[i].red = 2;
        customPallete[i].green = 2;
        customPallete[i].blue = 2;
    }*/
    /*for (int i = 0; i < 256; i++){
        customPallete[i].red = pow(linspace(pow(255,2.2),pow(0,2.2),256, i), 1/2.2);
        customPallete[i].green = pow(linspace(pow(0,2.2),pow(255,2.2),256, i), 1/2.2);
        customPallete[i].blue = pow(linspace(pow(0,2.2),pow(0,2.2),256, i), 1/2.2);
    }*/
    /*for (int i = 0; i < 256; i++){
        Serial.print("Red: ");
        Serial.print(customPallete[i].red);
        Serial.print("\t\tGreen: ");
        Serial.print(customPallete[i].green);
        Serial.print("\t\tBlue: ");
        Serial.println(customPallete[i].blue);
    }
    */
    //delay(100000);
    //====
    //====Inicializacion de SD====
    EEPROM.begin(EEPROM_SIZE);
    delay(500);
	//====Testing====
	int dat_pin;
	dat_pin = analogRead(PIN_ProducType);
	if(dat_pin > 1000 && dat_pin < 4000)
	{
		haloTest.Test();	
	}
    //====restore the value intermediateCalibration====
    intermediateCalibration = romGetIntermediateCalibration();
    //====
    //====Recuperar speedMotor====
    speedMotorGlobal = romGetSpeedMotor();
    if (speedMotorGlobal > MAX_SPEED_MOTOR || speedMotorGlobal < MIN_SPEED_MOTOR){
        speedMotorGlobal = SPEED_MOTOR_DEFAULT;
        romSetSpeedMotor(SPEED_MOTOR_DEFAULT);
    }
    halo.setSpeed(speedMotorGlobal);
    //====
    //====Recuperar Periodo de refresco de los leds====
    periodLedsGlobal = romGetPeriodLed();
    if (periodLedsGlobal > MAX_PERIOD_LED || periodLedsGlobal < MIN_PERIOD_LED){
        periodLedsGlobal = PERIOD_LED_DEFAULT;
        romSetPeriodLed(PERIOD_LED_DEFAULT);
    }
    //====
    //====Recuperar Paleta de color para leds====
    ledModeGlobal = romGetPallete();
    if (ledModeGlobal > MAX_PALLETE || ledModeGlobal < MIN_PALLETE){
        ledModeGlobal = PALLETE_DEFAULT;
        romSetPallete(PALLETE_DEFAULT);
    }
    changePalette(ledModeGlobal);
    //====
    
    //====Recuperar nombre del bluetooth====
    bluetoothNameGlobal = romGetBluetoothName();
    //====
    //====Configure the halo y bluetooth====
    halo.init();
    haloBt.init(bluetoothNameGlobal);
    //====
    //====new task for leds====
    xTaskCreatePinnedToCore(
                    ledsFunc,   /* Task function. */
                    "Task1",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
    delay(500); 
    //====
    //====Seleccionar tipo de producto====
    //pinMode(PIN_ProducType, INPUT);
    delay(1000);
    productType = analogRead(PIN_ProducType);
    //====Calibrar====
    Serial.println("init func");
    haloCalib.init();
    Serial.println("start func");
    haloCalib.start();
    Serial.println("Salio de start");
    
    pinMode(EN_PIN, OUTPUT);
    pinMode(EN_PIN2, OUTPUT);
    digitalWrite(EN_PIN, LOW);
    digitalWrite(EN_PIN2, LOW);
    //====
    //====Inicializar SD====
    while(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
    {
        changePalette(CODE_NOSD_PALLETE);
        Serial.println("Card failed, or not present");
        delay(200); 
    }
    changePalette(ledModeGlobal);
    Serial.println("Card present");
    //====
    myFile = SD.open("/");
    if (!myFile)
    {
        Serial.println("No se pudo abrir archivo");
        return;
    }
    root = SD.open("/");
    //====
    //====recuperar valores de playlist y ordenMode====    
    playListGlobal = romGetPlaylist();
    ordenModeGlobal = romGetOrdenMode();
    if (ordenModeGlobal < MIN_REPRODUCTION_MODE || ordenModeGlobal > MAX_REPRODUCTION_MODE){
        ordenModeGlobal = 3;
        romSetOrdenMode(3);
    }
    changePalette(romGetPallete());
    if (playListGlobal.equals("/")){
        Serial.print("No hay una playlist guardada, se reproduciran todos los archivos en la sd");
        //ordenModeGlobal = 3;
    }
    else
    {
        if (sdExists(playListGlobal)){
            Serial.print("lista guardada: ");
            Serial.println(playListGlobal);
        }
        else{
            Serial.println("La playlist no existe, se reproduciran todos los archivos en la sd");
            //ordenModeGlobal = 3;
        }
    }
    Serial.print("ordenMode guardado: ");
    Serial.println(ordenModeGlobal);
    //====
    //====Buscar por nueva actualizacion====
    findUpdate();
    //====
    #ifdef PROCESSING_SIMULATOR
        Serial.println("inicia");
    #endif
    
    
    //=====si existe el archivos llamado playlist.playlist se ejecutara esa playlist====
    if (sdExists("/playlist.playlist")){
        playListGlobal = "/playlist.playlist";
        ordenModeGlobal = 1;
    }
    //=====
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());
    //====new task for leds====
    xTaskCreatePinnedToCore(
                    bluetoothThread,   /* Task function. */
                    "Task2",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    4,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
    delay(500); 
    //====
}

void loop()
{
#ifdef DEBUGGING_DATA
    Serial.println("Iniciara la funcion runSansara");
#endif
    errorCode = run_sandsara(playListGlobal, ordenModeGlobal);
    Serial.print("errorCode de run: ");
    Serial.println(errorCode);
    if (errorCode == -10){
        while(!SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD))
        {
            changePalette(CODE_NOSD_PALLETE);
            Serial.println("Card failed, or not present");
            delay(200); 
        }
        changePalette(ledModeGlobal);
        Serial.println("Card present");
        

    }
    else if (errorCode == -4){
        changePalette(CODE_SDEMPTY_PALLETE);
        SD.begin(SD_CS_PIN, SPI_SPEED_TO_SD);
    }
    /*errorCode = haloBt.checkBlueTooth();
    executeCode(errorCode);*/
    delay(3000);
}

/**
 * @brief
 * @param playlist el nombre de la playlist a ejecutar, ejemplo "/animales.playlist"
 * @param ordenMode el tipo de reproduccion pudiendo ser
 * 1, se ejecuta en orden desendente segun la playlist.
 * 2, se ejecutan los archivos de la playlist en orden aleatorio.
 * 3, se ejecutan todos los archivos contenidos en el directorio principal de la SD en el orden que la sd los acomoda.
 * 4, se ejecutan todos los archivos contenidos en el directorio principal de la SD en orden aleatorio.
 * @return codigo de error.
 *  1, se cambio la playlist o se cambio el ordenMode
 *  0, termino el la funcion sin problemas.
 * -1, No se pudo abrir el archivo con la direccion dirFile de getLineNumber.
 * -2, El archivo abierto es un directorio de getLineNumber.
 * -3, La linea que se desea leer no es valida de getLineNumber.
 * -4, el numero de archivos es cero.
 * -10, no se detecta SD.
 */
int run_sandsara(String playList, int ordenMode)
{
    pListFileGlobal = 1;
    int numberOfFiles;
    String fileName;
    ordenModeChanged = false;
    playlistChanged = false;
    
    if (ordenMode == 1 || ordenMode == 2){
        File file;
        file = SD.open(playList);
        if (file && !file.isDirectory()){
            file.close();
            numberOfFiles = FileSara::numberOfLines(playList);
            if (numberOfFiles < 0){
                return -4;
            }
            if (ordenMode == 2){
                orderRandom(playList,numberOfFiles);
                playList = "/RANDOM.playlist";
            }
            Serial.print("Numero de archivos: ");
            Serial.println(numberOfFiles);
        }
        else
        {
            file.close();
            ordenMode = 3;
            numberOfFiles = FileSara::creatListOfFiles("/DEFAULT.playlist");
            if (numberOfFiles < 0){
                return -4;
            }
            playList = "/DEFAULT.playlist";
            Serial.print("Numero de archivos: ");
            Serial.println(numberOfFiles);
        }
    }
    else if (ordenMode == 4)
    {
        numberOfFiles = FileSara::creatListOfFiles("/auxList32132123.playlist");
        if (numberOfFiles < 0){
            return -4;
        }
        playList = "/auxList32132123.playlist";
        orderRandom(playList,numberOfFiles);
        playList = "/RANDOM.playlist";
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    }
    else
    {
        numberOfFiles = FileSara::creatListOfFiles("/DEFAULT.playlist");
        if (numberOfFiles < 0){
            return -4;
        }
        playList = "/DEFAULT.playlist";
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    }
    
    if (numberOfFiles == 0){
        return -4;
    }
    changePalette(romGetPallete());
    //====recuperar playlist====
    currentPlaylistGlobal = playList;
    //====
    while (true)
    {
        #ifdef DEBUGGING_DATA
            Serial.println("Abrira el siguiente archivo disponible");
        #endif
        errorCode = FileSara::getLineNumber(pListFileGlobal, playList, fileName);
        if (errorCode < 0)
        {
            //====playList no valida asi que regresa
            delay(1000);
            return errorCode;
        }
        errorCode = FileSara::getLineNumber(pListFileGlobal + 1, playList, nextProgramGlobal);
        if (errorCode < 0)
        {
            nextProgramGlobal = "none";
        }
        if (errorCode == 2 || errorCode == 3)
        {
            delay(1000);
            break;
        }
        File current_file = SD.open("/" + fileName);
        if (!current_file)
        {
            #ifdef DEBUGGING_DATA
                Serial.println("No hay archivo disponible");
            #endif
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
        //====Correr programa====
        errorCode = runFile(fileName);
        //====
        if (errorCode == -70)   {continue;}
        else if (errorCode == -71)   {break;}
        else if (errorCode == 20)    {pListFileGlobal = haloBt.getPositionList(); continue;}
        else if (errorCode == 30)    {
            while(errorCode == 30){
                fileName = haloBt.getProgram();
                errorCode = runFile(fileName);
            }
            if (errorCode == 20)    {pListFileGlobal = haloBt.getPositionList(); continue;}
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
        //====Aumentar la posicion en la lista de reproduccion====
        pListFileGlobal += 1;
        //====
    }
    return 0;
}
/**
 * @brief ejecuta un programa
 * @param dirFile, direccion en la SD del programa.
 * @return un codigo de error
 * -70, es en lugar del continue
 * -71, es en lugar de un break
 *   1, se desea cambiar de ordenMode o Playlist
 *  10, no se presento algun return
 *  20, se desea cambiar de programa por posicion.
 *  30, se desea cambiar de programa por nombre
 *  40, se desea suspender.
 */
int runFile(String fileName){
    double component_1, component_2, distance;
    int working_status = 0;
    //====recuperar el nombre del programa y su posicion en la lista
    currentProgramGlobal = fileName;
    currentPositionListGlobal = pListFileGlobal;
    //====
    double couplingAngle, startFileAngle, endFileAngle;
    int posisionCase;
    FileSara file(fileName);
    if (file.fileType < 0){
        pListFileGlobal += 1;
        return -70;
    }

    double zInit = halo.getCurrentModule();
    //se selecciona modo de lectura
    file.autoSetMode(zInit);
    if (!file.isValid()){
        pListFileGlobal += 1;
        return -70;
    }
    #ifdef PROCESSING_SIMULATOR
        Serial.print("fileName: ");
        Serial.println(fileName);
    #endif
    startFileAngle = file.getStartAngle();
    startFileAngle = MoveSara::normalizeAngle(startFileAngle);
    endFileAngle = file.getFinalAngle();
    endFileAngle = MoveSara::normalizeAngle(endFileAngle);
    
    posisionCase = halo.position();
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
        double zf, thetaf, zi, thetai, zFinal, thetaFinal;
        thetaf = MoveSara::normalizeAngle(file.getStartAngle() - couplingAngle);
        zf = file.getStartModule();
        thetai = halo.getCurrentAngle();
        zi = halo.getCurrentModule();
        halo.setZCurrent(zi);
        halo.setThetaCurrent(thetai);
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
    //si es thr, se guardan los valores del primer punto para que tenga referencia de donde empezar a moverse.
    if (file.fileType == 2)
    {
        working_status = file.getNextComponents(&component_1, &component_2);
        while (working_status == 3)
        {
            working_status = file.getNextComponents(&component_1, &component_2);
        }
        halo.setZCurrent(component_1);
        halo.setThetaCurrent(component_2 - couplingAngle);
    }

    //parara hasta que el codigo de error del archivo sea diferente de cero.
    while (true)
    {
        //====comprobar si se desea cambiar de archivo====
        if (changePositionList){
            changePositionList = false;
            changeProgram = false;
            goHomeSpiral(false);
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
            goHomeSpiral(false);
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
        //====Revisar si se suspende o pausa====
        if (pauseModeGlobal == true){
            while (pauseModeGlobal)
            {
                delay(200);
            }
        }
        if (suspensionModeGlobal == true){
            goHomeSpiral(false);
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            return 40;
        }
        //====Revisar si se cambia playlist u ordenMode====
        if (playlistChanged || ordenModeChanged){
            goHomeSpiral(false);
            playlistChanged = false;
            ordenModeChanged = false;
            #ifdef PROCESSING_SIMULATOR
                Serial.println("finished");
            #endif
            return 1;
        }
        //====
        //====se obtienen los siguientes componentes
        working_status = file.getNextComponents(&component_1, &component_2);                
        if (working_status == 3)
        {
            continue;
        }
        if (working_status != 0)
        {
            /*Serial.print("workingStatus= ");
            Serial.println(working_status);*/
            break;
        }
        //====revisar bluetooth====
        /*errorCode = haloBt.checkBlueTooth();
        executeCode(errorCode);*/
        //dependiendo del tipo de archivo se ejecuta la funcion correspondiente de movimiento.
        if (file.fileType == 1 || file.fileType == 3)
        {
            MoveSara::rotate(component_1, component_2, -couplingAngle);
            distance = halo.module(component_1, component_2, halo.x_current, halo.y_current);
            if (distance > 1.1)
            {
                errorCode = moveInterpolateTo(component_1, component_2, distance);
                if (errorCode != 0){
                    return errorCode;
                }
            }
            else
            {
                halo.moveTo(component_1, component_2);
            }
        }
        else if (file.fileType == 2)
        {
            errorCode = movePolarTo(component_1, component_2, couplingAngle);
            if (errorCode != 0){
                return errorCode;
            }
        }
        else
        {
            break;
        }
    }
    //====Actualizar z y theta current====
    halo.setZCurrent(halo.getCurrentModule());
    if (halo.getCurrentAngle() > PI){
        halo.setThetaCurrent(halo.getCurrentAngle() - 2*PI);
    }
    else{
        halo.setThetaCurrent(halo.getCurrentAngle());
    }
    //====
    //====Revisar si hubo problemas con la sd====
    if (working_status == -10)
    {
        Serial.println("There were problems for reading SD");
        Serial.println("Se mandara a cero");
        movePolarTo(0, 0, 0, true);
        #ifdef PROCESSING_SIMULATOR
            Serial.println("finished");
        #endif
        return -10;
    }
    //====
    posisionCase = halo.position();
    if (posisionCase == 2){
        movePolarTo(DISTANCIA_MAX, 0, 0, true);
    }
    else if (posisionCase == 0){
        Serial.println("Se mandara a cero");
        movePolarTo(0, 0, 0, true);
		if (intermediateCalibration == true)
		{
			haloCalib.verificacion_cal();
		}
	}
    #ifdef PROCESSING_SIMULATOR
        Serial.println("finished");
    #endif
    //====Revisar si se desea suspender====
    /*if (suspensionModeGlobal){

        ledsOffGlobal = true;
        while(suspensionModeGlobal){
            delay(100);
        }
        ledsOffGlobal = false;
    }*/
    
    //====
    //====Revisar si se desea cambiar de lista de reproduccion u ordenMode====
    /*if(playlistChanged || ordenModeChanged){
        playlistChanged = false;
        ordenModeChanged = false;
        return 1;
    }*/
    //====
    return 10;
}
/**
 * @brief regresa a la pocision 0,0
 * 
 */
void goHomeSpiral(bool stop){
    float degreesToRotate;
    halo.setZCurrent(halo.getCurrentModule());
    if (halo.getCurrentAngle() > PI){
        halo.setThetaCurrent(halo.getCurrentAngle() - 2*PI);
    }
    else{
        halo.setThetaCurrent(halo.getCurrentAngle());
    }
    degreesToRotate = halo.getCurrentModule()/EVERY_MILIMITERS * 2*PI;
    halo.setSpeed(SPEED_TO_CENTER);
    //degreesToRotate = 0;
    stopProgramChangeGlobal = stop;
    movePolarTo(0, degreesToRotate, 0, true);
    stopProgramChangeGlobal = true;
    halo.setSpeed(romGetSpeedMotor());
}
//=========================================================
/**
 * @brief Se usa esta funcion para avanzar de la posicion actual a un punto nuevo en linea recta por medio de puntos equidistantes a un 1 mm.
 * @param x coordenada en el eje x, medida en milimetros, a la que se desea avanzar.
 * @param y coordenada en el eje y, medida en milimetros, a la que se desea avanzar.
 * @param distance es la distancia, medida en milimetros, entre el punto actual y el punto al que se desea avanzar.
 * @return un codigo de error referente al bluetooth.
 * 1, se requiere cambio de playlist
 * 2, se requiere cambio de ordenMode
 * 0, termino 
 */
int moveInterpolateTo(double x, double y, double distance)
{
    double alpha = atan2(y - halo.y_current, x - halo.x_current);
    double delta_x, delta_y;
    double x_aux = halo.x_current, y_aux = halo.y_current;
    delta_x = cos(alpha);
    delta_y = sin(alpha);
    int intervals = distance;
	
    for (int i = 1; i <= intervals; i++)
    {
        //====comprobar si se desea cambiar de archivo o suspender o cambiar playlist u orden====
        if ((changePositionList || changeProgram || suspensionModeGlobal || playlistChanged || ordenModeChanged) && stopProgramChangeGlobal){
            return 0;
        }
        //====
        //====Comprobar si se pausa
        x_aux += delta_x;
        y_aux += delta_y;
        halo.moveTo(x_aux, y_aux);
        //executeCode(errorCode);
        /*errorCode = haloBt.checkBlueTooth();
        executeCode(errorCode);*/
    }
    halo.moveTo(x, y);
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
 * 2, se cambio el ordenMode
 */
int movePolarTo(double component_1, double component_2, double couplingAngle, bool littleMovement){
    double zNext = component_1;
    double thetaNext = component_2 - couplingAngle;
    double thetaCurrent = halo.getThetaCurrent();
    double zCurrent = halo.getZCurrent();
    double slicesFactor, distance;
    long slices;
    double deltaTheta, deltaZ;
    double thetaAuxiliar, zAuxliar, xAux, yAux;
    deltaTheta = thetaNext - thetaCurrent;
    deltaZ = zNext - zCurrent;
    slicesFactor = halo.arcLength(deltaZ, deltaTheta, zCurrent);
    slices = slicesFactor;
    if (slices < 1)
    {
        slices = 1;
    }
    deltaTheta = (thetaNext - thetaCurrent) / slices;
    deltaZ = (zNext - zCurrent) / slices;
    for (long i = 1; i < slices; i++)
    {
        //====comprobar si se desea cambiar de archivo o suspender o cambiar playlist u orden====
        if ((changePositionList || changeProgram || suspensionModeGlobal || playlistChanged || ordenModeChanged) && stopProgramChangeGlobal){
            return 0;
        }
        //====
        thetaAuxiliar = thetaCurrent + deltaTheta * double(i);
        zAuxliar = zCurrent + deltaZ * double(i);
        xAux = zAuxliar * cos(thetaAuxiliar);
        yAux = zAuxliar * sin(thetaAuxiliar);
        distance = halo.module(xAux, yAux, halo.x_current, halo.y_current);
        if (distance > 1.1)
        {
            errorCode = moveInterpolateTo(xAux, yAux, distance);
            if (errorCode != 0){
                return errorCode;
            }
        }
        else
        {
            halo.moveTo(xAux, yAux, littleMovement);
            /*errorCode = haloBt.checkBlueTooth();
            executeCode(errorCode);*/
        }
    }
    xAux = zNext * cos(thetaNext);
    yAux = zNext * sin(thetaNext);
    halo.moveTo(xAux, yAux, littleMovement);
    halo.setThetaCurrent(thetaNext);
    halo.setZCurrent(zNext);
    return 0;
}
/**
 * @brief mantiene en poling el bluetooth.
 */
void bluetoothThread(void * pvParameters ){
    for(;;){
        errorCode = haloBt.checkBlueTooth();
        executeCode(errorCode);
        vTaskDelay(100);
    }
}

/**
 * @brief Ejecuta los codigos que regresa la funcion checkBluetooth();
 * 
 */
void executeCode(int errorCode){
    if (errorCode == 10){
        playListGlobal = "/" + haloBt.getPlaylist();
        romSetPlaylist(playListGlobal);
        playlistChanged = true;
    }
    else if (errorCode == 20){
        ordenModeGlobal = haloBt.getOrdenMode();
        romSetOrdenMode(ordenModeGlobal);
        ordenModeChanged = true;
    }
    else if (errorCode == 30){
        ledModeGlobal = haloBt.getLedMode();
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
        //Neo_Pixel(ledModeGlobal);
    }
    else if (errorCode == 50){
        int speed = haloBt.getSpeed();
        halo.setSpeed(speed);
        romSetSpeedMotor(speed);
    }
    else if (errorCode == 60){
        int periodLed = haloBt.getPeriodLed();
        periodLedsGlobal = periodLed;
        delayLeds = periodLed;
        romSetPeriodLed(periodLedsGlobal);
    }
    else if (errorCode == 70){
        String blueName = haloBt.getBluetoothName();
        bluetoothNameGlobal = blueName;
        romSetBluetoothName(bluetoothNameGlobal);
    }
    else if (errorCode == 80){
        suspensionModeGlobal = true;
        pauseModeGlobal = false;
    }
    else if (errorCode == 90){
        pauseModeGlobal = true;
    }
    else if (errorCode == 100){
        pauseModeGlobal = false;
        suspensionModeGlobal = false;
    }
    else if (errorCode == 130){
        haloBt.writeBt("currentProgram= ");
        haloBt.writeBtln(currentProgramGlobal);
    }
    else if (errorCode == 140){
        haloBt.writeBt("nextProgram= ");
        haloBt.writeBtln(nextProgramGlobal);
    }
    else if (errorCode == 150){
        String fileName;
        int i = 1, errorCode;
        haloBt.writeBt("playlist: ");
        haloBt.writeBtln(currentPlaylistGlobal);
        for (;;){
            errorCode = FileSara::getLineNumber(i,currentPlaylistGlobal, fileName);
            if (errorCode < 0){
                haloBt.writeBtln("error");
                break;
            }
            if (errorCode > 0){
                if (!fileName.equals("")){
                    haloBt.writeBt(String(i) + ": ");
                    haloBt.writeBtln(fileName);
                }
                break;
            }
            haloBt.writeBt(String(i) + ": ");
            haloBt.writeBtln(fileName);
            i++;
        }
        haloBt.writeBtln("ok");
    }
    else if (errorCode == 160){
        changePositionList = true;
        changeProgram = false;
    }
    else if (errorCode == 170){
        changeProgram = true;
        changePositionList = false;
    }
    else if (errorCode == 190){
        incrementIndexGlobal = romGetIncrementIndexPallete();
    }
    else if (errorCode == 200){
        intermediateCalibration = romGetIntermediateCalibration();
    }
    else if (errorCode == 970){
        romSetSpeedMotor(SPEED_MOTOR_DEFAULT);
        romSetPlaylist(PLAYLIST_DEFAULT);
        romSetPallete(PALLETE_DEFAULT);
        romSetPeriodLed(PERIOD_LED_DEFAULT);
        romSetOrdenMode(ORDENMODE_DEFAULT);
        romSetBluetoothName(BLUETOOTHNAME);
        romSetIntermediateCalibration(false);
        for (int i = ADDRESSPOLESENSE1; i < ADDRESSPOLESENSE1 + ADDRESSESTOVERIFY; i++){
            EEPROM.write(i, -1);
        }
        delay(1000);
        rebootEspWithReason("Se hiso reset de fabrica, Reiniciando...");
    }
}

//------------Guardar variables importantes en FLASH----------------

/**
 * @brief actualiza el valor, en la ROM/FLASH, el nombre de la lista de reproduccion.
 * @param str es la direccion del archivo de lista de reproduccion, ejemplo "/animales.playlist"
 * @note unicamente guarda el nombre, ignorando '/' y ".playlist", ejemplo si str es "/animales.playlist" solo guarda "animales"
 *       pero la funcion romGetPlaylist la devuelve como "/animales.playList" 
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
 * @brief recupera el nombre de la playlist guardada en rom
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
 * @brief guarda el tipo de orden de reporduccion en la memoria rom
 * @param ordenMode el valor que corresponde al orden de reproduccion que va a ser almacenado en ROM.
 * @return 0
 */
int romSetOrdenMode(int ordenMode){
    uint8_t Mode = ordenMode;
    EEPROM.write(ADDRESSORDENMODE, Mode);
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera el valor correspondiente al tipo de reporduccion guardado en la memoria ROM/FLASH.
 * @return el valor correspondiente al tipo de orden de reporduccion guardado en la ROM/FLASH.
 */
int romGetOrdenMode(){
    uint8_t ordenMode;
    ordenMode = EEPROM.read(ADDRESSORDENMODE);
    return ordenMode;
}

/**
 * @brief guarda, en la ROM/FLASH, la posicion en la que va la lista de reproduccion actual.
 * @param pos es el la posicion en la que va la lista de reproduccion que se desea guardar en ROM/FLASH.
 * @return 0 
 */
int romSetPosition(int pos){
    uint8_t* p = (uint8_t* ) &pos;
    for (int i = 0; i < sizeof(pos); i++){
        EEPROM.write(ADDRESSPOSITION + i, *(p + i));
    }
    EEPROM.commit();
    return 0;
}

/**
 * @brief recupera, de la ROM/FLASH, la ultima posicion guardada en la que iba la lista de reproduccion.
 * @return la ultima posicion guardada, en ROM/FLASH, en la que iba la lista de reproduccion. 
 */
int romGetPosition(){
    int ordenMode;
    uint8_t* p = (uint8_t* ) &ordenMode;
    for (int i = 0; i < sizeof(ordenMode); i++){
        *(p + i) = EEPROM.read(ADDRESSPOSITION + i);
    }
    return ordenMode;
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
 * @brief recupera, de la ROM/FLASH, la ultima palleta de colores guardada.
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
 * @brief guarda la velocidad del robot en rom
 * @param speed es la velocidad en milimetros por segundo del robot.
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
 * @brief recupera, de la ROM/FLASH, la velocidad del robot.
 * @return un numero entero que indaca la velocidad del robot, en milimetros por segundo. 
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
 * @brief guarda el periodo de refresco de los leds en ROM.
 * @param periodLed es el periodo de refresco de los leds.
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
 * @brief recupera, de la ROM/FLASH, el periodo de refresco de los leds.
 * @return el periodo de refresco de los leds. 
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
 * @param str corresponde al nombre del bluetooth
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
 * @return un codigo de error
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
 * @brief guarda la variable incrementIndexPallete.
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
 * @brief guarda una custom pallete en la memoria ROM.
 * @param positions es la posicion del color en la paleta de colores va de 0 a 255.
 * @param red es un array que contiene los valores de Red de las posiciones en la paleta de colores.
 * @param green es un array que contiene los valores de green de las posiciones en la paleta de colores.
 * @param blue es un array que contiene los valores de blue de las posiciones en la paleta de colores.
 * @return un codigo de error que puede significar lo siguiente.
 * 0, todo salio normal.
 * -1, no habia un valor valido en ADDRESSCUSTOMPALLETE_COLORS y regreso un palleta predefinida.
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
    //pallete.loadDynamicGradientPalette(newPallete);
    rgb2Interpolation(palleteAuxiliar,newPallete);
    for (int i = 0; i < 256; i++){
        palleteAuxiliar[i].red = gamma8[palleteAuxiliar[i].red];
        palleteAuxiliar[i].green = gamma8[palleteAuxiliar[i].green];
        palleteAuxiliar[i].blue = gamma8[palleteAuxiliar[i].blue];
    }
    pallete = palleteAuxiliar;
    return 0;
}
//=======================================Leds========================================
//===================================================================================
//====================Libreria fastled==========================
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    startIndex = colorIndex;
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        /*Serial.print("led ");
        Serial.print(i);
        Serial.print(" = ");
        Serial.print(leds[i].red);
        Serial.print("\t");
        Serial.print(leds[i].green);
        Serial.print("\t");
        Serial.println(leds[i].blue);
        delay(1000);*/
        if (incrementIndexGlobal){
            colorIndex =startIndex + float(i+1)*(255.0/float(NUM_LEDS));
        }
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void changePalette(int pallet)
{
    incrementIndexGlobal = romGetIncrementIndexPallete();
    delayLeds = romGetPeriodLed();
    if     ( pallet == 0)  { currentPalette = RainbowColors_p;          currentBlending = LINEARBLEND;}
    else if( pallet == 1)   { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;}
    else if( pallet == 2)   { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND;}
    else if( pallet == 3)   { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND;}
    else if( pallet == 4)   { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND;}
    else if( pallet == 5)   { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND;}
    else if( pallet == 6)   { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND;}
    else if( pallet == 7)   { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND;}
    else if( pallet == 8)   { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND;}
    else if( pallet == 9)   { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;}
    else if( pallet == 10)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND;}
    else if( pallet == 11)  { romGetCustomPallete(currentPalette);      currentBlending = LINEARBLEND;}
    else if( pallet == CODE_NOSD_PALLETE    )     { currentPalette = NO_SD_PALLETE;           incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_UPDATING_PALLETE)     { currentPalette = UPTADATING_PALLETE;      incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_CALIBRATING_PALLETE)  { currentPalette = CALIBRATING_PALLETE;     incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else if( pallet == CODE_SDEMPTY_PALLETE)      { currentPalette = SDEMPTY_PALLETE;         incrementIndexGlobal = false;  delayLeds = DELAYCOLORCODE;}
    else   { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND;}
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 256, CRGB::Black);
    // and set every fourth one to white.
    for(int i = 0; i < 16; i++){
        currentPalette[i] = CRGB::White;
    }
    
    //currentPalette[4] = CRGB::White;
    //currentPalette[8] = CRGB::White;
    //currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette256(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

/**
 * @brief se encarga de animar la secuencia de leds
 * 
 */
void ledsFunc( void * pvParameters ){
    //====Configurar fastled====
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );
    //====
    for(;;){
        /*currentPalette = customPallete;
        FillLEDsFromPaletteColors(startIndex);
        FastLED.show();
        startIndex += 1;
        //vTaskDelay(delayLeds);
        FastLED.delay(delayLeds);
        continue;*/
        if (ledsOffGlobal){
            FastLED.clear();
            FastLED.show();
            while(ledsOffGlobal){
                vTaskDelay(100);
            }
        }
        FillLEDsFromPaletteColors(startIndex);
        FastLED.show();
        startIndex += 1;
        //vTaskDelay(delayLeds);
        FastLED.delay(delayLeds);
    } 
}

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
                Serial.println("No se encontraron 3 puntos y 2 dash");
                continue;
            }
            if (!fileName.substring(indexDot3).equals(".bin")){
                Serial.println("no es un .bin");
                continue;
            }
            int v1 = fileName.substring(indexDash1 + 1, indexDot1).toInt();
            int v2 = fileName.substring(indexDot1 + 1, indexDot2).toInt();
            int v3 = fileName.substring(indexDot2 + 1, indexDash2).toInt();
            int hash = fileName.substring(indexDash2 + 1, indexDot3).toInt();
            if (hash != v1 + v2 + v3 + 1){
                Serial.println("El hash no coincide");
                continue;
            }
            if (v1 > v1Current){
                changePalette(CODE_UPDATING_PALLETE);
                int errorCode = programming(fileName);
                if (errorCode == 1){
                    rebootEspWithReason("Reiniciando");
                }
                continue;
            }
            else if(v1 == v1Current){
                if (v2 > v2Current){
                    changePalette(CODE_UPDATING_PALLETE);
                    int errorCode = programming(fileName);
                    if (errorCode == 1){
                        rebootEspWithReason("Reiniciando");
                    }
                    continue;
                }
                else if(v2 == v2Current){
                    if(v3 > v3Current){
                        changePalette(CODE_UPDATING_PALLETE);
                        int errorCode = programming(fileName);
                        if (errorCode == 1){
                            rebootEspWithReason("Reiniciando");
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
        errorCode = FileSara::getLineNumber(availableNumber[randomNumber], dirFile, fileName);
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
 * @note ejemplo el array terminara con elemento  list[0] = 1, list[1] = 2, list[2] = 3...
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
 * @brief realiza una interpolacion lineal entre el valor init y stop dividio en amount partes.
 * @param init es el valor inicial de la interpolacion.
 * @param stop es el valor final de la interpolacion.
 * @param amount es la cantidad de divisiones que va a tener la linea.
 * @param index es la posicion en las divisiones de la linea, el index 0 representa el valor init.
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
