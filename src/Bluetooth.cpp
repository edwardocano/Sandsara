#include "Bluetooth.h"
#include <Update.h>
#include <SdFiles.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLEAdvertisedDevice.h>
#include "Motors.h"
#include <EEPROM.h>
#include <FastLED.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

extern  int     periodLedsGlobal;
extern  int     delayLeds;
extern  int     romSetPlaylist(String );
extern  String  romGetPlaylist();
extern  int     romSetOrderMode(int );
extern  int     romGetOrderMode();
extern  int     romSetPallete(int );
extern  int     romGetPallete();
extern  int     romSetSpeedMotor(int );
extern  int     romGetSpeedMotor();
extern  int     romSetPeriodLed(int );
extern  int     romGetPeriodLed();
extern  int     romSetCustomPallete(uint8_t* ,uint8_t* , uint8_t* ,uint8_t*, int);
extern  int     romSetBluetoothName(String );
extern  String  romGetBluetoothName();
extern  int     romSetIntermediateCalibration(bool );
extern  bool    romGetIntermediateCalibration();
extern  int     romSetPositionList(int );
extern  int     romGetPositionList();
extern  bool    romSetIncrementIndexPallete(bool );
extern  int     romGetIncrementIndexPallete();
extern  bool    romSetLedsDirection(bool );
extern  bool    romGetLedsDirection();
extern  int     romSetBrightness(uint8_t );
extern  int     romGetBrightness();
extern  bool    incrementIndexGlobal;
extern  bool    ledsDirection;
extern  int     ledModeGlobal;
extern  void    changePalette(int );
extern  int     stringToArray(String , uint8_t* , int );
extern  bool    changePositionList;
extern  bool    changeProgram;
extern  bool    sdExists(String );
extern  String  playListGlobal;
extern  SdFat   SD;
extern  bool     sdRemove(String );
extern  bool     sdExists(String );
extern  bool     readingSDFile;
extern  int      orderModeGlobal;
extern  bool     rewindPlaylist;
extern  bool     pauseModeGlobal;
extern  String  bluetoothNameGlobal;
extern  bool    suspensionModeGlobal;
extern  Motors Sandsara;
extern  bool    speedChangedMain;


//====variables====
extern  String      changedProgram;
extern  int         changedPosition;
bool        receiveFlag = false;
bool        sendFlag = false;
char        buffer[10000];
char        *pointerB;
int         bufferSize;
File        fileReceive;
File        fileSend;

//====Prototypes====
int performUpdate(Stream &, size_t );
int updateFromFS(SdFat &, String );
int programming(String );
void rebootWithMessage(String );
int stringToArray(String , uint8_t* , int );

//====BLE Characteristics======
//=============================

//Led strip config
BLECharacteristic *ledCharacteristic_speed;
BLECharacteristic *ledCharacteristic_update;
BLECharacteristic *ledCharacteristic_cycleMode;
BLECharacteristic *ledCharacteristic_direction;
BLECharacteristic *ledCharacteristic_brightness;
BLECharacteristic *ledCharacteristic_amountColors;
BLECharacteristic *ledCharacteristic_positions;
BLECharacteristic *ledCharacteristic_red;
BLECharacteristic *ledCharacteristic_green;
BLECharacteristic *ledCharacteristic_blue;
BLECharacteristic *ledCharacteristic_errorMsg;
BLECharacteristic *ledCharacteristic_indexPalette;

//playlist config
BLECharacteristic *playlistCharacteristic_name;
BLECharacteristic *playlistCharacteristic_pathAmount;
BLECharacteristic *playlistCharacteristic_pathName;
BLECharacteristic *playlistCharacteristic_pathPosition;
BLECharacteristic *playlistCharacteristic_readPlaylistFlag;
BLECharacteristic *playlistCharacteristic_readPath;
BLECharacteristic *playlistCharacteristic_addPath;
BLECharacteristic *playlistCharacteristic_mode;
BLECharacteristic *playlistCharacteristic_progress;
BLECharacteristic *playlistCharacteristic_errorMsg;

//Files
BLECharacteristic *fileCharacteristic_receiveFlag;
BLECharacteristic *fileCharacteristic_receive;
BLECharacteristic *fileCharacteristic_exists;
BLECharacteristic *fileCharacteristic_delete;
BLECharacteristic *fileCharacteristic_sendFlag;
BLECharacteristic *fileCharacteristic_send;
BLECharacteristic *fileCharacteristic_errorMsg;     

//====General====
BLECharacteristic *generalCharacteristic_version;
BLECharacteristic *generalCharacteristic_name;
BLECharacteristic *generalCharacteristic_status;
BLECharacteristic *generalCharacteristic_pause;
BLECharacteristic *generalCharacteristic_play;
BLECharacteristic *generalCharacteristic_sleep;
BLECharacteristic *generalCharacteristic_speed;
BLECharacteristic *generalCharacteristic_restart;
BLECharacteristic *generalCharacteristic_factoryReset;
BLECharacteristic *generalCharacteristic_errorMsg;

//================================Callbacks=============================
//======================================================================

class bleServerCallback : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer){
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("BLE Server connected to: ");
            Serial.println(pServer->getConnId());
            Serial.println();
        #endif
        Sandsara.setSpeed(SPEED_WHEN_IS_CONNECTED_TO_BLE);
        romSetSpeedMotor(SPEED_WHEN_IS_CONNECTED_TO_BLE);
        speedChangedMain = true;
    }

    void onDisconnect(BLEServer *pServer){
        #ifdef DEBUGGING_BLUETOOTH
            Serial.println("BLE Server disconnected");
        #endif
        int speed = String(generalCharacteristic_speed->getValue().c_str()).toInt();
        speed = map(speed,MIN_SLIDER_MSPEED,MAX_SLIDER_MSPEED,MIN_SPEED_MOTOR,MAX_SPEED_MOTOR);
        if (speed > MAX_SPEED_MOTOR || speed < MIN_SPEED_MOTOR){
            speed = SPEED_MOTOR_DEFAULT;
        }
        Sandsara.setSpeed(speed);
        romSetSpeedMotor(speed);
        speedChangedMain = true;
    }
};

class speedLedCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int periodLed = value.toInt();
        periodLed = map(periodLed, MIN_SLIDER_LEDSPEED,MAX_SLIDER_LEDSPEED,MAX_PERIOD_LED,MIN_PERIOD_LED);
        
        if(periodLed < MIN_PERIOD_LED || periodLed > MAX_PERIOD_LED){
            ledCharacteristic_errorMsg->setValue("error = -70");
            ledCharacteristic_errorMsg->notify();
            return;
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE ledSpeed: ");
            Serial.println(periodLed);
        #endif
        periodLedsGlobal = periodLed;
        delayLeds = periodLed;
        romSetPeriodLed(periodLedsGlobal);
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite
    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ ledSpeed: ");
            Serial.println(value);
        }
    #endif
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
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE cycleMode: ");
            Serial.println(incrementIndexGlobal);
        #endif
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ cycleMode: ");
            Serial.println(value);
        }
    #endif
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
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("WRITE direction: ");
                Serial.println("true");
            #endif
        }
        else{
            romSetLedsDirection(false);
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("WRITE direction: ");
                Serial.println("false");
            #endif
        }
        
        ledsDirection = romGetLedsDirection();
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ ledDirection: ");
            Serial.println(value);
        }
    #endif
};

class setBrightnessCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        //retorna ponteiro para o registrador contendo o valor atual da caracteristica
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int brightness = value.toInt();
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE brightness: ");
            Serial.println(brightness);
        #endif
        brightness = map(brightness,MIN_SLIDER_BRIGHTNESS,MAX_SLIDER_BRIGHTNESS,0,255);
        
        if(brightness < 0 || brightness > 255){
            characteristic->setValue(String(romGetBrightness()).c_str());
            ledCharacteristic_errorMsg->setValue("error = -1");
            ledCharacteristic_errorMsg->notify();
            return;
        }
        
        FastLED.setBrightness(brightness);
        romSetBrightness(brightness);
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ brightness: ");
            Serial.println(value);
        }
    #endif
};

class selectedPaletteCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int valueInt = value.toInt();
        if(valueInt < MIN_PALLETE || valueInt > MAX_PALLETE){
            ledCharacteristic_errorMsg->setValue("error = -31");
            ledCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("WRITE presetPalette ERROR");
            #endif
            return;
        }
        
        ledModeGlobal = valueInt;
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE presetPalete: ");
            Serial.println(ledModeGlobal);
        #endif
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ presetPalette: ");
            Serial.println(value);
        }
    #endif
};

class CallbacksToUpdate : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        //=====
        int sizeData = rxValue.length();
        #ifdef DEBUGGING_DATA
            Serial.print("data size: ");
            Serial.println(sizeData);
        #endif
        if (sizeData <= 0){
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("Write null data for palette");
            #endif
            ledCharacteristic_errorMsg->setValue("error= -1"); //null data
            ledCharacteristic_errorMsg->notify();
            return;
        }
        char data[sizeData+1];
        rxValue.copy(data,sizeData,0);
        Serial.print("data 0x ");
        for (int i = 0; i < sizeData; i++){
            Serial.print(data[i],HEX);
            Serial.print("-");
        }
        Serial.println("");
        int n = data[0]; //amount of colors
        //check if the amuont of colors is valid
        if (n < 2 || n > 16){
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("not a valid number of colors");
            #endif
            ledCharacteristic_errorMsg->setValue("error= -2"); //not a valid number of colors
            ledCharacteristic_errorMsg->notify();
            return;}
        // check for incomplete data
        if (n*4 != sizeData - 1){
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("WRITE incomplete data");
            #endif
            ledCharacteristic_errorMsg->setValue("error= -3"); //incomplete data
            ledCharacteristic_errorMsg->notify();
            return;
        }

        uint8_t positions[n];
        uint8_t red[n];
        uint8_t green[n];
        uint8_t blue[n];
        
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("Positions: ");
        #endif
        for(int i=0; i<n; i++){
            positions[i] = data[i + 1];
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print(positions[i]);
                Serial.print(",");
            #endif
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("\nred: ");
        #endif
        for(int i=0; i<n; i++){
            red[i] = data[n + i + 1];
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print(red[i]);
                Serial.print(",");
            #endif
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("\ngreen: ");
        #endif
        for(int i=0; i<n; i++){
            green[i] = data[2*n + i + 1];
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print(green[i]);
                Serial.print(",");
            #endif
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("\nblue: ");
        #endif
        for(int i=0; i<n; i++){
            blue[i] = data[3*n + i + 1];
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print(blue[i]);
                Serial.print(",");
            #endif
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.println("");
        #endif
        //=====
        // int amountOfColors = String(ledCharacteristic_amountColors->getValue().c_str()).toInt();
        // uint8_t positions[amountOfColors];
        // uint8_t red[amountOfColors];
        // uint8_t green[amountOfColors];
        // uint8_t blue[amountOfColors];
        // String positionsString = ledCharacteristic_positions->getValue().c_str();
        // String redString = ledCharacteristic_red->getValue().c_str();
        // String greenString = ledCharacteristic_green->getValue().c_str();
        // String blueString = ledCharacteristic_blue->getValue().c_str();
        // #ifdef DEBUGGING_BLUETOOTH
        //     Serial.println(positionsString);
        //     Serial.println(redString);
        //     Serial.println(greenString);
        //     Serial.println(blueString);
        // #endif
        // if (amountOfColors < 2 || amountOfColors > 16){
        //     ledCharacteristic_errorMsg->setValue("error= -181");
        //     ledCharacteristic_errorMsg->notify();
        //     return;}
        //if (stringToArray(positionsString, positions, amountOfColors) < 0){
        //     ledCharacteristic_errorMsg->setValue("error= -182");
        //     ledCharacteristic_errorMsg->notify();
        //     return;}
        // if (stringToArray(redString, red, amountOfColors) < 0){
        //     ledCharacteristic_errorMsg->setValue("error= -183");
        //     ledCharacteristic_errorMsg->notify();
        //     return;}
        // if (stringToArray(greenString, green, amountOfColors) < 0){
        //     ledCharacteristic_errorMsg->setValue("error= -184");
        //     ledCharacteristic_errorMsg->notify();
        //     return;}
        // if (stringToArray(blueString, blue, amountOfColors) < 0){
        //     ledCharacteristic_errorMsg->setValue("error= -185");
        //     ledCharacteristic_errorMsg->notify();
        //     return;}

        romSetCustomPallete(positions, red, green, blue, n);
        Bluetooth::setRed();
        Bluetooth::setGreen();
        Bluetooth::setBlue();
        Bluetooth::setPositions();
        Bluetooth::setAmountOfColors();

        ledModeGlobal = 16;
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
        ledCharacteristic_errorMsg->notify();
        #ifdef DEBUGGING_BLUETOOTH
            Serial.println("Custom palette was updated");
        #endif
        ledCharacteristic_errorMsg->setValue("ok");
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ toUpdate: ");
            Serial.println(value);
        }
    #endif
};

class genericCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        //retorna ponteiro para o registrador contendo o valor atual da caracteristica
        std::string rxValue = characteristic->getValue();
        std::string uuid = characteristic->getUUID().toString();
        String value = rxValue.c_str();
        #ifdef DEBUGGING_BLUETOOTH
            for (int i = 0; i < uuid.length(); i++)
            {
                Serial.print(uuid[i]);
            }
        #endif
        //verifica se existe dados (tamanho maior que zero)
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print(" Value was changed to: ");
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
            }
            Serial.println();
        #endif
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string uuid = characteristic->getUUID().toString();
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ ");
            String uuidString = characteristic->getUUID().toString().c_str();
            if (uuidString.equals(CHARACTERISTIC_UUID_AMOUNTCOLORS)){
                Serial.print("READ amountColors: ");
                Serial.println(value);
            }
            else if(uuidString.equals(CHARACTERISTIC_UUID_POSITIONS)){
                Serial.print("READ positions: ");
                Serial.println(value);
            }
            else if(uuidString.equals(CHARACTERISTIC_UUID_RED)){
                Serial.print("READ red: ");
                Serial.println(value);
            }
            else if(uuidString.equals(CHARACTERISTIC_UUID_GREEN)){
                Serial.print("READ green: ");
                Serial.println(value);
            }
            else if(uuidString.equals(CHARACTERISTIC_UUID_BLUE)){
                Serial.print("READ blue: ");
                Serial.println(value);
            }
            else{
                Serial.print("READ ");
                Serial.print(uuidString);
                Serial.print(": ");
                Serial.println(value);
            }
        }
    #endif
};

