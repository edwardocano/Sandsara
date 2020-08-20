#include "Bluetooth.h"
#include <Update.h>
#include <FileSara.h>

//====extern variables and functions====
extern SdFat SD;
extern bool sdExists(String );
extern bool sdRemove(String );
extern bool sdExists(String );
extern bool pauseModeGlobal;
extern int romSetCustomPallete(uint8_t* ,uint8_t* , uint8_t* ,uint8_t* , int);
extern int romSetIncrementIndexPallete(bool );
extern int romSetIntermediateCalibration(bool );
extern bool romGetIntermediateCalibration();
extern int romSetLedsDirection(bool );
//====
uint8_t dataBt[BUFFER_BLUETOOTH];
//====
// perform the actual update from a given stream
int performUpdate(Stream &, size_t );
// check given FS for valid update.bin and perform update if available
int updateFromFS(SdFat &, String );
int programming(String );
void rebootWithMessage(String );
int stringToArray(String , uint8_t* , int );
//====
extern String romGetPlaylist();
extern int romGetOrderMode();
extern int romGetPosition();
extern int romGetPallete();
extern int romGetSpeedMotor();
extern int romGetPeriodLed();
extern String romGetBluetoothName();
//====
//--------------Bluetooth--------------------------------------------------------
//-------------------------------------------------------------------------------
void callback(esp_spp_cb_event_t , esp_spp_cb_param_t* );

Bluetooth::Bluetooth(){
    
}

int Bluetooth::init(String name){
    if (!SerialBT.begin(name)){ //Bluetooth device name
        SerialBT.begin("Sandsara");
#ifdef BLUECOMMENTS
        Serial.println("No se inicializo con nombre por defecto");
#endif
    }
    SerialBT.register_callback(callback);
#ifdef BLUECOMMENTS
    Serial.println("The device started, now you can pair it with bluetooth!");
#endif
    return 0;
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    if (event == ESP_SPP_SRV_OPEN_EVT)
    {
#ifdef BLUECOMMENTS
        Serial.println("Client Connected");
#endif
    }

    if (event == ESP_SPP_CLOSE_EVT)
    {
#ifdef BLUECOMMENTS
        Serial.println("Client disconnected");
#endif
    }
}

