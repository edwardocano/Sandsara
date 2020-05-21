
#include "BlueSara.h"


//--------------Bluetooth--------------------------------------------------------
//-------------------------------------------------------------------------------
void callback(esp_spp_cb_event_t , esp_spp_cb_param_t* );

BlueSara::BlueSara(){
    
}

int BlueSara::init(String name){
    SerialBT.begin(name); //Bluetooth device name
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
 * 10, se solicito un cambio de playlist, para recuperar el nombre llamar a la funcion getPlaylist().
 * 20, se solicito un cambio en ordenMode, para recuperar el numero llamar a la funcion getOrdenMode().
 * 30, se solicito un cambio en ledMode, para recueperar el numero llamar a la funcion getLedMode().
 * -1, no se reconoce el comando enviado.
 * -2, se quiso enviar un numero de bytes incorrecto.
 * -3, se excedio el tiempo de respuesta del transmisor en la funcion readLine (depende de la variable timeOutBt, medida en milisegundos)
 * -4, se excedio el tiempo de respuesta del transmisor en la funcion readBt (depende de la variable timeOutBt, medida en milisegundos)
 * -5, se intento enviar un archivo con un nombre ya existente.
 * -7, no coincide checksum.
 * -8, no se pudo crear el archivo.
 * -9, se han leido mas de X numero de caracteres sin encontrar un '\n' en la funcion readLine()
 * @note interpreta los siguientes mensajes
 * code01, significa que va a transferer un archivo.
 * code02, significa que se esta solicitando el cambio de playlist.
 * code03, significa que se esta solicitando el cambio de orden de reproduccion.
 */
int BlueSara::checkBlueTooth()
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
        if (line.indexOf("code01") >= 0)
        {
            //SerialBT.println("request=name");
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
            playList = line;
            writeBtln("ok");
            return 10; //Se solicita el cambio de playlista
        }
        else if (line.indexOf("code03") >= 0)
        {
            writeBtln("request-ordenMode");
            codeError = readLine(line);
            if (codeError != 0)
            {
                writeBt("error= ");
                writeBtln(String(codeError));
                return codeError;
            }
            ordenMode = line.toInt();
            writeBtln("ok");
            return 20; //Se solicita el cambio de playlista
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
            writeBtln("ok");
            return 30; //Se solicita el cambio de leds
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
 * @brief recupera el ultimo modo de encendido de los leds (un entero) que se recibio por bluetooth.
 * @return el ultimo modo de encendido de los leds (un entero) recibio por bluetooth.
 */
int BlueSara::getLedMode(){
    return ledMode;
}
/**
 * @brief recupera el nombre de la ultima playlist solicitada a ser reproducida.
 * @return el nombre de la ultima playlist que se solicito por bluetooth.
 */
String BlueSara::getPlaylist(){
    return playList;
}

/**
 * @brief recupera el orden de la ultima solicitud de cambio de orden de reproduccion que se realizo.
 * @return el ultimo numero referente al oreden de reproduccion que se solicito por bluetooth.
 */
int BlueSara::getOrdenMode(){
    return ordenMode;
}

/**
 * @brief envia un mensaje por bluetooth.
 * @param msg es el mensaje que va a ser enviado por bluetooth.
 * @return 0 cuando termina la funcion.
 * @note es imporante usar el metodo write en lugar de print o println, si se usa uno de estos 2 ultimos puede haber comportamientos inesperados como el crasheo del programa. 
 */
int BlueSara::writeBt(String msg)
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
int BlueSara::writeBtln(String msg)
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
int BlueSara::readLine(String &line)
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
int BlueSara::readBt(uint8_t dataBt[], int bytesToRead)
{
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

String BlueSara::GetMD5String(uint8_t *msg, int mlen)
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