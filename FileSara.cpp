#include "FileSara.h"
#include "MoveSara.h"

FileSara::FileSara(String nameF, int directionMode) {
  fileName = nameF;
  fileType = getType(fileName);
  this->directionMode = directionMode;
  #ifdef DEBUGGING_DATA
  Serial.print("Se abrira el archivo: ");
  Serial.println(fileName);
  #endif
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

FileSara::~FileSara(){
  file.close();
  free(dataBufferBin);
  #ifdef DEBUGGING_DATA
  Serial.print("Se destruyo el archivo: ");
  Serial.println(fileName);
  #endif
}

int FileSara::getNextComponents(double* component1, double* component2) {
  int resp;
  currentRow = nextRow();
  //Serial.println(currentRow);
  if (currentRow == "") {
    //Serial.println("Leera otros 1000 datos");
    resp = readFile();
    if (resp == -1) {
      return 1;
    }
    //Serial.print(dataBuffer);
    currentRow = nextRow();
  }
  if (fileType == 3) {
    resp = getComponentsBin((dataBufferBin + pFileBin * 6), component1, component2);
  }
  else {
    resp = getComponents(currentRow, component1, component2);
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

int FileSara::getComponents(String line, double* c1, double* c2) {
    int commaIndex = line.indexOf(this->componentSeparator);
    if (commaIndex == -1) { //no separator detected
      this->statusFile = -1;
      return -1;
    }
    if (commaIndex >= line.length() - 1) { //no second component detected
      this->statusFile = -4;
      return -4;
    }
    String _c1 = line.substring(0, commaIndex);
    String _c2 = line.substring(commaIndex + 1);
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

int FileSara::getComponentsBin(uint8_t* line, double* c1, double* c2){
    if ( *(line + 2) != ',') {
      statusFile = -1;
      return -1;
    }
    int16_t* puint16;
    puint16 = (int16_t* )line;
    xbin = *puint16;
    puint16 = (int16_t* )(line + 3);
    ybin = *puint16;
    *c1 = double(xbin);
    *c2 = double(ybin);
    statusFile = 0;
    return 0;
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

/**
 * @param component, it can be 1 or 2 refering to first or sencond
 * component, respectively.
 * @param ignoreZero if is 1, it will iterate until find a point
 * defferent then 0,0, by default is 0.
 * @return the first component of the file
 */
double FileSara::getStartPoint(int component, int ignoreZero){
  int stackMode = directionMode;
  directionMode = 1;
  double component1, component2, zStart, thetaStart;
  pFile = 0;
  if (ignoreZero == 1 && fileType != 2){
    getNextComponents(&component1, &component2);
    while (component1 == 0 && component2 == 0){
      if (getNextComponents(&component1, &component2) == 1){
        break;
      }      
    }
  }
  else
  {
    getNextComponents(&component1, &component2);
  }
  pFile = 0;
  dataBuffer = "";
  directionMode = stackMode;

  if (fileType == 2){
    if (component == 1)
      return component1;
    else
      return component2;
  }
  zStart = MoveSara::zPolar(component1, component2);
  thetaStart = MoveSara::thetaPolar(component1, component2);
  if (component == 1)
    return zStart;
  else
    return thetaStart;
}

/**
 * @param component, it can be 1 or 2 refering to first or sencond
 * component, respectively.
 * @param ignoreZero if is 1, it will iterate until find a point
 * defferent then 0,0, by default is 0.
 * @return the last component of the file
 */
double FileSara::getFinalPoint(int component, int ignoreZero){
  int stackMode = directionMode;
  directionMode = 0;
  double component1, component2, zFinal, thetaFinal;
  pFile = file.size();
  if (ignoreZero == 1 && fileType != 2){
    getNextComponents(&component1, &component2);
    while (component1 == 0 && component2 == 0){
      if (getNextComponents(&component1, &component2) == 1){
        break;
      }
    }
  }
  else
  {
    getNextComponents(&component1, &component2);
  }
  pFile = file.size();
  dataBuffer = "";
  directionMode = stackMode;

  if (fileType == 2){
    if (component == 1)
      return component1;
    else
      return component2;
  }
  zFinal = MoveSara::zPolar(component1, component2);
  thetaFinal = MoveSara::thetaPolar(component1, component2);
  if (component == 1)
    return zFinal;
  else
    return thetaFinal;
}

void FileSara::autoSetMode(double zCurrent){
  double startZ, finalZ, diff1, diff2;
  startZ = getStartPoint();
  finalZ = getFinalPoint();
  diff1 = abs(zCurrent - startZ);
  diff2 = abs(zCurrent - finalZ);
  if (diff1 < diff2){
    directionMode = 1;
    pFile = 0;
  }
  else if (diff2 < diff1){
    directionMode = 0;
    pFile = file.size();
  }
  else{
    directionMode = 1;
    pFile = 0;
  }
}

/**
 * this function should be call before the process of reading file
 * because this restart the pFile and dataBuffer viariables
 * it should also call after set the directionMode viariable or
 * in the same way to have called autoSetMode()
 * @return the angle of the last point acoordint to the directionMode
 */
double FileSara::getFinalAngle(){
  if (directionMode == 0){
    return getStartPoint(2, 1);
  }
  else
  {
    return getFinalPoint(2, 1);
  }
}

double FileSara::getStartAngle(){
  if (directionMode == 0){
    return getFinalPoint(2, 1);
  }
  else
  {
    return getStartPoint(2, 1);
  }
}