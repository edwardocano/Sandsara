#include "Bluetooth.h"
#include <Update.h>
#include <SdFiles.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

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

//====variables====
extern  String      changedProgram;
extern  int         changedPosition;
bool        sendFlag = false;
char        buffer[10000];
char        *pointerB;
int         bufferSize;
File        fileReceive;

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
BLECharacteristic *ledCharacteristic_amountColors;
BLECharacteristic *ledCharacteristic_positions;
BLECharacteristic *ledCharacteristic_red;
BLECharacteristic *ledCharacteristic_green;
BLECharacteristic *ledCharacteristic_blue;
BLECharacteristic *ledCharacteristic_errorMsg;
BLECharacteristic *ledCharacteristic_indexPalette;

//path config
/*BLECharacteristic *pathCharacteristic_name;
BLECharacteristic *pathCharacteristic_position;
BLECharacteristic *pathCharacteristic_percentage;
BLECharacteristic *pathCharacteristic_errorMsg;*/

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
BLECharacteristic *fileCharacteristic_errorMsg;     

BLECharacteristic *characteristic_1;
BLECharacteristic *characteristic_2;
BLECharacteristic *characteristic_3;
BLECharacteristic *characteristic_4;
BLECharacteristic *characteristic_5;
BLECharacteristic *characteristic_6;
BLECharacteristic *characteristic_7;
BLECharacteristic *characteristic_8;
BLECharacteristic *characteristic_9;

//====Callbacks=============================
//==========================================

class speedLedCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int periodLed = value.toInt();
        if(periodLed < MIN_PERIOD_LED || periodLed > MAX_PERIOD_LED){
            ledCharacteristic_errorMsg->setValue("error = -70");
            ledCharacteristic_errorMsg->notify();
            return;
        }
        Serial.print("speed led was changed to: ");
        Serial.println(periodLed);
        periodLedsGlobal = periodLed;
        delayLeds = periodLed;
        romSetPeriodLed(periodLedsGlobal);
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
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
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
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
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
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
            ledCharacteristic_errorMsg->setValue("error = -31");
            ledCharacteristic_errorMsg->notify();
            return;
        }
        ledModeGlobal = valueInt;
        //verifica se existe dados (tamanho maior que zero)
        changePalette(ledModeGlobal);
        romSetPallete(ledModeGlobal);
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
    } //onWrite
};

class CallbacksToUpdate : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        int amountOfColors = String(ledCharacteristic_amountColors->getValue().c_str()).toInt();
        uint8_t positions[amountOfColors];
        uint8_t red[amountOfColors];
        uint8_t green[amountOfColors];
        uint8_t blue[amountOfColors];
        String positionsString = ledCharacteristic_positions->getValue().c_str();
        String redString = ledCharacteristic_red->getValue().c_str();
        String greenString = ledCharacteristic_green->getValue().c_str();
        String blueString = ledCharacteristic_blue->getValue().c_str();
        Serial.println(positionsString);
        Serial.println(redString);
        Serial.println(greenString);
        Serial.println(blueString);
        if (amountOfColors < 2 || amountOfColors > 16){
            ledCharacteristic_errorMsg->setValue("error= -181");
            ledCharacteristic_errorMsg->notify();
            return;}
        if (stringToArray(positionsString, positions, amountOfColors) < 0){
            ledCharacteristic_errorMsg->setValue("error= -182");
            ledCharacteristic_errorMsg->notify();
            return;}
        if (stringToArray(redString, red, amountOfColors) < 0){
            ledCharacteristic_errorMsg->setValue("error= -183");
            ledCharacteristic_errorMsg->notify();
            return;}
        if (stringToArray(greenString, green, amountOfColors) < 0){
            ledCharacteristic_errorMsg->setValue("error= -184");
            ledCharacteristic_errorMsg->notify();
            return;}
        if (stringToArray(blueString, blue, amountOfColors) < 0){
            ledCharacteristic_errorMsg->setValue("error= -185");
            ledCharacteristic_errorMsg->notify();
            return;}
        romSetCustomPallete(positions, red, green, blue, amountOfColors);
        ledCharacteristic_errorMsg->setValue("ok");
        ledCharacteristic_errorMsg->notify();
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

//===============================callbacks for playlist config==============================================
//==========================================================================================================
class playlistCallbacks_name : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        String playList = value + ".playlist";
        if (!sdExists(playList)){
            playlistCharacteristic_errorMsg->setValue("error=-1");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        playListGlobal = "/" + playList;
        romSetPlaylist(playListGlobal);
        rewindPlaylist = true;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
    } //onWrite
};

class playlistCallbacks_pathName : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String name = rxValue.c_str();
        
        Serial.print("name: ");
        Serial.println(name);
        if (SdFiles::getType(name) < 0){
            playlistCharacteristic_errorMsg->setValue("error= -2");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        if (!sdExists(name)){
            playlistCharacteristic_errorMsg->setValue("error= -3");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        changedProgram = name;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
        changeProgram = true;
        changePositionList = false;
    } //onWrite
};

