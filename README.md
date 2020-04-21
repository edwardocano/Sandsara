# Sandsara
Sandsara Firmware
## Pasos a seguir para quemarle el código al esp32

- Instalar el complemento de ESP32 al IDE de arduino siguiendo estas instrucciones [https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
- Instalar la librería AccelStepper (by Mike McCauley) desde el library manager de arduino
- Clonar el repositorio del código de github.
- cambiar el nombre de la carpeta por el nombre del archivo .ino (firmware_Sandsara_ESP32)
- Compilar y subir el programa desde el IDE de Arduino

-Note: si la consola marca un error de "timed out waiting for packet header" es porque no esta en modo flashing/uploading, para solucionar este error, al momento de intentar subir el codigo al esp32 se debe mantener presionado el boton "boot" de la placa.

![diagrama de conexiones](https://github.com/edwardocano/Sandsara/blob/master/PinoutConection.jpg)
