// Assorted useful functions and variables

// Global variables
boolean effectInit = false; // indicates if a pattern has been recently switched
uint16_t effectDelay = 0; // time between automatic effect changes
unsigned long effectMillis = 0; // store the time of last effect function run
unsigned long cycleMillis = 0; // store the time of last effect change
unsigned long currentMillis; // store current loop's millis value
unsigned long hueMillis; // store time of last hue change
unsigned long eepromMillis; // store time of last setting change
unsigned long audioMillis; // store time of last audio update
byte currentEffect = 0; // index to the currently running effect
boolean autoCycle = true; // flag for automatic effect changes
boolean eepromOutdated = false; // flag for when EEPROM may need to be updated
byte currentBrightness = STARTBRIGHTNESS; // 0-255 will be scaled to 0-MAXBRIGHTNESS
boolean audioEnabled = false; // flag for running audio patterns
boolean fadingActive = true;

CRGBPalette16 currentPalette(RainbowColors_p); // global palette storage

typedef void (*functionList)(); // definition for list of effect function pointers
extern byte numEffects;

// Increment the global hue value for functions that use it
byte cycleHue = 0;
byte cycleHueCount = 0;
void hueCycle(byte incr) {
    cycleHueCount = 0;
    cycleHue+=incr;
}

// Set every LED in the array to a specified color
void fillAll(CRGB fillColor) {
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = fillColor;
  }
}

// Fade every LED in the array by a specified amount
void fadeAll(byte fadeIncr) {
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = leds[i].fadeToBlackBy(fadeIncr);
  }
}

// Shift all pixels by one, right or left (0 or 1)
void scrollArray(byte scrollDir) {
  
    byte scrollX = 0;
    for (byte x = 1; x < kMatrixWidth; x++) {
      if (scrollDir == 0) {
        scrollX = kMatrixWidth - x;
      } else if (scrollDir == 1) {
        scrollX = x - 1;
      }
      
      for (byte y = 0; y < kMatrixHeight; y++) {
        leds[XY(scrollX,y)] = leds[XY(scrollX + scrollDir*2 - 1,y)];
      }
    }
}

void scrollArray2(byte scrollDir) {
  
    byte scrollY = 0;
    for (byte y = 0; y < kMatrixHeight; y++) {
      if (scrollDir == 0) {
        scrollY = kMatrixHeight - y;
      } else if (scrollDir == 1) {
        scrollY = y;
      }
      
      for (byte x = 0; x < kMatrixWidth; x++) {
        leds[XY(x, scrollY)] = leds[XY(x, scrollY + scrollDir * 2 - 1)];
      }
    }
}

// Pick a random palette from a list
void selectRandomPalette() {

  switch(random8(8)) {
    case 0:
    currentPalette = CloudColors_p;
    break;
    
    case 1:
    currentPalette = LavaColors_p;
    break;
    
    case 2:
    currentPalette = OceanColors_p;
    break;
    
    case 4:
    currentPalette = ForestColors_p;
    break;
    
    case 5:
    currentPalette = RainbowColors_p;
    break;
    
    case 6:
    currentPalette = PartyColors_p;
    break;
    
    case 7:
    currentPalette = HeatColors_p;
    break;
    
//    case 8:
//    currentPalette = CRGBPalette16(CRGB::LightGrey, CRGB::MidnightBlue, CRGB::DeepPink, CRGB::LimeGreen, CRGB::Aqua, CRGB::Maroon);
//    break;
  }
}

// Pick a random palette from a list
void selectRandomAudioPalette() {

  switch(random8(8)) {
    case 0:
    currentPalette = CRGBPalette16(CRGB::Red, CRGB::Orange, CRGB::Gray);
    break;
    
    case 1:
    currentPalette = CRGBPalette16(CRGB::Blue, CRGB::Red, CRGB::Red);
    break;
    
    case 2:
    currentPalette = CRGBPalette16(CRGB::LightGrey, CRGB::MidnightBlue, CRGB::Black);
    break;
    
    case 4:
    currentPalette = CRGBPalette16(CRGB::DarkGreen, CRGB::PaleGreen);
    break;
    
    case 5:
    currentPalette = RainbowColors_p;
    break;
    
    case 6:
    currentPalette = PartyColors_p;
    break;
    
    case 7:
    currentPalette = HeatColors_p;
    break;
  }
}
// Interrupt normal operation to indicate that auto cycle mode has changed
void confirmBlink(CRGB blinkColor, byte count) {

  for (byte i = 0; i < count; i++) {
    fillAll(blinkColor);
    FastLED.show();
    delay(200);
    fillAll(CRGB::Black);
    FastLED.show();
    delay(200);
  }
}

// Determine flash address of text string
unsigned int currentStringAddress = 0;
void selectFlashString(byte string) {
  currentStringAddress = pgm_read_word(&stringArray[string]);
}

// Fetch font character bitmap from flash
byte charBuffer[5] = {0};
void loadCharBuffer(byte character) {
  byte mappedCharacter = character;
  if (mappedCharacter >= 32 && mappedCharacter <= 95) {
    mappedCharacter -= 32; // subtract font array offset
  } else if (mappedCharacter >= 97 && mappedCharacter <= 122) {
    mappedCharacter -= 64; // subtract font array offset and convert lowercase to uppercase
  } else {
    mappedCharacter = 96; // unknown character block
  }
  
  for (byte i = 0; i < 5; i++) {
    charBuffer[i] = pgm_read_byte(Font[mappedCharacter]+i);
  }
}

// Fetch a character value from a text string in flash
char loadStringChar(byte string, byte character) {
  return (char) pgm_read_byte(currentStringAddress + character);
}

// write EEPROM value if it's different from stored value
void updateEEPROM(byte location, byte value) {
  if (EEPROM.read(location) != value) EEPROM.write(location, value);
}

// Write settings to EEPROM if necessary
void checkEEPROM() {
  if (eepromOutdated) {
    if (currentMillis - eepromMillis > EEPROMDELAY) {
      updateEEPROM(0, 99);
      updateEEPROM(1, currentEffect);
      updateEEPROM(2, autoCycle);
      updateEEPROM(3, currentBrightness);
      eepromOutdated = false;
    }
  }
}