//====================callbacks for playlist config==============================================
//==========================================================================================================
class playlistCallbacks_name : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        String playList = value + ".playlist";
        if (!sdExists(playList)){
            playlistCharacteristic_errorMsg->setValue("error= -1");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        playListGlobal = "/" + playList;
        romSetPlaylist(playListGlobal);
        orderModeGlobal = 1;
        romSetOrderMode(orderModeGlobal);
        rewindPlaylist = true;
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE playlist: ");
            Serial.println(playListGlobal);
        #endif
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ playlistName: ");
            Serial.println(value);
        }
    #endif
};

class playlistCallbacks_pathName : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String name = rxValue.c_str();
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE pathName: ");
            Serial.println(name);
        #endif
        if (SdFiles::getType(name) < 0){
            playlistCharacteristic_errorMsg->setValue("error= -2");
            playlistCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("Nombre incorrecto");
            #endif
            return;
        }
        if (!sdExists(name)){
            playlistCharacteristic_errorMsg->setValue("error= -3");
            playlistCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("no existe el archivo");
            #endif
            return;
        }
        changedProgram = name;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
        changeProgram = true;
        changePositionList = false;
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ pathName: ");
            Serial.println(value);
        }
    #endif
};

class playlistCallbacks_pathPosition : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int position = value.toInt();
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE pathPosition: ");
            Serial.println(position);
        #endif
        
        changedPosition = position;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
        changePositionList = true;
        changeProgram = false;
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ pathPosition: ");
            Serial.println(value);
        }
    #endif
};

class playlistCallbacks_addPath : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String pathName = rxValue.c_str();
        while (readingSDFile){
            delay(1);
        }
        readingSDFile = true;
        File file = SD.open(playListGlobal, FILE_WRITE);
        if (!file){
            playlistCharacteristic_errorMsg->setValue("error= -4");
            playlistCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("no existe el path en la SD");
            #endif
            return; 
        }
        pathName = "\r\n" + pathName;
        file.print(pathName);
        file.close();
        readingSDFile = false;
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("path name added: ");
            Serial.println(pathName);
        #endif
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ addPath: ");
            Serial.println(value);
        }
    #endif
};

class playlistCallbacks_mode : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int mode = value.toInt();
        if(mode < MIN_REPRODUCTION_MODE || mode > MAX_REPRODUCTION_MODE){
            characteristic->setValue(String(orderModeGlobal).c_str());
            playlistCharacteristic_errorMsg->setValue("error= -5");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        orderModeGlobal = mode;
        romSetOrderMode(orderModeGlobal);
        rewindPlaylist = true;
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE playlistMode: ");
            Serial.println(mode);
        #endif
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
    } //onWrite

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ playlistMode: ");
            Serial.println(value);
        }
    #endif
};

//================================Sending files=====================================
//==================================================================================
class FilesCallbacks_receiveFlag : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!receiveFlag){
            std::string rxData = characteristic->getValue();
            String name = rxData.c_str();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("Inicia recepcion de archivo: ");
                Serial.println(name);
            #endif
            while (readingSDFile){
                delay(1);
            }
            readingSDFile = true;
            if (sdExists(name)){
                //fileCharacteristic_errorMsg->setValue("error= -1"); //archivo ya existe
                //fileCharacteristic_errorMsg->notify();
                sdRemove(name);
                //readingSDFile = false;
                #ifdef DEBUGGING_BLUETOOTH
                    Serial.println("se borro el archivo para poder ser recibido");
                #endif
            }
            fileReceive = SD.open(name, FILE_WRITE);
            if (!fileReceive)
            {
                fileCharacteristic_errorMsg->setValue("error= -2"); //file cannot be opened 
                fileCharacteristic_errorMsg->notify();
                #ifdef DEBUGGING_BLUETOOTH
                    Serial.println("no se pudo abrir el archivo"); //file cannot be opened
                #endif
                readingSDFile = false;
                return;
            }
            #ifdef DEBUGGING_BLUETOOTH
                //Serial.println("recibiendo bytes...");
            #endif
            pointerB = buffer;
            bufferSize = 0;
            receiveFlag = true;
            pauseModeGlobal = true;
            readingSDFile = false;
            fileCharacteristic_errorMsg->setValue("ok");
            fileCharacteristic_errorMsg->notify();
        }
        else{
            pointerB = buffer;
            while (readingSDFile){
                delay(1);
            }
            readingSDFile = true;
            fileReceive.write(buffer, bufferSize);
            readingSDFile = false;
            #ifdef DEBUGGING_BLUETOOTH
                //Serial.print("ultimo buffer size: ");
                //Serial.println(bufferSize);
            #endif
            bufferSize = 0;
            fileReceive.close();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("termino archivo");
            #endif
            receiveFlag = false;
            pauseModeGlobal = false;
            fileCharacteristic_errorMsg->setValue("done");
            fileCharacteristic_errorMsg->notify();
        }
    }
};

