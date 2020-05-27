#include "CalibMotor.h"

hw_timer_t * timer1 = NULL; 

MeanFilter<long> meanFilter(5);
MeanFilter<long> meanFilter2(40);
MeanFilter<long> meanFilter3(10);
MeanFilter<long> meanFilter4(10);

int flag = 0;
int avoid = 0;
int avoid2 = 0;
int s_dir;                // 0 = positive dir ...... 1 = negative dir
int flag3 = 0;

int p = 0;
int motor_degrees = 0;
int Velocidad = 5000;

int dato[100];
int dato2[300];
int dato3[200];
int maximo;

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driver2(&SERIAL_PORT2, R_SENSE2, DRIVER_ADDRESS2);

void giro_normal();
void mover(int , int );
void slow_Calibration_hall(int );
void slow_Calibration_encoder(int s_dir);

CalibMotor::CalibMotor(){

}

void IRAM_ATTR onTimer() {
  
  //STEP_PORT ^= 1 << STEP_BIT_POS;
  if(flag == 0){
    digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
  }

  if (flag == 2){
    digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
    digitalWrite(STEP_PIN2, !digitalRead(STEP_PIN2));
  }

  if(flag == 3){
    digitalWrite(STEP_PIN2,!digitalRead(STEP_PIN2));
  }
  
} 

int CalibMotor::init(){
    Serial.println("\nStart...");

    SERIAL_PORT.begin(115200);
    SERIAL_PORT2.begin(115200);

    
    pinMode(DIAG_PIN, INPUT);
    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);

    pinMode(DIAG_PIN2,INPUT);
    pinMode(EN_PIN2, OUTPUT);
    pinMode(STEP_PIN2, OUTPUT);
    pinMode(DIR_PIN2, OUTPUT);
    
    digitalWrite(EN_PIN, LOW);
    digitalWrite(DIR_PIN,LOW);

    digitalWrite(EN_PIN2,HIGH);
    digitalWrite(DIR_PIN2,LOW);
    
    driver.begin();
    driver.pdn_disable(true);  
    driver.toff(4);
    driver.blank_time(24);
    driver.rms_current(500); 
    driver.microsteps(16);
    driver.TCOOLTHRS(0xFFFFF); // 20bit max
    driver.semin(0);
    //driver.semax(2);
    driver.shaft(false);
    driver.sedn(0b01);
    driver.SGTHRS(STALL_VALUE);

    driver2.begin();
    driver2.pdn_disable(true);  
    driver2.toff(4);
    driver2.blank_time(24);
    driver2.rms_current(500); 
    driver2.microsteps(16);
    driver2.TCOOLTHRS(0xFFFFF); // 20bit max
    driver2.semin(0);
    //driver.semax(2);
    driver2.shaft(false);
    driver2.sedn(0b01);
    driver2.SGTHRS(STALL_VALUE2);
}

