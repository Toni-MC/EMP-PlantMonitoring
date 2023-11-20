#include "header.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

Thread threadMeasurements;


// -------------------------- CONFIG ----------------------
// ----- STATISTICS -----
int readingsMaxN=120;   // reading max
int readings;           // counter for readings

// Reading values once every 30 seconds
// 120 readings every 1 hour 


// ----- MEASUREMENTS LIMITS -----
// Temperature
float limitTemperatureMIN=-10;
float limitTemperatureMAX=50;

// Relative humidity
float limitHumidityMIN=25;
float limitHumidityMAX=75;

// Ambient Light / Brightness
float limitBrightnessMIN=0;
float limitBrightnessMAX=60;

// Soil Moisture
float limitMoistureMIN=0;
float limitMoistureMAX=60;

// Color sensor       C R G B
int limitColorMIN[4]={0,0,0,0}; 
int limitColorMAX[4]={10000,5000,10000,5000};

// Acceleration              X Y Z
#define g                       9.81 // Gravity

float limitAccelerationMIN[3]={-2*g,-2*g,-2*g};
float limitAccelerationMAX[3]={2*g,2*g,2*g};


// ----- PIN DEFINITIONS -----
// Ambient Light
AnalogIn sensorLight(A0,3.3);

// Soil Moisture
AnalogIn sensorMoisture(A2,3.3);

// RGB LED
DigitalOut ledRed(D11);
DigitalOut ledGreen(D12);
DigitalOut ledBlue(D13);

// I2C Interface for Accelerometer and Temp/Hum/Color sensors 
// D14 - I2C1_SDA
// D15 - I2C1_SCL
I2C i2c(D14,D15);
char bufferWrite[2], bufferRead[8];
float bufferFloat;

// Device and register addresses
// Temperature and Humidity
#define address_TempHum_si7021  0x40 << 1
#define REG_HUMIDITY_HOLD       0xE5
#define REG_TEMPERATURE_PREV    0xE3 


// Accelerometer
// A not connected (SAO=1), if A connected to GND address would be 0x1C  
#define address_Accel_MMA8451   0x1D << 1    
// Register addresses
#define REG_ACC_WHO_AM_I        0x0D
#define REG_ACC_CTRL_REG_1      0x2A
#define REG_ACC_X_MSB           0x01
#define REG_ACC_Y_MSB           0x03
#define REG_ACC_Z_MSB           0x05
#define REG_ACC_SYSMOD          0x0B




// Color sensor
#define address_Color_TCS34725  0x29 << 1
DigitalOut ledColorSensor(D9);


// Register addresses
// COMMAND CODES NEED TO HAVE THEIR MSB SET TO 1 on color sensor
#define REG_COLOR_ID        146     // 0x12 | 0x80
#define REG_COLOR_TIMER     129     // 0x01 | 0x80
#define REG_COLOR_CONTROL   143     // 
#define REG_COLOR_ENABLE    128     // 0x00 | 0x80
#define REG_COLOR_CLEAR     148     // 0x14 | 0x80
#define REG_COLOR_RED       150     // 0x16 | 0x80
#define REG_COLOR_GREEN     152     // 0x18 | 0x80
#define REG_COLOR_BLUE      154     // 0x1A | 0x80




// Serial Interface for GPS
// D8 - Serial1_TX
// D2 - Serial1_RX
static BufferedSerial serialGPS(D8, D2);
char bufferGPS[256]; 
// 256 bytes should be enough to get GPGGA


// Serial Interface with PC through USB
// D0(USBRX) and D1(USBTX) pins reserved for USB
static BufferedSerial serialPC(USBTX, USBRX);
char bufferSerial[128];
int distanceLocal;



// Tickers logic
volatile bool flagTEST,flagNORMAL;
Ticker tickerNORMAL, tickerTEST;
void tickTEST(){flagTEST=true;}
void tickNORMAL(){flagNORMAL=true;}


