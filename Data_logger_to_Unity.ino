/* Script written by Tim J. Möller
 * Humboldt Universität zu Berlin
 * Berlin School of Mind and Brain, Berlin, Germany
 * Department of Psychiatry and Psychotherapy, Charité University Medicine, Berlin, Germany
 
 * Contact: tim.julian.moeller@gmail.com
 * 
 * This script records data from the AD8232 shield and plots the data.
 * Preprocessing and detect of the QRS compley is realized through the Pan-Tompkins algorithm
 * (Pan, J., & Tompkins, W. J. (1985). A real-time QRS detection algorithm. IEEE Trans. Biomed. 
 * Eng, 32(3), 230-236.).
 *  This script also saves the Date, the Timestamp, a Timer that counts the passed time and a 
 *  counter for the number of recorded data-points.
 *  
 * Some of the code contains element from available, open access, existing scripts:
 * https://github.com/adafruit/Adafruit_SSD1306
 * https://github.com/blakeMilner/real_time_QRS_detection
 * https://github.com/dxinteractive/ResponsiveAnalogRead
 * SD card read/write (Arduino example sketches) created Nov 2010 by David A. Mellis modified 9 Apr 2012 by Tom Igoe
 * This example code is in the public domain.
 * SD Card test created  28 Mar 2011 by Limor Fried modified 9 Apr 2012 by Tom Igoe
 * This example code is in the public domain.
 * 
 * Last modified: 31.12.2021
 */

  ///////////////// START OF DECLARATION OF IMPORTANT VARIABLES///////////////////////
/*  Here you can find the variables that might need adjustment to you own purpose. A short description what every
 *  variable is doing can be found as a comment after the variable declaration. */
 
const String Filename = "Test_01.txt"; /* Give the Outputfile a name how it will be displayed on the SD Card. Be aware that the
// filename may only consist of a maximum of 8 alphanumerics and has to end with .exe or .csv (e.g. Test_01.txt). If
a file with the same name already exists, this file will be extended. */

bool Plotting = true; /* If you want to plot the data in the Arduino Serial Plotter, set this variable to true. The QRS value
// is then changed to 400 instead of 1, to make the peak more visible. If you use the serial monitor or you want to save the data to
// the SD card, set 'Plotting' to false. */

bool Saving_Command = true; /* If you do not have an SD Card module or USB Card, you cannot save the data. Also, the Sketch
would not compile. In that case, change the Saving_Command to 'false'.*/

bool play_sound = true; /* You can define if you want to activate (true) or deactivate (false) your soundbuzzer. */

bool Experiment_settings = false;

/* If you want to use the sound buzzer, these variables might be relevant for your setup. The Tweakfactor by default is 1.0. 
// If you want to play tones faster or lower compared to the last QRS interval, lower or increase the Tweakfactor. Every 0.1 
// accounts for an increase or decrease of 10% time compared to the original heartbeat.*/
float Tweakfactor = 1.0; // That is the original heartbeat (default)
//float Tweakfactor = 0.7; // Used for playing the tones 30% faster than the actual heartbeat
//float Tweakfactor = 1.3; // Used for playing the tones 30% slower than the actual heartbeat
int tone_freq = 1000; // The frequency of the sound
int tonelength = 20; // The duration of the sound in millisconds

/* Here all the pins are defined*/ 
const int tone_pin =2;
const int heartPin = A1; 
const int SD_chipSelect = 53;

/* Here, the most important parameters for the Pan-Tompkins algorithm can be changed*/
#define M       5 // Here you can change the size for the Highpass filter
#define N       30 // Here you can change the size for the Lowpass filter
#define winSize     250   // this value defines the windowsize which effects the sensitivity of the QRS-detection. 
/* May need adjustments depending on your sample size. If you use a lower sampling rate, a lower windowSize might yield  better results. */
//////////////// END OF DECLARATION OF IMPORTANT VARIABLES///////////////////////

///////////////// Additional libraries and definitions that can be changed if you know what you are doing//////////////////// 
#include <ResponsiveAnalogRead.h> // If you get a compiler error here, you do not have this library. Go to "Tools" --> "Manage libraries" and copy this name in the field on the top. Simply install the library, then you have this library.
#include <SPI.h>// If you get a compiler error here, you do not have this library. Go to "Tools" --> "Manage libraries" and copy this name in the field on the top. Simply install the library, then you have this library.
#include <SD.h>// If you get a compiler error here, you do not have this library. Go to "Tools" --> "Manage libraries" and copy this name in the field on the top. Simply install the library, then you have this library.
#include <Adafruit_GFX.h>// If you get a compiler error here, you do not have this library. Go to "Tools" --> "Manage libraries" and copy this name in the field on the top. Simply install the library, then you have this library.
#define HP_CONSTANT   ((float) 1 / (float) M)
#define MAX_BPM     100 // Set an upper threshold of BPM for a better detection rate
#define RAND_RES 100000000

