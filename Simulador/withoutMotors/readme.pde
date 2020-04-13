/*En serial_data no se considera el caso en que value
sea null y el vector dvalue toma valores desconocidos*/

/*se prueba con un esp32 que manda los siguientes datos

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
