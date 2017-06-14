#include <EEPROM.h>
#include <Bounce2.h>
#include "config.h"

int knobLastVal = 0;
int lastBtnState = HIGH;
const int ADDR = 0;

int pedalMode = MODE_VOL;

struct Borders
{
  int lo = 0;
  int hi = 1024;
} borders;

Bounce btn_calib = Bounce(); 
Bounce btn_wah_en = Bounce(); 
Bounce btn_pitch_en = Bounce(); 


void setup() {
  Serial.begin(115200);
//  while(!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CALIB_PIN, INPUT_PULLUP);
  pinMode(WAH_EN_PIN, INPUT_PULLUP);
  pinMode(PITCH_EN_PIN, INPUT_PULLUP);
  pinMode(PEDAL_PIN, INPUT);

  readBorders();

  btn_calib.attach(CALIB_PIN);
  btn_calib.interval(5);
  btn_wah_en.attach(WAH_EN_PIN);
  btn_wah_en.interval(5);
  btn_pitch_en.attach(PITCH_EN_PIN);
  btn_pitch_en.interval(5);

}

void loop() {
  
  // Update the Bounce instances :
  btn_calib.update();
  btn_wah_en.update();
  btn_pitch_en.update();

  // Get the updated value :
  if (btn_calib.read() == LOW)
  {
    if (lastBtnState == HIGH)
    {
      lastBtnState = LOW;
      calibrate();
    }
  } else {
    lastBtnState = HIGH;
  }
  
  // Pedal
  int v = analogRead(PEDAL_PIN);
  int knobV = map(v, borders.lo, borders.hi, 0, 127);
  if (knobLastVal != knobV)
  {
    knobLastVal = knobV;
    Serial.print(CTRL_WAH_POS); Serial.print("  "); Serial.print(v); Serial.print("  "); Serial.println(knobV);
    controlChange(MIDI_CHANNEL, CTRL_WAH_POS, knobV);
    delay(5);
  }

}


void indicate(int n, int out)
{
  for (int i = 0; i < n ; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  delay(500);
  digitalWrite(LED_BUILTIN, out);
}

void calibrate()
{
  indicate(5, HIGH);
  while(digitalRead(CALIB_PIN) == HIGH)
  {
    borders.lo = analogRead(PEDAL_PIN);
  }
  indicate(3, HIGH);
  while(digitalRead(CALIB_PIN) == HIGH)
  {
    borders.hi = analogRead(PEDAL_PIN);
  }
  indicate(2, LOW);
  saveBorders();
}

void saveBorders()
{
  EEPROM.write(ADDR,   highByte(borders.lo));
  EEPROM.write(ADDR+1, lowByte(borders.lo));
  EEPROM.write(ADDR+2, highByte(borders.hi));
  EEPROM.write(ADDR+3, lowByte(borders.hi));
}

void readBorders()
{
  borders.lo = word(EEPROM.read(ADDR), EEPROM.read(ADDR+1));
  borders.hi = word(EEPROM.read(ADDR+2), EEPROM.read(ADDR+3));
}


// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOn(byte channel, byte pitch, byte velocity) {
  MIDIEvent noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MIDIUSB.write(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  MIDIEvent noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MIDIUSB.write(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
void controlChange(byte channel, byte control, byte value) {
  MIDIEvent event = {0x0B, 0xB0 | channel, control, value};
  MIDIUSB.write(event);
}