ResponsiveAnalogRead analog(heartPin, true);
File myFile; 
int saving_interval = 0; //create a variable that counts samples. When an aspired number of samples is gathered, the Arduino will save the data to the .txt file to save computational power
int saving_treshold = 300; //if this treshold is reached, the Arduino saves the gathered data to the output file
float difference = 0;
float interval;

/* Important for calculating the between QRS interval.*/
int cprTimeRead_1 = 0; 
int cprTimeRead_2 = 0;
int timeCPR = 0;
float CPRSUM;

/* Other important variables*/
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long i = 0;
int x=0;
int j=0;
int lastj=0;
int lasty=0;
int LastTime=0;
int ThisTime;
int next_ecg_pt;
int QRS = 0;
int tmp = 0;

/* pre-recorded ECG*/
int s_ecg_idx = 0;
const int S_ECG_SIZE = 226;
const float s_ecg[S_ECG_SIZE] = {1.5984,1.5992,1.5974,1.5996,1.5978,1.5985,1.5992,1.5973,1.5998,1.5976,1.5986,1.5992,1.5972,1.6,1.5973,1.5989,1.5991,1.5969,1.6006,1.5964,1.6,1.5979,1.5994,1.6617,1.7483,1.823,1.8942,1.9581,2.0167,2.0637,2.1034,2.1309,2.1481,2.1679,2.1739,2.1702,2.1543,2.1243,2.0889,2.037,1.982,1.9118,1.8305,1.7532,1.6585,1.6013,1.5979,1.5981,1.5996,1.5972,1.5992,1.599,1.5966,1.6015,1.5952,1.6008,1.5984,1.5953,1.606,1.5841,1.6796,1.9584,2.2559,2.5424,2.835,3.1262,3.4167,3.7061,4.0018,4.2846,4.5852,4.8688,5.1586,5.4686,5.4698,5.1579,4.8687,4.586,4.2833,4.0031,3.7055,3.4164,3.1274,2.8333,2.544,2.2554,1.9572,1.6836,1.5617,1.5143,1.4221,1.3538,1.2791,1.1951,1.1326,1.0407,0.99412,1.0445,1.1262,1.2017,1.2744,1.3545,1.4265,1.5044,1.5787,1.6006,1.5979,1.5988,1.5982,1.5989,1.5982,1.5986,1.5987,1.5983,1.5984,1.5992,1.5965,1.6082,1.6726,1.7553,1.826,1.903,1.9731,2.0407,2.1079,2.166,2.2251,2.2754,2.3215,2.3637,2.396,2.4268,2.4473,2.4627,2.4725,2.4721,2.4692,2.4557,2.4374,2.4131,2.3797,2.3441,2.2988,2.2506,2.1969,2.1365,2.0757,2.0068,1.9384,1.8652,1.7899,1.7157,1.6346,1.5962,1.5997,1.5979,1.5986,1.5989,1.5978,1.5995,1.5976,1.5991,1.5984,1.5981,1.5993,1.5976,1.5993,1.5982,1.5982,1.5993,1.5975,1.5994,1.5981,1.5983,1.5995,1.5967,1.6049,1.6248,1.647,1.664,1.6763,1.6851,1.6851,1.6816,1.6712,1.655,1.6376,1.613,1.599,1.5985,1.5982,1.5989,1.5982,1.5986,1.5987,1.598,1.5991,1.598,1.5987,1.5987,1.598,1.5992,1.5979,1.5988,1.5986,1.598,1.5992,1.5979,1.5988,1.5986,1.598,1.5992,1.5978,1.5989,1.5985,1.5981,1.5992,1.5978,1.599,1.5985,1.5981,1.5992,1.5977,1.599,1.5984,1.5981};

/* timing variables*/
unsigned long previousMicros  = 0;        // will store last time LED was updated
unsigned long foundTimeMicros = 0;        // time at which last QRS was found
unsigned long old_foundTimeMicros = 0;        // time at which QRS before last was found
unsigned long currentMicros   = 0;        // current time
unsigned long RR_peak = 0; 

/* interval at which to take samples and iterate algorithm (microseconds)*/
const long PERIOD = 1000000 / winSize;

