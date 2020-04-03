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
    //public Methods
    int getNextComponents(double* , double* );
    int getStatus();

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
    template <class T>
    int getComponents(T* , double* , double*) ;
    String nextRow();
    int readFile();
};

FileSara::FileSara(String nameF, int directionMode) {
  fileName = nameF;
  fileType = getType(fileName);
  this->directionMode = directionMode;
  file = SD.open(fileName);
  if (directionMode == 1) {
    pFile = 0;
  }
  else {
    pFile = file.size();
  }
  if (fileType == 2) {
    componentSeparator = ' ';
    lineSeparator = '\n';
  }
  if (fileType == 3)
  {
    charsToRead = 1002; //it must be a multiple of 6
    dataBufferBin = (uint8_t* ) calloc(charsToRead + 1, 1);
    if (directionMode == 0) {
      pFileBin = 0;
    }
    else {
      pFileBin = charsToRead / 6;
    }
  }
  else{
    dataBufferBin = (uint8_t* ) calloc(charsToRead + 1, 1);
  }

}

int FileSara::getNextComponents(double* component1, double* component2) {
  int resp;
  currentRow = nextRow();
  //Serial.println(currentRow);
  if (currentRow == "") {
    //Serial.println("Leera otros 1000 datos");
    resp = readFile();
    if (resp == -1) {
      file.close();
      return 1;
    }
    //Serial.print(dataBuffer);
    currentRow = nextRow();
  }
  if (fileType == 3) {
    resp = getComponents<uint8_t>((dataBufferBin + pFileBin * 6), component1, component2);
  }
  else {
    resp = getComponents<String>(&currentRow, component1, component2);
  }
  if ( resp != 0 ) {
    return statusFile;
  }

  if (fileType == 1 || fileType == 3) {
    /*Serial.print(*component1);
    Serial.print(", ");
    Serial.println(*component2);*/
    *component1 = DISTANCIA_MAX * *component1 / RESOLUCION_MAX;
    *component2 = DISTANCIA_MAX * *component2 / RESOLUCION_MAX;
  }
  else if (fileType == 2) {
    z = *component2 * DISTANCIA_MAX;
    theta = *component1;
    *component1 = z;
    *component2 = theta;
  }
  return 0;
}

int FileSara::getStatus() {
  return this->statusFile;
}

int FileSara::getType(String name_file) {
  name_file = name_file.substring(name_file.lastIndexOf('.') + 1);
  name_file.toLowerCase();
  if (name_file == "txt")
    return 1;
  else if (name_file == "thr")
    return 2;
  else if (name_file == "bin")
    return 3;
  else
    return -1;
}

template <class T>
int FileSara::getComponents(T* pline, double* c1, double* c2) {
  if (fileType != 3) {
    String* line = (String*) pline;
    int commaIndex = line->indexOf(this->componentSeparator);
    if (commaIndex == -1) { //no separator detected
      this->statusFile = -1;
      return -1;
    }
    if (commaIndex >= line->length() - 1) { //no second component detected
      this->statusFile = -4;
      return -4;
    }
    String _c1 = line->substring(0, commaIndex);
    String _c2 = line->substring(commaIndex + 1);
    if (_c1.indexOf('\r') != -1)
      _c1.remove(_c1.indexOf('\r'), 1);
    if (_c2.indexOf('\r') != -1)
      _c2.remove(_c2.indexOf('\r'), 1);
    if (_c1.indexOf('\n') != -1)
      _c1.remove(_c1.indexOf('\n'), 1);
    if (_c2.indexOf('\n') != -1)
      _c2.remove(_c2.indexOf('\n'), 1);
    *c1 = _c1.toDouble();
    *c2 = _c2.toDouble();
    this->statusFile = 0;
    return 0;
  }
  else
  {
    uint8_t* line = (uint8_t*) pline;
    if ( *(line + 2) != ',') {
      return -1;
    }
    int16_t* puint16;
    puint16 = (int16_t* )line;
    xbin = *puint16;
    puint16 = (int16_t* )(line + 3);
    ybin = *puint16;
    *c1 = double(xbin);
    *c2 = double(ybin);
    return 0;
  }
}