int CalibMotor::start(){
    //////////////////////////////////////////////////////////////////////
////////////////////////////WITH STALLGUARD///////////////////////////
    giro_normal();
    delay(100);
    while(true){
    if (digitalRead(DIAG_PIN) == 1){
        Serial.println("conflicto");
        flag = 1; 
    //Back a little  first arm
        digitalWrite(DIR_PIN,HIGH);
        mover(300,1);

    //Move second arm 90 degrees.    
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW);
        digitalWrite(DIR_PIN2 , LOW);
        mover(1600,2);

    //move both arms until encoder detects
        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 , LOW);
        while(analogRead(encoder)<600){
        flag=2;
        }
        flag=1;

    //move second arm until hall detects
        int mean = meanFilter.AddValue(analogRead(hall));
        while (mean < 2400){                                    //Revisar valor de umbral.
            mean = meanFilter.AddValue(analogRead(hall));
            Serial.println(mean);
            flag = 3;
        }

        flag=1;
        delay(200);
        s_dir = 0;
        slow_Calibration_encoder(s_dir);
        slow_Calibration_hall(s_dir);
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW); 
        avoid = 1;
        Serial.println("Done with stallGuard");
    }

    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////WITHOUT STALLGUARD////////////////////////////

    if(analogRead(encoder)>600 && avoid == 0){
        
        flag = 1;
        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 , HIGH);
        
        int mean = meanFilter.AddValue(analogRead(hall));
        digitalWrite(EN_PIN2,LOW);
        driver.rms_current(1500);                     //incrementar corriente a motor 1 para evitar interferencia con m2 
        delay(100);

        while (mean < 2400){
            mean = meanFilter.AddValue(analogRead(hall));
            flag = 3;
            //Serial.println(driver2.SG_RESULT(), DEC);
            /////////////////Secuencia donde se encuentra entre ambos picos
            delay(50);                            
            if(digitalRead(DIAG_PIN2) == 1 and avoid2 == 1){
                flag = 1;
                //secuencia donde el brazo se encuentra entre ambos picos//regresa main 30 grados, el otro gira 90, y ambos se mueven juntos.
                //main arm 30 degree
                delay(250);
                digitalWrite(DIR_PIN, HIGH);
                mover(540,1);
                    
                //second arm 90 degree
                digitalWrite(DIR_PIN2 , LOW); 
                mover(1600,2);
                
                    
                //move both arms at same time.
                digitalWrite(DIR_PIN , LOW);
                digitalWrite(DIR_PIN2 , LOW);
                while(analogRead(encoder)<600)
                {
                    flag=2;
                }
                
                //move second arm ultil hall
                while (mean < 2400)
                {
                    mean = meanFilter.AddValue(analogRead(hall));
                    flag = 3;
                }
                
                s_dir = 0;
                flag=1;
                break;
            }
            

            
            
            if(digitalRead(DIAG_PIN2) == 1 and avoid2 == 0){
            digitalWrite(DIR_PIN2 , LOW);
            avoid2 = 1; 
            delay(100);
            s_dir = 0; 
            flag3 = 1;
            }
            
        }
        
        flag=1; 
        
        if (flag3 != 1){
            s_dir = 1;
        }
        slow_Calibration_encoder(s_dir);
        slow_Calibration_hall(s_dir);

        Serial.println("Done without stallGuard");
        avoid = 1;
        driver.rms_current(600); 
        driver2.rms_current(600); 
        return 0;
        digitalWrite(EN_PIN,HIGH);
        digitalWrite(EN_PIN2,HIGH);         
    }
    }
////////////////////////////////////////////////////////////////////////
////////////////////////////////END_LOOP////////////////////////////////
}

void giro_normal(){
  {
    cli();//stop interrupts
  timer1 = timerBegin(3, 8,true); // Se configura el timer, en este caso uso el timer 4 de los 4 disponibles en el ESP(0,1,2,3)
                                 // el prescaler es 8, y true es una bandera que indica si la interrupcion se realiza en borde o en nivel
  timerAttachInterrupt(timer1, &onTimer, true); //Se vincula el timer con la funcion AttachInterrup 
                                               //la cual se ejecuta cuando se genera la interrupcion
  timerAlarmWrite(timer1, 10000, true); //En esta funcion se define el valor del contador en el cual se genera la interrupción del timer
  timerAlarmEnable(timer1); //Función para habilitar el temporizador.           
    sei();//allow interrupts
  }
}


