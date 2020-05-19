#include "FileSara.h"
#include "MoveSara.h"
#include "BlueSara.h"
#include <Adafruit_NeoPixel.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <math.h>

#include <EEPROM.h>

#define EEPROM_SIZE 512

File myFile;
File root;
//variables para ordenar secuencia de archivos
String playListGlobal;
int ordenModeGlobal;
//
MoveSara halo(16);
BlueSara haloBt;

int errorCode;

//Neopixel==========================================
#define PIN 15 
#define NUMPIXELS 30 // numero de pixels en la tira
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 1
int ledModeGlobal;
//==================================================

void setup()
{
    Serial.begin(115200);
    // Configure the halo
    halo.init();
    haloBt.init("Halo");

    if (!SD.begin())
    {
        Serial.println("Card failed, or not present");
        while (1)
            ;
    }
    myFile = SD.open("/");
    if (!myFile)
    {
        Serial.println("No se pudo abrir archivo");
        return;
    }
    root = SD.open("/");
    delay(1000);

#ifdef PROCESSING_SIMULATOR
    Serial.println("inicia");
#endif
    //recuperar valores de playlist y ordenMode
    EEPROM.begin(EEPROM_SIZE);
    delay(500);
    playListGlobal = romGetPlaylist();
    ordenModeGlobal = romGetOrdenMode();
    Serial.print("lista guardada: ");
    Serial.println(playListGlobal);
    Serial.print("ordenMode guardado: ");
    Serial.println(ordenModeGlobal);
    //si existe el archivos llamado playlist.playlist se ejecutara esa playlist
    if (SD.exists("/playlist.playlist")){
        playListGlobal = "/playlist.playlist";
        ordenModeGlobal = 1;
    }
//======Neopixel=============================================
    pixels.begin(); // Inicializa NeoPixel
//===========================================================
}

void loop()
{
#ifdef DEBUGGING_DATA
    Serial.println("Iniciara la funcion runSansara");
#endif
    errorCode = run_sandsara(playListGlobal, ordenModeGlobal);
    Serial.print("errorCode de run: ");
    Serial.println(errorCode);
    errorCode = haloBt.checkBlueTooth();
    if (errorCode == 10){
        playListGlobal = "/" + haloBt.getPlaylist();
        romSetPlaylist(playListGlobal);
    }
    else if (errorCode == 20){
        ordenModeGlobal = haloBt.getOrdenMode();
        romSetOrdenMode(ordenModeGlobal);
    }
    else if (errorCode == 30){
        ledModeGlobal = haloBt.getLedMode();
        Serial.print("ledmode = ");
        Serial.println(ledModeGlobal);
        Neo_Pixel(ledModeGlobal);
    }
    //root.rewindDirectory();
    delay(3000);
}

/**
 * @brief
 * @param playlist el nombre de la playlist a ejecutar, ejemplo "/animales.playlist"
 * @param ordenMode el tipo de reproduccion pudiendo ser
 * 1, se ejecuta en orden desendente segun la playlist.
 * 2, se ejecutan todos los archivos contenidos en el directorio principal de la SD en orden aleatorio.
 * 3, se ejecutan todos los archivos contenidos en el directorio principal de la SD en el orden que la sd los acomoda.
 * 4, se ejecutan los archivos de la playlist en orden aleatorio (no implementado).
 */
int run_sandsara(String playList, int ordenMode)
{
    double component_1, component_2, distance;
    double _z, _theta;
    double theta_aux;
    double x_aux, y_aux;
    double coupling_angle;
    int pListFile = 1;
    int numberOfFiles;
    bool randomMode = true;
    String fileName;

    numberOfFiles = FileSara::numberOfLines(playList);
    Serial.print("Numero de archivos: ");
    Serial.println(numberOfFiles);
    
    if (ordenMode == 2)
    {
        numberOfFiles = FileSara::creatListOfFiles("/RANDOM.txt");
        playList = "/RANDOM.txt";
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    }
    else if (ordenMode == 3)
    {
        numberOfFiles = FileSara::creatListOfFiles("/DEFAULT.txt");
        playList = "/DEFAULT.txt";
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    }

    while (true)
    {
#ifdef DEBUGGING_DATA
        Serial.println("Abrira el siguiente archivo disponible");
#endif
        if (ordenMode == 2 || ordenMode == 4)
        {
            pListFile = random(1, numberOfFiles + 1);
        }
        errorCode = FileSara::getLineNumber(pListFile, playList, fileName);
        if (errorCode < 0)
        {
            //playList no valida asi que regresa
            delay(1000);
            return errorCode;
        }
        if (!(ordenMode == 2 || ordenMode == 4) && (errorCode == 2 || errorCode == 3))
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
            pListFile += 1;
            continue;
        }
        if (current_file.isDirectory())
        {
            delay(1000);
            pListFile += 1;
            continue;
        }
        else
        {
            int working_status = 0;
            fileName = current_file.name();
            current_file.close();
#ifdef PROCESSING_SIMULATOR
            Serial.print("fileName: ");
            Serial.println(fileName);
#endif
            double couplingAngle, startRobotAngle, startFileAngle;
            FileSara file(fileName);
            double zInit = halo.getCurrentModule();
            //se selecciona modo de lectura
            file.autoSetMode(zInit);
            startFileAngle = file.getStartAngle();
            startFileAngle = MoveSara::normalizeAngle(startFileAngle);
            startRobotAngle = halo.getCurrentAngle();
            couplingAngle = startFileAngle - startRobotAngle;
            // si es thr, se guardan los valores del primer punto para que tenga referencia de donde empezar a moverse.
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
                //se obtienen los siguientes componentes
                working_status = file.getNextComponents(&component_1, &component_2);
                if (working_status == 3)
                {
                    continue;
                }
                if (working_status != 0)
                {
                    break;
                }
                //revisar bluetooth
                errorCode = haloBt.checkBlueTooth();
                executeCode(errorCode);
                if (errorCode == 10){
                    return 1; //cambio de playlist
                }
                else if (errorCode == 20){
                    return 2; //cambio de ordenMode
                }
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
                    double zNext = component_1;
                    double thetaNext = component_2 - couplingAngle;
                    double thetaCurrent = halo.getThetaCurrent();
                    double zCurrent = halo.getZCurrent();
                    double slicesFactor;
                    long slices;
                    double distancePoints = 100, deltaTheta, deltaZ;
                    double thetaAuxiliar, zAuxliar, xAux, yAux;
                    int overIterates = 0;
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
                            halo.moveTo(xAux, yAux);
                            errorCode = haloBt.checkBlueTooth();
                            executeCode(errorCode);
                            if (errorCode == 10){
                                return 1; //cambio de playlist
                            }
                            else if (errorCode == 20){
                                return 2; //cambio de ordenMode
                            }
                        }
                    }
                    xAux = zNext * cos(thetaNext);
                    yAux = zNext * sin(thetaNext);
                    halo.moveTo(xAux, yAux);
                    halo.setThetaCurrent(thetaNext);
                    halo.setZCurrent(zNext);
                }
                else
                {
                    break;
                }
            }