void configRGBSensor_TCS34725(){
    // Color sensor
    // Use the slowest mode, 700ms to have the highest accuracy since the application is not time dependant
    bufferWrite[0] = REG_COLOR_TIMER;
    bufferWrite[1] = 0;
    i2c.write(address_Color_TCS34725,bufferWrite,2);
    
    // Use the default gain
    bufferWrite[0] = REG_COLOR_CONTROL;
    bufferWrite[1] = 0;
    i2c.write(address_Color_TCS34725,bufferWrite,2);
    
    // ENABLE THE DEVICE AND RGBC 
    bufferWrite[0] = REG_COLOR_ENABLE;
    bufferWrite[1] = 3;                                     
    i2c.write(address_Color_TCS34725,bufferWrite,2);
}


void configAcc_MMA8451Q(){
    // Accelerometer
    bufferWrite[0]= REG_ACC_CTRL_REG_1;
    bufferWrite[1]= 0x01;
    i2c.write(address_Accel_MMA8451,bufferWrite,2);
}


// Aux function to calculate Mean and Max/Min values
void statistics(float *readings, float *statistics){
    
    statistics[0]=(float)accumulate(readings,readings+readingsMaxN,0.0)/(float)readingsMaxN;
    statistics[1]=*max_element(readings, readings + readingsMaxN);
    statistics[2]=*min_element(readings, readings + readingsMaxN);

}

char findMedianRGB(char *readings){
    // counter for color, R [0], G [1], B [2]
    int countRGB[3]={0,0,0};
    int maxCount;
    char returnDominantColor;

    for (int i=0; i < sizeof(readings)-1; i++){
        if (readings[i]=='R') {countRGB[0]=countRGB[0]+1;}
        if (readings[i]=='G') {countRGB[1]=countRGB[1]+1;}
        if (readings[i]=='B') {countRGB[2]=countRGB[2]+1;}
    }

    maxCount=*max_element(countRGB, countRGB+3);
    if (maxCount==countRGB[0]) {returnDominantColor='R';}
    if (maxCount==countRGB[1]) {returnDominantColor='G';}
    if (maxCount==countRGB[2]) {returnDominantColor='B';}

    return returnDominantColor;
}


// Functions to read sensors
float readTemp(){
    bufferWrite[0]=REG_TEMPERATURE_PREV;

    i2c.write(address_TempHum_si7021 ,bufferWrite,1);
    i2c.read(address_TempHum_si7021 ,bufferRead,2);

    bufferFloat= (float((bufferRead[0] << 8) | bufferRead[1]));
    return ((175.72*bufferFloat)/65536)-46.85;
}

float readHumidity(){
    bufferWrite[0]=REG_HUMIDITY_HOLD;

    i2c.write(address_TempHum_si7021 ,bufferWrite,1);
    i2c.read(address_TempHum_si7021 ,bufferRead,2);

    bufferFloat=(float((bufferRead[0] << 8) | bufferRead[1]));       
    return ((125*bufferFloat)/65536)-6;
}

void readAcc(float *accXYZ){
    // To convert from 2's complement
    int MAX_14BIT = 16383; // 0X3FFF
    int16_t BufferAcc;

    bufferWrite[0]=REG_ACC_X_MSB;
    // hold the slave because the accelerometer can drop the read instruction on its own
    i2c.write(address_Accel_MMA8451,bufferWrite,1,true);
    i2c.read(address_Accel_MMA8451,bufferRead,6);
    // 1st byte is the status
    // 2nd and 3rd, X
    // 4th and 5th, Y
    // 6th and 7th, Z
    // 14 bit, 2s complement numbers

    // X acc
    BufferAcc=(bufferRead[0] << 6) | (bufferRead[1] >> 2);
    if (BufferAcc > MAX_14BIT/2) {BufferAcc -= MAX_14BIT;} 
    accXYZ[0]=float(BufferAcc)/4096.0;

    // Y acc
    BufferAcc=(bufferRead[2] << 6) | bufferRead[3] >> 2;
    if (BufferAcc > MAX_14BIT/2) {BufferAcc -= MAX_14BIT;} 
    accXYZ[1]=float(BufferAcc)/4096.0;

    // Z acc
    BufferAcc=(bufferRead[4] << 6) | bufferRead[5] >> 2;
    if (BufferAcc > MAX_14BIT/2) {BufferAcc -= MAX_14BIT;} 
    accXYZ[2]=float(BufferAcc)/4096.0;   
    
}

