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

int A = 0;
int B = 0;
byte L;
byte H;

int p = 0;
int motor_degrees = 0;
int Velocidad = 5000;

int dato[100];
int dato2[800];
int dato3[200];        //vector utilizado en la verificacion
int vect_prom[5];
int vect_simi[5];
int vect_max[5];
int vect_min[5];
int vect_prom2[5];
int vect_simi2[5];
int vect_max2[5];
int vect_min2[5];
int maximo = 0;
int maximo2 = 0;
int minimo = 5000;
int minimo2 = 5000;
int max_hall1;
int min_hall1;
int nivel_cero1;
int nivel_cero2;
int max_hall2;
int min_hall2;
int read_hall2;
int read_hall1;
int Polo_sens1;
int Polo_sens2;
int Band_ajuste_ini = 0;
TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driver2(&SERIAL_PORT2, R_SENSE, DRIVER_ADDRESS2);

void giro_normal();
void mover(int,int,int);
void slow_Calibration_hall2(int );
void slow_Calibration_hall1(int );
void slow_Calibration_hall2_negativo(int);
void slow_Calibration_hall1_negativo(int);
int Cero_Hall1(void);
int Cero_Hall2(void);
int Polo1(void);
int Polo2(void);
int Check_ini(void);

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

    //pinMode(PIN_ProducType,INPUT_PULLUP);
    
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

    EEPROM.begin(EEPROM_SIZE);
}