#ifdef PROCESSING_SIMULATOR
            if (working_status == -10)
            {
                Serial.println("There were problems for reading SD");
            }
            Serial.println("finished");
#endif
        }
        pListFile += 1;
    }
    return 0;
}

//=========================================================
/**
 * @brief Se usa esta funcion para avanzar de la posicion actual a un punto nuevo en linea recta por medio de puntos equidistantes a un 1 mm.
 * @param x coordenada en el eje x, medida en milimetros, a la que se desea avanzar.
 * @param y coordenada en el eje y, medida en milimetros, a la que se desea avanzar.
 * @param distance es la distancia, medida en milimetros, entre el punto actual y el punto al que se desea avanzar.
 * @return un codigo de error referente al bluetooth.
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
        x_aux += delta_x;
        y_aux += delta_y;
        halo.moveTo(x_aux, y_aux);
        errorCode = haloBt.checkBlueTooth();
        executeCode(errorCode);
        if (errorCode == 10){
            return 1; //cambio de playlist
        }
        else if (errorCode == 20){
            return 2; //cambio de ordenMode
        }
    }
    halo.moveTo(x, y);
    return 0;
}
//===========================
void executeCode(int errorCode){
    if (errorCode == 10){
        playListGlobal = "/" + haloBt.getPlaylist();
        romSetPlaylist(playListGlobal);
#ifdef PROCESSING_SIMULATOR
        Serial.println("finished");
#endif
    }
    else if (errorCode == 20){
        ordenModeGlobal = haloBt.getOrdenMode();
        romSetOrdenMode(ordenModeGlobal);
#ifdef PROCESSING_SIMULATOR
        Serial.println("finished");
#endif
    }
    else if (errorCode == 30){
        ledModeGlobal = haloBt.getLedMode();
        Serial.print("ledmode = ");
        Serial.println(ledModeGlobal);
        Neo_Pixel(ledModeGlobal);
    }
}

//------------Guardar variables importantes en FLASH----------------
#define ADDRESSPLAYLIST 0
#define ADDRESSPOSITION 41
#define ADDRESSORDENMODE 50

#define MAX_CHARS_PLAYLIST 40

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

//------------------------------Leds------------------------
//----------------------------------------------------------
void Neo_Pixel(int color)
{

    for (int i = 0; i < NUMPIXELS; i++)
    { // Va recorriendo cada pixel

        if (color == 1) //se alterna el color verde y rojo en los leds, uso el operador modulo para alternar los leds
        {
            if ((i % 2) == 0)
            {
                pixels.setPixelColor(i, pixels.Color(0, 150, 0));
                pixels.show();
            }
            else
            {
                pixels.setPixelColor(i, pixels.Color(255, 0, 0)); //i maneja el led a encender pixel.Color permite configurar el color
                pixels.show();                                    // Envia la señal al led que tomara el color RGB seleccionado
            }
        }
        if (color == 2) //pone todos los leds en color rojo
        {
            pixels.setPixelColor(i, pixels.Color(255, 0, 0)); //i maneja el led a encender pixel.Color permite configurar el color
            pixels.show();                                    // Envia la señal al led que tomara el color RGB seleccionado
        }
        if (color == 3) //los leds toman colores aleatorios
        {
            pixels.setPixelColor(i, rainbow()); //i maneja el led a encender
            pixels.show();                      // Envia la señal al led que tomara el color RGB seleccionado
        }
    }
    pixels.clear(); //
}
//Función para leds de color aleatorio
uint32_t rainbow()
{
    return pixels.Color(random(0, 255), random(0, 255), random(0, 255));
}