String FileSara::nextRow() {
  String return_str;
  int index_separator;

  if (this->directionMode == 0) {
    //<BINCASE>
    if (fileType == 3) {
      pFileBin -= 1;
      if (pFileBin < 0) {
        return "";
      }
      return "nonStop";
    }
    //</BINCASE>
    index_separator = this->dataBuffer.lastIndexOf(this->lineSeparator);
    if (index_separator == -1) {
      return this->dataBuffer;
    }
    if (index_separator >= this->dataBuffer.length() - 2) { // -2 is because the end of a line can be \r\n or \n\r
      index_separator = this->dataBuffer.lastIndexOf(this->lineSeparator, index_separator - 1);
      if (index_separator == -1) {
        return_str = this->dataBuffer;
        this->dataBuffer = "";
        return return_str;
      }
      return_str = this->dataBuffer.substring(index_separator + 1);
      this->dataBuffer.remove(index_separator + 1);
      return return_str;
    }
    else {
      return_str = this->dataBuffer.substring(index_separator + 1);
      this->dataBuffer.remove(index_separator + 1);
      return return_str;
    }
  }
  else {
    //<BINCASE>
    if (fileType == 3) {
      pFileBin += 1;
      if (pFileBin > charsToRead / 6 - 1) {
        return "";
      }
      return "nonStop";
    }
    //</BINCASE>
    index_separator = this->dataBuffer.indexOf(this->lineSeparator);
    if (index_separator == -1 || index_separator == this->dataBuffer.length() - 1) {
      return_str = this->dataBuffer;
      this->dataBuffer = "";
      return return_str;
    }
    return_str = this->dataBuffer.substring(0, index_separator + 1);
    this->dataBuffer = this->dataBuffer.substring(index_separator + 1);
    return return_str;
  }
}

int FileSara::readFile() {
  if (this->directionMode == 0) {
    if (pFile >= charsToRead ) {
      pFile -= charsToRead;
    }
    else {
      charsToRead = pFile; //check if it has to be +1
      if (pFile == 0) {
        return -1;
      }
      pFile = 0;
    }
    this->file.seek(pFile);
    //char input_char[charsToRead + 1];
    this->file.read(dataBufferBin, charsToRead);
    *(dataBufferBin + charsToRead) = '\0';
    //<BINCASE>
    if (fileType == 3)
    {
      pFileBin = charsToRead / 6;
      return 0;
    }
    //</BINCASE>

    dataBuffer = String((char* )dataBufferBin);
    if (pFile == 0) {
      return 0;
    }
    int index_nl = dataBuffer.indexOf(this->lineSeparator);
    dataBuffer = dataBuffer.substring(index_nl + 1);
    pFile += index_nl + 1;
    return 0;
  }
  else
  {
    if (pFile >= file.size())
    {
      return -1;
    }

    if (pFile + charsToRead > file.size()) {
      charsToRead = file.size() - pFile; //
    }
    this->file.seek(pFile);
    //char input_char[charsToRead + 1];
    this->file.read(dataBufferBin, charsToRead);
    *(dataBufferBin + charsToRead) = '\0';
    //<BINCASE>
    if (fileType == 3)
    {
      pFile += charsToRead;
      pFileBin = -1;
      return 0;
    }
    //</BINCASE>
    dataBuffer = String((char* ) dataBufferBin);
    int index_nl = dataBuffer.lastIndexOf(this->lineSeparator);
    if (index_nl == -1)
    {
      pFile = file.size();
      return 0;
    }

    dataBuffer = dataBuffer.substring(0, index_nl + 1);
    pFile += index_nl + 1;
  }
}
