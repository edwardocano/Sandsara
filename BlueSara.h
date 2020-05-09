#ifndef _BLUETOOTHS_H_
#define _BLUETOOTHS_H_

//#define BLUECOMMENTS

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
/**
 * @class BlueSara es la clase que se encarga de manejar el bluetooth
 * 
 * 
 */
class BlueSara {
    private:
        BluetoothSerial SerialBT;
        File file;
        unsigned long timeOutBt = 10000;
        //int debugCount = 0;
        //int codeErrorC;
        int indexWord, bytesToRead;
        uint8_t dataBt[10000];
        const int MAX_BYTES_PER_SENDING = sizeof(dataBt);
        String line;
        String fileNameBt;
    public:
        BlueSara();
        int init(String );
        int checkBlueTooth();
        int writeBt(String );
        int writeBtln(String );
        int readLine(String &);
        int readBt(uint8_t [], int );
        static String GetMD5String(uint8_t *msg, int mlen);
        
};

#endif