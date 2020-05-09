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
String playList = "/lista1.txt";
//
MoveSara halo(16);
BlueSara haloBt;

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

}

void loop()
{
#ifdef DEBUGGING_DATA
    Serial.println("Iniciara la funcion runSansara");
#endif
    run_sandsara(playList);
    haloBt.checkBlueTooth();
    //root.rewindDirectory();
    delay(5000);
}

int run_sandsara(String playList)
{
    double component_1, component_2;
    double _z, _theta;
    double theta_aux;
    double x_aux, y_aux;
    double coupling_angle;
    int pListFile = 1;
    int errorCode;
    int numberOfFiles;
    int ordenMode = 3;
    bool randomMode = true;
    String fileName;

    if (ordenMode == 2)
    {
        numberOfFiles = creatListOfFiles("/RANDOM.txt");
        playList = "/RANDOM.txt";
        Serial.print("Numero de archivos: ");
        Serial.println(numberOfFiles);
    }
    else if (ordenMode == 3)
    {
        numberOfFiles = creatListOfFiles("/DEFAULT.txt");
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
        errorCode = getLineNumber(pListFile, playList, fileName);
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
                haloBt.checkBlueTooth();
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

//------------------------------Ordenar archivos----------------------------------
//--------------------------------------------------------------------------------
/**
 * @brief devuelve la linea lineNumber del archivo dirFile.
 * @param lineNumber es el numero de la linea que se desea leer, para la primera linea este parametro debe ser 1, no 0.
 * @param dirFile es la direccion del archivo, empezando con '/'.
 * @param lineText es la variable donde se va a aguardar el contenido de la linea leida.
 * @return un codigo de error, pudiendo ser alguno de los siguientes.
 * 0, Encontro la linea lineNumber y esta linea termina con un '\n'.
 * 1, Encontro la linea lineNumber, pero no termina con '\n'.
 * 2, No encontro la linea lineNumber y no se encontro un '\n' al final de esta linea, por lo que en lineText esta guardada la ultima linea del archivo.
 * 3, No encontro la linea lineNumber y sí se encontro un '\n' al final de esta linea, por lo que en lineText esta guardada la ultima linea del archivo.
 * -1, No se pudo abrir el archivo con la direccion dirFile.
 * -2, El archivo abierto es un directorio.
 * -3, La linea que se desea leer no es valida.
 */
int getLineNumber(int lineNumber, String dirFile, String &lineText)
{
    File file = SD.open(dirFile);
    long index = 0, lineN, count = 0;
    int character = 1;

    lineText = "";
    if (lineNumber < 1)
    {
        return -3; //La linea que se desea leer no es valida
    }
    if (!file)
    {
        return -1; //dirFile no se pudo abrir
    }
    if (file.isDirectory())
    {
        return -2; //el archivo es un directorio
    }
    for (int number = 1; number < lineNumber; number++)
    {
        character = 1;
        while (character != '\n')
        {
            character = file.read();
            if (character == -1)
            {
                break;
            }
            count += 1;
        }
        if (character == -1)
        {
            file.seek(index);
            while (character != '\n')
            {
                character = file.read();
                if (character == -1)
                {
                    if (lineText.indexOf('\r') != 1)
                    {
                        lineText.remove(lineText.indexOf('\r'), 1);
                    }
                    return 2; //termino sin encontrar la linea numero lineNumber y no encontro un '\n'
                }
                lineText.concat(char(character));
            }
            lineText.remove(lineText.indexOf('\n'), 1);
            if (lineText.indexOf('\r') != 1)
            {
                lineText.remove(lineText.indexOf('\r'), 1);
            }
            return 3; //termino sin llegar a la linea numero lineNumber, y encontro un '\n'
        }
        index = count;
    }
    character = 1;
    file.seek(index);
    while (character != '\n')
    {
        character = file.read();
        if (character == -1)
        {
            if (lineText.indexOf('\r') != 1)
            {
                lineText.remove(lineText.indexOf('\r'), 1);
            }
            return 1; //termino en la linea numero lineNumber, pero no encontro un '\n'
        }
        lineText.concat(char(character));
    }
    lineText.remove(lineText.indexOf('\n'), 1);
    if (lineText.indexOf('\r') != 1)
    {
        lineText.remove(lineText.indexOf('\r'), 1);
    }
    file.close();
    return 0; //termino en la linea numero lineNumber, y encontro un '\n'
}

/**
 * @brief Crea un archivo que almacena los nombres de los archivos en el directorio "/"
 * @param fileName es el nombre del archivo que se va a crear.
 * @return el numero de archivos que encontro y registro.
 * o -1 si no se pudo crear el archivo.
 */
int creatListOfFiles(String fileName)
{
    File file, root, fileObj;
    int numberOfFiles = 0;
    root = SD.open("/");
    file = SD.open("/" + fileName, FILE_WRITE);
    if (!file)
    {
        return -1; //no se pudo crear el archivo
    }
    fileName.toLowerCase();
    while (true)
    {
        fileObj = root.openNextFile();
        if (!fileObj)
        {
            fileObj.close();
            file.close();
            root.close();
            return numberOfFiles;
        }
        if (!fileObj.isDirectory())
        {
            String varName = fileObj.name();
            String varNameLower = varName;
            varNameLower.toLowerCase();
            if (varNameLower.equals(fileName))
            {
                continue;
            }
            file.print(varName.substring(1) + "\r\n");
            numberOfFiles += 1;
        }
    }
}
