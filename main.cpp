/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "header.h"
#include <stdexcept>
#include <iostream>


// using namespace std;

InterruptIn sw(PB_2);
Timer tmr;
bool on = false;
bool off = false;
void sw_on_isr(void) {on = true;}
void sw_off_isr(void) {off = true;}


DigitalOut ledRed(D10);
DigitalOut ledGreen(D9);
DigitalOut ledBlue(D8);

DigitalOut ledTEST(D4), ledNORMAL(LED2), ledADVANCED(D10);


// string for mode flag (TEST, NORMAL or ADVANCED)
string mode("TEST");

EventFlags Flags;
uint32_t currentFlag;


// string for colors and function
string test("TEST"), normal("NORMAL"), advanced("ADVANCED");
void ledMode(string mode_aux) {
        if (mode_aux==test)             {ledTEST=1; ledNORMAL=0; ledADVANCED=0;} 
        else if (mode_aux==normal)      {ledTEST=0; ledNORMAL=1; ledADVANCED=0;} 
        else if (mode_aux==advanced)    {ledTEST=0; ledNORMAL=0; ledADVANCED=1;} 

}


// ACCELEROMETER
const int addMMA8451_7bit= 0x1C; // A connected to GND
const int addMMA8451_8bit= 0x1C << 1;   
MMA8451Q acc(D14,D15,addMMA8451_7bit);   

uint8_t IDtest;

// SERIAL TO PC
// static BufferedSerial serialPC(USBTX, USBRX);

// GPS 
//static BufferedSerial serialGPS(D8, D2);

int main()
{
    sw.mode(PullDown); // Al pulsarlo pasa a 1
    sw.fall(sw_on_isr);
    sw.rise(sw_off_isr);

    // threadSerial.start(BrightnessSerial);

    // SERIAL PC
    //serialPC.set_baud(9600);
    //serialPC.set_format(8,serialPC.None,1);

    // SERIAL GPS
    //serialGPS.set_baud(9600);
    //serialGPS.set_format(8,serialGPS.None,1);


    while (true) {

            // ACCELEROMETER TESTING
           // float test = acc.getAccX();
           // std::cout << test << "\n" ;
           // std::cout << (int) test << "\n" ;
           std::cout << "----------" << "\n";

           IDtest = acc.getWhoAmI();

           std::cout << (int)IDtest << "\n";

           


            // GPS TESTING
            char GPSbuffer[250]={};
            // serialGPS.read(GPSbuffer, 250);
            
            // std::cout << GPSbuffer << "\n";
            

            // $GPGGA,064951.000,2307.1256,N,12016.4438,E,1,8,0.95,39.9,M,17.8,M,,*65 
            // $GPGSA,A,3,29,21,26,15,18,09,06,10,,,,,2.32,0.95,2.11*00 
            //
            // $GPGSV,3,1,09,29,36,029,42,21,46,314,43,26,44,020,43,15,21,321,39*7D
            // $GPGSV,3,2,09,18,26,314,40,09,57,170,44,06,20,229,37,10,26,084,37*77
            // $GPGSV,3,3,09,07,,,26*73 
            //
            // $GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,165.48,260406,3.05,W,A*2C 
            // $GPVTG,165.48,T,,M,0.03,N,0.06,K,A*37
            
            // serialPC.write(GPSbuffer, 100);

            ThisThread::sleep_for(2s);



        if(on) { tmr.reset(); tmr.start(); on = false; } // iniciar contador

        if(off) {
            tmr.stop(); // terminar contador y medir el tiempo para el cambio de modo
            if      (mode=="TEST" )     {mode="NORMAL";     ledMode(normal);}
            else if (mode=="NORMAL" )   {mode="ADVANCED";   ledMode(advanced);}
            else if (mode=="ADVANCED")  {mode="TEST";       // ledMode(test);}
            off = false;
        }

            if (mode=="NORMAL") {
                switch (LightLevel) {
                //case 1: ledLight(red); break;
                //case 2: ledLight(yellow); break;
                //case 3: ledLight(green); break;
                }
                
                //currentFlag=Flags.get();
                //Flags.wait_any(FLAG_LOW | FLAG_MID | FLAG_HIGH);
            }

            

    }
}
}