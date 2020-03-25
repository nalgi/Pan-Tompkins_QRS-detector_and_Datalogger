# Pan-Tompkins_QRS-detector_and_Datalogger
Arduino based AD8232 Pan-Tompkins QRS-detector and Datalogger


/* © Script written by Tim Möller
 * Humboldt Universität zu Berlin
 * Berlin School of Mind and Brain Berlin
 * 
 * This script records data from the AD8232 shield and plots the data.
 * Preprocessing and detect of the QRS compley is realized through the Pan-Tompkins algorithm
 * (Pan, J., & Tompkins, W. J. (1985). A real-time QRS detection algorithm. IEEE Trans. Biomed. 
 * Eng, 32(3), 230-236.).
 *  * This script also saves the Date, the Timestamp, a Timer that counts the passed time and a 
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
 * Last modified: 25.03.2020
 */