class FilesCallbacks_receive : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (receiveFlag)
        {
            std::string rxData = characteristic->getValue();
            int len = rxData.length();
            if (len > 0)
            {
                size_t lens = rxData.copy(pointerB, len, 0);
                pointerB = pointerB + lens;
                bufferSize += lens;
                if (bufferSize >= 9000){
                    pointerB = buffer;
                    while (readingSDFile){
                        delay(1);
                    }
                    readingSDFile = true;
                    fileReceive.write(buffer, bufferSize);
                    readingSDFile = false;
                    bufferSize = 0;
                }
                characteristic->setValue("1");
                characteristic->notify();
            }
            
        }
    }
};

class FilesCallbacks_sendFlag : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!sendFlag){
            std::string rxData = characteristic->getValue();
            
            String name = rxData.c_str();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("Begin sending... ");
                Serial.println(name);
            #endif
            while (readingSDFile){
                delay(1);
            }
            readingSDFile = true;
            if (!sdExists(name)){
                #ifdef DEBUGGING_BLUETOOTH
                    Serial.println("no existe el archivo");
                #endif
                fileCharacteristic_errorMsg->setValue("error= -1"); //archivo no existe
                fileCharacteristic_errorMsg->notify();
                return;
            }
            fileSend = SD.open(name, FILE_READ);
            if (!fileSend)
            {
                fileCharacteristic_errorMsg->setValue("error= -2"); //file cannot be opened
                fileCharacteristic_errorMsg->notify();
                #ifdef DEBUGGING_BLUETOOTH
                    Serial.println("error al abrir el archivo"); //file cannot be opened
                #endif
                return;
            }
            readingSDFile = false;
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("Enviando archivo...");
            #endif
            /*uint8_t data[CHUNKSIZE];
            int dataSize = fileSend.read(data, CHUNKSIZE);
            readingSDFile = false;
            fileCharacteristic_send->setValue(data,dataSize);
            //fileCharacteristic_send->notify();*/

            sendFlag = true;
            //pauseModeGlobal = true;
            
            fileCharacteristic_errorMsg->setValue("ok");
            fileCharacteristic_errorMsg->notify();
        }
        else{
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("sendflag puesta a false");
            #endif
            sendFlag = false;
            fileSend.close();
        }
    }
};

class FilesCallbacks_send : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *characteristic)
    {
        if (sendFlag)
        {
            uint8_t data[CHUNKSIZE];
            while (readingSDFile){
                delay(1);
            }
            readingSDFile = true;
            int dataSize = fileSend.read(data, CHUNKSIZE);
            readingSDFile = false;
            if (dataSize == 0){
                characteristic->setValue("");
            }else {
                characteristic->setValue(data,dataSize);
            }
            if(dataSize < CHUNKSIZE){
                fileSend.close();
                sendFlag = false;
                #ifdef DEBUGGING_BLUETOOTH
                    Serial.println("done");
                #endif
            }
        }
    }

};

class FilesCallbacks_checkFile : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxData = characteristic->getValue();
        String filename = rxData.c_str();

        if (!sdExists(filename)){
            if (filename.indexOf(FILENAME_BASE) == 0){
                int indexDot = filename.indexOf(".");
                String nameBase = filename.substring(0,indexDot);
                if (indexDot < 0){
                    #ifdef DEBUGGING_BLUETOOTH
                        Serial.print("no existe el archivo: ");
                        Serial.println(filename);
                    #endif
                    fileCharacteristic_errorMsg->setValue("0");
                    fileCharacteristic_errorMsg->notify();
                    return;
                }
                if (sdExists(nameBase + ".bin")){
                    #ifdef DEBUGGING_BLUETOOTH
                        Serial.print("Si existe el archivo pero con extension .bin: ");
                        Serial.println(filename);
                    #endif
                    fileCharacteristic_errorMsg->setValue("1");
                    fileCharacteristic_errorMsg->notify();
                    return;
                }
                if (sdExists(nameBase + ".txt")){
                    #ifdef DEBUGGING_BLUETOOTH
                        Serial.print("Si existe el archivo pero con extension .txt: ");
                        Serial.println(filename);
                    #endif
                    fileCharacteristic_errorMsg->setValue("1");
                    fileCharacteristic_errorMsg->notify();
                    return;
                }
                if (sdExists(nameBase + ".thr")){
                    #ifdef DEBUGGING_BLUETOOTH
                        Serial.print("Si existe el archivo pero con extension .thr: ");
                        Serial.println(filename);
                    #endif
                    fileCharacteristic_errorMsg->setValue("1");
                    fileCharacteristic_errorMsg->notify();
                    return;
                }
            }
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("no existe el archivo: ");
                Serial.println(filename);
            #endif
            fileCharacteristic_errorMsg->setValue("0");
            fileCharacteristic_errorMsg->notify();
            return;
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("Si existe el archivo : ");
            Serial.println(filename);
        #endif
        fileCharacteristic_errorMsg->setValue("1");
        fileCharacteristic_errorMsg->notify();
    }

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ checkFile: ");
            Serial.println(value);
        }
    #endif
};

class FilesCallbacks_deleteFile : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxData = characteristic->getValue();
        String filename = rxData.c_str();

        if (!sdRemove(filename)){
            #ifdef DEBUGGING_BLUETOOTH
                Serial.print("no se pudo eliminar el archivo : ");
                Serial.println(filename);
            #endif
            fileCharacteristic_errorMsg->setValue("0");
            fileCharacteristic_errorMsg->notify();
            return;
        }
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("se elimino el archivo : ");
            Serial.println(filename);
        #endif
        fileCharacteristic_errorMsg->setValue("1");
        fileCharacteristic_errorMsg->notify();
    }
    
    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ deleteFile: ");
            Serial.println(value);
        }
    #endif
};

//===========================Callbacks for general config==========================
//=================================================================================

