#include "header.h"

Thread threadMeasurements;


// -------------------------- CONFIG ----------------------
// ----- STATISTICS -----
int readingsMaxN=120;   // reading max
int readings;           // counter for readings



// Reading values once every 30 seconds
// 120 readings every 1 hour 


// ----- MEASUREMENTS LIMITS -----
// Temperature
int limitTemperatureMIN=-10;
int limitTemperatureMAX=-50;

// Relative humidity
int limitHumidityMIN=25;
int limitHumidityMAX=75;

// Ambient Light
int limitLightMIN=0;
int limitLightMAX=100;

// Soil Moisture
int limitMoistureMIN=0;
int limitMoistureMAX=100;

// Color sensor       C R G B
int limitColorMIN[4]={0,0,0,0}; 
int limitColorMAX[4]={0,0,0,0};

// Acceleration              X Y Z
int limitAccelerationMIN[3]={0,0,0}; 
int limitAccelerationMAX[3]={0,0,0};


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
char bufferWrite[2], bufferRead[5];
float bufferFloat;

// Device and register addresses
// Temperature and Humidity
#define address_TempHum_si7021  0x40 << 1
#define REG_HUMIDITY_HOLD       0xE5
#define REG_TEMPERATURE_PREV    0xE3

// Accelerometer
// A not connected (SAO=1), if A connected to GND address would be 0x1C  
#define address_Accel_MMA8451 0x1D << 1    

// Color sensor
#define address_Color_TCS34725  0x29 << 1




// Serial Interface for GPS
// D8 - Serial1_TX
// D2 - Serial1_RX
static BufferedSerial serialGPS(D8, D2);

// Serial Interface with PC through USB
// D0(USBRX) and D1(USBTX) pins reserved for USB
static BufferedSerial serialPC(USBTX, USBRX);
char bufferSerial[128];
   



// Tickers logic
volatile bool flagTEST,flagNORMAL;
Ticker tickerNORMAL, tickerADVANCED;
void tickTEST(){flagTEST=true;}
void tickNORMAL(){flagNORMAL=true;}


// Aux function to calculate Mean and Max/Min values
void statistics(float *readings, float *statistics){
    
    statistics[0]=(float)accumulate(readings,readings+readingsMaxN,0)/readingsMaxN;
    statistics[1]=*max_element(readings, readings + readingsMaxN);
    statistics[2]=*min_element(readings, readings + readingsMaxN);

}

void MeasurementsDisplay(void) {
    
    // Config serial communications
    serialPC.set_baud(9600);
    serialPC.set_format(8,serialPC.None,1);

    serialGPS.set_baud(9600);
    serialGPS.set_format(8,serialGPS.None,1);
   
    // Tickers of 2s and 30s for each mode
    tickerNORMAL.attach(&tickTEST,2s);
    tickerADVANCED.attach(&tickNORMAL,30s); 

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

    // Accelerometer

    // GPS
    char GPSbuffer[256];

    ledRed=0;
    ledGreen=1;
    ledBlue=0;
    
    while(true) {
        if (mode==TEST) {readings=0;}

        // -------------------------- TIMED LOOP ----------------------
        // Only activates every 2 seconds when its TEST mode
        // Only activates every 30 seconds when its NORMAL mode


        if((flagTEST & (mode==TEST))   ||    ((flagNORMAL) & (mode==NORMAL))){
            
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
            serialGPS.read(GPSbuffer, strlen(GPSbuffer));


            // COLOR


            // ACCELEROMETER


            // HUMIDITY
            bufferWrite[0]=REG_HUMIDITY_HOLD;

            i2c.write(address_TempHum_si7021 ,bufferWrite,1);
            i2c.read(address_TempHum_si7021 ,bufferRead,2);

            bufferFloat=(float((bufferRead[0] << 8) | bufferRead[1]));       
            humidity=((125*bufferFloat)/65536)-6;

            // TEMPERATURE
            bufferWrite[0]=REG_TEMPERATURE_PREV;

            i2c.write(address_TempHum_si7021 ,bufferWrite,1);
            i2c.read(address_TempHum_si7021 ,bufferRead,2);

            float tmp = (float((bufferRead[0] << 8) | bufferRead[1]));
            temperature=((175.72*tmp)/65536)-46.85;



            // ---------------------- NORMAL MODE ----------------------
            if (mode==NORMAL){
                // ------------------ ALARMS ----------------------
                // Check if every measurement is outside limits




                // ------------------ STATISTICS ------------------
                // Save measurements for statistics every 1 hour
                // 120 readings for each sensor
                readingsBrigthness[readings]=brightness;
                readingsTemperature[readings]=temperature;
                readingsMoisture[readings]=moisture;
                readingsHumidity[readings]=humidity;

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


                    // MAX, MIN of each axis
                    // ACCELEROMETER

                }
                //

                // 
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
            sprintf (bufferSerial, "\tHH:MM:SS \n");
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
            sprintf (bufferSerial, "COLOR \n\tCLEAR X \n\tRED   X \n\tGREEN X \n\tBLUE  X\n");
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // Accelerometer
            sprintf (bufferSerial, "ACCELEROMETER \n\tX X m/s^2 \n\tY X m/s^2 \n\tZ X m/s^2 \n");
            serialPC.write(bufferSerial, strlen(bufferSerial));

            // GPS 
            sprintf (bufferSerial, "GPS \n\tLATITUDE (UTC) X N/S \n\tLONGITUDE(UTC) X E/W \n\tALTITUDE       X m\n");
            serialPC.write(bufferSerial, strlen(bufferSerial));


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
                sprintf (bufferSerial, "ACCELEROMETER \n \tX \t%.1f \t%.1f \t%.1f \n", statisticsHumidity[0],statisticsHumidity[1]);
                serialPC.write(bufferSerial, strlen(bufferSerial));        
                sprintf (bufferSerial, "\tY \t%.1f \t%.1f \t%.1f \n", statisticsHumidity[0],statisticsHumidity[1]);
                serialPC.write(bufferSerial, strlen(bufferSerial));        
                sprintf (bufferSerial, "\tZ \t%.1f \t%.1f \t%.1f \n", statisticsHumidity[0],statisticsHumidity[1]);
                serialPC.write(bufferSerial, strlen(bufferSerial));        
                // Color sensor
                sprintf (bufferSerial, "DOMINANT COLOR \tX\n");
                serialPC.write(bufferSerial, strlen(bufferSerial));

            }     
                    

        }
    } 
}