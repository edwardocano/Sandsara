#include "FileSara.h"
#include "MoveSara.h"
extern SdFat SD;

//====prototipos de funcion====
bool sdExists(String );
bool sdRemove(String );
//====

/**
 * @brief es el contructor de la clase FileSara
 * @param nameF es el nombre del archivo que se va a leer desde la SD
 * @param directionMode es la direccion de lectura del archivo, por defecto es 1.
 * 1 representa que se va a leer el archivo de arriba hacia abajo y 0 que se debe leer de abajo para arriba.
 */
FileSara::FileSara(String nameF, int directionMode)
{
    fileName = nameF;
    fileType = getType(fileName);
    this->directionMode = directionMode;
#ifdef DEBUGGING_DATA
    Serial.print("Se abrira el archivo: ");
    Serial.println(fileName);
#endif
    file = SD.open(fileName);
    if (directionMode == 1)
    {
        pFile = 0;
    }
    else
    {
        pFile = file.size();
    }
    if (fileType == 2)
    {
        componentSeparator = ' ';
        lineSeparator = '\n';
    }
    if (fileType == 3)
    {
        charsToRead = 1002; //it must be a multiple of 6
        dataBufferBin = (uint8_t *)calloc(charsToRead + 1, 1);
        if (directionMode == 0)
        {
            pFileBin = 0;
        }
        else
        {
            pFileBin = charsToRead / 6;
        }
    }
    else
    {
        dataBufferBin = (uint8_t *)calloc(charsToRead + 1, 1);
    }
}

/**
 * @brief es el destructor de la clase
 * 
 * se utiliza un destructor para limpiar liberar la memoria dinamica y cerrar el
 * el archivo de tipo File
 */
FileSara::~FileSara()
{
    file.close();
    free(dataBufferBin);
#ifdef DEBUGGING_DATA
    Serial.print("Se destruyo el archivo: ");
    Serial.println(fileName);
#endif
}

/**
 * @brief lee la siguiente linea del archivo y extrae el valor de los componentes que tiene
 * @param component1 es donde se almacenara el primero componente (x o z)
 * @param component2 es donde se almacenara el segundo componente (y o theta)
 * @note los componentes se encuentran comprendidos en los siguientes rangos.
 * x,y --> [-150,150]
 * z   --> [0, 150]
 * theta --> (-inf, inf)
 * Sin embargo los datos se pueden salir de estos rangos si el archivo no sigue las indicaciones correctas.
 * @return un codigo de error que puede significar lo siguiente
 *  0 no hubo problemas
 *  1 ya no hay mas lineas por leer en el archivo
 *  3 la linea es un comentario.
 * -1 no se encontro el separador de los componentes (',' o ' ')
 * -4 no se encontro un segundo componente
 * -6 no es un tipo de archivo valido
 * -10 ya no hay mas archivos por leer por posible problema con memoria sd.
 */
int FileSara::getNextComponents(double *component1, double *component2)
{
    int resp;
    currentRow = nextRow();
    if (currentRow == "")
    {
        resp = readFile();
        if (resp != 0)
        {
            statusFile = resp;
            return resp;
        }
        currentRow = nextRow();
    }
    if (fileType == 3)
    {
        resp = getComponentsBin((dataBufferBin + pFileBin * 6), component1, component2);
    }
    else
    {
        resp = getComponents(currentRow, component1, component2);
    }
    if (resp != 0)
    {
        return statusFile;
    }

    if (fileType == 1 || fileType == 3)
    {
        *component1 = DISTANCIA_MAX * *component1 / RESOLUCION_MAX;
        *component2 = DISTANCIA_MAX * *component2 / RESOLUCION_MAX;
    }
    else if (fileType == 2)
    {
        z = *component2 * DISTANCIA_MAX;
        theta = *component1;
        *component1 = z;
        *component2 = theta;
    }
    else
    {
        statusFile = -6;
        return -6;
    }
    return 0;
}

/**
 * @return el estado actual del archivo leido
 * @see getNextComponents
 */
int FileSara::getStatus()
{
    return this->statusFile;
}

/**
 * @brief Encuentra el tipo de archivo que se va a leer.
 * @return un numero que representa uno de los siguientes tipos de archivo.
 * 1 para un .txt.
 * 2 para un .thr.
 * 3 para un .bin.
 * -1 ninguno de los anteriores.
 */