class generalCallbacks_name : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxData = characteristic->getValue();
        String bluetoothName = rxData.c_str();
        if (bluetoothName.length() > MAX_CHARACTERS_BTNAME){
            generalCharacteristic_errorMsg->setValue("error=  -1");
            generalCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("Numero maximo de characteres execedido");
            #endif
            return;
        }
        bluetoothNameGlobal = bluetoothName;
        romSetBluetoothName(bluetoothNameGlobal);
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE sandsaraName: ");
            Serial.println(bluetoothName);
        #endif
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
    }

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ SandsaraName: ");
            Serial.println(value);
        }
    #endif
};

class generalCallbacks_pause : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        pauseModeGlobal = true;
        Bluetooth::setStatus(MODE_PAUSE);
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE pause");
        #endif
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
    }

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ pause: ");
            Serial.println(value);
        }
    #endif
};

class generalCallbacks_play : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        pauseModeGlobal = false;
        suspensionModeGlobal = false;
        Bluetooth::setStatus(MODE_PLAY);
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE play");
        #endif
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
    }

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ play: ");
            Serial.println(value);
        }
    #endif
};

class generalCallbacks_Sleep : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        suspensionModeGlobal = true;
        pauseModeGlobal = false;
        Bluetooth::setStatus(MODE_SLEEP);
        #ifdef DEBUGGING_BLUETOOTH
            Serial.println("WRITE sleep");
        #endif
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
    }
    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.println("READ Sleep: ");
            Serial.println(value);
        }
    #endif
};

class generalCallbacks_speed : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxData = characteristic->getValue();
        String value = rxData.c_str();
        int speed = value.toInt();
        #ifdef DEBUGGING_BLUETOOTH
            Serial.print("WRITE speedball : ");
            Serial.println(speed);
        #endif
        //remap the speed acoording to the range of the ball speed
        speed = map(speed,MIN_SLIDER_MSPEED,MAX_SLIDER_MSPEED,MIN_SPEED_MOTOR,MAX_SPEED_MOTOR);
        if (speed > MAX_SPEED_MOTOR || speed < MIN_SPEED_MOTOR){
            generalCharacteristic_errorMsg->setValue("error= -2");
            generalCharacteristic_errorMsg->notify();
            #ifdef DEBUGGING_BLUETOOTH
                Serial.println("ballSpeed out of range");
            #endif
            return;
        }
        // Sandsara.setSpeed(speed);
        // romSetSpeedMotor(speed);
        // speedChangedMain = true;
        
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
    }

    #ifdef DEBUGGING_BLUETOOTH
        void onRead(BLECharacteristic *characteristic)
        {
            std::string rxValue = characteristic->getValue();
            String value = rxValue.c_str();
            Serial.print("READ speed: ");
            Serial.println(value);
        }
    #endif
};


class generalCallbacks_restart : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
        rebootWithMessage("Reiniciando por medio de callback restart");
    }
};

class generalCallbacks_factoryReset : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        for (int i = 0; i < 512; i++){
            EEPROM.write(i, -1);
        }
        EEPROM.commit();
        generalCharacteristic_errorMsg->setValue("ok");
        generalCharacteristic_errorMsg->notify();
        delay(1000);
        rebootWithMessage("Se hiso reset de fabrica, Reiniciando...");
    }
};

Bluetooth::Bluetooth(){

}

