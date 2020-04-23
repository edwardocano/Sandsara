#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
File file;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("The device started, now you can pair it with bluetooth!");
  delay(1000);
}

int codeError, indexWord, bytesToRead;
uint8_t dataBt[10000];
String line;
String fileNameBt;

void loop() {
    //Serial.println("antes de bt available");
    if (SerialBT.available()){
        codeError = readLine(line);
        if (line.equals("transferir")){
            SerialBT.println("ok");
            codeError = readLine(line);
            if (codeError != 0){
                SerialBT.print("error= ");
                SerialBT.println(codeError);
            }
            else if(line.indexOf("name=") != -1){
                indexWord = line.indexOf("name=");
                fileNameBt = line.substring(5);
                if (fileNameBt.charAt(0) == ' '){
                    fileNameBt.remove(0,1);
                }
                Serial.print("Nombre del archivo: ");
                Serial.println(fileNameBt);
                if (SD.exists("/" + fileNameBt)){
                    SerialBT.println("error= -5"); //Ya existe el archivo
                }
                else{
                    file = SD.open("/"+fileNameBt, FILE_WRITE);
                    while(true){
                        SerialBT.println("ok");
                        codeError = readLine(line);
                        if (codeError != 0){
                            SerialBT.print("error= ");
                            SerialBT.println(codeError);
                            break;
                        }                
                        if (line.indexOf("bytes=") != -1 ){
                            indexWord = line.indexOf("bytes=");
                            bytesToRead = line.substring(6).toInt();
                            if (bytesToRead == 0){
                              SerialBT.println("error= -2"); //Numero de bytes incorrecto
                              break; //
                            }
                            SerialBT.println("ok");
                            codeError = readBt(dataBt, bytesToRead);
                            if (codeError != 0){
                                SerialBT.print("error= ");
                                SerialBT.println(codeError);
                                break;
                            }
                            file.write(dataBt, bytesToRead);
                            for (int i = 0; i < bytesToRead; i++){
                                Serial.print(char(dataBt[i]));
                            }
                        }
                        else if(line.equals("done")){
                            file.close();
                            SerialBT.println("ok");
                            Serial.println("guardado en memoria");
                            break;
                        }
                        else{
                            SerialBT.println("error= -1"); //Comando incorrecto
                            break;
                        }
                    }
                }
            }
            else {
                SerialBT.println("error= -4"); //No se proporciono nombre
            }
        }
    }
    delay(1000);
}

unsigned long timeOutBt = 30000;

int readLine(String& line){
    line = "";
    int indexRemove;
    int byteRecived = -1;
    unsigned long tInit = millis();
    while(char(byteRecived) != '\n'){
        if (millis() - tInit < timeOutBt){
            if (SerialBT.available()) {
                byteRecived = SerialBT.read();
                line.concat(char(byteRecived));
            }        
        }
        else{
            return -3; //timeOut
        }
    }
    indexRemove = line.indexOf('\r');
    if (indexRemove != -1){
      line.remove(indexRemove, 1);
    }
    indexRemove = line.indexOf('\n');
    line.remove(indexRemove, 1);
    Serial.println(line);
    return 0;
}

int readBt(uint8_t dataBt[], int bytesToRead){
    int i = 0;
    unsigned long tInit = millis();
    while(i < bytesToRead){
        if (millis() - tInit < timeOutBt){
            if (SerialBT.available()) { 
                dataBt[i] = SerialBT.read();
                i += 1;
            }       
        }
        else{
            return -3; //timeOut
        }
        
    }
    while (SerialBT.available()) { 
        SerialBT.read();
    }
    return 0;
}
