#include "FileSara.h"
#include "MoveSara.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <math.h>

File myFile;
File root;

MoveSara halo(16);

void setup() {
  Serial.begin(115200);
  // Configure the halo
  halo.init();

  if (!SD.begin()) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  myFile = SD.open("/");
  if (!myFile) {
    Serial.println("No se pudo abrir archivo");
    return;
  }
  root = SD.open("/");
  delay(1000);
  #ifdef PROCESSING_SIMULATOR
  Serial.println("inicia");
  #endif
}


void loop() {
  //halo.moveTo(50, 50);
  #ifdef DEBUGGING_DATA
  Serial.println("Iniciara la funcion runSansara");
  #endif
  run_sandsara(root);
  root.rewindDirectory();
  delay(5000);
}

void run_sandsara(File& dircurrent) {
  double component_1, component_2;
  double _z, _theta;
  double theta_aux;
  double x_aux, y_aux;
  double coupling_angle;
  while (1) {
    #ifdef DEBUGGING_DATA
    Serial.println("Abrira el siguiente archivo disponible");
    #endif
    File current_file =  dircurrent.openNextFile();
    if (! current_file) {
      #ifdef DEBUGGING_DATA
      Serial.println("No hay archivo disponible");
      #endif
      break;
    }
    else if (!current_file.isDirectory()) {
      int working_status = 0;
      String name_file = current_file.name();
      current_file.close();
      #ifdef PROCESSING_SIMULATOR
      Serial.print("fileName: ");
      Serial.println(name_file);
      #endif
      double couplingAngle, startRobotAngle, startFileAngle;
      FileSara file(name_file);
      double zInit = halo.getCurrentModule();
      //se selecciona modo de lectura
      file.autoSetMode(zInit);
      startFileAngle = file.getStartAngle();
      startFileAngle = MoveSara::normalizeAngle(startFileAngle);
      startRobotAngle = halo.getCurrentAngle();
      couplingAngle = startFileAngle - startRobotAngle;
      // si es thr, se guardan los valores del primer punto para que tenga referencia de donde empezar a moverse.
      if (file.fileType == 2) {
        file.getNextComponents(&component_1, &component_2);
        halo.setZCurrent(component_1);
        halo.setThetaCurrent(component_2 - couplingAngle);
      }
      //parara hasta que el codigo de error del archivo sea diferente de cero.
      while ( 1 ) {
        //se obtienen los siguientes componentes
        working_status = file.getNextComponents(&component_1, &component_2);
        if (working_status != 0){
          break;
        }
        //dependiendo del tipo de archivo se ejecuta la funcion correspondiente de movimiento.
        if (file.fileType == 1 || file.fileType == 3){
          MoveSara::rotate(component_1 , component_2, -couplingAngle);
          halo.moveTo(component_1, component_2);
        }
        else if (file.fileType == 2) {
          halo.movePolarTo(component_1, component_2 - couplingAngle);
        }
        else{
          break;
        }
      }
      #ifdef PROCESSING_SIMULATOR
      Serial.println("finished");
      #endif
    }
  }
}