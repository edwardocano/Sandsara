#ifndef _FILESARA_H_
#define _FILESARA_H_

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define DISTANCIA_MAX 150.0
#define RESOLUCION_MAX 32768.0

class FileSara {
    public:
        //public Members
        String fileName;
        long pFile;
        int pRow = 0;
        int fileType;//1 for txt 2 for thr and 3 for bin
        int directionMode; //1 for fordward and 0 for backwards
        File file;
        //Constructor
        FileSara(String , int = 1);
        //destructor
        ~FileSara();
        //public Methods
        int getNextComponents(double* , double* );
        int getStatus();
        void autoSetMode(double );
        double getStartPoint(int , int);
        double getFinalPoint(int , int);
        double getFinalAngle();

    private:
        //private Members
        int16_t xbin, ybin;
        int pFileBin;
        int charsToRead = 1000;
        int statusFile = 0;
        double z, theta;
        char componentSeparator = ',';
        char lineSeparator = '\n';
        uint8_t* dataBufferBin;
        String currentRow = "";
        String dataBuffer = "";
        //private Methods
        int getType(String);
        //template <class T>
        int getComponents(String , double* , double*);
        int getComponentsBin(uint8_t* , double* , double* );
        String nextRow();
        int readFile();
};

#endif