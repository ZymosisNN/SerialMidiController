#include <EEPROM.h>

int knobLastVal = 0;
int lastBtnState = HIGH;
const int ADDR = 0;

struct Borders
{
  int lo = 0;
  int hi = 1024;
} borders;

void setup() {
  Serial.begin(115200);
//  while(!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CALIB_PIN, INPUT_PULLUP);
  pinMode(PEDAL_PIN, INPUT);

  EEPROM.get(ADDR, borders);


}

void loop() {
  
  
  // Calibrate button
  if (digitalRead(CALIB_PIN) == LOW)
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

