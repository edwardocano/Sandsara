# Sandsara
Sandsara Firmware
#Features version 1.0.0
-25 comandos para la comunicacion con Sandsara por medio de Bluetooth.
-Interpolacion de puntos para archivos con extension THR.
-Control de los LED's RGB.
-16 paletas de colres precargadas para los leds.
-Lectura de archivos .TXT, .BIN y .THR desde la memoria SD.
-Secuencia de calibracion de los motores para obtener su posicion inicial.
-Calculo de la cinematica directa e inversa de Sandsara.
-4 modos de orden de reproduccion de los archivos
--Orden desendente de los paths en la lista de reproduccion.
--Orden random de los paths de la lista de reproduccion.
--Orden fijo de todos los paths contenidos en la memoria SD.
--Orden random de todos los paths contenidos en la memoria SD.
-Interpolacion lineal entre puntos separados a mas de 1 mm.
-Restricciones del area de trabajo para Hallo y Stelle.
-mezcla de colores en el espacio de rgb^2 para las paletas de colores.
-Thread para el control de los motores.
-Thread para el control de los leds.
-Thread para el manejo de los comandos por bluetooth.
-Guardado de datos importantes en la EEPROM(lista de reproduccion, orden de reproduccion, paleta de colores, velocidad de la esfera, velocidad de los leds, etc.)
-Funcion de testeo para verificar el correcto funcionamiento de todos los componentes.

#proximas caracteristicas caracteristicas
-aceleracion y desaceleracion para evitar los movimientos bruscos de los motores que se presentan en ciertos paths.
-aumentar la corriente de los motores a 600mA en la calibracion.

Se esta trabajando en la generacion de la documentacion del código.

#Compilacion del proyecto
Para una rapida y facil compilacion del proyecto nosotros recomendamos utilizar platformIO y Visual Studio Code, en el archivo platformio.ini se encuentran las librerias que necesarias para compilar el proyecto. Para descargar y compilar el proyecto recomendamos los siguientes pasos.
1. Descargar Visual Studio Code.
2. Instalar el complemento de PlatformIO.
3. Clonar el repositorio en una nueva carpeta.
4. Abrir la carpeta, donde se clono el proyecto, desde Visual Studio Code.
5. Seguir los pasos del apartado de Consideraciones.
6. Presionar el Boton de "platformIO: Build".
7. Listo.

##Consideraciones
Es importante moficar las siguientes librerias antes de compilar el proyecto.

El archivo HardwareSerial.cpp modificar las siguientes lineas
linea 10: #define RX1 9 cambiar por #define RX1 26
linea 14: #define TX1 10 cambiar por #define TX1 27

En el archivo BluetoothSerial.cpp modificar las siguientes lineas para aumentar el buffer del bluetooth.
Linea 45: #define RX_QUEUE_SIZE 512 cambiar por #define RX_QUEUE_SIZE 30100
Linea 46: #define TX_QUEUE_SIZE 32  cambiar por #define TX_QUEUE_SIZE 1024
