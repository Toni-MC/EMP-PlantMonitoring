/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "header.h"
#include <cstdio>


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



// Telemetric
Timer timerEcho,timerLEDControl;
DigitalOut trigger(PH_1);
InterruptIn echo(D7);

// Also using PB_7 (LED4) for serial communication with the GPS in
DigitalOut ledDistance(LED4);

bool flagEcho,flagTrigger;
int elapsedTime;
float distanceCM=10;
float blinktime;

void flipLED(){
    // ledDistance=!ledDistance;
}


void echoStart(){
    timerEcho.start();
}

void echoEnd(){
    timerEcho.stop();
    flagEcho=true;
}


int main()
{
    sw.mode(PullDown); // Al pulsarlo pasa a 1
    sw.fall(sw_on_isr);
    sw.rise(sw_off_isr);

    // distance sensor
    echo.rise(echoStart);
    echo.fall(echoEnd);


    // Initialize mode to NORMAL
    changeMode(NORMAL);

    // Start the second thread for measurements
    threadMeasurements.start(MeasurementsDisplay);

    // threadDistance.start(Distance);
    timerLEDControl.start();



    while (true) {

        if(on) {on = false; }

        if(off) {
            switch(mode)
            {
                case TEST  :    changeMode(NORMAL);   break;
                case NORMAL:    changeMode(ADVANCED); break;
                case ADVANCED:  changeMode(TEST);     break;
            }
            off = false;
        }


        // switch(mode)
        //     {
        //         case TEST  :        break;
        //         case NORMAL:        break;
        //         case ADVANCED:      break;
        //     }


        // distance sensor
        // if it hasnt been sent, send a trigger signal
        if (flagTrigger==false){
            flagTrigger=true;

            // 10 us pulse
            trigger=1;
            wait_us(10);
            trigger=0;
        }

        if (flagEcho==true){
            elapsedTime=timerEcho.elapsed_time().count(); // if timer is 0 it returns  58000, check it!
            timerEcho.reset();

            if (elapsedTime >= 58000){
                // sensor measurement error 
                // timer gave a wrong reading (10100)
                distanceCM=0;
            }
            
            distanceCM = (float)elapsedTime / 58;
            
            if (distanceCM>=99) distanceCM=99;
            // map values (wrong)
            float minInRange= 0;
            float maxInRange= 99;
            float minOutRange= 0;
            float maxOutRange= 2;
            float x;

            x = (distanceCM - minInRange) / (maxInRange - minInRange);
            blinktime = minOutRange + (maxOutRange - minOutRange) * x;

            // time to read again
            flagTrigger=false;
            
        }

        if (timerLEDControl.elapsed_time().count() >= blinktime*1000000){

            flipLED();
            timerLEDControl.reset();
        }

        // ledDistance=1;
        // ThisThread::sleep_for(blinktime*1000);
        // ledDistance=0;

        // ThisThread::sleep_for(1s);
        
    }
}
