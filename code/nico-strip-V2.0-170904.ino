
////////////////////////////////////////////////
// This program is designed for Nico's Matrix
// When the sound is on, the LED is breathing
// When the sound is off, the LED is off
//
// Author: littleJames (xiaomingsmart@163.com)
// Date: 2017-08-14
// Ver. 2.0.2
////////////////////////////////////////////////

#include <Adafruit_NeoPixel.h>

// define the LED Control parameter 
#define PIN 6
#define NUM_LEDS 180
#define LED_BRIGHTNESS 100
#define BREATHE_SPEED_01 0.065
#define BREATHE_SPEED_02 0.045
#define LED_PERIOD 1780
#define LED_SPECIAL 5
#define LED_WAIT_LONG 1500
#define LED_WAIT_SHORT 50

// define the MCU's pins
const int dataPin = 9;
const int csPin = 10;
const int clkPin = 11;
const int rstPin = 12;
const int busyPin = 8;

// define the command string
unsigned char sendStop = 254;
unsigned char sendRepeat = 242;
unsigned char sendIOgo = 245;
unsigned char sendIOback = 246;
unsigned char sendVoiceMin = 224;
unsigned char sendVoiceMax = 231;

// define the sound address(00H-03H)
unsigned char ss_startup = 0;
unsigned char ss_heart = 1;
unsigned char ss_womb = 2;
unsigned char ss_mom_womb = 3;

// switch the play mode: playWomb() or not.
boolean playPink=true;
boolean flag_busy=false;

// init the LED Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// We have many colors for LEDS
uint32_t ledColor_none = 0X000000;
uint32_t ledColor[]={
    0XC0C0C0,0XF88D1E,
    0XEED5D2,0XCD5B45,
    0XA52A2A,0X8B0000
                    };

uint32_t colorCount=6;


void setup() {
    
    Serial.begin(9600);

    //Serial.print("init the controller's pins""\n");
    pinMode(dataPin, OUTPUT);
    pinMode(csPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    pinMode(rstPin, OUTPUT);
    pinMode(busyPin, INPUT);

    // set sound volume to max
    send_ThreeLines(sendVoiceMax);
    delay(15);

    // stop the sound
    send_ThreeLines(sendStop);
    delay(15);
    
    // init the led strip, and set all pixels to off
    strip.begin();
    strip.setBrightness(LED_BRIGHTNESS);
    cleanStrip();
    strip.show(); // Initialize all pixels to 'off'
    delay(LED_WAIT_SHORT);

    // play the startup sound
    play_Startup(ss_startup,busyPin);
    delay(LED_WAIT_LONG);

    colorWipe(ledColor_none,4);
    delay(LED_WAIT_LONG);

}


void loop() {
    //Serial.print("The loop start!""\n");
    if(playPink==true){
    
        //play_Pink(ss_mom_womb,ledColor[2]);
        play_Pink(ss_mom_womb,0XF5F5F5);
        delay(LED_WAIT_LONG);

    }else{

        play_Warm(ss_mom_womb,ledColor[1]);
        delay(LED_WAIT_LONG);

    }
}


// the play_Pink() function
// play the womb sound that is stored in the addr
// light on LEDS in breathe manner.
void play_Pink(unsigned char addr,uint32_t colorSelect){
       
    send_ThreeLines(addr);
    delay(30);

    flag_busy = true;
    
    while(flag_busy==true){
        // Play the LED
        ledBreathe(NUM_LEDS,colorSelect,LED_BRIGHTNESS,LED_PERIOD,BREATHE_SPEED_01);
        delay(LED_WAIT_SHORT);
    }
        
    delay(LED_WAIT_SHORT);
}


// the play_Warm() function
// play the sound that is stored in the addr
void play_Warm(unsigned char addr,uint32_t colorSelect){

    send_ThreeLines(addr);
    delay(30);

    flag_busy = true;

    while(flag_busy==true){
    
        // detect the feedback signal per 30ms
        ledBreathe(NUM_LEDS,colorSelect,LED_BRIGHTNESS,LED_PERIOD,BREATHE_SPEED_01);
        delay(LED_WAIT_SHORT);
    }

    colorWipe(ledColor_none,10);
    strip.show();
    
    delay(LED_WAIT_SHORT);
}


// the play_Startup() function
// play the sound that is stored in the addr
void play_Startup(unsigned char addr,int busypin){

    bool flag_busy = true;
    uint32_t colorSelect;
    
    send_ThreeLines(addr);
    delay(30);
    
    // detect the feedback signal per 30ms
    while(flag_busy==true){

       for(uint32_t i=0; i<colorCount; i++){ 
           colorWipe(ledColor[i],5);
           delay(LED_WAIT_SHORT);
       }
        
        flag_busy =!digitalRead(busypin);
        delay(30);
        
    }
    
    flag_busy = false;
    delay(100);

}



// Sets all LEDs to off, but Does not update the display;
// call strip.show() to actually turn them off after this.
void cleanStrip()
{
    for(uint32_t i=0; i<NUM_LEDS; i++){
        strip.setPixelColor(i,0);
    }
}

// Sets all LEDs to Gray
void showGray()
{
    for(uint32_t i=0; i<NUM_LEDS; i++){
        strip.setPixelColor(i,0XC0C0C0);
    }
}


// Make the LEDS breathe
void ledBreathe(uint16_t n, uint32_t color, float maxBrightness, uint32_t ledPeriod,float breatheSpeed){

    uint16_t TOTAL_LEDS = n;
    float MaximumBrightness = maxBrightness;
    float SpeedFactor = breatheSpeed; // set the leds breathe speed
    float StepDelay = 1; // ms for a step delay on the lights
    
        // Make the lights breathe
        for (float i = 0; i <ledPeriod; i++) {

            flag_busy=!analogRead(busyPin);
            delay(30);
    
            // Intensity will go from 10 - MaximumBrightness in a "breathing" manner
            float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i+LED_SPECIAL));
            strip.setBrightness(intensity);
            
            // Now set every LED to that color
            for (int ledNumber=0; ledNumber<TOTAL_LEDS; ledNumber++) {
                strip.setPixelColor(ledNumber, color);
                
            }
            
            strip.show();
            
            //Wait a bit before continuing to breathe
            delay(StepDelay);
        }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

// Control the sound module by three lines.
void send_ThreeLines(unsigned char addr){
    digitalWrite(rstPin, LOW);
    delay(5);   
    digitalWrite(rstPin, HIGH);
    delay(20);
    digitalWrite(csPin, LOW);
    delay(5);
    
    for(int i=0;i<8;i++){
        digitalWrite(clkPin, LOW);

        if(addr&1){
            digitalWrite(dataPin, HIGH);

        }else{
            digitalWrite(dataPin, LOW);

        }

        addr >>=1;
        delayMicroseconds(150);

        digitalWrite(clkPin, HIGH);
        delayMicroseconds(150);
    }
    
    digitalWrite(csPin, HIGH);
}

