#include "FileSara.h"
#include "MoveSara.h"
#include "bluetooth.h"

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

void setup()
{
    Serial.begin(115200);
    // Configure the halo
    halo.init();

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
    //Configuracion de bluetooth
    //--------------------------
    SerialBT.begin("HALO"); //Bluetooth device name

    SerialBT.register_callback(callback);

    if (!SD.begin())
    {
        Serial.println("Card Mount Failed");
        return;
    }
#ifdef BLUECOMMENTS
    Serial.println("The device started, now you can pair it with bluetooth!");
#endif
    //---------------------------
    //---------------------------
}

void loop()
{
#ifdef DEBUGGING_DATA
    Serial.println("Iniciara la funcion runSansara");
#endif
    run_sandsara(playList);
    checkBlueTooth();
    root.rewindDirectory();
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
                checkBlueTooth();
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

/**************************************************************/

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
//--------------Bluetooth--------------------------------------------------------
//-------------------------------------------------------------------------------

/**
 * @brief revisa si hay algo dispobible por bluetooth y lo interpreta
 * puede interpretar las siguientes palabras clave
 * ->transferir, indica que se va a enviar un archivo
 * ---->bytes=x, donde x es el numero de bytes que se van a transferir (Maximo 10000)
 * ---->done, significa que ya se termino de transferir los datos y entonces se guardaran en memoria;
 * 
 * tambien puede enviar los siguientes mensajes
 * ->ok indica que esta listo para recibir algun mensaje
 * @return un codigo de error que indica lo siguiente
 *  1, se escribio un nuevo archivo.
 *  0, no hay informacion disponible del bluetooth.
 * -1, no se reconoce el comando enviado.
 * -2, se quiso enviar un numero de bytes incorrecto.
 * -3, se excedio el tiempo de respuesta del transmisor en la funcion readLine (depende de la variable timeOutBt, medida en milisegundos)
 * -4, se excedio el tiempo de respuesta del transmisor en la funcion readBt (depende de la variable timeOutBt, medida en milisegundos)
 * -5, se intento enviar un archivo con un nombre ya existente.
 * -7, no coincide checksum.
 * -8, no se pudo crear el archivo.
 * -9, se han leido mas de X numero de caracteres sin encontrar un '\n' en la funcion readLine()
 */
int checkBlueTooth()
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
            //SerialBT.println("request=name");
            writeBtln("request=name");
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
            if (SD.exists("/" + fileNameBt))
            {
                codeError = -5;
                writeBtln("error= -5"); //Ya existe el archivo
            }
            else
            {
                file = SD.open("/" + fileNameBt, FILE_WRITE);
                if (!file)
                {
                    codeError = -8;
                    writeBtln("error= -8"); //no se pudo abrir el archivo
                }
                else
                {
                    for (int i = 0; i < 1000; i++) //while (true)
                    {
                        //yield();
                        // SerialBT.println("ok");
                        writeBtln("ok");
                        //delay(20);
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
                            if (bytesToRead <= 0 || bytesToRead > MAX_BYTES_PER_SENDING)
                            {
                                codeError = -2;
                                writeBtln("error= -2"); //Numero de bytes incorrecto
                                break;                  //
                            }
                            //yield();
                            //writeBtln("ok");
                            codeError = readBt(dataBt, bytesToRead);
                            if (codeError != 0)
                            {
                                writeBt("error= ");
                                writeBtln(String(codeError));
                                break;
                            }
                            //yield();
                            //SerialBT.println("request=checksum");
                            //writeBtln("request=checksum");
                            codeError = readLine(line);
                            if (codeError != 0)
                            {
                                writeBt("error= ");
                                writeBtln(String(codeError));
                                break;
                            }
                            //long timeStart = millis(), timeEnd;
                            checksum = GetMD5String(dataBt, bytesToRead);
//checksum = md5("hola");
#ifdef BLUECOMMENTS
                            Serial.print("checksum: ");
                            Serial.println(checksum);
#endif
                            if (!line.equals(checksum))
                            {
                                codeError = -7;
                                writeBtln("error= -7"); //no coincide checksum
                                break;
                            }
                            file.write(dataBt, bytesToRead);
                            /*for (int i = 0; i < bytesToRead; i++)
                            {
                                Serial.print(char(dataBt[i]));
                            }*/
                        }
                        //else if (line.equals("done"))
                        else if (line.indexOf("done") >= 0)
                        {
                            file.close();
                            writeBtln("ok");
#ifdef BLUECOMMENTS
                            Serial.println("guardado en memoria");
#endif
                            codeError = 1;
                            return codeError; //se escribio archivo
                        }
                        else
                        {
                            codeError = -1;
                            writeBtln("error= -1"); //Comando incorrecto
                            break;
                        }
                    }
                    if (codeError != 0)
                    {
                        file.close();
                        SD.remove("/" + fileNameBt);
#ifdef BLUECOMMENTS
                        Serial.println("transferencia cancelada");
#endif
                    }
                }
            }
        }
        else
        {
            writeBtln("error= -1"); //Comando incorrecto
            codeError = -1;         //comando incorrecto
        }

        return codeError;
    }
    return 0;
}

