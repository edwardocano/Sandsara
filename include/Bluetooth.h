#pragma once
//#define BLUECOMMENTS
#include <Arduino.h>
#include "config.h"


#define SERVICE_UUID1 "fd31a2be-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID2 "fd31a58e-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID3 "fd31a688-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID4 "fd31a778-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID5 "fd31a840-22e7-11eb-adc1-0242ac120002"
#define SERVICE_UUID6 "fd31abc4-22e7-11eb-adc1-0242ac120002"


//====Led configuration Characteristics====
#define CHARACTERISTIC_UUID_LEDSPEED        "1a9a7b7e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_CYCLEMODE       "1a9a7dea-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_DIRECTION       "1a9a8042-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_SELECTEDPALETTE "1a9a813c-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_AMOUNTCOLORS    "1a9a820e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_POSITIONS       "1a9a82d6-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_RED             "1a9a83a8-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_GREEN           "1a9a8466-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_BLUE            "1a9a852e-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_UPDATECPALETTE  "1a9a87b8-2305-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_MSGERRORLEDS    "1a9a8880-2305-11eb-adc1-0242ac120002"
/*
1a9a8948-2305-11eb-adc1-0242ac120002
1a9a8a06-2305-11eb-adc1-0242ac120002
1a9a8ac4-2305-11eb-adc1-0242ac120002
1a9a8b8c-2305-11eb-adc1-0242ac120002*/

//Path config characteristics
/*#define PATH_UUID_NAME          "ff9f243a-2851-11eb-adc1-0242ac120002"
#define PATH_UUID_POSITION      "ff9f27b4-2851-11eb-adc1-0242ac120002"
#define PATH_UUID_PERCENTAGE    "ff9f2a48-2851-11eb-adc1-0242ac120002"
#define PATH_UUID_ERRORMSG      "ff9f2c50-2851-11eb-adc1-0242ac120002"
/*ff9f2c50-2851-11eb-adc1-0242ac120002
ff9f2d36-2851-11eb-adc1-0242ac120002
ff9f2e08-2851-11eb-adc1-0242ac120002
ff9f2ed0-2851-11eb-adc1-0242ac120002
ff9f2f98-2851-11eb-adc1-0242ac120002
ff9f3182-2851-11eb-adc1-0242ac120002
ff9f3254-2851-11eb-adc1-0242ac120002*/

//====File config====
#define FILE_UUID_RECEIVEFLAG   "fcbff68e-2af1-11eb-adc1-0242ac120002" 
#define FILE_UUID_RECEIVE       "fcbffa44-2af1-11eb-adc1-0242ac120002"
#define FILE_UUID_EXISTS        "fcbffb52-2af1-11eb-adc1-0242ac120002"
#define FILE_UUID_DELETE        "fcbffc24-2af1-11eb-adc1-0242ac120002"
#define FILE_UUID_SENDFLAG      "fcbffdaa-2af1-11eb-adc1-0242ac120002"
#define FILE_UUID_SEND          "fcbffe72-2af1-11eb-adc1-0242ac120002"
#define FILE_UUID_ERRORMSG      "fcbffce2-2af1-11eb-adc1-0242ac120002"
/*
fcc0012e-2af1-11eb-adc1-0242ac120002
fcc0020a-2af1-11eb-adc1-0242ac120002
fcc002c8-2af1-11eb-adc1-0242ac120002
*/

#define CHARACTERISTIC_UUID_1 "903cfcc2-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_2 "903cfede-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_3 "903cffe2-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_4 "903d00be-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_5 "903d0190-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_6 "903d024e-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_7 "903d0316-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_8 "903d0596-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_9 "903d065e-22eb-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_10 "903d071c-22eb-11eb-adc1-0242ac120002"

//====Playlist Config====
#define PLAYLIST_UUID_NAME              "9b12a048-2c6e-11eb-adc1-0242ac120002"
#define PLAYLIST_UUID_PATHAMOUNT        "9b12a26e-2c6e-11eb-adc1-0242ac120002"
#define PLAYLIST_UUID_PATHNAME          "9b12a534-2c6e-11eb-adc1-0242ac120002"
#define PLAYLIST_UUID_PATHPOSITION      "9b12a62e-2c6e-11eb-adc1-0242ac120002"

//#define PLAYLIST_UUID_READPLAYLISTFLAG  ""
//#define PLAYLIST_UUID_READPATH          "9b12a6f6-2c6e-11eb-adc1-0242ac120002"

#define PLAYLIST_UUID_ADDPATH           "9b12a7be-2c6e-11eb-adc1-0242ac120002"

#define PLAYLIST_UUID_MODE              "9b12a886-2c6e-11eb-adc1-0242ac120002"
#define PLAYLIST_UUID_PATHPROGRESS      "9b12a944-2c6e-11eb-adc1-0242ac120002"
#define PLAYLIST_UUID_ERRORMSG          "9b12aa02-2c6e-11eb-adc1-0242ac120002"
/*
9b12ac28-2c6e-11eb-adc1-0242ac120002
9b12acfa-2c6e-11eb-adc1-0242ac120002
9b12adb8-2c6e-11eb-adc1-0242ac120002
*/


/**
 * @class Bluetooth
 * @brief Se encarga de gestionar la comunicacion por bluetooth
 * @param timeOutBt es el tiempo, en milisegundos, que se va a esperar para recibir respuesta del dispositivo bluetooth conectado.
 * @param dataBt es donde se va a almacenar la informacion recibida.
 */
class Bluetooth {
    private:
        
    public:
        Bluetooth(); 
        int init(String = "Sandsara");

        void setPlaylistName(String );
        void setPathAmount(int);
        void setPathName(String);
        void setPathPosition(int);
        void setPlayMode(int);
        void setPathProgress(int);

        void setLedSpeed(int);
        void setCycleMode(int);
        void setLedDirection(int);
        void setIndexPalette(int);

};