 // To upload you need to double tap reset button after starting to upload
 
 #include <EEPROM.h>  
 #include <TimeAlarms.h>
 #include <DS3232RTC.h>  
 #include <avr/wdt.h>
 #define minutos  60000;
 
 // pins
 //IGNORE rj45 needs update !! RJ45 GND = white/blue + white/green  ; 2 = RELAY ; 3 = white/brown ; 4 = brown/solid ; 5 = green/solid ; 6= blue/solid  ; 7 = orange/solid ; 8 = white/orange;
 int ledasper_pin = 16; 
 int ledauto_pin = 10;
 int stopsw_pin = 14;
 int startsw_pin = 15; 
 int aspersw_pin = 18;
 int autosw_pin = 19;
 int asper_relay_pin = 9; 
 int motor_relay_pin = 8;
 
 
 //running variables
 unsigned long timerun_millis = 0;
 unsigned long current_millis = 0;
 unsigned long timer1 = 0; 
 unsigned long timeauto = 0; 
 int caso=0; 
 
 boolean run_bool = false;
 boolean auto1_bool = false; 
 boolean autopress_bool = true;
 boolean armed_bool = false;
 
 DS3232RTC myRTC;
 
void setup() {
  //pins and settings declared 
     myRTC.begin();    
     setSyncProvider(myRTC.get);
          
     pinMode(ledasper_pin, OUTPUT);
     pinMode(ledauto_pin, OUTPUT);     
     pinMode(asper_relay_pin, OUTPUT);
     pinMode(motor_relay_pin, OUTPUT);
     
     pinMode(startsw_pin, INPUT_PULLUP);
     pinMode(stopsw_pin, INPUT_PULLUP);
     pinMode(aspersw_pin, INPUT_PULLUP);
     pinMode(autosw_pin, INPUT_PULLUP);        
         
     digitalWrite(asper_relay_pin, LOW);  
     digitalWrite(motor_relay_pin, LOW);
     digitalWrite(ledasper_pin, LOW);
     digitalWrite(ledauto_pin, LOW);
     
     //eeprom
     if(EEPROM.read(3) == 1) auto1_bool = true;          
     
     //led
     if(auto1_bool == true) digitalWrite(ledauto_pin, HIGH); 
     
     //alarms and dst
    
      Alarm.alarmRepeat(dowFriday,9,0,0,WeeklyAlarmdst);  // TEMPO EM UTC !!  hora real verao(dst) = utc   || hora real inverno = utc-1
      Alarm.alarmRepeat(dowFriday,10,0,0,WeeklyAlarm); // Alarme de inverno
    
     
      myRTC.writeRTC(16,1); // RTC aging offset (+1 = 0.1ppm) Fazer as contas para afinal da proxima "manutenção"
      wdt_enable(WDTO_8S);  // watchdog       
 }
 
 
void loop() {   
   //---------------------------------------------------------
   //              [ 1 ]    RUN MOTOR: relay and start/stop   
   
    //start/stop  


   if(digitalRead(startsw_pin) == LOW && armed_bool == true) {
       run_bool = true;     
       timerun_millis = millis();
       armed_bool = false;
       caso=1;
       timeauto = 25*minutos;          
   }   
   if(digitalRead(aspersw_pin) == LOW && armed_bool == true) {
      run_bool = true;
      timerun_millis = millis();      
      armed_bool = false;
      caso=2;
      timeauto = 25*minutos;                
   }
   if(digitalRead(stopsw_pin) == LOW) {       
      run_bool = false;        
      armed_bool = true;  
   }
   
   // timer   
   if( milli(timerun_millis) > timeauto) run_bool = false;  
   
   //run motor
   if(run_bool == true){
    if(caso == 1)digitalWrite(motor_relay_pin, HIGH);
    if(caso == 2){
      digitalWrite(asper_relay_pin, HIGH);
      digitalWrite(ledasper_pin, HIGH);         
    }
   }
   if(run_bool == false){
    digitalWrite(motor_relay_pin, LOW);
    digitalWrite(asper_relay_pin, LOW);
    digitalWrite(ledasper_pin, LOW);
   }
     
   
     //---------------------------------------------------------
     //                 [ 2 ]    AUTO, led and eeprom
   
   
   if(digitalRead(autosw_pin) == HIGH) {
     timer1 = millis();     //reset timer   
     autopress_bool = false;
   }     
   
   if(digitalRead(autosw_pin) == LOW && autopress_bool == false) { 
                  
      if( milli(timer1) > 2500) {
        autopress_bool = true;
                  
        if(auto1_bool == true) {
          auto1_bool = false;
          EEPROM.write(3,0);
          digitalWrite(ledauto_pin, LOW);          
        }        
        else {
          auto1_bool = true;
          EEPROM.write(3,1);
          digitalWrite(ledauto_pin, HIGH);          
        }         
      }  
   }      
   
        
       
   
   
     //---------------------------------------------------------   
     //    [ 3 ]    WATCHDOG
     
    wdt_reset(); 

     //---------------------------------------------------------
     //    [ 4 ]    RTC 
     
   // alarms declared at setup
   
     Alarm.delay(10);  // runs scheduler  SHOULD BE LAST THING !     
} 

 
unsigned long milli(unsigned long teste){  // millis since "teste"
  unsigned long timerlong;
  current_millis = millis();
  timerlong = current_millis - teste;
  return timerlong;
}

 
void WeeklyAlarm(){  
   if(month() < 4 || month() > 10){
    alarm_func();
    }
 } 

void WeeklyAlarmdst(){  // alarme verao
  if(month() > 3 && month() < 11){
    alarm_func();
    }
   
 }

void alarm_func(){
   if(auto1_bool == true && run_bool == false) {
     timerun_millis = millis();
     run_bool = true;
     caso=2;
     armed_bool= false;
     timeauto = 20*minutos ;      //Nov,Dez,Jan,Fev,Mar
     if(month() > 3 && month() < 11){
      timeauto = 30*minutos;      //Abr,Mai,Set,Out
      }
     if(month() > 5 && month() < 9){
      timeauto = 40*minutos;  //Jun,Jul,Ago
      }          
    }  
}
 