int Bluetooth::init(String name){
    BLEDevice::init(name.c_str());
    BLEServer *pServer = BLEDevice::createServer();    
    BLEService *pServiceLedConfig = pServer->createService(BLEUUID(SERVICE_UUID1), 40);
    //BLEService *pServicePath = pServer->createService(BLEUUID(SERVICE_UUID2), 30);
    //BLEService *pServiceSphere = pServer->createService(BLEUUID(SERVICE_UUID3), 30);
    BLEService *pServicePlaylist = pServer->createService(BLEUUID(SERVICE_UUID4), 30);
    BLEService *pServiceGeneralConfig = pServer->createService(BLEUUID(SERVICE_UUID5), 30);
    BLEService *pServiceFile = pServer->createService(BLEUUID(SERVICE_UUID6), 30);
    pServer->setCallbacks(new bleServerCallback());
    //====Characteristics for LEDs configuration====
    
    ledCharacteristic_indexPalette = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_SELECTEDPALETTE,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_speed = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_LEDSPEED,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_cycleMode = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_CYCLEMODE,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_direction = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_DIRECTION,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_brightness = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_BRIGHTNESS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_amountColors = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_AMOUNTCOLORS,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ);
    ledCharacteristic_positions = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_POSITIONS,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ);
    ledCharacteristic_red = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_RED,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ);
    ledCharacteristic_green = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_GREEN,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ);
    ledCharacteristic_blue = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_BLUE,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ);
    ledCharacteristic_update = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_UPDATECPALETTE,
        BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_errorMsg = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_MSGERRORLEDS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    ledCharacteristic_errorMsg->addDescriptor(new BLE2902());
    
    ledCharacteristic_speed->setCallbacks(new speedLedCallbacks());
    ledCharacteristic_cycleMode->setCallbacks(new cycleModeCallbacks());
    ledCharacteristic_direction->setCallbacks(new directionCallbacks());
    ledCharacteristic_brightness->setCallbacks(new setBrightnessCallbacks());
    ledCharacteristic_amountColors->setCallbacks(new genericCallbacks());
    ledCharacteristic_positions->setCallbacks(new genericCallbacks);
    ledCharacteristic_red->setCallbacks(new genericCallbacks);
    ledCharacteristic_green->setCallbacks(new genericCallbacks);
    ledCharacteristic_blue->setCallbacks(new genericCallbacks);
    ledCharacteristic_update->setCallbacks(new CallbacksToUpdate());
    ledCharacteristic_indexPalette->setCallbacks(new selectedPaletteCallbacks());
    setLedSpeed(periodLedsGlobal);
    if(romGetIncrementIndexPallete()){
        ledCharacteristic_cycleMode->setValue("1");}
    else{
        ledCharacteristic_cycleMode->setValue("0");}
    if(romGetLedsDirection()){
        ledCharacteristic_direction->setValue("1");}
    else{
        ledCharacteristic_direction->setValue("0");}
    ledCharacteristic_indexPalette->setValue(String(romGetPallete()).c_str());

    setRed();
    setGreen();
    setBlue();
    setPositions();
    setAmountOfColors();

    //====Characteristics for playlist configuration====
    playlistCharacteristic_name = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_NAME,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_pathAmount = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_PATHAMOUNT,
        BLECharacteristic::PROPERTY_READ);

    playlistCharacteristic_pathName = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_PATHNAME,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_pathPosition = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_PATHPOSITION,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_addPath = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_ADDPATH,
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_mode = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_MODE,
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_progress = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_PATHPROGRESS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    playlistCharacteristic_progress->addDescriptor(new BLE2902());
    
    playlistCharacteristic_errorMsg = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_ERRORMSG,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    playlistCharacteristic_errorMsg->addDescriptor(new BLE2902());

    playlistCharacteristic_name->setCallbacks(new playlistCallbacks_name());
    playlistCharacteristic_pathName->setCallbacks(new playlistCallbacks_pathName());
    playlistCharacteristic_pathPosition->setCallbacks(new playlistCallbacks_pathPosition());
    playlistCharacteristic_addPath->setCallbacks(new playlistCallbacks_addPath());
    playlistCharacteristic_mode->setCallbacks(new playlistCallbacks_mode());

    //====Characteristics for General configuration====
    generalCharacteristic_version = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_VERSION,
        BLECharacteristic::PROPERTY_READ);

    generalCharacteristic_name = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_NAME,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_status= pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_STATUS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    generalCharacteristic_status->addDescriptor(new BLE2902());

    generalCharacteristic_pause = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_PAUSE,
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_play = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_PLAY,
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_sleep = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_SLEEP,
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_speed = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_SPEED,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_restart = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_RESTART,
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_factoryReset = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_FACTORYRESET,
            BLECharacteristic::PROPERTY_WRITE);

    generalCharacteristic_errorMsg = pServiceGeneralConfig->createCharacteristic(
        GENERAL_UUID_ERRORMSG,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    generalCharacteristic_errorMsg->addDescriptor(new BLE2902());

    generalCharacteristic_name->setCallbacks(new generalCallbacks_name());
    generalCharacteristic_pause->setCallbacks(new generalCallbacks_pause());
    generalCharacteristic_play->setCallbacks(new generalCallbacks_play());
    generalCharacteristic_sleep->setCallbacks(new generalCallbacks_Sleep());
    generalCharacteristic_speed->setCallbacks(new generalCallbacks_speed());
    generalCharacteristic_restart->setCallbacks(new generalCallbacks_restart());
    generalCharacteristic_factoryReset->setCallbacks(new generalCallbacks_factoryReset());

    //====Characteristics for File configuration====
    fileCharacteristic_receiveFlag = pServiceFile->createCharacteristic(
        FILE_UUID_RECEIVEFLAG,
            BLECharacteristic::PROPERTY_WRITE);

    fileCharacteristic_receive = pServiceFile->createCharacteristic(
        FILE_UUID_RECEIVE,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);
    fileCharacteristic_receive->addDescriptor(new BLE2902());

    fileCharacteristic_exists = pServiceFile->createCharacteristic(
        FILE_UUID_EXISTS,
            BLECharacteristic::PROPERTY_WRITE);

    fileCharacteristic_delete = pServiceFile->createCharacteristic(
        FILE_UUID_DELETE,
            BLECharacteristic::PROPERTY_WRITE);

    fileCharacteristic_sendFlag = pServiceFile->createCharacteristic(
        FILE_UUID_SENDFLAG,
            BLECharacteristic::PROPERTY_WRITE);

    fileCharacteristic_send = pServiceFile->createCharacteristic(
        FILE_UUID_SEND,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    fileCharacteristic_send->addDescriptor(new BLE2902());

    fileCharacteristic_errorMsg = pServiceFile->createCharacteristic(
        FILE_UUID_ERRORMSG,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    fileCharacteristic_errorMsg->addDescriptor(new BLE2902());

    fileCharacteristic_receiveFlag->setCallbacks(new FilesCallbacks_receiveFlag());
    fileCharacteristic_receive->setCallbacks(new FilesCallbacks_receive());
    fileCharacteristic_exists->setCallbacks(new FilesCallbacks_checkFile());
    fileCharacteristic_delete->setCallbacks(new FilesCallbacks_deleteFile());
    fileCharacteristic_send->setCallbacks(new FilesCallbacks_send());
    fileCharacteristic_sendFlag->setCallbacks(new FilesCallbacks_sendFlag());

    //ledCharacteristic_speed->addDescriptor(new BLE2902());
    
    pServiceLedConfig->start();
    pServicePlaylist->start();
    pServiceGeneralConfig->start();
    pServiceFile->start();
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
    #ifdef DEBUGGING_DATA
        Serial.println("BLE is anable");
    #endif
    
    return 0;
}

/**
 * Las siguientes funciones se encargan de calcular el hash por el metodo de MD5.
 */
typedef union uwb {
    unsigned w;
    unsigned char b[4];
} MD5union;

typedef unsigned DigestArray[4];

static unsigned func0(unsigned abcd[])
{
    return (abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);
}

static unsigned func1(unsigned abcd[])
{
    return (abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);
}

static unsigned func2(unsigned abcd[])
{
    return abcd[1] ^ abcd[2] ^ abcd[3];
}

static unsigned func3(unsigned abcd[])
{
    return abcd[2] ^ (abcd[1] | ~abcd[3]);
}

typedef unsigned (*DgstFctn)(unsigned a[]);

static unsigned *calctable(unsigned *k)
{
    double s, pwr;
    int i;

    pwr = pow(2.0, 32);
    for (i = 0; i < 64; i++)
    {
        s = fabs(sin(1.0 + i));
        k[i] = (unsigned)(s * pwr);
    }
    return k;
}

static unsigned rol(unsigned r, short N)
{
    unsigned mask1 = (1 << N) - 1;
    return ((r >> (32 - N)) & mask1) | ((r << N) & ~mask1);
}

static unsigned *MD5Hash(uint8_t *msg, int mlen)
{
    static DigestArray h0 = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
    static DgstFctn ff[] = {&func0, &func1, &func2, &func3};
    static short M[] = {1, 5, 3, 7};
    static short O[] = {0, 1, 5, 0};
    static short rot0[] = {7, 12, 17, 22};
    static short rot1[] = {5, 9, 14, 20};
    static short rot2[] = {4, 11, 16, 23};
    static short rot3[] = {6, 10, 15, 21};
    static short *rots[] = {rot0, rot1, rot2, rot3};
    static unsigned kspace[64];
    static unsigned *k;

    static DigestArray h;
    DigestArray abcd;
    DgstFctn fctn;
    short m, o, g;
    unsigned f;
    short *rotn;
    union {
        unsigned w[16];
        char b[64];
    } mm;
    int os = 0;
    int grp, grps, q, p;
    unsigned char *msg2;

    if (k == NULL)
        k = calctable(kspace);

    for (q = 0; q < 4; q++)
        h[q] = h0[q];

    {
        grps = 1 + (mlen + 8) / 64;
        msg2 = (unsigned char *)malloc(64 * grps);
        memcpy(msg2, (unsigned char *)msg, mlen);
        msg2[mlen] = (unsigned char)0x80;
        q = mlen + 1;
        while (q < 64 * grps)
        {
            msg2[q] = 0;
            q++;
        }
        {
            MD5union u;
            u.w = 8 * mlen;
            q -= 8;
            memcpy(msg2 + q, &u.w, 4);
        }
    }

    for (grp = 0; grp < grps; grp++)
    {
        memcpy(mm.b, msg2 + os, 64);
        for (q = 0; q < 4; q++)
            abcd[q] = h[q];
        for (p = 0; p < 4; p++)
        {
            fctn = ff[p];
            rotn = rots[p];
            m = M[p];
            o = O[p];
            for (q = 0; q < 16; q++)
            {
                g = (m * q + o) % 16;
                f = abcd[1] + rol(abcd[0] + fctn(abcd) + k[q + 16 * p] + mm.w[g], rotn[q % 4]);

                abcd[0] = abcd[3];
                abcd[3] = abcd[2];
                abcd[2] = abcd[1];
                abcd[1] = f;
            }
        }
        for (p = 0; p < 4; p++)
            h[p] += abcd[p];
        os += 64;
    }
    free(msg2); //THIS IS IMPORTANT
    return h;
}

String GetMD5String(uint8_t *msg, int mlen)
{
    String str;
    int j;
    unsigned *d = MD5Hash(msg, mlen);
    MD5union u;
    for (j = 0; j < 4; j++)
    {
        u.w = d[j];
        char s[9];
        sprintf(s, "%02x%02x%02x%02x", u.b[0], u.b[1], u.b[2], u.b[3]);
        str += s;
    }
    return str;
}

/**
 * Hasta aqui terminan las funciones para MD5
 */

/**
 * @brief Actualiza el firmware.
 * @return Uno de los siguientes numeros.
 * -4, No se pudo finalizar la actualizacion
 * -5, No hay suficiente espacio para el OTA.
 * -6, Ocurrio un error al actualizar el firmware
 */
int performUpdate(Stream &updateSource, size_t updateSize)
{
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            #ifdef DEBUGGING_DATA
                Serial.println("Written : " + String(written) + " successfully");
            #endif
        }
        else
        {
            #ifdef DEBUGGING_DATA
                Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
            #endif
            
        }
        if (Update.end())
        {
            Serial.println("OTA done!");
            if (Update.isFinished())
            {
                #ifdef DEBUGGING_DATA
                    Serial.println("Update successfully completed. Rebooting.");
                #endif
                
                return 1;
            }
            else
            {
                #ifdef DEBUGGING_DATA
                    Serial.println("Update not finished? Something went wrong!");
                #endif
                return -4;
            }
        }
        else
        {
            #ifdef DEBUGGING_DATA
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            #endif
            return -6;
        }
    }
    else
    {
        #ifdef DEBUGGING_DATA
            Serial.println("Not enough space to begin OTA");
        #endif
        return -5;
    }
}