/**
 * @brief envia un mensaje por bluetooth.
 * @param msg es el mensaje que va a ser enviado por bluetooth.
 * @return 0 cuando termina la funcion.
 * @note es imporante usar el metodo write en lugar de print o println, si se usa uno de estos 2 ultimos puede haber comportamientos inesperados como el crasheo del programa. 
 */
int writeBt(String msg)
{
    //Serial.println("Entro a writeBt");
    uint8_t *msg8 = (uint8_t *)msg.c_str();
    SerialBT.write(msg8, msg.length());
    SerialBT.flush();
    delay(20);
    //Serial.println("salio a writeBt");
    return 0;
}
/**
 * @brief envia un mensaje por bluetooth con un salto de linea.
 * @param msg es el mensaje que va a ser enviado por bluetooth.
 * @return 0 cuando termina la funcion.
 * @note el salto de linea se hace con \r\n
 */
int writeBtln(String msg)
{
    msg.concat("\r\n");
    writeBt(msg);
    return 0;
}

/**
 * @brief lee informacion del bluetooth hasta encontrar un '\n'
 * @param line, es donde se almacena lo que se envio por bluetooth (se eliminan los valores '\n' y '\r')
 * @return un codigo de error que puede significar lo siguiente.
 *  0, todo termino con normalidad.
 * -3, significa que ha transcurrido mas tiempo del esperado sin encontrar un '\n' (depende de la variable timeOutBt).
 * -9, significa que se han leido mas de X caracteres sin econtrar un '\n'.
 */
int readLine(String &line)
{
    //yield();
    //Serial.print("Entro a readLine ");
    //Serial.println(ESP.getFreeHeap());
    line = "";
    int indexRemove;
    int byteRecived = -1;
    unsigned long tInit = millis();
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
            //Serial.println("salio de readLine por timeOut");
            return -3; //timeOut de readLine
        }
        if (outOfRange > 1000)
        {
            //Serial.println("salio de readLine por OutOfRange");
            line = "";
            return -9; //outOfRange de readLine
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
    //Serial.println("salio de readLine normal");
    return 0;
}

/**
 * @brief lee una cantidad de bytes igual a bytesToRead del bluetooth y los almacena en dataBt.
 * @param dataBt, es un arreglo que va a almacenar los bytes transmitidos por bluetooth.
 * @param bytesToRead, es la cantidad de bytes que se leeran del bluetooth.
 * @return un codigo de error que puede significar lo siguiente.
 *  0, todo termino con normalidad.
 * -10, significa que ha transcurrido mas tiempo del esperado sin encontrar un '\n' (depende de la variable timeOutBt).
 * @note se intentan leer los bytes sobrantes, si es que se enviaron mas de los indicados, con la funcion while (SerialBT.available()).
 */
int readBt(uint8_t dataBt[], int bytesToRead)
{
    //yield();
    //Serial.print("Entro a readBt: ");
    //Serial.println(debugCount);
    debugCount += 1;
    int i = 0;
    unsigned long tInit = millis();
    //Serial.println("Entra a while");
    delay(20);
    while (i < bytesToRead)
    {
        //yield();
        if (SerialBT.available())
        {
            dataBt[i] = SerialBT.read();
            //Serial.write(dataBt[i]);
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
            //Serial.println("salio de readBt por timeOut");
            return -4; //timeOut de readBt
        }
    }
/*while (SerialBT.available())
    {
        Serial.println("Entro por exceso de data tu crees");
        SerialBT.read();
    }*/
#ifdef BLUECOMMENTS
    Serial.println("salio de readBt normal");
#endif
    return 0;
}

//MD5----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
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
    free(msg2); //ES IMPORTANTE LIBERAR EL ESPACIO PORQUE SINO HABRA OVERFLOW EN LA HEAP MEMORY
    return h;
}

static String GetMD5String(uint8_t *msg, int mlen)
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
    //free(d);
    return str;
}