/**
 * @brief revisa si hay algo dispobible por bluetooth. Puede recibir alguno de los siguientes mensajes.
 * 
 * code01, significa que va a transferir un archivo.
 * code02, significa que se esta solicitando el cambio de playlist.
 * code03, significa que se esta solicitando el cambio de orden de reproduccion.
 * code04, significa que se esta solicitando el cambio de paleta de leds.
 * code05, significa que se solicita el cambio de velocidad de motores.
 * code06, significa que se solicita el cambio de velocidad de los leds.
 * code07, significa que se desea cambiar el nombre del bluetooth.
 * code08, significa que se desea entrar en modo de suspencion.
 * code09, significa que se desea pausar el programa.
 * code10, significa que se desea salir del modo suspencion o pausa.
 * code11, significa que se desea conocer la version del firmware
 * code12, significa que se desea conocer los parametros guardados en ROM.
 * code13, significa que se desea conocer el nombre del programa actual.
 * code14, significa que se desea conocer el nombre del siguiente programa.
 * code15, significa que se desea conocer la lista de reproduccion actual.
 * code16, significa que se desea cambiar de programa por posicion en la playlist.
 * code17, significa que se desea cambiar de programa por nombre.
 * code18, significa que se modifico la paleta de colores custom.
 * code19, means that you want to change indexIncrement Variable.
 * code20, means that you want to change the intermediateCalibration Variable.
 * code21, means that you want to rewind the playlist.
 * code22, means that you want to change the direction of leds.
 * code66, actualizar firmware
 * code80, Reiniciar Sandsara
 * 
 * @return un codigo de reporte que indica lo siguiente
 *  1, se escribio un nuevo archivo.
 *  0, no hay informacion disponible del bluetooth.
 * 10, se solicito un cambio de playlist, para recuperar el nombre llamar a la funcion miembro getPlaylist().
 * 20, se solicito un cambio en orderMode, para recuperar el numero llamar a la funcion miembro getOrderMode().
 * 30, se solicito un cambio en ledMode, para recueperar el numero llamar a la funcion miembro getLedMode().
 * 50, se solicita el cambio de velocidad de motores, la velocidad se almacena en la variable miembro speed. speed se obtiene con getSpeed().
 * 60, se solicita el cambio de velocidad de los leds, la velocidad se almacena en la variable miembro periodLed. periodLed se obtiene con getPeriodLed().
 * 70, se solicita cambio de nombre de bluetooth.
 * 80, se solicita entrar en modo de suspencion.
 * 90, se solicita entrar en modo de pausa.
 * 100, se solicita salir del modo suspencion o pausa.
 * 110, se envio la version del firmware por bluetooth.
 * 120, se enviaron datos de sandsara
 * 130, se solicita el nombre del programa actual.
 * 140, se solicita el nombre del programa siguiente.
 * 150, se solicita la lista de reproduccion completa.
 * 160, se solicita cambiar de pista por medio de su posicion en la playlist, para conocer la posicion a la que se desea cambiar llamar a la funcion miembro getPositionList().
 * 170, se solicita reproducir un archivo, para conocer el nombre llamar la funcion miembro getProgram().
 * 180, se modifico la paleta custom en rom.
 * 200, intermediateCalibration variable was midified in ROM.
 * 970, se solicita un reset de fabrica.
 * -1, no se reconoce el comando enviado.
 * -2, se quiso enviar un numero de bytes incorrecto.
 * -3, se excedio el tiempo de respuesta del transmisor en la funcion readLine (depende de la variable timeOutBt, medida en milisegundos)
 * -4, se excedio el tiempo de respuesta del transmisor en la funcion readBt (depende de la variable timeOutBt, medida en milisegundos)
 * -5, se intento enviar un archivo con un nombre ya existente.
 * -7, no coincide checksum.
 * -8, no se pudo crear el archivo.
 * -9, se han leido mas de X numero de caracteres sin encontrar un '\n' en la funcion readLine().
 * -21, orderMode no valido.
 * -31, ledMode no valido.
 * -51, es un directorio
 * -52, el archivo esta vacio
 * -53, el archivo no se pudo abrir.02
 * -54, no se pudo finalizar la actualizacion
 * -55, no hay suficiente espacio para el OTA.
 * -56, Ocurrio un error al actualizar el firmware
 * -60, Velocidad no permitida para los motores
 * -70, periodo no permitido para los leds
 * -80, nombre muy largo para bluetooth
 * -81, No se pudo cambiar el nombre del bluetooth, pero se vera el cambio cuando se reinicie.
 * -97, se intento un reset de fabrica pero no se confirmo.
 * -171, se desea correr un programa que no existe en la SD.
 * -172, se intento cambiar a un archivo con una extension no valida.
 * -181, numero de colores en una paleta incorrecto.
 * -182, numero de posiciones distinto a numero de colores en la paleta.
 * -183, numero de colores red distinto a numero de colores en la paleta.
 * -184, numero de colores green distinto a numero de colores en la paleta.
 * -185, numero de colores blue distinto a numero de colores en la paleta.

 */