/**
 * @brief Revisa si el archivo es valido y si lo es actualiza el firmware
 * @return Uno de los siguientes numeros
 *  1, Se actualizo el firmware
 * -1, Es un directorio
 * -2, El archivo esta vacio
 * -3, El archivo no se pudo abrir.
 * -4, No se pudo finalizar la actualizacion
 * -5, No hay suficiente espacio para el OTA.
 * -6, Ocurrio un error al actualizar el firmware
 */
int updateFromFS(SdFat &fs, String name)
{
    int errorCode;
    File updateBin = fs.open(name);
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            Serial.println("Error, update.bin is not a file");
            updateBin.close();
            return -1;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            Serial.println("Try to start update");
            errorCode = performUpdate(updateBin, updateSize);
            updateBin.close();
            sdRemove(name);
            return errorCode;
        }
        else
        {
            Serial.println("Error, file is empty");
            updateBin.close();
            return -2;
        }

        updateBin.close();
        sdRemove(name);
    }
    else
    {
        Serial.println("Could not load update.bin from sd root");
        return -3;
    }
}

/**
 * @brief Intenta actualizar el firmware
 * @return Un codigo para saber si ocurre un error a la hora de realizar la actualizacion.
 */
int programming(String name){
    int errorCode;
    errorCode = updateFromFS(SD, name);
    return errorCode;
}

/**
 * @brief Reinicia el Esp32 pero antes escribe un mensaje por Serial.
 * @return Un codigo para saber si ocurre un error a la hora de realizar la actualizacion.
 */
void rebootWithMessage(String reason){
    #ifdef DEBUGGING_DATA
        Serial.println(reason);
    #endif
    delay(2000);
    ESP.restart();
}

/**
 * @brief Convierte un string en un array
 * un string de la forma x1,x2,...,xn se convierte en un array [0]=x1, [1]=x1, ...,[n-1]=xn
 * @param str Es el string que se desea convertir.
 * @param array Es el array donde se van a guardar los valores del string
 * @param n Es el numero de elementos que tiene el string
 * @return Uno de los siguientes numeros
 * 0, Todo salio normal.
 * -1, No hay n elementos en el string
 */
int stringToArray(String str, uint8_t* array, int n){
    int i;
    for (i = 0; i<n ; i++){
        if (str.indexOf(",") < 0){
            *(array + i) = str.toInt();
            break;
        }
        *(array + i) = str.substring(0, str.indexOf(",")).toInt();
        str = str.substring(str.indexOf(",") + 1);
    }
    i++;
    if(i != n){
        return -1;
    }
    return 0;
}

