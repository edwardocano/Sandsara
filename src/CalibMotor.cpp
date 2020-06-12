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
int dato2[100];
int dato3[200];
int vect_prom[5];
int vect_simi[5];
int vect_max[5];
int vect_min[5];
int vect_prom2[5];
int vect_simi2[5];
int vect_max2[5];
int vect_min2[5];
int maximo;
int maximo2;
int max_hall1;
int min_hall1;
int max_hall2;
int min_hall2;
int read_hall2;
int read_hall1;

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driver2(&SERIAL_PORT2, R_SENSE, DRIVER_ADDRESS2);

void giro_normal();
void mover(int,int,int);
void slow_Calibration_hall2(int );
void slow_Calibration_hall1(int s_dir);
int Cero_Hall1(void);
int Cero_Hall2(void);

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

    SERIAL_PORT.begin(115200);
    SERIAL_PORT2.begin(115200);

    pinMode(PIN_ProducType,INPUT_PULLUP);
    
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

    digitalWrite(EN_PIN2,LOW);
    digitalWrite(DIR_PIN2,LOW);
    
    driver.begin();
    driver.pdn_disable(true);  
    driver.toff(4);
    driver.blank_time(24);
    driver.rms_current(500); 
    driver.microsteps(MICROSTEPPING);
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
    driver2.microsteps(MICROSTEPPING);
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
    int ref_sensor1 = 0;
    int ref_sensor2 = 0;
    int dato_f;
    int mean;
    while(ref_sensor1 != 1)
    {
      ref_sensor1 = Cero_Hall1(); 
    }

    while(ref_sensor2 != 1)
    {
      ref_sensor2 = Cero_Hall2(); 
    }

    flag = 0;
    max_hall1 = max_hall1 + 20;
    max_hall2 = max_hall2 + 20;
    Serial.println("max_hall1: ");
    Serial.println(max_hall1);
    Serial.println("max_hall2: ");
    Serial.println(max_hall2);
    digitalWrite(EN_PIN , LOW);
    digitalWrite(DIR_PIN , LOW);
    giro_normal();
    delay(100);
    while(true){
    digitalWrite(EN_PIN2 , HIGH); 
    if(digitalRead(PIN_ProducType) == 1)
    { 
    if (digitalRead(DIAG_PIN) == 1){
        flag = 1; 
    //Back a little  first arm
        digitalWrite(DIR_PIN,HIGH);
        mover(300,1,5000);

    //Move second arm 90 degrees.    
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW);
        digitalWrite(DIR_PIN2,LOW);
        mover(1600,2,5000);
        delay(5000);
    //move both arms until encoder detects
        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 , LOW);
        read_hall1 = (analogRead(hall1))/4;
        while(read_hall1 < max_hall1)
        {
          read_hall1 = (analogRead(hall1))/4;
          flag=2;
        }
        flag=1;

    //move second arm until hall detects
        
        read_hall2 = (analogRead(hall2))/4;
        mean = meanFilter.AddValue(read_hall2);
        while (mean < max_hall2){
            read_hall2 = (analogRead(hall2))/4;                      
            mean = meanFilter.AddValue(read_hall2);
            flag = 3;
        }
        flag=1;
        s_dir = 0;
        slow_Calibration_hall1(s_dir);
        slow_Calibration_hall2(s_dir);
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW); 
        avoid = 1;
    }
    }
    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////WITHOUT STALLGUARD////////////////////////////

    
    digitalWrite(EN_PIN , LOW);
    for(int i = 0; i < 40; i++)
    {
      dato_f = meanFilter2.AddValue(analogRead(hall1));
    }
    read_hall1 = dato_f/4;
    if(read_hall1 > max_hall1 && avoid == 0){
        flag = 1;
    
        //digitalWrite(DIR_PIN , HIGH);
        digitalWrite(DIR_PIN2 ,LOW);
        for(int i = 0; i < 40; i++)
        {
          dato_f = meanFilter2.AddValue(analogRead(hall2));
        }
        mean = dato_f /4;
        digitalWrite(EN_PIN2,LOW);
        driver.rms_current(1500);     
        delay(500);                     
        
        while (mean < max_hall2){
            for(int i = 0; i < 40; i++)
            {
              dato_f = meanFilter2.AddValue(analogRead(hall2));
            }
            mean = dato_f /4;
            flag = 3;
            delay(10);
           if(digitalRead(PIN_ProducType) == 1)
           {                            
            if(digitalRead(DIAG_PIN2) == 1 and avoid2 == 1){
                flag = 1;
                //secuencia donde el brazo se encuentra entre ambos picos//regresa main 30 grados, el otro gira 90, y ambos se mueven juntos.
                //main arm 30 degree
                delay(250);
                digitalWrite(DIR_PIN, HIGH);
                mover(540,1,5000);
                    
                //second arm 90 degree
                digitalWrite(DIR_PIN2 , LOW); 
                mover(1600,2,5000);
                
                //move both arms at same time.
                digitalWrite(DIR_PIN , LOW);
                digitalWrite(DIR_PIN2 , LOW);
                read_hall1 = (analogRead(hall1))/4;
                while(read_hall1 < max_hall1)
                {
                    read_hall1 = (analogRead(hall1))/4;
                    flag=2;
                }
                
                //move second arm ultil hall
                while (mean < max_hall2)
                {
                    mean = (analogRead(hall2))/4;
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
        }
        
        flag=1; 
        
        if (flag3 != 1){
            s_dir = 1;
        }
        slow_Calibration_hall1(s_dir);
        slow_Calibration_hall2(s_dir);
        driver.rms_current(500); 
        driver2.rms_current(500);
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


void slow_Calibration_hall1(int s_dir){
  int dato_f;
  digitalWrite(EN_PIN,LOW);
  for(int i = 0 ; i < 100 ; i++){
    dato[i] = -1 ;
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
  digitalWrite(DIR_PIN,LOW);
  
  for(int i = 0; i < 40; i++)
  {
    dato_f = meanFilter2.AddValue(analogRead(hall1));
  }
  read_hall1 = dato_f/4;
  while(read_hall1 > max_hall1)
  {
     for(int i = 0; i < 40; i++)
    {
      dato_f = meanFilter2.AddValue(analogRead(hall1));
    }
    read_hall1 = dato_f/4;
    mover(1,1,5000);
  }
  
  digitalWrite(DIR_PIN, HIGH);
  while(k < 100) { 
    mover(1,1,5000);
    for(int y = 0; y < 40; y++)
    {
      read_hall1 = (analogRead(hall1))/4;
      val_sens = meanFilter2.AddValue(read_hall1);
    }
    delay(1);
    //val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens > maximo)        //if(val_sens_filt > maximo)
    {
      maximo = val_sens;     //maximo = val_sens_filt;
    }
    dato[k] = val_sens;  //dato[k] = val_sens_filt;
    delay(1);
    k++;

  }
  limit = maximo * 0.9;
  k = 100;
  while(k > 0)
  {
    if(dato[k] > limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+10;
        band_ini = 1;
      }
    }
    if(dato[k] < limit && band_ini == 1 && band_fin == 0)
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
  
  digitalWrite(DIR_PIN, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (100 - pasos_ini)+media;
  mover(pas,1,5000); 
 
}
///////////////////////////////Second arm////////////////////////////////////////
void slow_Calibration_hall2(int s_dir2){
  int dato2_f;
  digitalWrite(EN_PIN2,LOW);
  for(int i = 0 ; i < 100 ; i++){
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
  for(int i = 0; i < 40; i++)
  {
    dato2_f = meanFilter2.AddValue(analogRead(hall2));
  }
  read_hall2 = dato2_f/4;
  while(read_hall2 > max_hall2)
  {
    for(int i = 0; i < 40; i++)
    {
      dato2_f = meanFilter2.AddValue(analogRead(hall2));
    }
    read_hall2 = dato2_f/4;
    mover(1,2,5000);
  }
  
  digitalWrite(DIR_PIN2, HIGH);
  while(k < 100) { 
    mover(1,2,5000);
    for(int y = 0; y < 40; y++)
    {
      read_hall2 = (analogRead(hall2))/4;
      val_sens = meanFilter2.AddValue(read_hall2);
    }
    delay(1);
    //val_sens_filt = meanFilter4.AddValue(val_sens);
    if(val_sens > maximo2)
    {
      maximo2 = val_sens;
    }
    dato2[k] = val_sens;
    delay(1);
    k++;

  }
  limit = maximo2 * 0.9;
  k = 100;
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
  pas = (100 - pasos_ini)+media;
  mover(pas,2,5000); 
}

void mover(int pasos, int motor_d, int Velocidad){
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

 }

 void CalibMotor::verificacion_cal(void){
   int encoder;
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
      mover(1,1,5000);
      mover(1,2,5000);
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
      while(busq < 100)
      {
        mover(1,1,5000);
        mover(1,2,5000);
        busq++;
        if(analogRead(encoder) > 600)
        {
          busq = 100;
        }
      }
    }    
  }
  slow_Calibration_hall1(s_dir);
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
    dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
  }
  dato_anterior = dato_sens_filt;
  for(int y = 0; y < 5; y++)
  {
    mover(1,2,5000);
  }
  for(int y = 0; y < 20; y++)
  {
    dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
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
    mover(1,2,5000);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
    }
  }
  while(dato_sens_filt > limite_verif)
  {
    digitalWrite(DIR_PIN2,LOW);
    mover(1,2,5000);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
    }
  }

  pasos_comp = 0;

  if(band_dir == 1)
  {
    digitalWrite(DIR_PIN2,LOW);
    mover(1,2,5000);
    for(int y = 0; y < 20; y++)
    {
      dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
    }
    while(dato_sens_filt > limite_verif)
    {
      mover(1,2,5000);
      for(int y = 0; y < 20; y++)
      {
        dato_sens_filt = meanFilter3.AddValue(analogRead(hall2));
      }
    }
  }

  for(int i = 0 ; i < 100 ; i++){
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
  while(k < 100) { 
    mover(1,2,5000);
    for(int y = 0; y < 20; y++)
    {
      val_sens = meanFilter3.AddValue(analogRead(hall2));
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
  k = 100;
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
  pas = (100 - pasos_ini)+media;
  for(int t =0; t < pas;t++)
  {
    mover(1,2,5000);
    delay(50);
  }
  
  digitalWrite(EN_PIN2,LOW);
  digitalWrite(EN_PIN,LOW); 
  driver.rms_current(500); 
  driver2.rms_current(500);
 }


 int Cero_Hall1(void){
   flag = 1;
   digitalWrite(EN_PIN,LOW);
   int dato_hall1;
   int dato_filt;
   int min = 5000;
   int max = 0;
   int suma = 0;
   int promedio;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall1));
      }
      dato_hall1 = dato_filt/4;
      if(dato_hall1 > max)
      {
        max = dato_hall1;
      }
      if(dato_hall1 < min)
      {
        min = dato_hall1;
      }
      suma = suma + dato_hall1;
      vect_max[0] = max;
      vect_min[0] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom[0] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN,HIGH);
   mover(178,1,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall1));
      }
      dato_hall1 = dato_filt/4;
      if(dato_hall1 > max)
      {
        max = dato_hall1;
      }
      if(dato_hall1 < min)
      {
        min = dato_hall1;
      }
      suma = suma + dato_hall1;
      vect_max[1] = max;
      vect_min[1] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom[1] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN,HIGH);
   mover(178,1,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall1));
      }
      dato_hall1 = dato_filt/4;
      if(dato_hall1 > max)
      {
        max = dato_hall1;
      }
      if(dato_hall1 < min)
      {
        min = dato_hall1;
      }
      suma = suma + dato_hall1;
      vect_max[2] = max;
      vect_min[2] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom[2] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN,LOW);
   mover(533,1,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall1));
      }
      dato_hall1 = dato_filt/4;
      if(dato_hall1 > max)
      {
        max = dato_hall1;
      }
      if(dato_hall1 < min)
      {
        min = dato_hall1;
      }
      suma = suma + dato_hall1;
      vect_max[3] = max;
      vect_min[3] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom[3] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN,LOW);
   mover(178,1,3000);


   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall1));
      }
      dato_hall1 = dato_filt/4;
      if(dato_hall1 > max)
      {
        max = dato_hall1;
      }
      if(dato_hall1 < min)
      {
        min = dato_hall1;
      }
      suma = suma + dato_hall1;
      vect_max[4] = max;
      vect_min[4] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom[4] = promedio;
   suma = 0;

  
   int  similitud;
   int cont_simi = 0;
   for(int i = 0; i < 5; i++)
   {
     for(int j = 0; j < 5; j++)
     {
       similitud = vect_prom[i] - vect_prom[j];
       if(similitud < 0)
       {
         similitud = similitud * (-1);
       }
       if(similitud <= 10)
       {
         cont_simi++;
       }
     }
     vect_simi[i] = cont_simi;
     cont_simi = 0;
   }
   int accum5 = 0;
   int accum4 = 0;
   int accum3 = 0;
   for(int i = 0; i < 5; i++)
   {
     if(vect_simi[i] == 5)
     {
       accum5++;
     }
     if(vect_simi[i] == 4)
     {
       accum4++;
     }
     if(vect_simi[i] == 3)
     {
       accum3++;
     }

   }
   Serial.println("Acumm5:  ");
   Serial.println(accum5);
   Serial.println("Acumm4:  ");
   Serial.println(accum4);
   Serial.println("Acumm3:  ");
   Serial.println(accum3);
  
   min = 5000;
   max = 0;
   for(int i = 0; i < 5; i++)
   {
     if(vect_simi[i] >= 3)
     {
       if(vect_max[i] > max)
       {
         max = vect_max[i];
       }
       if(vect_min[i] < min)
       {
         min = vect_min[i];
       }
     }

   }
   Serial.println("max:  ");
   Serial.println(max);
   Serial.println("min:  ");
   Serial.println(min);
   max_hall1 = max;
   min_hall1 = min;
   Serial.println("vector promedio");
   for(int i = 0; i < 5; i++)
   {
     Serial.println(vect_prom[i]);

   }

   Serial.println("vector similitud");
   for(int i = 0; i < 5; i++)
   {
     Serial.println(vect_simi[i]);

   }

   if(accum5 != 0)
   {
     return 1;
   }
   if(accum4 != 0)
   {
     return 1;
   }
   if(accum3 != 0)
   {
     return 1;
   }

 }


  int Cero_Hall2(void){
   flag = 1;
   digitalWrite(EN_PIN2,LOW);
   int dato_hall2;
   int dato_filt;
   int min = 5000;
   int max = 0;
   int suma = 0;
   int promedio;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall2));
      }
      dato_hall2 = dato_filt/4;
      if(dato_hall2 > max)
      {
        max = dato_hall2;
      }
      if(dato_hall2 < min)
      {
        min = dato_hall2;
      }
      suma = suma + dato_hall2;
      vect_max2[0] = max;
      vect_min2[0] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom2[0] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN2,HIGH);
   mover(178,2,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall2));
      }
      dato_hall2 = dato_filt/4;
      if(dato_hall2 > max)
      {
        max = dato_hall2;
      }
      if(dato_hall2 < min)
      {
        min = dato_hall2;
      }
      suma = suma + dato_hall2;
      vect_max2[1] = max;
      vect_min2[1] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom2[1] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN2,HIGH);
   mover(178,2,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall2));
      }
      dato_hall2 = dato_filt/4;
      if(dato_hall2 > max)
      {
        max = dato_hall2;
      }
      if(dato_hall2 < min)
      {
        min = dato_hall2;
      }
      suma = suma + dato_hall2;
      vect_max2[2] = max;
      vect_min2[2] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom2[2] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN2,LOW);
   mover(533,2,3000);

   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall2));
      }
      dato_hall2 = dato_filt/4;
      if(dato_hall2 > max)
      {
        max = dato_hall2;
      }
      if(dato_hall2 < min)
      {
        min = dato_hall2;
      }
      suma = suma + dato_hall2;
      vect_max2[3] = max;
      vect_min2[3] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom2[3] = promedio;
   suma = 0;

   digitalWrite(DIR_PIN2,LOW);
   mover(178,2,3000);


   min = 5000;
   max = 0;
   for(int x = 0; x < 5; x++)
   {
      for(int i = 0; i < 40; i++)
      {
        dato_filt = meanFilter2.AddValue(analogRead(hall2));
      }
      dato_hall2 = dato_filt/4;
      if(dato_hall2 > max)
      {
        max = dato_hall2;
      }
      if(dato_hall2 < min)
      {
        min = dato_hall2;
      }
      suma = suma + dato_hall2;
      vect_max2[4] = max;
      vect_min2[4] = min;
      delay(100);
   }
   promedio = suma / 5;
   vect_prom2[4] = promedio;
   suma = 0;

  
   int  similitud;
   int cont_simi = 0;
   for(int i = 0; i < 5; i++)
   {
     for(int j = 0; j < 5; j++)
     {
       similitud = vect_prom2[i] - vect_prom2[j];
       if(similitud < 0)
       {
         similitud = similitud * (-1);
       }
       if(similitud <= 10)
       {
         cont_simi++;
       }
     }
     vect_simi2[i] = cont_simi;
     cont_simi = 0;
   }
   int accum5 = 0;
   int accum4 = 0;
   int accum3 = 0;
   for(int i = 0; i < 5; i++)
   {
     if(vect_simi2[i] == 5)
     {
       accum5++;
     }
     if(vect_simi2[i] == 4)
     {
       accum4++;
     }
     if(vect_simi2[i] == 3)
     {
       accum3++;
     }

   }
   Serial.println("Acumm5:  ");
   Serial.println(accum5);
   Serial.println("Acumm4:  ");
   Serial.println(accum4);
   Serial.println("Acumm3:  ");
   Serial.println(accum3);
  
   min = 5000;
   max = 0;
   for(int i = 0; i < 5; i++)
   {
     if(vect_simi2[i] >= 3)
     {
       if(vect_max2[i] > max)
       {
         max = vect_max2[i];
       }
       if(vect_min2[i] < min)
       {
         min = vect_min2[i];
       }
     }

   }
   Serial.println("max:  ");
   Serial.println(max);
   Serial.println("min:  ");
   Serial.println(min);
   max_hall2 = max;
   min_hall2 = min;
   Serial.println("vector promedio2");
   for(int i = 0; i < 5; i++)
   {
     Serial.println(vect_prom2[i]);

   }

   Serial.println("vector similitud2");
   for(int i = 0; i < 5; i++)
   {
     Serial.println(vect_simi2[i]);

   }

   if(accum5 != 0)
   {
     return 1;
   }
   if(accum4 != 0)
   {
     return 1;
   }
   if(accum3 != 0)
   {
     return 1;
   }

 }













