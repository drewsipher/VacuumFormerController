#define ENCODER_OPTIMIZE_INTERRUPTS 1

#include <Encoder.h>
#include <EduIntro.h>
#include <U8g2lib.h>
#include "RunningAverage.hpp"

#define BUZZER 8
#define BTN_STOP A2
#define BTN A5
#define BTN_ENC1 A4
#define BTN_ENC2 A3
#define SID 11
#define CS 10
#define SLCK 13
#define THERMISTOR A6
#define RELAY 9

// resistance at 25 degrees C
#define THERMISTORNOMINAL 1000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 200   
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3750
// the value of the 'other' resistor
#define SERIESRESISTOR 939    

Encoder knob(BTN_ENC1, BTN_ENC2);
Button button(BTN);
Button emergencyBtn(BTN_STOP);

// U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, SLCK, SID, CS, U8X8_PIN_NONE);
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, CS, U8X8_PIN_NONE);

void tripleBuzz();
void emergencyBuzz();

enum state {
  temp,
  seconds,
  go
};

state _state = temp;
int32_t positionKnob  = 0;
int targetTemp = 200;
long int countdown = 180;

int secondTimer = 0;
int screenUpdateTimer = 0;
int tempTimer = 0;

RunningAverage tempRA = RunningAverage(10);
float averageTemp = 0;

bool emergency = false;

void setup() {
  Serial.begin(115200);
  // tripleBuzz();  
  u8g2.begin();
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
}


long int prevTime = millis();
bool updateLCD = true;

float GetTemperature()
{
  float average = analogRead(THERMISTOR);
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  // Serial.print("analog: ");
  // Serial.println(average); 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15; 
  return steinhart;

}

void relayOn()
{
  digitalWrite(RELAY, HIGH);
}

void relayOff()
{
  digitalWrite(RELAY, LOW);
}

void controlTemp()
{
  if ( averageTemp < targetTemp)
  {
    relayOn();
  } else {
    relayOff();
  }
}

void loop() {
  if (emergency)
    return;

  // put your main code here, to run repeatedly:
  int32_t newKnob = floor(knob.read()/4);
  
  if (newKnob != positionKnob) {
    Serial.println(newKnob);
    if (_state == temp)
    {
      targetTemp += newKnob - positionKnob;
    } else if (_state == seconds) {
      countdown += newKnob - positionKnob;
      countdown = max(countdown, 0);
    }
    positionKnob = newKnob;
    updateLCD = true;
  }
  if (button.pressed())
  {
    switch (_state)
    {
      case temp:
        _state = seconds;
        break;
      case seconds:
        _state = go;
        break;
      case go:
        _state = temp;
        break;
    }
    knob.write(0);
    positionKnob = 0;
    updateLCD = true; 
  }

  if (emergencyBtn.pressed())
  {
      relayOff();
      delay(10);
      emergencyBuzz();
      targetTemp = 0;
      _state = temp;
      emergency = true;
  }

  long int deltaTime = millis() - prevTime;
  prevTime = millis();
  
  secondTimer += deltaTime;
  screenUpdateTimer += deltaTime;
  tempTimer += deltaTime;

  if (tempTimer > 100)
  {
    tempTimer = 0;
    float T = GetTemperature();
    tempRA.addValue(T);
    averageTemp = tempRA.getAverage();
  }

  if (secondTimer > 1000)
  {
    secondTimer = 0;
    controlTemp();
    if (_state == go)
    {
      
      countdown = max(countdown-1, 0);
      updateLCD = true;
      if (countdown == 0)
      {
        tripleBuzz();
        countdown = 180;
        _state = temp;
      }
    }
  }

  if (screenUpdateTimer > 250)
  {
    screenUpdateTimer = 0;
  
    // if (updateLCD)
    {
      updateLCD = false;
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB12_tr);	
      u8g2.drawStr(5,30,("T/"+String(targetTemp)).c_str());
      u8g2.drawStr(60,30,("C/"+String(averageTemp)).c_str());
      u8g2.drawStr(5,50,("S/"+String(countdown)).c_str());	
      u8g2.drawStr(90,50,"GO");	
      if (_state == temp)
      {
        u8g2.drawLine(5,32,55,32);
      }else if (_state == seconds){
        u8g2.drawLine(5,52,60,52);
      } else if (_state == go)
      {
        u8g2.drawLine(90,52,100,52);
      }
      u8g2.sendBuffer();
    }
  }
  
}

void tripleBuzz()
{
  tone(BUZZER, 262, 100);
  delay(150);
  noTone(BUZZER);
  tone(BUZZER, 262, 100);
  delay(150);
  noTone(BUZZER);
  tone(BUZZER, 262, 100);
  delay(150);
  noTone(BUZZER);
}

void emergencyBuzz()
{
  tone(BUZZER, 262, 500);
  delay(500);
  // noTone(BUZZER);
  
  
  tone(BUZZER, 262, 500);
  delay(500);
  // noTone(BUZZER);
  
  
  tone(BUZZER, 262, 500);
  delay(500);
  noTone(BUZZER);
  
}