#include "header.h"
#include <cstdio>
#include <cstring>

#include <bitset>
#include <cassert>
#include <cstddef>
#include <iostream>

AnalogIn photoDiode(A0,3.3);
Thread threadSerial;
Ticker tickSerial;
int LightLevel;
volatile bool serialFlag;

#define MAXIMUM_BUFFER_SIZE     32
// static BufferedSerial serial(USBTX, USBRX);
FILE * pc;

// D14: SDA
// D15: SCL

// I2C i2c(D14,D15);
    const int si7021addr7bit = 0x40;      // 7 bit I2C address
    const int si7021addr8bit = 0x40 << 1 ;  // 8 bit 

    const int addMMA8451_7bit= 0x1C;
    const int addMMA8451_8bit= 0x1C << 1;    

    // char cmd[1];
   


unsigned short Brightness, BrightnessMAX=65535;
float temp;


void tick(){serialFlag=true;}



void BrightnessSerial(void) {

   // serial.set_baud(9600);
   // serial.set_format(8,serial.None,1);
   
    tickSerial.attach(&tick,2s); // Cada 2 segundos actualizar  Brightness


    
    while(true) {

/*
                std::cout << "-------------" << "\n";
                // Read Accelerometer
                char cmd[1], cmd_X_MSB[1];
                char buf2[2]={}, buf1[1]={};

                cmd_X_MSB[0]=0x01;

                std:bitset<8> BUFFER;
                BUFFER=buf2[1]; 
                std::cout << "BUFFER2:" << BUFFER << "\n" ;
                BUFFER=buf1[0];
                std::cout << "BUFFER1:" << BUFFER << "\n" ;


                // STATUS READ
                cmd[0]=0x0D;    // status register
                i2c.write(addMMA8451_7bit ,cmd,1);
                i2c.read(addMMA8451_7bit ,buf1,1);

                std::bitset<8> STATUS;
                STATUS=buf1[0]; 
                std::cout << "STATUS:" << STATUS << "\n" ;

                // SYSTEM MODE READ
                cmd[0]=0x0B;    // system mode register
                i2c.write(addMMA8451_7bit ,cmd,1);
                i2c.read(addMMA8451_7bit ,buf1,1);

                BUFFER=buf1[0]; 
                std::cout << "MODE:" << BUFFER << "\n" ;


                //  X accel
                i2c.write(addMMA8451_7bit ,cmd_X_MSB,1,true);
                i2c.read(addMMA8451_7bit ,buf2,2);

                std::bitset<8> X_MSB;
                std::bitset<8> X_LSB;

                X_MSB |= buf2[0];
                X_LSB |= buf2[1];

                std::cout << "X_MSB:" << X_MSB << "; L_MSB:" << X_LSB << "\n" ;
                std::cout << "\n";




                // string modeSerial= "MODE= " + mode + " ";
                // serial.write(modeSerial.c_str(), modeSerial.length());

                /*
                // Read brightness
                Brightness = photoDiode.read_u16();
                if (Brightness<=BrightnessMAX*0.33)                                             {LightLevel=1;}
                else if (Brightness>=BrightnessMAX*0.33 && Brightness<=BrightnessMAX*0.66)      {LightLevel=2;}
                else if (Brightness>=BrightnessMAX*0.66)                                        {LightLevel=3;}

                // Read temperature
                char cmd[1];
                cmd[0]=0xE3;
                i2c.write(si7021addr8bit ,cmd,1);
                i2c.read(si7021addr8bit ,buf2,2);
            
                float tmp = (float((buf2[0] << 8) | buf2[1]));
                temp=((175.72*tmp)/65536)-46.85;

                // Display Mode, Temp and Brightness
                char buf[MAXIMUM_BUFFER_SIZE] = {0};

                // Temperature
                sprintf (buf, "T= %f ºC    ", temp);
                serial.write(buf, strlen(buf));

                // Brightness measurement
                sprintf (buf, "Brightness= %f %%", ((float)Brightness*100/BrightnessMAX));
                serial.write(buf, strlen(buf));
                
                serial.write("\n", 2);
                serialFlag=false;
                */

            
                ThisThread::sleep_for(5s);

    } 
}