void Bluetooth::setPlaylistName(String playlistName){
    if (playlistName.charAt(0) == '/'){
        playlistName.remove(0,1);
    }
    int index = playlistName.lastIndexOf('.');
    if (index > 0){
        playlistName.remove(index);
    }
    playlistCharacteristic_name->setValue(playlistName.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET Playlist Name: ");
        Serial.println(playlistName.c_str());
    #endif
}
void Bluetooth::setPathAmount(int pathAmount){
    playlistCharacteristic_pathAmount->setValue(String(pathAmount).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET pathAmount: ");
        Serial.println(String(pathAmount).c_str());
    #endif
}
void Bluetooth::setPathName(String pathName){
    playlistCharacteristic_pathName->setValue(pathName.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET pathName: ");
        Serial.println(pathName.c_str());
    #endif
}
void Bluetooth::setPathPosition(int pathPosition){
    playlistCharacteristic_pathPosition->setValue(String(pathPosition).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET pathPosition: ");
        Serial.println(String(pathPosition).c_str());
    #endif
}
void Bluetooth::setPlayMode(int mode){
    playlistCharacteristic_mode->setValue(String(mode).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET PlaylistMode: ");
        Serial.println(String(mode).c_str());
    #endif
}
// void Bluetooth::setPathProgress(int progress){
//     playlistCharacteristic_progress->setValue(String(progress).c_str());
//     #ifdef DEBUGGING_BLUETOOTH
//         Serial.print("SET progress: ");
//         Serial.println(String(progress).c_str());
//     #endif
// }


void Bluetooth::setLedSpeed(int speed){
    speed = map(speed,MIN_PERIOD_LED,MAX_PERIOD_LED,MAX_SLIDER_LEDSPEED,MIN_SLIDER_LEDSPEED);
    ledCharacteristic_speed->setValue(String(speed).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET ledSoeed: ");
        Serial.println(String(speed).c_str());
    #endif
}
void Bluetooth::setCycleMode(int cycleMode){
    ledCharacteristic_cycleMode->setValue(String(cycleMode).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET cycleMode: ");
        Serial.println(String(cycleMode).c_str());
    #endif
}
void Bluetooth::setLedDirection(int ledDirection){
    ledCharacteristic_direction->setValue(String(ledDirection).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET ledDirection: ");
        Serial.println(String(ledDirection).c_str());
    #endif
}
void Bluetooth::setBrightness(int brightness){
    brightness = map(brightness,0,255,MIN_SLIDER_BRIGHTNESS,MAX_SLIDER_BRIGHTNESS);
    ledCharacteristic_brightness->setValue(String(brightness).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET brightness: ");
        Serial.println(String(brightness).c_str());
    #endif
}
void Bluetooth::setIndexPalette(int indexPalette){
    ledCharacteristic_indexPalette->setValue(String(indexPalette).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET indexPalette: ");
        Serial.println(String(indexPalette).c_str());
    #endif
}

void Bluetooth::setVersion(String version){
    generalCharacteristic_version->setValue(version.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET version: ");
        Serial.println(version.c_str());
    #endif
}
void Bluetooth::setName(String name){
    generalCharacteristic_name->setValue(name.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET ble name: ");
        Serial.println(name.c_str());
    #endif
}
void Bluetooth::setStatus(int status){
    generalCharacteristic_status->setValue(String(status).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET status: ");
        Serial.println(String(status).c_str());
    #endif
}
void Bluetooth::setMotorSpeed(int speed){
    speed = map(speed,MIN_SPEED_MOTOR,MAX_SPEED_MOTOR,MIN_SLIDER_MSPEED,MAX_SLIDER_MSPEED);
    generalCharacteristic_speed->setValue(String(speed).c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET motor Speed: ");
        Serial.println(String(speed).c_str());
    #endif
}
void Bluetooth::setPercentage(int percentage){
    if (percentage < 0 ){
        percentage = 0;
    }
    if (percentage > 100){
        percentage = 100;
    }
    playlistCharacteristic_progress->setValue(String(percentage).c_str());
    playlistCharacteristic_progress->notify();
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET percentage: ");
        Serial.println(String(percentage).c_str());
    #endif
}

void Bluetooth::setRed(){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    if (numberOfColors > MAX_COLORSPERPALLETE || numberOfColors < 2){
        return;
    }
    uint8_t red[numberOfColors];
    String reds = "";
    for (int i = 0; i < numberOfColors; i++){
        red[i] = EEPROM.read(ADDRESSCUSTOMPALLETE_RED + i);
        reds.concat(red[i]);
        reds.concat(",");
    }
    reds.remove(reds.length() - 1);
    ledCharacteristic_red->setValue(reds.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET red: ");
        Serial.println(reds.c_str());
    #endif
}
void Bluetooth::setGreen(){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    if (numberOfColors > MAX_COLORSPERPALLETE || numberOfColors < 2){
        return;
    }
    uint8_t green[numberOfColors];
    String greens = "";
    for (int i = 0; i < numberOfColors; i++){
        green[i] = EEPROM.read(ADDRESSCUSTOMPALLETE_GREEN + i);
        greens.concat(green[i]);
        greens.concat(",");
    }
    greens.remove(greens.length() - 1);
    ledCharacteristic_green->setValue(greens.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET green: ");
        Serial.println(greens.c_str());
    #endif
}
void Bluetooth::setBlue(){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    if (numberOfColors > MAX_COLORSPERPALLETE || numberOfColors < 2){
        return;
    }
    uint8_t blue[numberOfColors];
    String blues = "";
    for (int i = 0; i < numberOfColors; i++){
        blue[i] = EEPROM.read(ADDRESSCUSTOMPALLETE_BLUE + i);
        blues.concat(blue[i]);
        blues.concat(",");
    }
    blues.remove(blues.length() - 1);
    ledCharacteristic_blue->setValue(blues.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET blue: ");
        Serial.println(blues.c_str());
    #endif
}
void Bluetooth::setPositions(){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    if (numberOfColors > MAX_COLORSPERPALLETE || numberOfColors < 2){
        return;
    }
    uint8_t position[numberOfColors];
    String positions = "";
    for (int i = 0; i < numberOfColors; i++){
        position[i] = EEPROM.read(ADDRESSCUSTOMPALLETE_POSITIONS + i);
        positions.concat(position[i]);
        positions.concat(",");
    }
    positions.remove(positions.length() - 1);
    ledCharacteristic_positions->setValue(positions.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET pathPositions: ");
        Serial.println(positions.c_str());
    #endif
}
void Bluetooth::setAmountOfColors(){
    uint8_t numberOfColors = EEPROM.read(ADDRESSCUSTOMPALLETE_COLORS);
    String amount;
    amount.concat(numberOfColors);
    ledCharacteristic_amountColors->setValue(amount.c_str());
    #ifdef DEBUGGING_BLUETOOTH
        Serial.print("SET amount: ");
        Serial.println(amount.c_str());
    #endif
}