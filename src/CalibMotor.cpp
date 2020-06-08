#include "CalibMotor.h"
#include <HardwareSerial.h>

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

//TMC2208Stepper driver = TMC2208Stepper(&Serial1);
//TMC2208Stepper driver2 = TMC2208Stepper(&Serial2);  
TMC2208Stepper driver(&SERIAL_PORT, R_SENSE);
TMC2208Stepper driver2(&SERIAL_PORT2, R_SENSE);

void giro_normal();
void mover(int , int );
void slow_Calibration_hall(int );
void slow_Calibration_encoder(int s_dir);

CalibMotor::CalibMotor(){

}

void IRAM_ATTR onTimer() {
  
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

    //SERIAL_PORT.begin(115200);
    //SERIAL_PORT2.begin(115200);

    
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
    
    Serial1.begin(115200);
    Serial2.begin(115200);


    driver.begin();
    driver.pdn_disable(1);							// Use PDN/UART pin for communication
	  driver.I_scale_analog(0);						// Adjust current from the registers
	  driver.rms_current(600);
    driver.mstep_reg_select(1);
    driver.microsteps(MICROSTEPPING);						// Set driver current 500mA
	  driver.toff(5); 
    driver.pwm_autoscale(1);
    driver.shaft(false);

    driver2.begin();
    driver2.pdn_disable(1);							// Use PDN/UART pin for communication
	  driver2.I_scale_analog(0);						// Adjust current from the registers
	  driver2.rms_current(600);	
    driver2.mstep_reg_select(1);
  	driver2.microsteps(MICROSTEPPING);			// Set driver current 500mA
	  driver2.toff(5); 
    driver2.pwm_autoscale(1);
    
    driver2.shaft(false);
}

int CalibMotor::start(){
//////////////////////////////////////////////////////////////////////
////////////////////////////WITH STALLGUARD///////////////////////////
    giro_normal();
    delay(100);
    while(true){
      Serial.println("aqui");
    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////WITHOUT STALLGUARD////////////////////////////

    if(analogRead(encoder)>600 && avoid == 0){
        flag = 1;
        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 ,HIGH);
        int mean = meanFilter.AddValue(analogRead(hall));
        digitalWrite(EN_PIN2,LOW);
        driver.rms_current(1300);     
        delay(500);                      
        
        while (mean < 2400){
            mean = meanFilter.AddValue(analogRead(hall));
            flag = 3;
            delay(10);
                                      
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
        driver.rms_current(600); 
        driver2.rms_current(600);
        avoid = 1;
        return 0;        
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
 
}
///////////////////////////////Second arm////////////////////////////////////////
void slow_Calibration_hall(int s_dir2){
  digitalWrite(EN_PIN2,LOW);
  for(int i = 0 ; i < 300 ; i++){
    dato2[i] = -1 ;
  } 
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
    delay(1);
    val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens_filt > maximo)
    {
      maximo = val_sens_filt;
    }
    dato2[k] = val_sens_filt;
    delay(1);
    k++;

  }
  limit = maximo * 0.9;
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
  
  digitalWrite(DIR_PIN2, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (300 - pasos_ini)+media;
  mover(pas,2); 

  Serial.println(driver.microsteps());
  Serial.println(driver2.microsteps());
  delay(5000);
}

void mover(int pasos, int motor_d){
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
  Serial.println(driver.microsteps());
  Serial.println(driver2.microsteps());
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
    delay(25);
    val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens_filt > maximo)
    {
      maximo = val_sens_filt;
    }
    dato3[k] = val_sens_filt;
    delay(25);
    k++;

  }
  limit = maximo * 0.98;
  delay(50);
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
  delay(50);
  digitalWrite(DIR_PIN2, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (200 - pasos_ini)+media;
  for(int t =0; t < pas;t++)
  {
    mover(1,2);
    delay(50);
  }
  
  digitalWrite(EN_PIN2,LOW);
  digitalWrite(EN_PIN,LOW); 
  driver.rms_current(600); 
  driver2.rms_current(600);

 }
