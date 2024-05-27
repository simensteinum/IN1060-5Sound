#include <FastLED.h>

#define LED_PIN 3  //Ledstrip connected to analog output
#define NUM_LEDS 1
#define BRIGHTNESS 30

#define LED_TYPE WS2813  //Type of ledstrip used
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

const int sampleTime = 50;  //50 mS = 20Hz
const int micPin = A0;      //Mic pin connected analog output
unsigned int micSample;
const int baseline = 53;  //Depends on microphone placement and background noise. Needs to be calibrated to extern decibel meter with every use

const long interval = 60000;  //Milliseconds in 1 minute
int minutes = 0;
int tenMins = 0;

/**
 * Lists to store decibel values to calculate average from
 * minValues: Receives 399 values per minute
 * tenMinValues: Receives 10 values per 10 minutes
 * hourValues: Receives 60 values per hour
 */
int minValues[399];
int tenMinValues[10];
int hourValues[60];

//Separate indexes for lists to be increased independently
int minIndex = 0;
int tenMinIndex = 0;
int hourIndex = 0;

//Variables to store average values for all time frames in
int minAvg;
int tenMinAvg;
int hourAvg;

void setup() {
  //Initialize ledstrip with type, pin and number of leds
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //Set brightness level
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
}

void loop() {
  //Start tracking decibel values
  startTracker();
}

void startTracker() {
  //Starting minute tracker
  unsigned long lastMinute = millis();
  //Do while less than a minute has passed
  while (millis() - lastMinute < interval) {
    unsigned long currentMillis = millis();        //Initialize current time frame
    int dB = getDecibels(currentMillis);  //Get decibels for current timeframe
    if (dB > 110) {                                //Imminently dangerous dB level
      redLed();
    }
    addToMinValues(dB);               //Store every dB value in minute values
    Serial.print("Desibelnivå på ");  //Print values for researchers (not subjects) to have overview of testing
    Serial.print(millis() / 1000);    //Print number of seconds passed
    Serial.print(" sekunder: ");
    Serial.println(dB);
    delay(100);  //Delay for stability
  }
  //1 minute has passed
  minAvg = getDBAvg(minValues, 399);  //Calculating the decibel average after every minute. There are 399 iterations per minute
  Serial.print("Gjennomsnittlig dB-nivå det siste minuttet: ");
  Serial.println(minAvg);
  ledMinute(minAvg);  //Setting ledstrip to 1 minute case
  minutes++;
  addToTenMinValues(minAvg);  //Adding minute average to tenMinValues
  //10 minutes have passed
  if (minutes == 10) {
    tenMinAvg = getDBAvg(tenMinValues, 10);  //Calculating the decibel average for 10 minutes
    addToHourValues(tenMinAvg);
    Serial.print("Gjennomsnittlig dB-nivå de siste 10 minuttene: ");
    Serial.println(tenMinAvg);
    ledTenMinutes(tenMinAvg);  //Setting ledstrip to 10 minute case
    minutes = 0;               //Restarting the count to get continuous values, and avoid unnecessary math
    tenMins++;
  }
  //1 hour has passed
  if (tenMins == 6) {
    hourAvg = getDBAvg(hourValues, 60);  //Calculating the decibel average for one hour
    Serial.print("Gjennomsnittlig dB-nivå den siste timen: ");
    Serial.println(hourAvg);
    ledHour(hourAvg);  //Setting ledstrip to hour case
    tenMins = 0;
  }
}

/**
 * Ledstrip reacts differently based on time frame
 * Shorter time frame means higher decibel level needed to turn on LED
 * redLed(): Decibel level is very harmful
 * orangeLed(): Decibel level is close to turning harmful
 * yellowLed(): Decibel level may turn harmful
 * blackLed(): Decibel level is unharmful, turn off LED
 */
void ledMinute(int dB) {
  if (dB >= 70) {
    redLed();
  } else if (dB >= 65 && dB < 70) {
    orangeLed();
  } else if (dB >= 60 && dB < 65) {
    yellowLed();
  } else {
    blackLed();
  }
}

void ledTenMinutes(int dB) {
  if (dB >= 60) {
    redLed();
  } else if (dB >= 55 && dB < 60) {
    orangeLed();
  } else if (dB >= 50 && dB < 55) {
    yellowLed();
  } else {
    blackLed();
  }
}

void ledHour(int dB) {
  if (dB >= 45) {
    redLed();
  } else if (dB >= 40 && dB < 45) {
    orangeLed();
  } else if (dB >= 35 && dB < 40) {
    yellowLed();
  } else {
    blackLed();
  }
}

/**
 * Get average dB level over time
 * Divide total sum by number of values in list
 * return: dB average of list
 */
int getDBAvg(int list[], int numberOfValues) {
  int sum = 0;
  for (int i = 0; i < numberOfValues; i++) {
    sum += list[i];
  }
  return (sum / numberOfValues);
}

/**
 * Add dB values to different list based on time frame
 * Modulo operator increases indexes to maximum value, and then goes back to 0 
 */
void addToMinValues(int dBValue) {
  minValues[minIndex] = dBValue;
  minIndex = (minIndex + 1) % 399;
}

void addToTenMinValues(int dBValue) {
  tenMinValues[tenMinIndex] = dBValue;
  tenMinIndex = (tenMinIndex + 1) % 10;
}

void addToHourValues(int dBValue) {
  hourValues[hourIndex] = dBValue;
  hourIndex = (hourIndex + 1) % 60;
}

//Control ledstrip colors
void redLed() {
  leds[0] = CRGB(196, 30, 45);
  FastLED.show();
  delay(100);
}

void orangeLed() {
  leds[0] = CRGB(200, 95, 31);
  FastLED.show();
  delay(100);
}

void yellowLed() {
  leds[0] = CRGB(255, 255, 0);
  FastLED.show();
  delay(100);
}

void blackLed() {
  leds[0] = CRGB(0, 0, 0);
  FastLED.show();
  delay(100);
}

/**
 * Calculate current dB level
 * Add baseline to account for microphone displacement and background noise
 * return: dB value for current time
 */
int getDecibels(unsigned long currentMillis) {
  return peakToDB(peakToPeak(currentMillis)) + baseline;
}

/**
 * Measure signal for 50mS
 * Remove readings above 1024 for correctness
 * Maximum signal levels minus minimum signal levels equal peak-to-peak amplitude
 * return: peak-to-peak value
 */
float peakToRMS(int peak) {
  return peak / (2.0 * sqrt(2.0));
}

/**
 * Convert peak-to-peak amplitude to dB
 * voltage: root mean square voltage calculated from peak
 * refVoltage: Reference voltage from Arduino output
 * Use logarithmic conversion
 * return: dB value
 */
float peakToDB(int peak) {
  float voltage = peakToRMS(peak);
  float refVoltage = 3.3;
  return 20.0 * log10(voltage / refVoltage);
}

/**
 * Measure signal for 50mS
 * Remove readings above 1024 for correctness
 * Maximum signal levels minus minimum signal levels equal peak-to-peak amplitude
 * return: peak-to-peak value
 */
unsigned int peakToPeak(unsigned long currentMillis) {
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  while (millis() - currentMillis < sampleTime) {
    micSample = analogRead(micPin);
    if (micSample < 1024) {
      if (micSample > signalMax) {
        signalMax = micSample;
      } else if (micSample < signalMin) {
        signalMin = micSample;
      }
    }
  }
  return signalMax - signalMin;
}