int readRGBC(int *RGB){

    // Save RGB values on the int array
    bufferWrite[0]=REG_COLOR_RED;
    i2c.write(address_Color_TCS34725,bufferWrite,1);
    i2c.read(address_Color_TCS34725,bufferRead,2);
    RGB[0] = (bufferRead[1] << 8) | bufferRead[0];

    bufferWrite[0]=REG_COLOR_GREEN;
    i2c.write(address_Color_TCS34725,bufferWrite,1);
    i2c.read(address_Color_TCS34725,bufferRead,2);
    RGB[1] = (bufferRead[1] << 8) | bufferRead[0];

    bufferWrite[0]=REG_COLOR_BLUE;
    i2c.write(address_Color_TCS34725,bufferWrite,1);
    i2c.read(address_Color_TCS34725,bufferRead,2);
    RGB[2] = (bufferRead[1] << 8) | bufferRead[0];

    // return Clear value
    bufferWrite[0]=REG_COLOR_CLEAR;
    i2c.write(address_Color_TCS34725,bufferWrite,1);
    i2c.read(address_Color_TCS34725,bufferRead,2);

    ledColorSensor=0;
    return (bufferRead[1] << 8) | bufferRead[0];

}

bool readGPS(char *GPSTime, char *GPSLatitude, char *GPSLatIndicator, char *GPSLongitude, char *GPSLongIndicator, char*GPSAltitude){

    bool GPSFixed,flagGPGGAfound=false; // control flags for parsing
    char GPGGATag[]="GPGGA";
    char GPStag[]={}; 
    int gpggaIndex=0; // index to parse 


    // REAL DATA
    serialGPS.read(bufferGPS, sizeof(bufferGPS));

    // RAW GPS data
    // serialPC.write(bufferGPS, sizeof(bufferGPS));


    // TESTING DATA
    // char bufferGPS[]=
    //         "$GPGGA,064951.000,2307.1256,N,12016.4438,E,1,8,0.95,39.9,M,17.8,M,,*65" 
    //         "$GPGSA,A,3,29,21,26,15,18,09,06,10,,,,,2.32,0.95,2.11*00" 
    //         "$GPGSV,3,1,09,29,36,029,42,21,46,314,43,26,44,020,43,15,21,321,39*7D"
    //         "$GPGSV,3,2,09,18,26,314,40,09,57,170,44,06,20,229,37,10,26,084,37*77"
    //         "$GPGSV,3,3,09,07,,,26*73 "
    //         "$GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,165.48,260406,3.05,W,A*2C"
    //         "$GPVTG,165.48,T,,M,0.03,N,0.06,K,A*37";

    const char divideBy[]= ",$";
    char* token = strtok(bufferGPS, divideBy);

    while (token != NULL)
    {
        if (flagGPGGAfound==true){
            switch (gpggaIndex) {
            case 1: strcpy(GPSTime,token); break;
            case 2: if (*token=='0'){
                        GPSFixed=false;
                        gpggaIndex=100;
                        flagGPGGAfound=false;}
                    else {
                        strcpy(GPSLatitude,token);
                        GPSFixed=true;}               
                    break;

            case 3: strcpy(GPSLatIndicator,token);      break;
            case 4: strcpy(GPSLongitude,token);         break;
            case 5: strcpy(GPSLongIndicator,token);     break;
            case 9: strcpy(GPSAltitude,token);          break;
            }
            gpggaIndex++;
        }

        if (*token== 'G'){
            GPStag[0] = *token;     // G
            GPStag[1] = *(token+1); // P
            GPStag[2] = *(token+2); // G
            GPStag[3] = *(token+3); // G
            GPStag[4] = *(token+4); // A     
            
            // sprintf (bufferSerial, "GPStag: %s == %s \n", GPStag, GPGGATag);
            // serialPC.write(bufferSerial, strlen(bufferSerial));


            // if (strcmp(GPStag,GPGGATag)==0) not working??
            
            // check that the tag is GPGGA (char by char)
            if ((GPStag[0]==GPGGATag[0]) && (GPStag[1]==GPGGATag[1]) && (GPStag[2]==GPGGATag[2]) 
                && (GPStag[3]==GPGGATag[3]) &&(GPStag[4]==GPGGATag[4])){
                flagGPGGAfound=true;
                gpggaIndex=1;

                // sprintf (bufferSerial, "GPGGA FOUND \n");
                // serialPC.write(bufferSerial, strlen(bufferSerial));
            }
        }
        
        
        // sprintf (bufferSerial, "token: %s \n", token);
        //serialPC.write(bufferSerial, strlen(bufferSerial));

        token = strtok (NULL, divideBy);
    }
    flagGPGGAfound=false;
    return GPSFixed;
}

