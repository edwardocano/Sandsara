/*Para poder usar el simulador se deben modificar las primeras lineas del archivo MoveSara.h como se muestra a continuacion.

#define PROCESSING_SIMULATOR
//#define DEBUGGING_DATA
//#define DEBUGGING_DETAIL
//#define DISABLE_MOTORS

se prueba con un esp32 que manda los siguientes datos

  Serial.print(q1_steps);
  Serial.print(",");
  Serial.print(q2_steps + q1_steps);
  Serial.print(",");
  Serial.println(velocidad);
  
  con un mesaje de inicio al inicio
  Serial.println("inicia");
  delay(1000);
  
  solo considerar eso y modificar el puerto COM o USB
*/