int FileSara::getType(String name_file)
{
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

/**
 * @brief de una linea de texto extrae la informacion de los 2 componentes.
 * @param line Es el texto con informacion de los componentes.
 * @param c1 es donde se va a almacenar el valor del componente 1.
 * @param c2 es donde se va a almacenar el valor del componente 2.
 * @note los valores devuelven por medio de c1 y c2.
 * @return un codigo de error, pudiendo ser uno de los siguientes
 *  3, la linea es un comentario
 * -1, no encuentra el caracter separador.
 * -4, no encuentra el segundo componente.
 * 
 * @see getNextComponents.
 */
int FileSara::getComponents(String line, double *c1, double *c2)
{
    if (line.charAt(0) == '/' || line.charAt(0) == '#' || line.charAt(0) == '\r' || line.charAt(0) == '\n')
    {
        statusFile = 3;
        return 3;
    }
    if (line.indexOf('\t') >= 0)
    {
        line.replace("\t", " ");
    }
    int commaIndex = line.indexOf(this->componentSeparator);
    if (commaIndex == -1)
    { //no separator detected
        this->statusFile = -1;
        return -1;
    }
    if (commaIndex >= line.length() - 1)
    { //no second component detected
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

/**
 * @brief De 6 bytes se extrae la informacion de los 2 componentes.
 * @param line es un apuntador al inicio de lo que se va a leer.
 * @param c1 es donde se va a almacenar el valor del componente 1.
 * @param c2 es donde se va a almacenar el valor del componente 2.
 * @note los valores devuelven por medio de c1 y c2.
 * @return un codigo de error.
 * @see getNextComponents.
 */
int FileSara::getComponentsBin(uint8_t *line, double *c1, double *c2)
{
    if (*(line + 2) != ',')
    {
        statusFile = -1;
        return -1;
    }
    int16_t *puint16;
    puint16 = (int16_t *)line;
    xbin = *puint16;
    puint16 = (int16_t *)(line + 3);
    ybin = *puint16;
    *c1 = double(xbin);
    *c2 = double(ybin);
    statusFile = 0;
    return 0;
}

/**
 * @brief Encuentra la siguiente linea a ser leida del archivo o su direccion en el caso de un archivo .bin.
 * @return Un texto que representa la siguiente linea del archivo. En el caso de estar leyendo un .bin
 * regresara la pablabra "nonStop" si aun hay lineas por leer y un "" si ya no hay mas lineas por leer.
 * @note En el caso de estar leyendo un .bin se modificara la variable pFileBin la cual
 * representa el el numero de linea siguiente.
 */
String FileSara::nextRow()
{
    String return_str;
    int index_separator;

    if (this->directionMode == 0)
    {
        //<BINCASE>
        if (fileType == 3)
        {
            pFileBin -= 1;
            if (pFileBin < 0)
            {
                return "";
            }
            return "nonStop";
        }
        //</BINCASE>
        index_separator = this->dataBuffer.lastIndexOf(this->lineSeparator);
        if (index_separator == -1)
        {
            return this->dataBuffer;
        }
        if (index_separator >= this->dataBuffer.length() - 2)
        { // -2 is because the end of a line can be \r\n or \n\r
            index_separator = this->dataBuffer.lastIndexOf(this->lineSeparator, index_separator - 1);
            if (index_separator == -1)
            {
                return_str = this->dataBuffer;
                this->dataBuffer = "";
                return return_str;
            }
            return_str = this->dataBuffer.substring(index_separator + 1);
            this->dataBuffer.remove(index_separator + 1);
            return return_str;
        }
        else
        {
            return_str = this->dataBuffer.substring(index_separator + 1);
            this->dataBuffer.remove(index_separator + 1);
            return return_str;
        }
    }
    else
    {
        //<BINCASE>
        if (fileType == 3)
        {
            pFileBin += 1;
            if (pFileBin > charsToRead / 6 - 1)
            {
                return "";
            }
            return "nonStop";
        }
        //</BINCASE>
        index_separator = this->dataBuffer.indexOf(this->lineSeparator);
        if (index_separator == -1 || index_separator == this->dataBuffer.length() - 1)
        {
            return_str = this->dataBuffer;
            this->dataBuffer = "";
            return return_str;
        }
        return_str = this->dataBuffer.substring(0, index_separator + 1);
        this->dataBuffer = this->dataBuffer.substring(index_separator + 1);
        return return_str;
    }
}

/**
 * @brief lee no mas de 1002 bytes del archivo y los almacena.
 * 
 * Los datos los almacena en dataBuffer en el caso de archivos .txt o .thr y en dataBufferBin para archivos .bin
 * @return un codigo de error que puede significar lo siguiente.
 *  0 no hubo problemas.
 *  1 ya no hay datos restantes por leer.
 * -10 ya no hay mas archivos por leer por posible problema con memoria sd.
 */
int FileSara::readFile()
{
    int bytesRead;
    if (this->directionMode == 0)
    {
        if (pFile >= charsToRead)
        {
            pFile -= charsToRead;
        }
        else
        {
            charsToRead = pFile; //check if it has to be +1
            if (pFile == 0)
            {
                return 1;
            }
            pFile = 0;
        }
        this->file.seek(pFile);
        //char input_char[charsToRead + 1];
        bytesRead = this->file.read(dataBufferBin, charsToRead);
        if (bytesRead < charsToRead)
        {
            statusFile = -10;
            return -10;
        }
        *(dataBufferBin + charsToRead) = '\0';
        //<BINCASE>
        if (fileType == 3)
        {
            pFileBin = charsToRead / 6;
            return 0;
        }
        //</BINCASE>

        dataBuffer = String((char *)dataBufferBin);
        if (pFile == 0)
        {
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
            return 1;
        }
        if (pFile + charsToRead > file.size())
        {
            charsToRead = file.size() - pFile; //
        }
        this->file.seek(pFile);
        //char input_char[charsToRead + 1];
        bytesRead = this->file.read(dataBufferBin, charsToRead);
        if (bytesRead < charsToRead)
        {
            statusFile = -10;
            return -10;
        }
        *(dataBufferBin + charsToRead) = '\0';
        //<BINCASE>
        if (fileType == 3)
        {
            pFile += charsToRead;
            pFileBin = -1;
            return 0;
        }
        //</BINCASE>
        dataBuffer = String((char *)dataBufferBin);
        int index_nl = dataBuffer.lastIndexOf(this->lineSeparator);
        if (index_nl == -1)
        {
            pFile = file.size();
            return 0;
        }

        dataBuffer = dataBuffer.substring(0, index_nl + 1);
        pFile += index_nl + 1;
        return 0;
    }
}

/**
 * @brief regresa un componente perteneciente al primer punto del archivo.
 * @param component es la poscion del componente que se quiere obtener, es decir:
 * 1 --> para el primer componente
 * 2 --> para el segundo componente
 * en el caso de archivos .txt o .bin el primer componente es 'x', y el segundo 'y'.
 * en el caso de archivos .thr el primer componente es 'z', y el segundo 'theta'.
 * @param ignoreZero si este valor es 1, devolvera el primer punto que no sea 0,0 (coordenadas cartesianas)
 * si es un archivo .thr este parametro no aplica.
 * @return el componente deseado del primer punto del archivo.
 */
double FileSara::getStartPoint(int component, int ignoreZero)
{
    int resp;
    int stackMode = directionMode;
    int stackPFileBin = pFileBin;
    long stackPFile = pFile;
    directionMode = 1;
    double component1, component2, zStart, thetaStart;
    pFile = 0;
    pFileBin = charsToRead / 6;
    if (ignoreZero == 1 && fileType != 2)
    {
        resp = getNextComponents(&component1, &component2);
        while (resp == 3)
        {
            resp = getNextComponents(&component1, &component2);
        }
        while (component1 == 0 && component2 == 0)
        {
            if (getNextComponents(&component1, &component2) == 1)
            {
                break;
            }
        }
    }
    else
    {
        resp = getNextComponents(&component1, &component2);
        while (resp == 3)
        {
            resp = getNextComponents(&component1, &component2);
        }
    }
    pFile = stackPFile;
    pFileBin = stackPFileBin;
    dataBuffer = "";
    directionMode = stackMode;

    if (component == 1)
    {
        return component1;
    }
    else
    {
        return component2;
    }
}

/**
 * @brief regresa un componente perteneciente al ultimo punto del archivo.
 * @param component es la poscion del componente que se quiere obtener, es decir:
 * 1 --> para el primer componente
 * 2 --> para el segundo componente
 * en el caso de archivos .txt o .bin el primer componente es 'x', y el segundo 'y'
 * en el caso de archivos .thr el primer componente es 'z', y el segundo 'theta'
 * @param ignoreZero si este valor es 1, devolvera el primer punto que no sea 0,0 (coordenadas cartesianas)
 * si es un archivo .thr este parametro no aplica.
 * @return el componente deseado del ultimo punto del archivo.
 */
double FileSara::getFinalPoint(int component, int ignoreZero)
{
    int resp;
    int stackMode = directionMode;
    int stackPFileBin = pFileBin;
    long stackPFile = pFile;
    directionMode = 0;
    double component1, component2, zFinal, thetaFinal;
    pFile = file.size();
    pFileBin = 0;
    if (ignoreZero == 1 && fileType != 2)
    {
        resp = getNextComponents(&component1, &component2);
        while (resp == 3)
        {
            resp = getNextComponents(&component1, &component2);
        }
        while (component1 == 0 && component2 == 0)
        {
            if (getNextComponents(&component1, &component2) == 1)
            {
                break;
            }
        }
    }
    else
    {
        resp = getNextComponents(&component1, &component2);
        while (resp == 3)
        {
            resp = getNextComponents(&component1, &component2);
        }
    }
    pFile = stackPFile;
    pFileBin = stackPFileBin;
    dataBuffer = "";
    directionMode = stackMode;

    if (component == 1)
    {
        return component1;
    }
    else
    {
        return component2;
    }
}

/**
 * @brief Seleccion la direccion de lectura del archivo dependiendo del estado actual del robot
 * 
 * para ello encuentra el modulo del primer y utlimo punto del archivo y calcula cual es el más cercano de la posicion actual del robot
 * si es el ultimo punto del archivo, entonces iniciará a leerlo de arriba hacia abajo y si no, de abajo hacia arriba
 * si la distancia a la que se encuentra es la misma de ambos puntos del archivo entonces leera el archivo de arriba hacia abajo
 * @param zCurrent Es la distancia actual del centro a la punta del robot
 * @note No regresa nada, pero cambia el valor de la variable directionMode
 */
void FileSara::autoSetMode(double zCurrent)
{
    double startZ, finalZ, diff1, diff2;
    double angle, x, y;
    x = getStartPoint(1, 0);
    y = getStartPoint(2, 0);
    startZ = MoveSara::zPolar(x, y);
    if (fileType == 2)
    {
        startZ = x;
    }

    x = getFinalPoint(1, 0);
    y = getFinalPoint(2, 0);
    finalZ = MoveSara::zPolar(x, y);
    if (fileType == 2)
    {
        finalZ = x;
    }

    diff1 = abs(zCurrent - startZ);
    diff2 = abs(zCurrent - finalZ);
    if (diff1 < diff2)
    {
        directionMode = 1;
        pFile = 0;
        pFileBin = charsToRead / 6;
    }
    else if (diff2 < diff1)
    {
        directionMode = 0;
        pFile = file.size();
        pFileBin = 0;
    }
    else
    {
        directionMode = 1;
        pFile = 0;
        pFileBin = charsToRead / 6;
    }
}

/**
 * @brief la funcion encuentra el angulo del ultimo punto que va a ser leido, esto
 * depende de la dirección de lectura (directionMode)
 * @note Esta funcion debe ser llamada antes del proceso de lectura porque modifica
 * las variables pFile y dataBuffer, y despues de haber asignado un valor a directionMode
 * @return un valor que representa un angulo en radianes pudiendo tomar cualquier valor que permita un tipo de dato double
 */
double FileSara::getFinalAngle()
{
    double angle, x, y;
    if (directionMode == 0)
    {
        if (fileType == 2)
        {
            angle = getStartPoint(2, 0);
            return angle;
        }
        else
        {
            x = getStartPoint(1, 1);
            y = getStartPoint(2, 1);
            angle = MoveSara::thetaPolar(x, y);
            return angle;
        }
    }
    else
    {
        if (fileType == 2)
        {
            angle = getFinalPoint(2, 0);
            return angle;
        }
        else
        {
            x = getFinalPoint(1, 1);
            y = getFinalPoint(2, 1);
            angle = MoveSara::thetaPolar(x, y);
            return angle;
        }
    }
}
/**
 * @brief la funcion encuentra el angulo del primer punto que va a ser leido, esto
 * depende de la dirección de lectura (directionMode)
 * @return un valor que representa un angulo en radianes pudiendo tomar cualquier valor que permita un tipo de dato double
 */
double FileSara::getStartAngle()
{
    double angle, x, y;
    if (directionMode == 0)
    {
        if (fileType == 2)
        {
            angle = getFinalPoint(2, 0);
            return angle;
        }
        else
        {
            x = getFinalPoint(1, 1);
            y = getFinalPoint(2, 1);
            angle = MoveSara::thetaPolar(x, y);
            return angle;
        }
    }
    else
    {
        if (fileType == 2)
        {
            angle = getStartPoint(2, 0);
            return angle;
        }
        else
        {
            x = getStartPoint(1, 1);
            y = getStartPoint(2, 1);
            angle = MoveSara::thetaPolar(x, y);
            return angle;
        }
    }
}

double FileSara::getStartModule(){
    double module, x, y;
    if (directionMode == 0)
    {
        if (fileType == 2)
        {
            module = getFinalPoint(1, 0);
            return module;
        }
        else
        {
            x = getFinalPoint(1, 1);
            y = getFinalPoint(2, 1);
            module = MoveSara::zPolar(x, y);
            return module;
        }
    }
    else
    {
        if (fileType == 2)
        {
            module = getStartPoint(1, 0);
            return module;
        }
        else
        {
            x = getStartPoint(1, 1);
            y = getStartPoint(2, 1);
            module = MoveSara::zPolar(x, y);
            return module;
        }
    }
}

//------------------------------Ordenar archivos----------------------------------
//--------------------------------------------------------------------------------
/**
 * @brief devuelve la linea lineNumber del archivo dirFile.
 * @param lineNumber es el numero de la linea que se desea leer, para la primera linea este parametro debe ser 1, no 0.
 * @param dirFile es la direccion del archivo, empezando con '/'.
 * @param lineText es la variable donde se va a guardar el contenido de la linea leida.
 * @return un codigo de error, pudiendo ser alguno de los siguientes.
 * 0, Encontro la linea lineNumber y esta linea termina con un '\n'.
 * 1, Encontro la linea lineNumber, pero no termina con '\n'.
 * 2, No encontro la linea lineNumber y no se encontro un '\n' al final de esta linea, por lo que en lineText esta guardada la ultima linea del archivo.
 * 3, No encontro la linea lineNumber y sí se encontro un '\n' al final de esta linea, por lo que en lineText esta guardada la ultima linea del archivo.
 * -1, No se pudo abrir el archivo con la direccion dirFile.
 * -2, El archivo abierto es un directorio.
 * -3, La linea que se desea leer no es valida.
 */
int FileSara::getLineNumber(int lineNumber, String dirFile, String &lineText)
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
int FileSara::creatListOfFiles(String fileName)
{
    File file, root, fileObj;
    int numberOfFiles = 0;
    root = SD.open("/");
    sdRemove(fileName);
    file = SD.open(fileName, FILE_WRITE);
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
            char nameF[NAME_LENGTH];
            fileObj.getName(nameF,NAME_LENGTH);
            String varName = nameF;
            String varNameLower = varName;
            varNameLower.toLowerCase();
            if (varNameLower.equals(fileName) || getType(varNameLower) == -1)
            {
                continue;
            }
            file.print(varName + "\r\n");
            numberOfFiles += 1;
        }
    }
}

/**
 * @brief calcula el numero de lineas de un archivo
 * @param dir es la direccion en la sd del archivo, debe contener un '/', ejemplo "/archivo.thr".
 * @return el numero de lineas que contiene el archivo
 */
int FileSara::numberOfLines(String dir){
    File f;
    int character, count = 1;
    f = SD.open(dir);
    if (!f){
        return 0;
    }
    if (f.isDirectory()){
        return 0;
    }
    while(true)
    {
        character = f.read();
        if (character == '\n'){
            count += 1;
        }
        else if (character == -1){
            break;
        }
    }
    return count;
}

/**
 * @brief comprueba si un archivo es valido.
 * @return si es valido regresa true si no false.
 */
bool FileSara::isValid(){
    int resp;
    int stackMode = directionMode;
    int stackPFileBin = pFileBin;
    long stackPFile = pFile;
    double component1, component2;
    resp = getNextComponents(&component1, &component2);
    while (resp == 3)
    {
        resp = getNextComponents(&component1, &component2);
    }
    pFile = stackPFile;
    pFileBin = stackPFileBin;
    dataBuffer = "";
    directionMode = stackMode;

    if (resp == 0){
        return true;
    }
    else{
        return false;
    }
}

bool sdExists(String dirName){
    return SD.exists(dirName.c_str());
}

bool sdRemove(String dirName){
    return SD.remove(dirName.c_str());
}