int Bluetooth::checkBlueTooth()
{
    int codeError = 0;
    String checksum;
    if (SerialBT.available())
    {
        codeError = readLine(line);
        if (codeError != 0)
        {
            writeBt("error= ");
            writeBtln(String(codeError));
            return codeError;
        }
        if (line.indexOf("transferir") >= 0)
        {
            Serial.print("Entro a readLine ");
            Serial.println(ESP.getFreeHeap());
            Serial.print("memoria disponible en heap: ");
            Serial.println(xPortGetFreeHeapSize());
            writeBtln("request-name");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            fileNameBt = line;
            if (fileNameBt.charAt(0) == ' ')
            {
                fileNameBt.remove(0, 1);
            }
#ifdef BLUECOMMENTS
            Serial.print("Nombre del archivo: ");
            Serial.println(fileNameBt);
#endif
            if (sdExists("/" + fileNameBt))
            {
                codeError = -5;
                writeBtln("error= -5"); //file alrady exits
            }
            else
            {
                file = SD.open("/" + fileNameBt, FILE_WRITE);
                if (!file)
                {
                    codeError = -8;
                    writeBtln("error= -8"); //file cannot be opened
                }
                else
                {
                    pauseModeGlobal = true;
                    unsigned long timeVar, timeVarTotal = 0, cont = 0;
                    for (;;) 
                    {
                        writeBtln("ok");
                        codeError = readLine(line);
                        if (codeError != 0)
                        {
                            writeBt("error= ");
                            writeBtln(String(codeError));
                            break;
                        }
                        if (line.indexOf("bytes=") != -1)
                        {
                            indexWord = line.indexOf("bytes=");
                            bytesToRead = line.substring(6).toInt();
                            #ifdef BLUECOMMENTS
                                Serial.print("bytesToRead: ");
                                Serial.println(bytesToRead);
                            #endif
                            if (bytesToRead <= 0 || bytesToRead > BUFFER_BLUETOOTH)
                            {
                                codeError = -2;
                                writeBtln("error= -2"); //invalid number of bytes
                                break;                  
                            }
                            codeError = readBt(dataBt, bytesToRead);
                            if (codeError != 0)
                            {
                                writeBt("error= ");
                                writeBtln(String(codeError));
                                break;
                            }
                            codeError = readLine(line);
                            if (codeError != 0)
                            {
                                writeBt("error= ");
                                writeBtln(String(codeError));
                                break;
                            }
                            checksum = GetMD5String(dataBt, bytesToRead);
                            #ifdef BLUECOMMENTS
                                Serial.print("checksum: ");
                                Serial.println(checksum);
                            #endif
                            if (!line.equals(checksum))
                            {
                                codeError = -7;
                                writeBtln("error= -7"); //checkSum doesn't match
                                break;
                            }
                            timeVar = millis();
                            file.write(dataBt, bytesToRead);
                            timeVar = millis() - timeVar;
                            timeVarTotal += timeVar;
                            cont += 1;
                        }
                        else if (line.indexOf("done") >= 0)
                        {
                            Serial.print("tiempo total:");
                            Serial.println(timeVarTotal);
                            file.close();
                            pauseModeGlobal = false;
                            writeBtln("ok");
#ifdef BLUECOMMENTS
                            Serial.println("guardado en memoria");
#endif
                            codeError = 1;
                            return codeError; //file was written
                        }
                        else
                        {
                            codeError = -1;
                            writeBtln("error= -1"); //invalid command
                            break;
                        }
                    }
                    
                    if (codeError != 0)
                    {
                        file.close();
                        sdRemove("/" + fileNameBt);
#ifdef BLUECOMMENTS
                        Serial.println("transferencia cancelada");
#endif
                    }
                }
            }
        }
        else if (line.indexOf("code02") >= 0)
        {
            writeBtln("request-playList");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            playList = line + ".playlist";
            writeBtln("ok");
            return 10;
        }
        else if (line.indexOf("code03") >= 0)
        {
            writeBtln("request-orderMode");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            orderMode = line.toInt();
            if(orderMode < 1 || orderMode > 4){
                writeBtln("error= -21");
                return -21;
            }
            writeBtln("ok");
            return 20;
        }
        else if (line.indexOf("code04") >= 0)
        {
            writeBtln("request-ledmode");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            ledMode = line.toInt();
            if (ledMode < MIN_PALLETE || ledMode > MAX_PALLETE){
                writeBtln("error= -31");
                return -31;
            }
            writeBtln("ok");
            return 30;
        }
        else if (line.indexOf("code05") >= 0)
        {
            writeBtln("request-speedMotor");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            speed = line.toInt();
            if (speed > MAX_SPEED_MOTOR || speed < MIN_SPEED_MOTOR){
                writeBtln("error= -60"); //invalid motor speed.
                return -60;
            }
            writeBtln("ok");
            return 50;
        }
        else if (line.indexOf("code06") >= 0)
        {
            writeBtln("request-speedLed");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            periodLed = line.toInt();
            if (periodLed > MAX_PERIOD_LED || periodLed < MIN_PERIOD_LED){
                writeBtln("error= -70"); //invalid led speed.
                return -70;
            }
            writeBtln("ok");
            return 60;
        }
        else if (line.indexOf("code07") >= 0)
        {
            writeBtln("request-bluetoothName");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (line.length() >= MAX_CHARACTERS_BTNAME){
                writeBtln("error= -80");
                return -80; //bluetooth name too large.
            }
            bluetoothName = line;
            char deviceName[MAX_CHARACTERS_BTNAME];
            line.toCharArray(deviceName, MAX_CHARACTERS_BTNAME);
            if (esp_bt_dev_set_device_name(deviceName) != ESP_OK) {
                writeBtln("error = -81");
                return -81;
            }
            writeBtln("ok");
            return 70;
        }
        else if (line.indexOf("code08") >= 0)
        {
            writeBtln("ok");
            return 80;
        }
        else if (line.indexOf("code09") >= 0)
        {
            writeBtln("ok");
            return 90;
        }
        else if (line.indexOf("code10") >= 0)
        {
            writeBtln("ok");
            return 100;
        }
        else if (line.indexOf("code11") >= 0)
        {
            writeBt("version= ");
            writeBt(String(v1Current));
            writeBt(".");
            writeBt(String(v2Current));
            writeBt(".");
            writeBtln(String(v3Current));
            return 110;
        }
        else if (line.indexOf("code12") >= 0)
        {
            writeBt("version= ");
            writeBt(String(v1Current));
            writeBt(".");
            writeBt(String(v2Current));
            writeBt(".");
            writeBtln(String(v3Current));
            writeBt("Nombre del bluetooth= ");
            writeBtln(romGetBluetoothName());
            writeBt("Velocidad de motores= ");
            writeBtln(String(romGetSpeedMotor()));
            writeBt("Paleta de colores= ");
            writeBtln(String(romGetPallete()));
            writeBt("Periodo de los leds= ");
            writeBtln(String(romGetPeriodLed()));
            writeBt("Lista de reproduccion= ");
            writeBtln(romGetPlaylist());
            writeBt("Orden de reproduccion= ");
            writeBtln(String(romGetOrderMode()));
            writeBt("Intermediate calibration= ");
            if (romGetIntermediateCalibration){
                writeBtln(String("enabled"));
            }
            else{
                writeBtln(String("disabled"));
            }
            
            return 120;
        }
        else if (line.indexOf("code13") >= 0)
        {
            return 130;
        }
        else if (line.indexOf("code14") >= 0)
        {
            return 140;
        }
        else if (line.indexOf("code15") >= 0)
        {
            return 150;
        }
        else if (line.indexOf("code16") >= 0)
        {
            writeBtln("request-newPosition");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            positionList = line.toInt();
            writeBtln("ok");
            return 160;
        }
        else if (line.indexOf("code17") >= 0)
        {
            writeBtln("request-programName");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            program = line;
            if (FileSara::getType(line) < 0){
                writeBtln("error= -172");
                return -172;
            }
            if (!sdExists(program)){
                writeBtln("error= -171");
                return -171;
            }
            
            writeBtln("ok");
            return 170;
        }
        else if (line.indexOf("code18") >= 0)
        {
            int colors;
            writeBtln("request-numberOfColors");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            colors = line.toInt();
            if (colors < 1 || colors > MAX_COLORSPERPALLETE){
                writeBtln("error= -181");
                return -181;
            }
            uint8_t positions[colors];
            uint8_t red[colors];
            uint8_t green[colors];
            uint8_t blue[colors];
            writeBtln("request-positions");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (stringToArray(line, positions, colors) < 0){
                writeBtln("error= -182");
                return -182;
            }
            
            //====reading red colors====
            writeBtln("request-red");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (stringToArray(line, red, colors) < 0){
                writeBtln("error= -183");
                return -183;
            }
            //====
            //====reading green colors====
            writeBtln("request-green");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (stringToArray(line, green, colors) < 0){
                writeBtln("error= -184");
                return -184;
            }
            //====
            //====reading blue colors====
            writeBtln("request-blue");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (stringToArray(line, blue, colors) < 0){
                writeBtln("error= -185");
                return -185;
            }
            
            romSetCustomPallete(positions, red, green, blue, colors);
            writeBtln("ok");
            return 180;
        }
        else if (line.indexOf("code19") >= 0){
            writeBtln("request-incrementIndexStatus");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (line.toInt() > 0){
                romSetIncrementIndexPallete(true);
            }
            else{
                romSetIncrementIndexPallete(false);
            }
            writeBtln("ok");
            return 190;
        }
        else if (line.indexOf("code20") >= 0){
            writeBtln("request-intermediateCalibration");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (line.toInt() > 0){
                romSetIntermediateCalibration(true);
            }
            else{
                romSetIntermediateCalibration(false);
            }
            writeBtln("ok");
            return 200;
        }
        else if(line.indexOf("code21") >= 0){
            writeBtln("ok");
            return 210;
        }
        else if(line.indexOf("code22") >= 0){
            writeBtln("request-ledsDirection");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            int direction = line.toInt();
            if (direction != 0){
                romSetLedsDirection(true);
            }
            else{
                romSetLedsDirection(false);
            }
            writeBtln("ok");
            return 220;
        }
        else if (line.indexOf("code66") >= 0)
        {
            writeBtln("request-nameUpdate");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            line = "/" + line;
            codeError = programming(line);
            if (codeError == 1)
            {
                writeBtln("ok");
                rebootWithMessage("Reiniciando");
                return codeError;
            }
            else{
                writeBt("error=");
                writeBtln(String(codeError - 50));
                return codeError - 50; //firmware cannot be updated
            }
        }
        else if (line.indexOf("code80") >= 0)
        {
            writeBtln("ok");
            rebootWithMessage("Reiniciando");
            return -666;
        }
        else if (line.indexOf("code97") >= 0)
        {
            writeBtln("request-Y/N");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            if (line.equals("Y")){
                writeBtln("ok");
                return 970;
            }
            writeBtln("reset Canceled");
            return -97;
        }
        else
        {
            writeBtln("error= -1");
            codeError = -1;         //invalid command
        }
        pauseModeGlobal = false;
        return codeError;
    }
    return 0;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al index de la paleta de colores.
 * @return el index de la paleta de colores que se envio por bluetooth.
 */
int Bluetooth::getLedMode(){
    return ledMode;
}
/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al nombre de la playlist
 * @return el nombre de la playlist.
 */
String Bluetooth::getPlaylist(){
    return playList;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde a la velocidad de los motores
 * @return la valocidad de los motores, medida en mm/s
 */
int Bluetooth::getSpeed(){
    return speed;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al tiempo de actualizacion de los leds
 * @return el tiempo de actualizacion de los leds, medido en milisegundos.
 */
int Bluetooth::getPeriodLed(){
    return periodLed;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al orden de reproduccion.
 * @return el orden de reproduccion de los archivos.
 */
int Bluetooth::getOrderMode(){
    return orderMode;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al nombre del bluetooth.
 * @return el ultimo numero referente al oreden de reproduccion que se solicito por bluetooth.
 */
String Bluetooth::getBluetoothName(){
    return bluetoothName;
}

/**
 * @brief envia un mensaje por bluetooth.
 * @param msg es el mensaje que va a ser enviado por bluetooth.
 * @return 0 cuando termina la funcion.
 */
int Bluetooth::writeBt(String msg)
{
    uint8_t* msg8 = (uint8_t *) calloc(msg.length() + 1, 1);
    msg8 = (uint8_t *) msg.c_str();
    SerialBT.write(msg8, msg.length());
    SerialBT.flush();
    delay(20);
    return 0;
}
/**
 * @brief envia un mensaje por bluetooth con un salto de linea.
 * @param msg es el mensaje que va a ser enviado por bluetooth.
 * @return 0 cuando termina la funcion.
 * @note el salto de linea se hace con "\r\n"
 */
int Bluetooth::writeBtln(String msg)
{
    msg.concat("\r\n");
    writeBt(msg);
    return 0;
}

/**
 * @brief lee informacion del bluetooth hasta encontrar un '\n'
 * @param line, es donde se almacena lo que se envio por bluetooth (se eliminan los valores '\n' y '\r')
 * @return uno de los siguientes numeros.
 *  0, todo termino con normalidad.
 * -3, significa que ha transcurrido mas tiempo del esperado sin encontrar un '\n' (depende de la variable timeOutBt).
 * -9, significa que se han leido mas de MAX_CHARACTER_PER_LINE caracteres sin econtrar un '\n'.
 */
int Bluetooth::readLine(String &line)
{
    line = "";
    int indexRemove;
    int byteRecived = -1;
    unsigned long tInit = millis();
    unsigned long wdtFeedTime = millis();
    int outOfRange = 0;
    delay(20);
    while (char(byteRecived) != '\n')
    {
        if (SerialBT.available())
        {
            byteRecived = SerialBT.read();
            line.concat(char(byteRecived));
            outOfRange += 1;
        }
        if (millis() - tInit > timeOutBt)
        {
            return -3; //timeOut in readLine
        }
        if (outOfRange > MAX_CHARACTER_PER_LINE)
        {
            line = "";
            return -9; //outOfRange in readLine
        }
        if (millis() - wdtFeedTime > 500){
            wdtFeedTime = millis();
            delay(1);
        }
    }
    indexRemove = line.indexOf('\r');
    if (indexRemove != -1)
    {
        line.remove(indexRemove, 1);
    }
    indexRemove = line.indexOf('\n');
    line.remove(indexRemove, 1);
#ifdef BLUECOMMENTS
    Serial.println(line);
#endif
    return 0;
}

/**
 * @brief lee una cantidad de bytes igual a bytesToRead del bluetooth y los almacena en dataBt.
 * @param dataBt, es un array que va a almacenar los bytes transmitidos por bluetooth.
 * @param bytesToRead, es la cantidad de bytes que se leeran del bluetooth.
 * @return uno de los siguientes codigos.
 *  0, todo termino con normalidad.
 * -10, significa que ha transcurrido mas tiempo del esperado sin encontrar un '\n' (depende de la variable timeOutBt).
 */
int Bluetooth::readBt(uint8_t dataBt[], int bytesToRead)
{
    int i = 0;
    unsigned long tInit = millis();
    unsigned long wdtFeedTime = millis();
    delay(20);
    while (i < bytesToRead)
    {
        if (SerialBT.available())
        {
            dataBt[i] = SerialBT.read();
            i += 1;
        }
        if (millis() - tInit > timeOutBt)
        {
            writeBt("bytes enviados: ");
            writeBtln(String(i));
        #ifdef BLUECOMMENTS
            Serial.print("bytes enviados: ");
            Serial.println(i);
        #endif
            return -4; //timeOut in readBt
        }
        if (millis() - wdtFeedTime > 500){
            wdtFeedTime = millis();
            delay(1);
        }
    }
    #ifdef BLUECOMMENTS
        Serial.println("salio de readBt normal");
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

String Bluetooth::GetMD5String(uint8_t *msg, int mlen)
{
    String str;
    int j, k;
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
 * @return uno de los siguientes numeros.
 * -4, no se pudo finalizar la actualizacion
 * -5, no hay suficiente espacio para el OTA.
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
 * @brief revisa si el archivo es valido y si lo es actualiza el firmware
 * @return uno de los siguientes numeros
 *  1, se actualizo el firmware
 * -1, es un directorio
 * -2, el archivo esta vacio
 * -3, el archivo no se pudo abrir.
 * -4, no se pudo finalizar la actualizacion
 * -5, no hay suficiente espacio para el OTA.
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
 * @brief intenta actualizar el firmware
 * @return un codigo para saber si ocurre un error a la hora de realizar la actualizacion.
 */
int programming(String name){
    int errorCode;
    errorCode = updateFromFS(SD, name);
    return errorCode;
}

/**
 * @brief reinicia el Esp32 pero antes escribe un mensaje por Serial.
 * @return un codigo para saber si ocurre un error a la hora de realizar la actualizacion.
 */
void rebootWithMessage(String reason){
    #ifdef DEBUGGING_DATA
        Serial.println(reason);
    #endif
    delay(1000);
    ESP.restart();
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde al nombre del programa que se desea reproducir.
 * @return el nombre del archivo que se desea reproducir.
 */
String Bluetooth::getProgram(){
    return program;
}

/**
 * @brief recupera el dato que se envio por medio de bluetooth que corresponde a la posicion del archivo que se desea reproducir.
 * @return la posicion en la playlist que se desea reproducir.
 */
int Bluetooth::getPositionList(){
    return positionList;
}

/**
 * @brief convierte un string en un array
 * un string de la forma x1,x2,...,xn se convierte en un array [0]=x1, [1]=x1, ...,[n-1]=xn
 * @param str es el string que se desea convertir.
 * @param array es el array donde se van a guardar los valores del string
 * @param n es el numero de elementos que tiene el string
 * @return uno de los siguientes numeros
 * 0, todo salio normal.
 * -1, no hay n elementos en el string
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