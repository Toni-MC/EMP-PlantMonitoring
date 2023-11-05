/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "header.h"


// using namespace std

// mode LEDS definition
//         LED1=D4        LED2=PA_5        LED3=D10
DigitalOut ledTEST(LED1), ledNORMAL(LED2), ledADVANCED(LED3);

// Mode enum
deviceMode mode;

// Blue Button logic for mode change
InterruptIn sw(PB_2);

bool on = false;
bool off = false;
void sw_on_isr(void) {on = true;}
void sw_off_isr(void) {off = true;}


void changeMode(deviceMode mode_aux){
         switch(mode_aux)
            {
                case TEST  :    mode=TEST;     ledTEST=1; ledNORMAL=0; ledADVANCED=0;   break;
                case NORMAL:    mode=NORMAL;   ledTEST=0; ledNORMAL=1; ledADVANCED=0;   break;
                case ADVANCED:  mode=ADVANCED; ledTEST=0; ledNORMAL=0; ledADVANCED=1;   break;
            }
}


int main()
{
    sw.mode(PullDown); // Al pulsarlo pasa a 1
    sw.fall(sw_on_isr);
    sw.rise(sw_off_isr);

    // Initialize mode to NORMAL
    changeMode(TEST);

    // Start the second thread for measurements
    threadMeasurements.start(MeasurementsDisplay);

    while (true) {

            // GPS TESTING
            //char GPSbuffer[250]={};
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

            // ThisThread::sleep_for(2s);



        if(on) {on = false; } // iniciar contador

        if(off) {
            switch(mode)
            {
                case TEST  :    changeMode(NORMAL);   break;
                case NORMAL:    changeMode(ADVANCED); break;
                case ADVANCED:  changeMode(TEST);     break;
            }
            off = false;
        }


        switch(mode)
            {
                case TEST  :        break;
                case NORMAL:        break;
                case ADVANCED:      break;
            }

            
    }
}