void ledLight(Colors color){
    switch (color)
    {
        case RED:       ledRed=0; ledGreen=1; ledBlue=1; break;    
        case GREEN:     ledRed=1; ledGreen=0; ledBlue=1; break;    
        case BLUE:      ledRed=1; ledGreen=1; ledBlue=0; break;    
        case YELLOW:    ledRed=0; ledGreen=0; ledBlue=1; break;    
        case CYAN:      ledRed=1; ledGreen=0; ledBlue=0; break;    
        case MAGENTA:   ledRed=0; ledGreen=1; ledBlue=0; break;    
        case WHITE:     ledRed=0; ledGreen=0; ledBlue=0; break;    
        case OFF:       ledRed=1; ledGreen=1; ledBlue=1; break;    
    }
}

void MeasurementsDisplay(void) {
    
    // Config serial communications
    serialPC.set_baud(9600);
    serialPC.set_format(8,serialPC.None,1);

    serialGPS.set_baud(9600);
    serialGPS.set_format(8,serialGPS.None,1);
   
    // Tickers of 2s and 30s for each mode
    tickerTEST.attach(&tickTEST,2s);
    tickerNORMAL.attach(&tickNORMAL,2s); 

    // Measurement variables
    // Light
    float brightness, brightnessMAX=65535;
    float readingsBrigthness[readingsMaxN];
    float statisticsBrigthness[3];

    // Soil Moisture
    float moisture, moistureMAX=65535;
    float readingsMoisture[readingsMaxN];
    float statisticsMoisture[3];
    
    // Temperature and relative humidity
    float temperature, humidity;
    float readingsTemperature[readingsMaxN],readingsHumidity[readingsMaxN];
    float statisticsTemperature[3],statisticsHumidity[3];

    // Color 
    int clear;
    // RED      [0]
    // GREEN    [1]
    // BLUE     [2]
    int RGB[3]; 
    int dominantColorValue;
    char dominantColor, readingsDominantColor[readingsMaxN],medianColor;

    
    // Accelerometer
    float bufferAccXYZ[3];
    float xAcc,yAcc,zAcc;
    float readingsXAcc[readingsMaxN],readingsYAcc[readingsMaxN],readingsZAcc[readingsMaxN];
    float statisticsXAcc[3],statisticsYAcc[3],statisticsZAcc[3];

    // GPS
    char GPSTime[12]={}, GPSLatitude[12]={}, GPSLatIndicator[2]={}, GPSLongitude[12]={}, GPSLongIndicator[12]={}, GPSAltitude[12]={};
    char bufferFormat[12];
    bool GPSFix;
    

    // Config Devices
    configAcc_MMA8451Q();

    configRGBSensor_TCS34725();

    ledLight(OFF);

    while(true) {
        if (mode==TEST) {readings=0;}

        // -------------------------- TIMED LOOP ----------------------
        // Only activates every 2 seconds when its TEST mode
        // Only activates every 30 seconds when its NORMAL/ADVANCED mode


        if((flagTEST & (mode==TEST))   ||    ((flagNORMAL) & ((mode==NORMAL) || (mode==ADVANCED)))){
            
            sprintf(bufferSerial,"----------------------------- \n");
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Clean timer flags
            flagTEST=false;
            flagNORMAL=false;

            // ---------------------- MEASUREMENTS ----------------------
            // SOIL MOISTURE
            moisture = (sensorMoisture.read_u16()*100)/moistureMAX;

            // LIGHT SENSOR 
            brightness = (sensorLight.read_u16()*100)/brightnessMAX;

            // GPS  
            
            GPSFix=readGPS(GPSTime, GPSLatitude, GPSLatIndicator, GPSLongitude, GPSLongIndicator, GPSAltitude);
            // GPSFix = false if there isnt a fix
            // GPSFix = true if there is a fix

            // format data
            if (GPSFix==true){
                // Latitude from DDMM.MMMM to DD.MMMMMM
                // Longitude from DDDMM.MMMM to DDD.MMMMMM

                strcpy(bufferFormat,GPSLatitude);
                GPSLatitude[2]='.';
                GPSLatitude[3]=bufferFormat[2];
                GPSLatitude[4]=bufferFormat[3];

                strcpy(bufferFormat,GPSLongitude);
                GPSLongitude[3]='.';
                GPSLongitude[4]=bufferFormat[3];
                GPSLongitude[5]=bufferFormat[4];
            }

            // Time from HHMMSS.SSS to HH:MM:SS
                strcpy(bufferFormat, GPSTime);
                GPSTime[2]=':';
                GPSTime[3]=bufferFormat[2];
                GPSTime[4]=bufferFormat[3];
                GPSTime[5]=':';
                GPSTime[6]=bufferFormat[4];
                GPSTime[7]=bufferFormat[5];
                GPSTime[8]=' ';
                GPSTime[9]=' ';
            

            // COLOR
            // Light the LED for 800ms since the config of the sensor is to take measures for 700ms
            ledColorSensor=1;
            ThisThread::sleep_for(800ms);

            clear=readRGBC(RGB);
            ledColorSensor=0;

            dominantColorValue=*max_element(RGB, RGB + 3);
            if (dominantColorValue==RGB[0]) {dominantColor='R'; if (mode==TEST) ledLight(RED);}
            if (dominantColorValue==RGB[1]) {dominantColor='G'; if (mode==TEST) ledLight(GREEN);}
            if (dominantColorValue==RGB[2]) {dominantColor='B'; if (mode==TEST) ledLight(BLUE);}



            // ACCELEROMETER
            readAcc(bufferAccXYZ);

            // read values in g, to get m/s^2, g=9.81
            xAcc=bufferAccXYZ[0]*g;
            yAcc=bufferAccXYZ[1]*g;
            zAcc=bufferAccXYZ[2]*g;


            // HUMIDITY
            humidity=readHumidity();
            
            // TEMPERATURE
            temperature=readTemp();
            


            // ---------------------- NORMAL/ADVANCED MODE ----------------------
            if (mode==NORMAL || mode==ADVANCED){
               
                // ------------------ STATISTICS ------------------
                // Save measurements for statistics every 1 hour
                // 120 readings for each sensor
                readingsBrigthness[readings]=brightness;
                readingsTemperature[readings]=temperature;
                readingsMoisture[readings]=moisture;
                readingsHumidity[readings]=humidity;
                
                readingsXAcc[readings]=xAcc;
                readingsYAcc[readings]=yAcc;
                readingsZAcc[readings]=zAcc;

                readingsDominantColor[readings]=dominantColor;

                readings++;

                if (readings==readingsMaxN){
                // When 120 readings are done: 

                    // Mean[0], MAX[1], MIN[2]
                    // TEMPERATURE
                    statistics(readingsTemperature, statisticsTemperature);
                    
                    // RELATIVE HUMIDITY
                    statistics(readingsHumidity, statisticsHumidity);

                    // AMBIENT LIGHT
                    statistics(readingsBrigthness, statisticsBrigthness);

                    // SOIL MOISTURE
                    statistics(readingsMoisture, statisticsMoisture);


                    // Median 
                    // RGB SENSOR
                    medianColor=findMedianRGB(readingsDominantColor);


                    // MAX, MIN of each axis
                    // ACCELEROMETER
                    statistics(readingsXAcc, statisticsXAcc);
                    statistics(readingsYAcc, statisticsYAcc);
                    statistics(readingsZAcc, statisticsZAcc);

                }
            }

            // ---------------------- SERIAL COMMUNICATION ----------------------

            if (mode==TEST){
            // TEST
            sprintf(bufferSerial, "TEST");
            serialPC.write(bufferSerial, strlen(bufferSerial));}
            
            else if (mode==NORMAL) {
            // N of readings
            sprintf(bufferSerial, "%d/%d",readings,readingsMaxN);
            serialPC.write(bufferSerial, strlen(bufferSerial));}
            
            else if (mode==ADVANCED) {
            sprintf(bufferSerial, "ADV");
            serialPC.write(bufferSerial, strlen(bufferSerial));}
            


            // TIME
            sprintf (bufferSerial, "\t %s \n", GPSTime);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Brigthness
            sprintf (bufferSerial, "BRIGHTNESS         %.1f %% \n", brightness);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Moisture
            sprintf (bufferSerial, "SOIL MOISTURE      %.1f %% \n", moisture);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Relative humidity
            sprintf (bufferSerial, "TEMPERATURE        %.1f ºC \n", temperature);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Temperature
            sprintf (bufferSerial, "RELATIVE HUMIDITY  %.1f %% \n", humidity);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Color sensor
            sprintf (bufferSerial, "COLOR \n\tCLEAR %d \n\tRED   %d \n\tGREEN %d \n\tBLUE  %d\n\tDOMINANT %c\n", clear, RGB[0], RGB[1], RGB[2], dominantColor);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Accelerometer
            sprintf (bufferSerial, "ACCELEROMETER \n\tX %.2f m/s^2 \n\tY %.2f m/s^2 \n\tZ %.2f m/s^2 \n", xAcc, yAcc, zAcc);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // GPS 
            if (GPSFix==false){
                sprintf (bufferSerial, "NO GPS FIX \nLast known location (if avaliable) \n");
                serialPC.write(bufferSerial, strlen(bufferSerial));
            }
            sprintf (bufferSerial, "GPS \n\tLATITUDE (UTC) %s %s \n\tLONGITUDE(UTC) %s %s \n\tALTITUDE       %s m\n", GPSLatitude,GPSLatIndicator,GPSLongitude,GPSLongIndicator,GPSAltitude);
            serialPC.write(bufferSerial, strlen(bufferSerial));

            
            // ---------------------------------
            // DISTANCE (ADVANCED MODE)
            if (mode==ADVANCED){
            distanceLocal=distanceCM;
            sprintf (bufferSerial, "DISTANCE         %d cm \n", distanceLocal);
            serialPC.write(bufferSerial, strlen(bufferSerial));
            }

            // GPS RAW 
            //serialPC.write(bufferGPS, sizeof(bufferGPS));

            // Show statistics if 120 readings are done
            if (readings==readingsMaxN){
                readings=0; // reset the readings to 0

                sprintf (bufferSerial, "\nSTATISTICS \tMEAN \tMAX \tMIN \n");
                serialPC.write(bufferSerial, strlen(bufferSerial));         

                // Brigthness
                sprintf (bufferSerial, "BRIGHTNESS \t%.1f \t%.1f \t%.1f \n",statisticsBrigthness[0],statisticsBrigthness[1],statisticsBrigthness[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));

                // Soil moisture
                sprintf (bufferSerial, "SOIL MOISTURE \t%.1f \t%.1f \t%.1f \n",statisticsMoisture[0],statisticsMoisture[1],statisticsMoisture[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));

                // Temperature
                sprintf (bufferSerial, "TEMPERATURE \t%.1f \t%.1f \t%.1f \n",statisticsTemperature[0],statisticsTemperature[1],statisticsTemperature[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));

                // Relative humidity
                sprintf (bufferSerial, "REL. HUMIDITY \t%.1f \t%.1f \t%.1f \n",statisticsHumidity[0],statisticsHumidity[1],statisticsHumidity[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));                

                // Accelerometer
                sprintf (bufferSerial, "ACCELEROMETER \n \tX \t%.2f \t%.2f \t%.2f \n",statisticsXAcc[0],statisticsXAcc[1], statisticsXAcc[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));        
                sprintf (bufferSerial, "\tY \t%.2f \t%.2f \t%.2f \n",statisticsYAcc[0],statisticsYAcc[1], statisticsYAcc[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));        
                sprintf (bufferSerial, "\tZ \t%.2f \t%.2f \t%.2f \n",statisticsZAcc[0],statisticsZAcc[1], statisticsZAcc[2]);
                serialPC.write(bufferSerial, strlen(bufferSerial));   

                // Color sensor
                sprintf (bufferSerial, "DOMINANT COLOR \t%c\n",medianColor);
                serialPC.write(bufferSerial, strlen(bufferSerial));

            }     
                    

            if (mode==NORMAL || mode==ADVANCED){
            
            
             // ------------------ ALARMS ----------------------
                // Check if every measurement is outside limits
                // If there is more than one alarm, with this code it will only show the first one in the following order:

                // ALARM COLORS and ORDER TO CHECK
                // Temperature      RED 
                // Humidity         CYAN 
                // Ambient light    WHITE 
                // Soil             GREEN 
                // Colour           MAGENTA
                // Accel            YELLOW

                // To show more than one alarm, we could add a flag for every alarm that activated and then light sequentially the alarm colors

            
                if ((temperature > limitTemperatureMAX) || (temperature < limitTemperatureMIN))     ledLight(RED);

                else if ((humidity > limitHumidityMAX) || (humidity < limitHumidityMIN))            ledLight(CYAN);

                else if ((brightness > limitBrightnessMAX) || (brightness < limitBrightnessMIN))    ledLight(WHITE);

                else if ((moisture > limitTemperatureMAX) || (moisture < limitTemperatureMIN))      ledLight(GREEN);

                // Color
                else if ((clear > limitColorMAX[0]) || (clear < limitColorMIN[0]))                  ledLight(MAGENTA);
                else if ((RGB[0] > limitColorMAX[1]) || (RGB[0] < limitColorMIN[1]))                ledLight(MAGENTA);
                else if ((RGB[1] > limitColorMAX[2]) || (RGB[1] < limitColorMIN[2]))                ledLight(MAGENTA);
                else if ((RGB[2] > limitColorMAX[3]) || (RGB[2] < limitColorMIN[3]))                ledLight(MAGENTA);

                // Accel
                else if ((xAcc > limitAccelerationMAX[0]) || (xAcc < limitAccelerationMIN[0]))      ledLight(YELLOW);
                else if ((yAcc > limitAccelerationMAX[1]) || (yAcc < limitAccelerationMIN[1]))      ledLight(YELLOW);
                else if ((zAcc > limitAccelerationMAX[2]) || (zAcc < limitAccelerationMIN[2]))      ledLight(YELLOW);
                
                else ledLight(OFF);

            }
        }
    } 
}