class playlistCallbacks_pathPosition : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int position = value.toInt();

        Serial.print("position: ");
        Serial.println(position);
        
        changedPosition = position;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
        changePositionList = true;
        changeProgram = false;
    } //onWrite
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
            return;
        }
        pathName = "\r\n" + pathName;
        file.print(pathName);
        file.close();
        readingSDFile = false;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();
    } //onWrite
};

class playlistCallbacks_mode : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxValue = characteristic->getValue();
        String value = rxValue.c_str();
        int mode = value.toInt();
        if(mode < 1 || mode > 4){
            playlistCharacteristic_errorMsg->setValue("error= -5");
            playlistCharacteristic_errorMsg->notify();
            return;
        }
        orderModeGlobal = mode;
        romSetOrderMode(orderModeGlobal);
        rewindPlaylist = true;
        playlistCharacteristic_errorMsg->setValue("ok");
        playlistCharacteristic_errorMsg->notify();

    } //onWrite
};

//================================Sending files=====================================
//==================================================================================
class FilesCallbacks_receiveFlag : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        if (!sendFlag){
            std::string rxData = characteristic->getValue();
            Serial.println("BeginOTA");
            String name = rxData.c_str();
            if (sdExists(name)){
                fileCharacteristic_errorMsg->setValue("error=-1"); //archivo ya existe
                fileCharacteristic_errorMsg->notify();
            }
            fileReceive = SD.open(name, FILE_WRITE);
            if (!fileReceive)
            {
                fileCharacteristic_errorMsg->setValue("error=-2"); //file cannot be opened
                fileCharacteristic_errorMsg->notify();
                Serial.println("error= -2"); //file cannot be opened
            }
            Serial.println("se creo archivo");
            pointerB = buffer;
            bufferSize = 0;
            sendFlag = true;
            pauseModeGlobal = true;
            fileCharacteristic_errorMsg->setValue("ok");
            fileCharacteristic_errorMsg->notify();
        }
        else{
            pointerB = buffer;
            fileReceive.write(buffer, bufferSize);
            Serial.print("buffer size: ");
            Serial.println(bufferSize);
            bufferSize = 0;
            fileReceive.close();
            Serial.println("EndOTA");
            sendFlag = false;
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
        if (sendFlag)
        {
            std::string rxData = characteristic->getValue();
            int len = rxData.length();
            if (len > 0)
            {
                //rxData.c_str();
                size_t lens = rxData.copy(pointerB, len, 0);
                //memcpy(pointerB, rxData.c_str(),);
                pointerB = pointerB + lens;
                //Serial.print(rxData.c_str());
                bufferSize += lens;
                if (bufferSize >= 9000){
                    pointerB = buffer;
                    fileReceive.write(buffer, bufferSize);
                    //Serial.print("buffer size: ");
                    //Serial.println(bufferSize);
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
            Serial.println("Begin sending...");
            String name = rxData.c_str();
            if (!sdExists(name)){
                fileCharacteristic_errorMsg->setValue("error=-1"); //archivo no existe
                fileCharacteristic_errorMsg->notify();
            }
            fileReceive = SD.open(name, FILE_READ);
            if (!fileReceive)
            {
                fileCharacteristic_errorMsg->setValue("error=-2"); //file cannot be opened
                fileCharacteristic_errorMsg->notify();
                Serial.println("error al abrir el archivo"); //file cannot be opened
            }
            Serial.println("se abrio el archivo");
            pointerB = buffer;
            bufferSize = 0;
            sendFlag = true;
            pauseModeGlobal = true;
            fileCharacteristic_errorMsg->setValue("ok");
            fileCharacteristic_errorMsg->notify();
        }
        else{
            pointerB = buffer;
            fileReceive.write(buffer, bufferSize);
            Serial.print("buffer size: ");
            Serial.println(bufferSize);
            bufferSize = 0;
            fileReceive.close();
            Serial.println("EndOTA");
            sendFlag = false;
            pauseModeGlobal = false;
            fileCharacteristic_errorMsg->setValue("done");
            fileCharacteristic_errorMsg->notify();
        }
    }
};

class FilesCallbacks_send : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *characteristic)
    {
        if (sendFlag)
        {
            std::string rxData = characteristic->getValue();
            int len = rxData.length();
            if (len > 0)
            {
                //rxData.c_str();
                size_t lens = rxData.copy(pointerB, len, 0);
                //memcpy(pointerB, rxData.c_str(),);
                pointerB = pointerB + lens;
                //Serial.print(rxData.c_str());
                bufferSize += lens;
                if (bufferSize >= 9000){
                    pointerB = buffer;
                    fileReceive.write(buffer, bufferSize);
                    //Serial.print("buffer size: ");
                    //Serial.println(bufferSize);
                    bufferSize = 0;
                }
                characteristic->setValue("1");
                characteristic->notify();
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
            fileCharacteristic_errorMsg->setValue("0");
            fileCharacteristic_errorMsg->notify();
            return;
        }
        fileCharacteristic_errorMsg->setValue("1");
        fileCharacteristic_errorMsg->notify();
    }
};

