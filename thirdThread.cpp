// #include "header.h"

// Thread threadDistance;

// // Telemetric
// Timer timerEcho;
// DigitalOut trigger(PH_1);
// InterruptIn echo(D7);

// DigitalOut ledDistance(LED4);

// bool flagEcho,flagTrigger;
// int elapsedTime;
// int distanceCM=10;
// float blinktime;


// void echoStart(){
//     timerEcho.start();
// }

// void echoEnd(){
//     timerEcho.stop();
//     flagEcho=true;
// }

// void Distance(void){
    
//     echo.rise(echoStart);
//     echo.fall(echoEnd);

//         // distance sensor
//         // if it hasnt been sent, send a trigger signal
//         if (flagTrigger==false){
//             flagTrigger=true;

//             // 10 us pulse
//             trigger=1;
//             wait_us(10);
//             trigger=0;
//         }

//         if (flagEcho==true){
//             elapsedTime=timerEcho.elapsed_time().count(); // if timer is 0 it returns  58000, check it!
//             timerEcho.reset();

//             if (elapsedTime >= 58000){
//                 // sensor measurement error 
//                 // timer gave a wrong reading (10100)
//                 distanceCM=0;
//             }
//             else {
//             distanceCM = elapsedTime / 58;
            
//             if (distanceCM>=99) distanceCM=99;
//             // map values (wrong)
//             float minInRange= 0;
//             float maxInRange= 99;
//             float minOutRange= 0;
//             float maxOutRange= 2;
//             float x;

//             x = (distanceCM - minInRange) / (maxInRange - minInRange);
//             blinktime = minOutRange + (maxOutRange - minOutRange) * x;

//             // time to read again
//             flagTrigger=false;
//             }
//         }
//         ledDistance=1;
//         ThisThread::sleep_for(blinktime*1000);
//         ledDistance=0;
//         ThisThread::sleep_for(1s);
// }