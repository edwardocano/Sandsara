#include "FileSara.h"
#include "MoveSara.h"
#include "BlueSara.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <math.h>

File myFile;
File root;
//variables para ordenar secuencia de archivos
String playListGlobal;
int ordenModeGlobal = 1;
//
MoveSara halo(16);
BlueSara haloBt;

int errorCode;

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
    playListGlobal = "/lista1.txt";
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
        Serial.print("Cambiar a: ");
        Serial.println(playListGlobal);
    }
    else if (errorCode == 20){
        ordenModeGlobal = haloBt.getOrdenMode();
        Serial.print("Cambiar orden a: ");
        Serial.println(ordenModeGlobal);
    }
    //root.rewindDirectory();
    delay(3000);
}

int run_sandsara(String playList, int ordenMode)
{
    double component_1, component_2;
    double _z, _theta;
    double theta_aux;
    double x_aux, y_aux;
    double coupling_angle;
    int pListFile = 1;
    int numberOfFiles;
    bool randomMode = true;
    String fileName;

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
        if (ordenMode == 2)
        {
            pListFile = random(1, numberOfFiles + 1);
        }
        errorCode = FileSara::getLineNumber(pListFile, playList, fileName);
        if (errorCode < 0)
        {
            delay(1000);
            return errorCode; //playList no valida
        }
        if (!(ordenMode == 2) && (errorCode == 2 || errorCode == 3))
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
                if (errorCode == 10){
                    playListGlobal = "/" + haloBt.getPlaylist();
#ifdef PROCESSING_SIMULATOR
                    Serial.println("finished");
#endif
                    return 1; //cambio de playlist
                }
                else if (errorCode == 20){
                    ordenModeGlobal = haloBt.getOrdenMode();
                    Serial.print("Cambiar orden a: ");
                    Serial.println(ordenModeGlobal);
#ifdef PROCESSING_SIMULATOR
                    Serial.println("finished");
#endif
                    return 2; //cambio de ordenMode
                }
                //dependiendo del tipo de archivo se ejecuta la funcion correspondiente de movimiento.
                if (file.fileType == 1 || file.fileType == 3)
                {
                    MoveSara::rotate(component_1, component_2, -couplingAngle);
                    halo.moveTo(component_1, component_2);
                }
                else if (file.fileType == 2)
                {
                    halo.movePolarTo(component_1, component_2 - couplingAngle);
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

