#ifndef _BLUETOOTHS_H_
#define _BLUETOOTHS_H_

//#define BLUECOMMENTS

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
File file;
unsigned long timeOutBt = 30000;
int indexWord, bytesToRead;
uint8_t dataBt[10000];
String line;
String fileNameBt;
int debugCount = 0;
int codeErrorC;
const int MAX_BYTES_PER_SENDING = sizeof(dataBt);

void callback(esp_spp_cb_event_t , esp_spp_cb_param_t *);
int checkBlueTooth();
int writeBt(String );
int writeBtln(String );
int readLine(String& line);
int readBt(uint8_t[] , int );
static String GetMD5String(uint8_t* , int );

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#endif