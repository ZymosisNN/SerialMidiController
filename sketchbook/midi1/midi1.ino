#include <EEPROM.h>
#include <ardumidi.h>

int knobPin = 0;
int calibPin = 8;
int knobLastVal = 0;

int lastBtnState = HIGH;

int hi = 1023;
int lo = 0;
const int ADDR = 0;
struct Borders
{
  int lo = 0;
  int hi = 1023;
} borders;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(calibPin, INPUT_PULLUP);
  pinMode(knobPin, INPUT);

  EEPROM.get(ADDR, borders);


}

void loop()
{
  // Knobs
  int v = analogRead(knobPin);
  int knobV = map(v, borders.lo, borders.hi, 0, 127);
  if (knobLastVal != knobV)
  {
    knobLastVal = knobV;
    //Serial.print(v); Serial.print("  "); Serial.println(knobV);
    midi_controller_change(1, 4, knobV);
    delay(5);
  }

  // Calibrate button
  if (digitalRead(calibPin) == LOW)
  {
    if (lastBtnState == HIGH)
    {
      lastBtnState = LOW;
      calibrate();
    }
  } else {
    lastBtnState = HIGH;
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

  while(digitalRead(calibPin) == HIGH)
  {
    borders.lo = analogRead(knobPin);
  }

  indicate(3, HIGH);

  while(digitalRead(calibPin) == HIGH)
  {
    borders.hi = analogRead(knobPin);
  }

  indicate(2, LOW);

  EEPROM.put(ADDR, borders);

}

// Available commands:
//
// Start/stop playing a certain note:
//   midi_note_on(byte channel, byte key, byte velocity);
//   midi_note_off(byte channel, byte key, byte velocity);
//
// Change pressure of specific keys:
//   midi_key_pressure(byte channel, byte key, byte value);
//
// Change controller value (used for knobs, etc):
//   midi_controller_change(byte channel, byte controller, byte value);
//
// Change "program" (change the instrument):
//   midi_program_change(byte channel, byte program);
//
// Change key pressure of entire channels:
//   midi_channel_pressure(byte channel, byte value);
//
// Change pitch-bend wheel:
//   midi_pitch_bend(byte channel, int value);
//
// Send a comment:
//   midi_comment(char* str);
//
// Send a series of bytes (to be interpreted by another program):
//   midi_printbytes(char* bytes, int len);
//
// Parameters:
//   channel             an integer from 0 to 15
//   pitch-bend value    an integer from 0 to 16383
//   all other values    an integer from 0 to 127
//

