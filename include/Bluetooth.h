#pragma once
//#define BLUECOMMENTS

#include "config.h"
#include <SPI.h>
#include "SdFat.h"
#include "BluetoothSerial.h"
#include "esp_bt_device.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
/**
 * @class Bluetooth es la clase que se encarga de manejar el bluetooth
 * @param timeOutBt es el tiempo, en milisegundos, que se va a esperar para recibir respuesta del dispositivo bluetooth conectado.
 * @param dataBt es donde se va a almacenar la informacion recibida.
 */
class Bluetooth {
    private:
        BluetoothSerial SerialBT;
        File file;
        unsigned long timeOutBt = OUTOFTIME_BLUETOOTH;
        //int debugCount = 0;
        //int codeErrorC;
        int indexWord, bytesToRead;
        
        //const int MAX_BYTES_PER_SENDING = sizeof(dataBt);
        String line;
        String fileNameBt;
        //datos enviados por bluetooth
        String playList = "";
        int orderMode;
        int ledMode;
        int speed;
        int periodLed;
        String bluetoothName;
        String program;
        int positionList;
    public:
        
        Bluetooth(); 
        int init(String );
        int checkBlueTooth();
        int writeBt(String );
        int writeBtln(String );
        int readLine(String &);
        int readBt(uint8_t [], int );
        static String GetMD5String(uint8_t *msg, int mlen);
        String getPlaylist();
        int getOrderMode();
        int getLedMode();
        int getSpeed();
        int getPeriodLed();
        String getBluetoothName();
        String getProgram();
        int getPositionList();
};