void slow_Calibration_encoder(int s_dir){
  
  //int dato[100];
  for(int i = 0 ; i < 100 ; i++){
    dato[i] = -1 ;
  } 

  //go out from encoder
  digitalWrite(DIR_PIN , HIGH);
  while (analogRead(encoder) > 600) { 
    mover(1,1);
    mover(1,2);
  }

  digitalWrite(DIR_PIN , LOW);
  int conteo = 0;
  int accum = 0;
  int flag_1 = 0;
  int foo = 0;
  int foo1 = 0;
  int back = 0;

  Serial.println("Before");
  delay(500);

  //sweep for encoder
  while(conteo < 5){
    mover(1,1);
    mover(1,2);
    foo = analogRead(encoder);

    //analog to digital
    if (foo > 600){
      foo1 = 1;
    }
    if (foo <= 600){
      foo1 = 0;
    }
    dato[accum] = foo1;

    //to avoid first zeros
    if(dato[accum] == 1 && flag_1 == 0){
      flag_1 = 1;
    }

    //safe zone of 5 zeros.
    if(dato[accum] == 0 && flag_1 ==1){
      conteo++;
    }
  accum++;
  
  }
  //Serial.println(accum);
  int maxValue = 0;
  int indexValue = 0;
  int indexValue2 = 0;

  //find first and last max values. 
  for(int i = 0 ; i < 100 ; i++) {
    if (dato[i] > maxValue){ 
    maxValue = dato[i];
    indexValue = i;
    indexValue2 = 0;
    }
  if(dato[i] == maxValue){
    indexValue2 = i;
    }
  }

////DEBUG encoder. 
 
  int as = ((indexValue2+2)-indexValue)/2;
  back = (as) + (5) ;
  digitalWrite(DIR_PIN , HIGH);
  mover(back,1);
  mover(back,2);

  delay(1000);
  digitalWrite(EN_PIN,HIGH);
  digitalWrite(EN_PIN2,HIGH);
 
 
}
///////////////////////////////Second arm////////////////////////////////////////
void slow_Calibration_hall(int s_dir2){  

  digitalWrite(EN_PIN2,LOW);
  for(int i = 0 ; i < 300 ; i++){
    dato2[i] = -1 ;
  } 
  //go out from encoder
  int k=0;
  int val_sens;
  int val_sens_filt;
  int band_ini = 0;
  int band_max = 0;
  int band_fin = 0;
  int pasos_ini;
  int pasos_fin;
  int limit;
  int count_ini = 0;
  int count_fin = 0;
  digitalWrite(DIR_PIN2,LOW);
  while(analogRead(hall) > 2400)
  {
    mover(1,2);
  }
  
  digitalWrite(DIR_PIN2, HIGH);
  while(k < 300) { 
    mover(1,2);
    for(int y = 0; y < 20; y++)
    {
      val_sens = meanFilter3.AddValue(analogRead(hall));
    }
    Serial.print(val_sens);
    Serial.print("\t");
    val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens_filt > maximo)
    {
      maximo = val_sens_filt;
    }
    dato2[k] = val_sens_filt;
    //Serial.print(k);
    //Serial.print("\t");
    Serial.println(dato2[k]);
    k++;

  }
  Serial.println("Maximo");
  Serial.println(maximo);
  limit = maximo * 0.9;
  Serial.println("limit");
  Serial.println(limit);
  k = 300;
  while(k > 0)
  {
    if(dato2[k] > limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+10;
        band_ini = 1;
      }
    }
    if(dato2[k] < limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k+10;
        band_fin = 1;
      }
    }
    k--;
  }
  Serial.println("pasos_ini");
  Serial.println(pasos_ini);
  Serial.println("pasos_fin");
  Serial.println(pasos_fin);
  digitalWrite(DIR_PIN2, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (300 - pasos_ini)+media;
  mover(pas,2);
 
}