class FilesCallbacks_deleteFile : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string rxData = characteristic->getValue();
        String filename = rxData.c_str();

        if (!sdRemove(filename)){
            fileCharacteristic_errorMsg->setValue("0");
            fileCharacteristic_errorMsg->notify();
            return;
        }
        fileCharacteristic_errorMsg->setValue("1");
        fileCharacteristic_errorMsg->notify();
    }
};

Bluetooth::Bluetooth(){

}

int Bluetooth::init(String name){
    BLEDevice::init(name.c_str());
    BLEServer *pServer = BLEDevice::createServer();    
    BLEService *pServiceLedConfig = pServer->createService(BLEUUID(SERVICE_UUID1), 30);
    //BLEService *pServicePath = pServer->createService(BLEUUID(SERVICE_UUID2), 30);
    //BLEService *pServiceSphere = pServer->createService(BLEUUID(SERVICE_UUID3), 30);
    BLEService *pServicePlaylist = pServer->createService(BLEUUID(SERVICE_UUID4), 30);
    BLEService *pServiceGeneralConfig = pServer->createService(BLEUUID(SERVICE_UUID5), 30);
    BLEService *pServiceFile = pServer->createService(BLEUUID(SERVICE_UUID6), 30);

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
    ledCharacteristic_amountColors = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_AMOUNTCOLORS,
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_positions = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_POSITIONS,
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_red = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_RED,
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_green = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_GREEN,
            BLECharacteristic::PROPERTY_WRITE);
    ledCharacteristic_blue = pServiceLedConfig->createCharacteristic(
        CHARACTERISTIC_UUID_BLUE,
            BLECharacteristic::PROPERTY_WRITE);
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
    ledCharacteristic_amountColors->setCallbacks(new genericCallbacks());
    ledCharacteristic_positions->setCallbacks(new genericCallbacks);
    ledCharacteristic_red->setCallbacks(new genericCallbacks);
    ledCharacteristic_green->setCallbacks(new genericCallbacks);
    ledCharacteristic_blue->setCallbacks(new genericCallbacks);
    ledCharacteristic_update->setCallbacks(new CallbacksToUpdate());
    ledCharacteristic_indexPalette->setCallbacks(new selectedPaletteCallbacks());

    ledCharacteristic_speed->setValue(String(periodLedsGlobal).c_str());
    if(romGetIncrementIndexPallete()){
        ledCharacteristic_cycleMode->setValue("1");}
    else{
        ledCharacteristic_cycleMode->setValue("0");}
    if(romGetLedsDirection()){
        ledCharacteristic_direction->setValue("1");}
    else{
        ledCharacteristic_direction->setValue("0");}
    ledCharacteristic_indexPalette->setValue(String(romGetPallete()).c_str());

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
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    playlistCharacteristic_progress = pServicePlaylist->createCharacteristic(
        PLAYLIST_UUID_PATHPROGRESS,
        BLECharacteristic::PROPERTY_READ);

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
    characteristic_5 = pServiceGeneralConfig->createCharacteristic(
        CHARACTERISTIC_UUID_5,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

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

    fileCharacteristic_errorMsg = pServiceFile->createCharacteristic(
        FILE_UUID_ERRORMSG,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    fileCharacteristic_errorMsg->addDescriptor(new BLE2902());
    fileCharacteristic_receiveFlag->setCallbacks(new FilesCallbacks_receiveFlag());
    fileCharacteristic_receive->setCallbacks(new FilesCallbacks_receive());
    fileCharacteristic_exists->setCallbacks(new FilesCallbacks_checkFile());
    fileCharacteristic_delete->setCallbacks(new FilesCallbacks_deleteFile());

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
    Serial.println("Characteristic defined! Now you can read it in your phone!");
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
    delay(1000);
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
    playlistCharacteristic_name->setValue(playlistName.c_str());
}
void Bluetooth::setPathAmount(int pathAmount){
    playlistCharacteristic_pathAmount->setValue(String(pathAmount).c_str());
}
void Bluetooth::setPathName(String pathName){
    playlistCharacteristic_pathName->setValue(pathName.c_str());
}
void Bluetooth::setPathPosition(int pathPosition){
    playlistCharacteristic_pathPosition->setValue(String(pathPosition).c_str());
}
void Bluetooth::setPlayMode(int mode){
    playlistCharacteristic_mode->setValue(String(mode).c_str());
}
void Bluetooth::setPathProgress(int progress){
    playlistCharacteristic_progress->setValue(String(progress).c_str());
}


void Bluetooth::setLedSpeed(int speed){
    ledCharacteristic_speed->setValue(String(speed).c_str());
}
void Bluetooth::setCycleMode(int cycleMode){
    ledCharacteristic_cycleMode->setValue(String(cycleMode).c_str());
}
void Bluetooth::setLedDirection(int ledDirection){
    ledCharacteristic_direction->setValue(String(ledDirection).c_str());
}
void Bluetooth::setIndexPalette(int indexPalette){
    ledCharacteristic_indexPalette->setValue(String(indexPalette).c_str());
}
