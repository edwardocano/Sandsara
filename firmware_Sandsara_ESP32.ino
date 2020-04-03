#include "FileSara.h"
#include "MoveSara.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <math.h>

File myFile;
File root;

int movement = 1;
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
  Serial.println("inicia");
}


void loop() {
  //halo.moveTo(50, 50);
  run_sandsara(root);
  //while(1){delay(10000);}
  delay(5000);
}

void run_sandsara(File dircurrent) {
  double component_1, component_2;
  double _z, _theta;
  double theta_aux;
  double x_aux, y_aux;
  double coupling_angle;
  while (1) {

    File current_file =  dircurrent.openNextFile();
    if (! current_file) {
      dircurrent.rewindDirectory();
      break;
    }
    else if (!current_file.isDirectory()) {
      int working_status = 0;
      String name_file = current_file.name();
      current_file.close();

      FileSara file(name_file, movement);
      if (file.fileType == 2) {
        file.getNextComponents(&component_1, &component_2);
        halo.z_current = component_1;
        halo.theta_current = component_2;
        halo.setCouplingAngle();
      }
      while ( working_status == 0) {
        working_status = file.getNextComponents(&component_1, &component_2);
        Serial.print(component_1);
        Serial.print(", ");
        Serial.println(component_2);
        if (file.fileType == 1 || file.fileType == 3)
          halo.moveTo(component_1, component_2);
        else if (file.fileType == 2) {
          halo.movePolarTo(component_1, component_2);
        }
      }
      Serial.println("finished");
      if (movement == 1)
        movement = 0;
      else
        movement = 1;
    }
  }
}