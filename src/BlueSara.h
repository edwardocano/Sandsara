#ifndef _BLUETOOTHS_H_
#define _BLUETOOTHS_H_

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
 * @class BlueSara es la clase que se encarga de manejar el bluetooth
 * @param timeOutBt es el tiempo, en milisegundos, que se va a esperar para recibir respuesta del dispositivo bluetooth conectado.
 * @param dataBt es donde se va a almacenar la informacion recibida.
 */
class BlueSara {
    private:
        BluetoothSerial SerialBT;
        File file;
        unsigned long timeOutBt = 20000;
        //int debugCount = 0;
        //int codeErrorC;
        int indexWord, bytesToRead;
        uint8_t dataBt[10000];
        const int MAX_BYTES_PER_SENDING = sizeof(dataBt);
        String line;
        String fileNameBt;
        //datos enviados por bluetooth
        String playList = "";
        int ordenMode;
        int ledMode;
        int speed;
        int periodLed;
        String bluetoothName;
        String program;
        int positionList;
    public:
        BlueSara(); 
        int init(String );
        int checkBlueTooth();
        int writeBt(String );
        int writeBtln(String );
        int readLine(String &);
        int readBt(uint8_t [], int );
        static String GetMD5String(uint8_t *msg, int mlen);
        String getPlaylist();
        int getOrdenMode();
        int getLedMode();
        int getSpeed();
        int getPeriodLed();
        String getBluetoothName();
        String getProgram();
        int getPositionList();
};

#endif