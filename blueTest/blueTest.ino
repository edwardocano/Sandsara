#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
    int codeError, index, bytesToRead;
    uint8_t dataBt[10000];
    String line;
    if (SerialBT.available()){
        CodeError = readLine(line);
        if (line.equals("transferir")){
            while(true){
                SerialBT.write("ok");
                CodeError = readLine(line);
                index = line.indexOf("bytes");
                if (index != -1 ){
                    bytesToRead = int(line.substring(5) );
                    CodeError = readBt(dataBt, bytesToRead);
                    for (int i = 0; i < bytesToRead; i++){
                        serial.print(dataBt[i]);
                    }
                }
                else if(line.equals("done")){
                    serial.println("guardado en memoria");
                    break;
                }
            }
        }
    }
    delay(1000);
}


int readLine(String& line){
    line = "";
    int byte = -1;
    while(char(byte) != '\n'){
        if (SerialBT.available()) {
            byte = SerialBT.read();
            line.concat(char(byte));
        }
    }
    serial.print(line);
    return 0;
}

int readBt(uint8_t dataBt, int bytesToRead){
    int i = 0;
    while(i < bytesToRead){
        if (SerialBT.available()) { 
            dataBt[i] = SerialBT.read();
            i += 1;
        }
    }
    return 0;
}