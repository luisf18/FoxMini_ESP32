#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "LED.h"
#include "SumoIR.h"
#include "DRV8833.h"
//#define UART2_PIN 17 // Defina o pino TX/RX para half-duplex


//==========================================================================
// Motors
//==========================================================================
DRV8833 motor = DRV8833( 18, 4, 23, 19 );

//==========================================================================
// global variables
//==========================================================================
uint32_t timeout = 0;

//==========================================================================
// Pins
//==========================================================================
uint8_t pinList[10] = { // lista dos pinos disponiveis
  39, 34, 35,
  32, 33, 25,
  26, 14, 16,
  17
};

bool is_analog_gpio( int pin ){
  return ( (pin>=32) || ( (pin>=25) && (pin<=27) ) || ( (pin>=12) && (pin<=15) ) || (pin==0) || (pin==2) );
}

//==========================================================================
// sensores
//==========================================================================
#define pin_sensor_left  32
#define pin_sensor_right 35
bool sensor_left_is_on(){
  return digitalRead( pin_sensor_left );
}
bool sensor_right_is_on(){
  return digitalRead( pin_sensor_right );
}

//==========================================================================
// OLED
//==========================================================================

// Inicializa o display OLED 128x32 via I2C
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
void oled_plot( const char *text ){
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(1,25);
    u8g2.clearBuffer();
    u8g2.print(text);
    u8g2.sendBuffer();
}

//==========================================================================
// IR receiver
//==========================================================================

SumoIR IR;
uint32_t IR_timeout = 0;
uint32_t IR_moving_timeout = 0;

void IR_handler(){
  if( millis() >= IR_timeout ){
    IR_timeout = millis() + 140;
    PX.fill(PX_BLUE);
    PX.write();
    switch( IR.read() ){
      case 0: oled_plot("Reset..."); ESP.restart(); break;
      case 1: oled_plot("PREPARE"); break;
      case 2: oled_plot("START");   break;
      case 3: oled_plot("STOP");    break;
      // Move robot
      case 117: oled_plot("Front"); motor.move( 1000, 1000); IR_moving_timeout = millis() + 170;  break; // move front
      case 118: oled_plot("Back");  motor.move(-1000,-1000); IR_moving_timeout = millis() + 170;  break; // move back
      case 53:  oled_plot("Left");  motor.move( -850,  850); IR_moving_timeout = millis() + 170;  break; // move left
      case 52:  oled_plot("Right"); motor.move(  850, -850); IR_moving_timeout = millis() + 170;  break; // move right
      case -1:  break; //motor.stop(); break;
      default:  oled_plot( ("CMD: "+String(IR.read())).c_str() ); break; //motor.stop(); break;
    }
    PX.push_blink(0,1,PX_BLACK,PX_GREEN,1200);
  }
}

//==========================================================================
// Setup
//==========================================================================
void setup() {
    u8g2.begin();
    u8g2.setFont(u8g2_font_helvB18_tf); // Fonte que suporta números e letras

    // sensor IR conectado no pino 15
    IR.begin(15);
    IR.onRecive( IR_handler );

    Serial.begin(115200);
    // Inicializa Serial2 (UART2) com 115200 baud, modo 8N1
    //Serial2.begin(115200, SERIAL_8N1, UART2_PIN, UART2_PIN);
    //Serial2.setTimeout( 50 );

    // Sensores
    pinMode( pin_sensor_left,  INPUT_PULLUP );
    pinMode( pin_sensor_right, INPUT_PULLUP );

    // Pixel
    PX.begin();
    PX.setBrightness(20);

    // Alerta de inicialização
    PX.fill( PX_PURPLE );
    PX.write();
    delay(200);
    //PX.push_blink(0,1,PX_BLUE, PX_RED, 500);
    PX.push_blink(0,1,PX_BLACK,PX_GREEN,1500);

    // Button 13
    pinMode( 13, INPUT_PULLUP );
    // Button 0
    pinMode( 0, INPUT_PULLUP );

    pinMode(25,INPUT);
    pinMode(26,INPUT);

    // OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_9x15_tf);
    //u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.setCursor(5,14);
    u8g2.print("FoxMini ESP32");
    //u8g2.setCursor(15,25);
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.setCursor(5,29);
    u8g2.print("Fox Dynamics");
    u8g2.sendBuffer();

    // Begin Motors
    motor.begin();
    motor.sound_vol(12);
    motor.bip( 2, 100, 2000, 0 ); // Motor 0
    motor.bip( 2, 100, 2000, 1 ); // Motor 1
    motor.bip( 1, 50, 5500 );
    motor.bip( 1, 50, 3500 );
    motor.bip( 1, 50, 1500 );
    delay(500);
}

//==========================================================================
// Loop
//==========================================================================
void loop() {
  
  IR.update();
  PX.update();
  delay(5);

  // IR motor moving
  if(IR_moving_timeout){
    if( millis() >= IR_moving_timeout ){
      IR_moving_timeout = 0;
      motor.stop();
    }
  }else{
    // movimento autonomo
    // centraliza com um objeto
    pinMode( pin_sensor_left,  INPUT_PULLUP );
    pinMode( pin_sensor_right, INPUT_PULLUP );
    bool sl = sensor_left_is_on();
    bool sr = sensor_right_is_on();
    if( sl == sr ){ // center
      //Serial.println( "=" );
      motor.stop();
    }else if( sr ){ // move left
      //Serial.println( "R" );
      motor.move( 900, -900 );
    }else{ // move right
      //Serial.println( "L" );
      motor.move( -900, 900 );
    }
  }

  // Read button 0
  if( !digitalRead(0) ){
    oled_plot("Button 0");
    delay(500);
    
  }

  // Read button 13
  if( !digitalRead(13) ){
    oled_plot("Button 13");
    delay(400);
  }

  // update oled display with pin reading
  if( millis() > timeout ){
    timeout = millis() + 300;
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.clearBuffer();
    for(int i=0;i<10;i++){
      u8g2.setCursor(13*i,32);
      u8g2.drawFrame(13*i,0, 9, 21 );
      //u8g2.drawBox(11*i,0, 9, 2+i*1.5 );
      //int h = analogRead(pinList[i])*(21.0/4095.0);
      int h = (
        is_analog_gpio( pinList[i] ) ? 
        analogRead(pinList[i])*(21.0/4095.0) : 
        digitalRead(pinList[i])*21
      );
      u8g2.drawBox(13*i,1, 9, h );
      u8g2.print( pinList[i] );
    }
    u8g2.sendBuffer();
  }

}