int CalibMotor::start(){ 
//////////////////////////////////////////////////////////////////////
////////////////////////////WITH STALLGUARD///////////////////////////
    int dato_f;
    int mean;
    int dif_ref;
    int cont_giro = 0;
    flag = 0;

  Band_ajuste_ini = Check_ini();
  if(Band_ajuste_ini == 1)
  {
    int ref_sensor1 = 0;
    int ref_sensor2 = 0;
    while(ref_sensor2 != 1)
    {
      ref_sensor2 = Cero_Hall2(); 
    }
    while(ref_sensor1 != 1)
    {
      ref_sensor1 = Cero_Hall1();
    }
    max_hall1 = max_hall1 + 20;
    A = max_hall1;
    H = highByte(A);
    L = lowByte(A);
    EEPROM.write(407, H);
    EEPROM.commit();
    EEPROM.write(408, L);
    EEPROM.commit();
    max_hall2 = max_hall2 + 20;
    A = max_hall2;
    H = highByte(A);
    L = lowByte(A);
    EEPROM.write(409, H);
    EEPROM.commit();
    EEPROM.write(410, L);
    EEPROM.commit();
    Serial.println("max_hall1: ");
    Serial.println(max_hall1);
    Serial.println("max_hall2: ");
    Serial.println(max_hall2);
    min_hall1 = min_hall1 - 20;
    A = min_hall1;
    H = highByte(A);
    L = lowByte(A);
    EEPROM.write(411, H);
    EEPROM.commit();
    EEPROM.write(412, L);
    EEPROM.commit();
    min_hall2 = min_hall2 - 20;
    A = min_hall2;
    H = highByte(A);
    L = lowByte(A);
    EEPROM.write(413, H);
    EEPROM.commit();
    EEPROM.write(414, L);
    EEPROM.commit();
    Serial.println("min_hall1: ");
    Serial.println(min_hall1);
    Serial.println("min_hall2: ");
    Serial.println(min_hall2);
  }
  if(Band_ajuste_ini == 0)
  {
    L=EEPROM.read(403);
    H=EEPROM.read(404);
    nivel_cero1 = (L<<8) | H;

    L=EEPROM.read(405);
    H=EEPROM.read(406);
    nivel_cero2 = (L<<8) | H;

    L=EEPROM.read(407);
    H=EEPROM.read(408);
    max_hall1 = (L<<8) | H;

    L=EEPROM.read(409);
    H=EEPROM.read(410);
    max_hall2 = (L<<8) | H;

    L=EEPROM.read(411);
    H=EEPROM.read(412);
    min_hall1 = (L<<8) | H;
    
    L=EEPROM.read(413);
    H=EEPROM.read(414);
    min_hall2 = (L<<8) | H;

  }
    flag = 0;
    digitalWrite(EN_PIN , LOW);
    digitalWrite(DIR_PIN , LOW);
    giro_normal();
    delay(100);
    while(true){
    digitalWrite(EN_PIN2 , HIGH);
    digitalWrite(DIR_PIN2,HIGH);
    
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
        mover(2400,2,5000);
    //move both arms until encoder detects
        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 , LOW);
        //read_hall1 = (analogRead(hall1))/4;
      
        for(int i = 0; i < 40; i++)
        {
            dato_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato_f/4;
        
        
        while(read_hall1 < max_hall1 && read_hall1 > min_hall1)
        {
            //read_hall1 = (analogRead(hall1))/4;
            for(int i = 0; i < 40; i++)
            {
                dato_f = meanFilter2.AddValue(analogRead(hall1));
            }
            read_hall1 = dato_f/4;
                    
            flag=2;
        }
        flag=1;

    //move second arm until hall detects
        digitalWrite(DIR_PIN2 ,LOW);
        for(int i = 0; i < 40; i++)
        {
            dato_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato_f/4;
        mean = read_hall2;
        while(mean > max_hall2 || mean < min_hall2)
        {
          for(int i = 0; i < 40; i++)
            {
              dato_f = meanFilter2.AddValue(analogRead(hall2));
            }
            read_hall2 = dato_f / 4;
            mean = read_hall2;
            flag = 3;
        }
        flag = 1;
        digitalWrite(DIR_PIN2 ,HIGH);
        for(int i = 0; i < 40; i++)
        {
            dato_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato_f/4;
        
        mean = read_hall2;
        while(mean < max_hall2 && mean > min_hall2){
            for(int i = 0; i < 40; i++)
            {
              dato_f = meanFilter2.AddValue(analogRead(hall2));
            }
            read_hall2 = dato_f / 4;
            mean = read_hall2;
            flag = 3;
            delay(10);
            cont_giro++;
           if(digitalRead(PIN_ProducType) == 1)
           { 
              
             //Serial.print(driver2.SG_RESULT(), DEC);
             //Serial.print("\t");
             //Serial.println((digitalRead(DIAG_PIN2))*80);
             
                                        
            if((digitalRead(DIAG_PIN2) == 1 and avoid2 == 1) || cont_giro == 1280){
                cont_giro = 0;
                flag = 1;
                //secuencia donde el brazo se encuentra entre ambos picos//regresa main 30 grados, el otro gira 90, y ambos se mueven juntos.
                //main arm 30 degree
                delay(250);
                digitalWrite(DIR_PIN, HIGH);
                mover(540,1,5000);
                    
                //second arm 90 degree
                digitalWrite(DIR_PIN2 , LOW); 
                mover(3200,2,2000);
                
                //move both arms at same time.
                digitalWrite(DIR_PIN , LOW);
                digitalWrite(DIR_PIN2 , LOW);
                //read_hall1 = (analogRead(hall1))/4;
                for(int i = 0; i < 40; i++)
                {
                    dato_f = meanFilter2.AddValue(analogRead(hall1));
                }
                read_hall1 = dato_f/4;
          
                while(read_hall1 < max_hall1 && read_hall1 > min_hall1)
                {
                    //read_hall1 = (analogRead(hall1))/4;
                    for(int i = 0; i < 40; i++)
                    {
                        dato_f = meanFilter2.AddValue(analogRead(hall1));
                    }
                    read_hall1 = dato_f/4;
                    
                    flag=2;
                }
                digitalWrite(DIR_PIN2 , HIGH);
                //move second arm ultil hall
                while (mean < max_hall2 && mean > min_hall2)
                {
                    //mean = (analogRead(hall2))/4;
                    for(int i = 0; i < 40; i++)
                    {
                        dato_f = meanFilter2.AddValue(analogRead(hall2));
                    }
                    read_hall2 = dato_f/4;
                    
                    mean = read_hall2;
                    flag = 3;
                }
                
                s_dir = 0;
                flag=1;
                break;
            }
              
            if(digitalRead(DIAG_PIN2) == 1 and avoid2 == 0){
            digitalWrite(DIR_PIN2 , HIGH);
            avoid2 = 1; 
            delay(100);
            s_dir = 0; 
            flag3 = 1;
            }
            
          } 
        }
        flag=1;
        s_dir = 0;
      if(Band_ajuste_ini == 1)
      {  
        Polo_sens1 = Polo1();
        EEPROM.write(401,Polo_sens1);
        EEPROM.commit();    
        Polo_sens2 = Polo2();
        EEPROM.write(402,Polo_sens2);
        EEPROM.commit(); 
        Serial.println("Polo iman 1:  ");
        Serial.println(Polo_sens1);
        Serial.println("Polo iman 2:  ");
        Serial.println(Polo_sens2);
        
      }
      if(Band_ajuste_ini == 0)
      { 
        Polo_sens1=EEPROM.read(401);
        Polo_sens2=EEPROM.read(402);
      }
      
        if(Polo_sens1 == 1)
        {
            slow_Calibration_hall1(s_dir);
        }
        if(Polo_sens1 == 0)
        {
            slow_Calibration_hall1_negativo(s_dir);
        }
        if(Polo_sens2 == 1)
        {
            slow_Calibration_hall2(s_dir);
        }
        if(Polo_sens2 == 0)
        {
            slow_Calibration_hall2_negativo(s_dir);
        }
        driver.rms_current(500); 
        driver2.rms_current(500);
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW); 
        avoid = 1;
        return 0;
    }
    }
    //////////////////////////////////////////////////////////////////////////
    ////////////////////////////WITHOUT STALLGUARD////////////////////////////

    
    for(int i = 0; i < 40; i++)
    {
      dato_f = meanFilter2.AddValue(analogRead(hall1));
    }
    read_hall1 = dato_f/4;
    //Serial.println(read_hall1);
    if(read_hall1 > max_hall1 || read_hall1 < min_hall1)     ///===================condicional para detener el primer brazo
    {
      if(avoid == 0)
      {
        ///if(read_hall1 > max_hall1 && avoid == 0){
        flag = 1;

        digitalWrite(DIR_PIN2 ,LOW);
        for(int i = 0; i < 40; i++)
        {
            dato_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato_f/4;
        mean = read_hall2;
        while(mean > max_hall2 || mean < min_hall2)
        {
          for(int i = 0; i < 40; i++)
            {
              dato_f = meanFilter2.AddValue(analogRead(hall2));
            }
            read_hall2 = dato_f / 4;
            mean = read_hall2;
            flag = 3;
        }
        flag = 1;

        digitalWrite(DIR_PIN , LOW);
        digitalWrite(DIR_PIN2 ,HIGH);
        for(int i = 0; i < 40; i++)
        {
          dato_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato_f / 4;
        mean = read_hall2;
        digitalWrite(EN_PIN2,LOW);
        digitalWrite(EN_PIN,LOW);
        driver.rms_current(1500);     
        delay(500);                     
        
        while (mean < max_hall2 && mean > min_hall2){                      ////condicional para detener el segundo brazo
            for(int i = 0; i < 40; i++)
            {
              dato_f = meanFilter2.AddValue(analogRead(hall2));
            }
            read_hall2 = dato_f / 4;
            mean = read_hall2;
            flag = 3;
            delay(10);
            cont_giro++;
           if(digitalRead(PIN_ProducType) == 1)
           { 
              
             //Serial.print(driver2.SG_RESULT(), DEC);
             //Serial.print("\t");
             //Serial.println((digitalRead(DIAG_PIN2))*80);
             
                                        
            if((digitalRead(DIAG_PIN2) == 1 and avoid2 == 1) || cont_giro == 1280){
                cont_giro = 0;
                flag = 1;
                //secuencia donde el brazo se encuentra entre ambos picos//regresa main 30 grados, el otro gira 90, y ambos se mueven juntos.
                //main arm 30 degree
                delay(250);
                digitalWrite(DIR_PIN, HIGH);
                mover(540,1,5000);
                    
                //second arm 90 degree
                digitalWrite(DIR_PIN2 , LOW); 
                mover(3200,2,2000);
                
                //move both arms at same time.
                digitalWrite(DIR_PIN , LOW);
                digitalWrite(DIR_PIN2 , LOW);
                //read_hall1 = (analogRead(hall1))/4;
                for(int i = 0; i < 40; i++)
                {
                    dato_f = meanFilter2.AddValue(analogRead(hall1));
                }
                read_hall1 = dato_f/4;
          
                while(read_hall1 < max_hall1 && read_hall1 > min_hall1)
                {
                    //read_hall1 = (analogRead(hall1))/4;
                    for(int i = 0; i < 40; i++)
                    {
                        dato_f = meanFilter2.AddValue(analogRead(hall1));
                    }
                    read_hall1 = dato_f/4;
                    
                    flag=2;
                }
                digitalWrite(DIR_PIN2 , HIGH);
                //move second arm ultil hall
                while (mean < max_hall2 && mean > min_hall2)
                {
                    //mean = (analogRead(hall2))/4;
                    for(int i = 0; i < 40; i++)
                    {
                        dato_f = meanFilter2.AddValue(analogRead(hall2));
                    }
                    read_hall2 = dato_f/4;
                    
                    mean = read_hall2;
                    flag = 3;
                }
                
                s_dir = 0;
                flag=1;
                break;
            }
              
            if(digitalRead(DIAG_PIN2) == 1 and avoid2 == 0){
            digitalWrite(DIR_PIN2 , HIGH);
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

      
      if(Band_ajuste_ini == 1)
      {  
        Polo_sens1 = Polo1();
        EEPROM.write(401,Polo_sens1);
        EEPROM.commit();     
        Polo_sens2 = Polo2();
        EEPROM.write(402,Polo_sens2);
        EEPROM.commit(); 
        Serial.println("Polo iman 1:  ");
        Serial.println(Polo_sens1);
        Serial.println("Polo iman 2:  ");
        Serial.println(Polo_sens2);
      }
      if(Band_ajuste_ini == 0)
      { 
        Polo_sens1=EEPROM.read(401);
        Polo_sens2=EEPROM.read(402);
      }
      
        if(Polo_sens1 == 1)
        {
            slow_Calibration_hall1(s_dir);
        }
        if(Polo_sens1 == 0)
        {
            slow_Calibration_hall1_negativo(s_dir);
        }
        if(Polo_sens2 == 1)
        {
            slow_Calibration_hall2(s_dir);
        }
        if(Polo_sens2 == 0)
        {
            slow_Calibration_hall2_negativo(s_dir);
        }
        driver.rms_current(500); 
        driver2.rms_current(500);
        avoid = 1;
        return 0;        
      }
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
  digitalWrite(EN_PIN2,LOW);

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
  int dif_ref;
  digitalWrite(DIR_PIN,LOW);
  
  for(int i = 0; i < 40; i++)
  {
    dato_f = meanFilter2.AddValue(analogRead(hall1));
  }
  read_hall1 = dato_f/4;
  
  //while(read_hall1 > max_hall1)
  for(int t = 0; t < 80; t++)
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
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(dato[k]);
    delay(1);
    k++;

  }
  limit = maximo * 0.9;
  k = 99;
  while(k >= 0)
  {
    if(dato[k] > limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+9;
        band_ini = 1;
      }
    }
    if(dato[k] < limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k-9;
        band_fin = 1;
      }
    }
    k--;
  }
  
  digitalWrite(DIR_PIN, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (99 - pasos_ini)+media;
  mover(pas,1,5000); 
 
}
///////////////////////////////Second arm////////////////////////////////////////
void slow_Calibration_hall2(int s_dir2){
  int dato2_f;
  digitalWrite(EN_PIN2,LOW);
  for(int i = 0 ; i < 800 ; i++){
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
  int dif_ref;
  digitalWrite(DIR_PIN2,HIGH);
  for(int i = 0; i < 40; i++)
  {
    dato2_f = meanFilter2.AddValue(analogRead(hall2));
  }
  read_hall2 = dato2_f/4;
  //while(read_hall2 > max_hall2)
  for(int t = 0; t < 800; t++)
  {
    for(int i = 0; i < 40; i++)
    {
      dato2_f = meanFilter2.AddValue(analogRead(hall2));
    }
    read_hall2 = dato2_f/4;
    mover(1,2,5000);
  }
  
  digitalWrite(DIR_PIN2, LOW);
  while(k < 800) { 
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
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(dato2[k]);
    delay(1);
    k++;

  }
  limit = maximo2 * 0.9;
  k = 799;
  while(k >= 0)
  {
    if(dato2[k] > limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+9;
        band_ini = 1;
      }
    }
    if(dato2[k] < limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k-9;
        band_fin = 1;
      }
    }
    k--;
  }
  
  digitalWrite(DIR_PIN2, HIGH);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (799 - pasos_ini)+media;
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

 void CalibMotor::verif_cal_Positivob1(void){
   digitalWrite(EN_PIN2,LOW);
   digitalWrite(EN_PIN,LOW); 
   int dato1_f;
   flag = 1; 
   int busq = 0;
   int band_b = 0;
   int band_der = 0;
   int band_izq = 0;
   //go out from hall1
   digitalWrite(DIR_PIN , HIGH);
   digitalWrite(DIR_PIN2 , HIGH);
   
   for(int i = 0; i < 40; i++)
  {
    dato1_f = meanFilter2.AddValue(analogRead(hall1));
  }
  read_hall1 = dato1_f/4;
  if(read_hall1 < max_hall1)
  {
    while(busq < 200)
    {
      mover(1,1,5000);
      mover(1,2,5000);
      busq++;
      
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;

      if(read_hall1 > max_hall1)
      {
        busq = 200;
        band_b = 1;
        band_izq = 1;
      }
    }
    if(band_b == 0)
    {
      busq = 0;
      digitalWrite(DIR_PIN , LOW);
      digitalWrite(DIR_PIN2 , LOW);
      while(busq < 400)
      {
        mover(1,1,5000);
        mover(1,2,5000);
        busq++;
        for(int i = 0; i < 40; i++)
        {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
        if(read_hall1 > max_hall1)
        {
          busq = 400;
          band_der = 1;
        }
      }
    }
    if(band_izq == 1)
    {
      mover(100,1,5000);
      digitalWrite(DIR_PIN,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;
      while(read_hall1 < max_hall1)
      {
        mover(1,1,5000);
        for(int i = 0; i < 40; i++)
        {
            dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
      }
    }
        
  }
  if(read_hall1 > max_hall1)
  {
    if(band_der == 0 && band_izq == 0)
    {
      digitalWrite(DIR_PIN,HIGH);
      mover(100,1,5000);
      digitalWrite(DIR_PIN,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;
      while(read_hall1 < max_hall1)
      {
        mover(1,1,5000);
        for(int i = 0; i < 40; i++)
        {
            dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
      }
    }
  }
  
  delay(5000);
  slow_Calibration_hall1(s_dir);
  delay(5000);  
}

   //Verificacion del segundo brazo////////////////////////
void CalibMotor::verif_cal_Positivob2(void){
   digitalWrite(EN_PIN2,LOW);
   digitalWrite(EN_PIN,LOW); 
   int dato2_f;
   flag = 1; 
   int busq2 = 0;
   int band2_b = 0;
   int band2_der = 0;
   int band2_izq = 0;
   int dato1_f;
   //go out from hall1
   digitalWrite(DIR_PIN , HIGH);
   digitalWrite(DIR_PIN2 , HIGH);
   
  for(int i = 0; i < 40; i++)
  {
    dato2_f = meanFilter2.AddValue(analogRead(hall2));
  }
  read_hall2 = dato1_f/4;
  if(read_hall2 < max_hall2)
  {
    while(busq2 < 500)
    {
      //mover(1,1,5000);
      mover(1,2,5000);
      busq2++;
      
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;

      if(read_hall2 > max_hall2)
      {
        busq2 = 500;
        band2_b = 1;
        band2_izq = 1;
      }
    }
    if(band2_b == 0)
    {
      busq2 = 0;
      //digitalWrite(DIR_PIN , LOW);
      digitalWrite(DIR_PIN2 , LOW);
      while(busq2 < 1000)
      {
        //mover(1,1,5000);
        mover(1,2,5000);
        busq2++;
        for(int i = 0; i < 40; i++)
        {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
        if(read_hall2 > max_hall2)
        {
          busq2 = 1000;
          band2_der = 1;
        }
      }
    }
    if(band2_izq == 1)
    {
      mover(500,2,5000);
      digitalWrite(DIR_PIN2,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;
      while(read_hall2 < max_hall2)
      {
        mover(1,2,5000);
        for(int i = 0; i < 40; i++)
        {
            dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
      }
    }
        
  }
  if(read_hall2 > max_hall2)
  {
    if(band2_der == 0 && band2_izq == 0)
    {
      digitalWrite(DIR_PIN2,HIGH);
      mover(500,1,5000);
      digitalWrite(DIR_PIN2,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;
      while(read_hall2 < max_hall2)
      {
        mover(1,2,5000);
        for(int i = 0; i < 40; i++)
        {
            dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
      }
    }
  }
  
  delay(5000);
  slow_Calibration_hall2(s_dir);
  delay(5000);                   

  
 }
////////////////////////

 void CalibMotor::verif_cal_Negativob1(void){
   digitalWrite(EN_PIN2,LOW);
   digitalWrite(EN_PIN,LOW); 
   int dato1_f;
   flag = 1; 
   int busq = 0;
   int band_b = 0;
   int band_der = 0;
   int band_izq = 0;
   //go out from hall1
   digitalWrite(DIR_PIN , HIGH);
   digitalWrite(DIR_PIN2 , HIGH);
   
   for(int i = 0; i < 40; i++)
  {
    dato1_f = meanFilter2.AddValue(analogRead(hall1));
  }
  read_hall1 = dato1_f/4;
  if(read_hall1 > min_hall1)
  {
    while(busq < 200)
    {
      mover(1,1,5000);
      mover(1,2,5000);
      busq++;
      
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;

      if(read_hall1 < min_hall1)
      {
        busq = 200;
        band_b = 1;
        band_izq = 1;
      }
    }
    if(band_b == 0)
    {
      busq = 0;
      digitalWrite(DIR_PIN , LOW);
      digitalWrite(DIR_PIN2 , LOW);
      while(busq < 400)
      {
        mover(1,1,5000);
        mover(1,2,5000);
        busq++;
        for(int i = 0; i < 40; i++)
        {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
        if(read_hall1 < min_hall1)
        {
          busq = 400;
          band_der = 1;
        }
      }
    }
    if(band_izq == 1)
    {
      mover(100,1,5000);
      digitalWrite(DIR_PIN,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;
      while(read_hall1 > min_hall1)
      {
        mover(1,1,5000);
        for(int i = 0; i < 40; i++)
        {
            dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
      }
    }
        
  }
  if(read_hall1 < min_hall1)
  {
    if(band_der == 0 && band_izq == 0)
    {
      digitalWrite(DIR_PIN,HIGH);
      mover(100,1,5000);
      digitalWrite(DIR_PIN,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato1_f = meanFilter2.AddValue(analogRead(hall1));
      }
      read_hall1 = dato1_f/4;
      while(read_hall1 > min_hall1)
      {
        mover(1,1,5000);
        for(int i = 0; i < 40; i++)
        {
            dato1_f = meanFilter2.AddValue(analogRead(hall1));
        }
        read_hall1 = dato1_f/4;
      }
    }
  }
  
  delay(5000);
  slow_Calibration_hall1_negativo(s_dir);
  delay(5000);  
 }
   //Verificacion del segundo brazo////////////////////////

void CalibMotor::verif_cal_Negativob2(void){
   digitalWrite(EN_PIN2,LOW);
   digitalWrite(EN_PIN,LOW); 
   int dato2_f;
   flag = 1; 
   int busq2 = 0;
   int band2_b = 0;
   int band2_der = 0;
   int band2_izq = 0;
   int dato1_f;
   //go out from hall1
   digitalWrite(DIR_PIN , HIGH);
   digitalWrite(DIR_PIN2 , HIGH);
   
  for(int i = 0; i < 40; i++)
  {
    dato2_f = meanFilter2.AddValue(analogRead(hall2));
  }
  read_hall2 = dato1_f/4;
  if(read_hall2 > min_hall2)
  {
    while(busq2 < 500)
    {
      //mover(1,1,5000);
      mover(1,2,5000);
      busq2++;
      
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;

      if(read_hall2 < min_hall2)
      {
        busq2 = 500;
        band2_b = 1;
        band2_izq = 1;
      }
    }
    if(band2_b == 0)
    {
      busq2 = 0;
      //digitalWrite(DIR_PIN , LOW);
      digitalWrite(DIR_PIN2 , LOW);
      while(busq2 < 1000)
      {
        //mover(1,1,5000);
        mover(1,2,5000);
        busq2++;
        for(int i = 0; i < 40; i++)
        {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
        if(read_hall2 < min_hall2)
        {
          busq2 = 1000;
          band2_der = 1;
        }
      }
    }
    if(band2_izq == 1)
    {
      mover(500,2,5000);
      digitalWrite(DIR_PIN2,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;
      while(read_hall2 > min_hall2)
      {
        mover(1,2,5000);
        for(int i = 0; i < 40; i++)
        {
            dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
      }
    }
        
  }
  if(read_hall2 < min_hall2)
  {
    if(band2_der == 0 && band2_izq == 0)
    {
      digitalWrite(DIR_PIN2,HIGH);
      mover(500,2,5000);
      digitalWrite(DIR_PIN2,LOW);
      for(int i = 0; i < 40; i++)
      {
          dato2_f = meanFilter2.AddValue(analogRead(hall2));
      }
      read_hall2 = dato2_f/4;
      while(read_hall2 > min_hall2)
      {
        mover(1,2,5000);
        for(int i = 0; i < 40; i++)
        {
            dato2_f = meanFilter2.AddValue(analogRead(hall2));
        }
        read_hall2 = dato2_f/4;
      }
    }
  }
  
  delay(5000);
  slow_Calibration_hall2_negativo(s_dir);
  delay(5000);                   

  
 }

///////////////////////

 int Cero_Hall1(void){
   flag = 1;
   digitalWrite(EN_PIN,LOW);
   digitalWrite(EN_PIN2,LOW);
   int dato_hall1;
   int dato_filt;
   int min = 5000;
   int max = 0;
   int suma = 0;
   int promedio;
   int suma_promedios = 0;
   int datos_similares = 0;
   int p = 0;
   int dif_ref;
   flag = 2;
   int flag_c = 0;
   digitalWrite(DIR_PIN,LOW);
   digitalWrite(DIR_PIN2,LOW);
   giro_normal();
   delay(500);
   while(p < 300)
   {
     if (digitalRead(DIAG_PIN) == 1){
        flag = 1;
        flag_c = 1; 
        //Back a little  first arm
        digitalWrite(DIR_PIN,HIGH);
        mover(300,1,3000);

        //Move second arm 90 degrees.    
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW);
        digitalWrite(DIR_PIN2,LOW);
        mover(1600,2,2000);
        p = 300;
     }
     p++;
    delay(10);
   }
   if(flag_c == 0)
   {
     flag = 1; 
     //Move second arm 90 degrees.    
     digitalWrite(EN_PIN,LOW);
     digitalWrite(EN_PIN2,LOW);
     digitalWrite(DIR_PIN,HIGH);
     mover(800,1,2000);

   }

   flag = 1;
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
       suma_promedios = suma_promedios + vect_prom[i];
       datos_similares++;
     } 
   }
   nivel_cero1 = suma_promedios / datos_similares;
   A = nivel_cero1;
   H = highByte(A);
   L = lowByte(A);
   EEPROM.write(403, H);
   EEPROM.commit();
   EEPROM.write(404, L);
   EEPROM.commit();
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
   Serial.println("Nivel cero promedio");
   Serial.println(nivel_cero1);
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
   digitalWrite(EN_PIN,HIGH);
   int dato_hall2;
   int dato_filt;
   int min = 5000;
   int max = 0;
   int suma = 0;
   int promedio;
   int suma_promedios2 = 0;
   int datos_similares2 = 0;
   int p = 0;
   int dif_ref;
   int flag_c = 0;
   flag = 3;
   giro_normal();
   delay(500);
   while(p < 300)
   {
     if (digitalRead(DIAG_PIN2) == 1){
        flag = 1; 
        flag_c = 1;
        //Move second arm 90 degrees.    
        digitalWrite(EN_PIN,LOW);
        digitalWrite(EN_PIN2,LOW);
        digitalWrite(DIR_PIN2,HIGH);
        mover(1600,2,2000);
        p = 300;
     }
     p++;
    delay(10);
   }
   if(flag_c == 0)
   {
     flag = 1; 
     //Move second arm 90 degrees.    
     digitalWrite(EN_PIN,LOW);
     digitalWrite(EN_PIN2,LOW);
     digitalWrite(DIR_PIN2,HIGH);
     mover(800,2,2000);

   }
   flag = 1;
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
       suma_promedios2 = suma_promedios2 + vect_prom2[i];
       datos_similares2++;
     }

   }
   nivel_cero2 = suma_promedios2 / datos_similares2;
   A = nivel_cero2;
   H = highByte(A);
   L = lowByte(A);
   EEPROM.write(405, H);
   EEPROM.commit();
   EEPROM.write(406, L);
   EEPROM.commit();
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
   Serial.println("Nivel cero promedio2");
   Serial.println(nivel_cero2);

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

 int Polo1(void)
 {
  int vect_polo1[100];
  int maximo_polo1 = 0;
  int minimo_polo1 = 5000;
  int dif_max_abs1;
  int dif_min_abs1;
  digitalWrite(EN_PIN,LOW);
  digitalWrite(EN_PIN2,LOW);

  for(int i = 0 ; i < 100 ; i++){
    vect_polo1[i] = -1 ;
  } 
  int k=0;
  int val_sens;
  digitalWrite(DIR_PIN,LOW);
  
  for(int t = 0; t < 100; t++)
  {
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
    if(val_sens > maximo_polo1)        //if(val_sens_filt > maximo)
    {
      maximo_polo1 = val_sens;     //maximo = val_sens_filt;
    }
    if(val_sens < minimo_polo1)        //if(val_sens_filt > maximo)
    {
      minimo_polo1 = val_sens;     //maximo = val_sens_filt;
    }
    vect_polo1[k] = val_sens;  //dato[k] = val_sens_filt;
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(vect_polo1[k]);
    delay(1);
    k++;
  }
  dif_max_abs1 = maximo_polo1 - nivel_cero1;
  dif_min_abs1 = nivel_cero1 - minimo_polo1;
  if(dif_max_abs1 > dif_min_abs1)
  {
      return 1;
  }
  if(dif_max_abs1 < dif_min_abs1)
  {
      return 0;
  }
}


 int Polo2(void)
 {
  int vect_polo2[800];
  int maximo_polo2 = 0;
  int minimo_polo2 = 5000;
  int dif_max_abs2;
  int dif_min_abs2;
  digitalWrite(EN_PIN,LOW);
  digitalWrite(EN_PIN2,LOW);

  for(int i = 0 ; i < 800 ; i++){
    vect_polo2[i] = -1 ;
  } 
  int k=0;
  int val_sens;
  digitalWrite(DIR_PIN2,HIGH);
  
  for(int t = 0; t < 800; t++)
  {
    mover(1,2,5000);
  }
  digitalWrite(DIR_PIN2,LOW);
  while(k < 800) { 
    mover(1,2,5000);
    for(int y = 0; y < 40; y++)
    {
      read_hall2 = (analogRead(hall2))/4;
      val_sens = meanFilter2.AddValue(read_hall2);
    }
    delay(1);
    if(val_sens > maximo_polo2)        //if(val_sens_filt > maximo)
    {
      maximo_polo2 = val_sens;     //maximo = val_sens_filt;
    }
    if(val_sens < minimo_polo2)        //if(val_sens_filt > maximo)
    {
      minimo_polo2 = val_sens;     //maximo = val_sens_filt;
    }
    vect_polo2[k] = val_sens;  //dato[k] = val_sens_filt;
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(vect_polo2[k]);
    delay(1);
    k++;
  }
  dif_max_abs2 = maximo_polo2 - nivel_cero2;
  dif_min_abs2 = nivel_cero2 - minimo_polo2;
  if(dif_max_abs2 > dif_min_abs2)
  {
      return 1;
  }
  if(dif_max_abs2 < dif_min_abs2)
  {
      return 0;
  }
}

void slow_Calibration_hall2_negativo(int s_dir2){
  int dato2_f;
  digitalWrite(EN_PIN2,LOW);
  for(int i = 0 ; i < 800 ; i++){
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
  int dif_ref;
  digitalWrite(DIR_PIN2,HIGH);
  for(int i = 0; i < 40; i++)
  {
    dato2_f = meanFilter2.AddValue(analogRead(hall2));
  }
  read_hall2 = dato2_f/4;
  //while(read_hall2 > max_hall2)
  for(int t = 0; t < 800; t++)
  {
    for(int i = 0; i < 40; i++)
    {
      dato2_f = meanFilter2.AddValue(analogRead(hall2));
    }
    read_hall2 = dato2_f/4;
    mover(1,2,5000);
  }
  
  digitalWrite(DIR_PIN2, LOW);
  while(k < 800) { 
    mover(1,2,5000);
    for(int y = 0; y < 40; y++)
    {
      read_hall2 = (analogRead(hall2))/4;
      val_sens = meanFilter2.AddValue(read_hall2);
    }
    delay(1);
    //val_sens_filt = meanFilter4.AddValue(val_sens);
    
    if(val_sens < minimo2)
    {
      minimo2 = val_sens;
    }
    dato2[k] = val_sens;
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(dato2[k]);
    delay(1);
    k++;

  }
  limit = minimo2 + ((nivel_cero2 - minimo2)*0.1);
  k = 799;
  while(k >= 0)
  {
    if(dato2[k] < limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+9;
        band_ini = 1;
      }
    }
    if(dato2[k] > limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k-9;
        band_fin = 1;
      }
    }
    k--;
  }
  
  digitalWrite(DIR_PIN2, HIGH);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (799 - pasos_ini)+media;
  mover(pas,2,5000); 
}

void slow_Calibration_hall1_negativo(int s_dir2){
  int dato1_f;
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
  int dif_ref;
  digitalWrite(DIR_PIN,LOW);
  for(int i = 0; i < 40; i++)
  {
    dato1_f = meanFilter2.AddValue(analogRead(hall1));
  }
  read_hall1 = dato1_f/4;
  //while(read_hall2 > max_hall2)
  for(int t = 0; t < 80; t++)
  {
    for(int i = 0; i < 40; i++)
    {
      dato1_f = meanFilter2.AddValue(analogRead(hall1));
    }
    read_hall1 = dato1_f/4;
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
    
    if(val_sens < minimo)
    {
      minimo = val_sens;
    }
    dato[k] = val_sens;
    //Serial.print(k);
    //Serial.print("\t");
    //Serial.println(dato[k]);
    delay(1);
    k++;

  }
  limit = minimo + ((nivel_cero1 - minimo)*0.1);
  k = 99;
  while(k >= 0)
  {
    if(dato[k] < limit && band_ini == 0)
    {
      count_ini++;
      if(count_ini == 9)
      {
        pasos_ini = k+9;
        band_ini = 1;
      }
    }
    if(dato[k] > limit && band_ini == 1 && band_fin == 0)
    {
      count_fin++;
      if(count_fin == 9)
      {
        pasos_fin = k-9;
        band_fin = 1;
      }
    }
    k--;
  }
  
  digitalWrite(DIR_PIN, LOW);
  int pas;
  int media;
  media = (pasos_ini - pasos_fin)/2;
  pas = (99 - pasos_ini)+media;
  mover(pas,1,5000); 
}

int Check_ini(void)
{
    int dato_eeprom;
    int cont_eeprom = 0;
    for(int i = 0; i < 512; i++)
    {
      dato_eeprom = EEPROM.read(i);
      if(dato_eeprom == 255)
      {
        cont_eeprom++;        
      }
    }
    if(cont_eeprom == 512)
    {
      return 1;
    }
    else
    {
      return 0;
    }
}