void mover(int pasos, int motor_d){
  //Serial.println(pasos);

  if(motor_d == 1)
  {
      for (int i = 0; i < pasos; i++)
      {
          digitalWrite(STEP_PIN, HIGH);
          delayMicroseconds(Velocidad);
          digitalWrite(STEP_PIN, LOW);
          delayMicroseconds(Velocidad);
      }
  }

  if(motor_d == 2)
  {
      for (int j = 0; j < pasos; j++)
      {
          digitalWrite(STEP_PIN2, HIGH);
          delayMicroseconds(Velocidad);
          digitalWrite(STEP_PIN2, LOW);
          delayMicroseconds(Velocidad);
      }
  }

  //delay(2000);
 }

 void CalibMotor::verificacion_cal(void){
   digitalWrite(EN_PIN2,LOW);
   digitalWrite(EN_PIN,LOW); 
   flag = 1; 
   int busq = 0;
   int band_b = 0;
   //go out from encoder
   digitalWrite(DIR_PIN , HIGH);
   digitalWrite(DIR_PIN2 , HIGH);
  if(encoder < 600)
  {
    while(busq < 100)
    {
      mover(1,1);
      mover(1,2);
      busq++;
      if(analogRead(encoder) > 600)
      {
        busq = 100;
        band_b = 1;
      }
    }
    if(band_b == 0)
    {
      busq = 0;
      digitalWrite(DIR_PIN , LOW);
      digitalWrite(DIR_PIN2 , LOW);
      while(busq < 200)
      {
        mover(1,1);
        mover(1,2);
        busq++;
        if(analogRead(encoder) > 600)
        {
          busq = 200;
        }
      }
    }    
  }
  slow_Calibration_encoder(s_dir);
  digitalWrite(EN_PIN2,LOW);
  digitalWrite(EN_PIN,HIGH);
  //Brazo 2/////////////////////
  int limite_verif;
  int pos_ini_der = 0;
  int pos_ini_izq = 0;
  int dato_anterior;
  int dato_actual;
  int dato_sens_filt;
  int dir_ini;
  int band_dir = 0;
  int pasos_comp = 0;
  int pasos_media;
  limite_verif = maximo * (0.90);
  digitalWrite(DIR_PIN2,HIGH);
  for(int y = 0; y < 20; y++)
  {
    dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
  }
  dato_anterior = dato_sens_filt;
  for(int y = 0; y < 5; y++)
  {
    mover(1,2);
  }
  for(int y = 0; y < 20; y++)
  {
    dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
  }
  dato_actual = dato_sens_filt;
  dir_ini = dato_actual - dato_anterior;
  if(dir_ini > 0)
  {
    digitalWrite(DIR_PIN2,HIGH);
    band_dir = 0;
  }
  if(dir_ini < 0)
  { 
    digitalWrite(DIR_PIN2,LOW);
    band_dir = 1;
  }
  while(dato_sens_filt < limite_verif)
  {
    mover(1,2);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
    }
  }
  while(dato_sens_filt > limite_verif)
  {
    digitalWrite(DIR_PIN2,LOW);
    mover(1,2);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
    }
  }

  pasos_comp = 0;

  if(band_dir == 1)
  {
    digitalWrite(DIR_PIN2,LOW);
    mover(1,2);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
    }
    while(dato_sens_filt > limite_verif)
    {
      mover(1,2);
      for(int y = 0; y < 20; y++)
      {
        dato_sens_filt = meanFilter3.AddValue(analogRead(hall));
      }
    }
  }

  for(int i = 0 ; i < 200 ; i++){
    dato3[i] = -1 ;
  } 
  //go out from encoder
  int k=0;
  int val_sens;
  int val_sens_filt;
  int band_ini = 0;
  int band_max = 0;
  int band_fin = 0;
  int pasos_ini;
  int pasos_fin;
  int limit;
  int count_ini = 0;
  int count_fin = 0; 
  digitalWrite(DIR_PIN2, HIGH);
  while(k < 200) { 
    mover(1,2);
    for(int y = 0; y < 20; y++)
    {
      val_sens = meanFilter3.AddValue(analogRead(hall));
    }
    Serial.print(val_sens);
    Serial.print("\t");
    val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens_filt > maximo)
    {
      maximo = val_sens_filt;
    }
    dato3[k] = val_sens_filt;
    //Serial.print(k);
    //Serial.print("\t");
    Serial.println(dato3[k]);
    k++;

  }
  Serial.println("Maximo");
  Serial.println(maximo);
  limit = maximo * 0.98;
  Serial.println("limit");
  Serial.println(limit);
  k = 200;
  while(k > 0)
  {
    if(dato3[k] > limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+10;
        band_ini = 1;
      }
    }
    if(dato3[k] < limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k+10;
        band_fin = 1;
      }
    }
    k--;
  }
  Serial.println("pasos_ini");
  Serial.println(pasos_ini);
  Serial.println("pasos_fin");
  Serial.println(pasos_fin);
  digitalWrite(DIR_PIN2, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (200 - pasos_ini)+media;
  mover(pas,2);


 }