#define BPM_BUFFER_SIZE 5
unsigned long bpm_buff[BPM_BUFFER_SIZE] = {0};
int bpm_buff_WR_idx = 0;
int bpm_buff_RD_idx = 0;

/* set up variables to use the SD utility library functions:*/
Sd2Card card;
SdVolume volume;
SdFile root;


void setup() {
    Serial.begin(115200);

    // SD-Card module Setup and error display
      if (Saving_Command == true) {
pinMode(SD_chipSelect, OUTPUT);
    #ifdef SERIAL_USB
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
  #endif

    Serial.print("Initializing SD card...");
  if (!card.init(SPI_HALF_SPEED, SD_chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
  if (!SD.begin(SD_chipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
myFile = SD.open(Filename, FILE_WRITE);
if (myFile)
      {
        Serial.println("File created successfully.");
        return 1;
      } else
      {
        Serial.println("Error while creating file.");
        return 0;
      }
      
/* Define the labels for your data transfer*/
  myFile.println("Counter,Raw_data,QRS_detected");
  myFile.close();
  myFile = SD.open(Filename, FILE_WRITE);
}
}
void loop() {    
delay(2);
// Update the time, reset the QRS detection and read the next value
  currentMicros = micros();
    previousMicros = currentMicros;
    boolean QRS_detected = false;
      int next_ecg_pt = analogRead(heartPin);
      
////////////////////// WRITE DOWN ALL THE VARIABLES THAT SHOULD BE DISPLAYED HERE ////////////////////////////////////// 
if (Experiment_settings == false){
//Serial.print(i++); // Sample number
//Serial.print(","); 
//Serial.print(millis()); // Sample number
//Serial.print(",");
Serial.print(next_ecg_pt);
Serial.print(","); 
Serial.println(QRS);
//Serial.print(","); 
//Serial.println(RR_peak);
}

/*//////////////////DEFINE WHICH VARIABLES YOU WANT TO TRANSFER TO THE SD CARD EVERY ITERATION. IF YOU WANT TO ADD MORE
// VARIABLES, ADD myFile.PRINT('your datatype'); FOLLOWED BY myFile.print(","); //////////////////////////////////////*/

if (Saving_Command == true) {
if(myFile){
myFile.print(i++); // Sample number
myFile.print(",");
myFile.print(millis()); // Sample number
myFile.print(",");
myFile.print(next_ecg_pt); // Raw ECG data
myFile.print(",");
myFile.print(QRS); //Decision if QRS complex was detected (binary)
myFile.print(","); 
myFile.println(RR_peak);
saving_interval++; // Add a token to the saving interval

/* For performance reasons, the data will be only saved every 300 samples (around every 0.75 seconds). Can be changed by changing the saving threshold
saving_interval++; // Add a token to the saving interval*/
if (saving_interval > saving_treshold){
 saving_interval = 0;
  myFile.flush();
  myFile.close();
myFile = SD.open(Filename, FILE_WRITE);
}
}
}
/* If a QRS is found here, it can be send to an interface e.g. Unity 3D (sendData("QRS")) */

      // give next data point to algorithm
      QRS_detected = detect(next_ecg_pt);
        if(QRS_detected == false) {
        QRS = 0;
      }
      else if(QRS_detected == true){
        foundTimeMicros = micros();
        RR_peak = millis();
        if (Experiment_settings == true){
        Serial.println(QRS);}
        if (Plotting == true) {
        QRS = 400; }
        else if (Plotting == false) {
          QRS = 1; }
        //Serial.println(QRS);
        //delay(10);
        
        sendData("QRS");
        cprTimeRead_2 = cprTimeRead_1;
        cprTimeRead_1 = millis();
        CPRSUM = cprTimeRead_1-cprTimeRead_2;
        interval = CPRSUM*Tweakfactor;
        }

/* Here, the tone will be played according to its Tweakfactor*/
     if((difference > interval) && (play_sound == true)){  
     tone(tone_pin, tone_freq, tonelength); 
     previousMillis = millis();   
  }
  
difference = millis() - previousMillis;
}


/* This section contains the Pan-Tompkins algorithm and is adapted from https://github.com/blakeMilner/real_time_QRS_detection
 Portion pertaining to Pan-Tompkins QRS detection */
// circular buffer for input ecg signal
// we need to keep a history of M + 1 samples for HP filter
float ecg_buff[M + 1] = {0};
int ecg_buff_WR_idx = 0;
int ecg_buff_RD_idx = 0;

// circular buffer for input ecg signal
// we need to keep a history of N+1 samples for LP filter
float hp_buff[N + 1] = {0};
int hp_buff_WR_idx = 0;
int hp_buff_RD_idx = 0;

// LP filter outputs a single point for every input point
// This goes straight to adaptive filtering for eval
float next_eval_pt = 0;

// running sums for HP and LP filters, values shifted in FILO
float hp_sum = 0;
float lp_sum = 0;

// working variables for adaptive thresholding
float treshold = 0;
boolean triggered = false;
int trig_time = 0;
float win_max = 0;
int win_idx = 0;

// number of starting iterations, used determine when moving windows are filled
int number_iter = 0;

boolean detect(float new_ecg_pt) {
        // copy new point into circular buffer, increment index
  ecg_buff[ecg_buff_WR_idx++] = new_ecg_pt;  
  ecg_buff_WR_idx %= (M+1);
 
 
  /* High pass filtering */
  if(number_iter < M){
    // first fill buffer with enough points for HP filter
    hp_sum += ecg_buff[ecg_buff_RD_idx];
    hp_buff[hp_buff_WR_idx] = 0;
  }
  else{
    hp_sum += ecg_buff[ecg_buff_RD_idx];
    
    tmp = ecg_buff_RD_idx - M;
    if(tmp < 0) tmp += M + 1;
    
    hp_sum -= ecg_buff[tmp];
    
    float y1 = 0;
    float y2 = 0;
    
    tmp = (ecg_buff_RD_idx - ((M+1)/2));
    if(tmp < 0) tmp += M + 1;
    
    y2 = ecg_buff[tmp];
    
    y1 = HP_CONSTANT * hp_sum;
    
    hp_buff[hp_buff_WR_idx] = y2 - y1;
  }
  
  // done reading ECG buffer, increment position
  ecg_buff_RD_idx++;
  ecg_buff_RD_idx %= (M+1);
  
  // done writing to HP buffer, increment position
  hp_buff_WR_idx++;
  hp_buff_WR_idx %= (N+1);
  

  /* Low pass filtering */
  
  // shift in new sample from high pass filter
  lp_sum += hp_buff[hp_buff_RD_idx] * hp_buff[hp_buff_RD_idx];
  
  if(number_iter < N){
    // first fill buffer with enough points for LP filter
    next_eval_pt = 0;
    
  }
  else{
    // shift out oldest data point
    tmp = hp_buff_RD_idx - N;
    if(tmp < 0) tmp += (N+1);
    
    lp_sum -= hp_buff[tmp] * hp_buff[tmp];
    
    next_eval_pt = lp_sum;
  }
  
  // done reading HP buffer, increment position
  hp_buff_RD_idx++;
  hp_buff_RD_idx %= (N+1);
  

  /* Adapative thresholding beat detection */
  // set initial threshold        
  if(number_iter < winSize) {
    if(next_eval_pt > treshold) {
      treshold = next_eval_pt;
    }

                number_iter++;
  }
  
  // check if detection hold off period has passed
  if(triggered == true){
    trig_time++;
    
    if(trig_time >= 100){
      triggered = false;
      trig_time = 0;
    }
  }
  
  // find if we have a new max
  if(next_eval_pt > win_max) win_max = next_eval_pt;
  
  // find if we are above adaptive threshold
  if(next_eval_pt > treshold && !triggered) {
    triggered = true;
    return true;
  }
        // else we'll finish the function before returning FALSE,
        // to potentially change threshold
          
  // adjust adaptive threshold using max of signal found 
  // in previous window            
  if(win_idx++ >= winSize){
    // weighting factor for determining the contribution of
    // the current peak value to the threshold adjustment
    float gamma = 0.4;
    
    // forgetting factor - 
    // rate at which we forget old observations
                // choose a random value between 0.01 and 0.1 for this, 
    float alpha = 0.1 + ( ((float) random(0, RAND_RES) / (float) (RAND_RES)) * ((0.1 - 0.01)));
    
                // compute new threshold
    treshold = alpha * gamma * win_max + (1 - alpha) * treshold;
    
    // reset current window index
    win_idx = 0;
    win_max = -10000000;
  }
      
        // return false if we didn't detect a new QRS
  return false;
    
    
}

// When you want to send the data to an interface, it can be done here, but it is possible through the normal Serial.print command, too. 
void sendData(String data){

 #ifdef SERIAL_USB
   Serial.println(QRS); // need a end-line because wrmlh.csharp use readLine method to receive data
   //serialPort.ReadTimeout = 1;
   //delay(1); // Choose your delay having in mind your ReadTimeout in Unity3D
#endif
}
