# sensingtheenvpm25

Note for the web site:  the purpose of "sqlconnect-example.php" is to maintain privacy of the database access credentials; it should be copied to the web server, renamed to "sqlconnect.php", and then edited to contain the actual username etc.  The resulting sqlconnect.php should NOT then be included in the github source.


to do: docs, e.g. process for wiring, materials list (e.g. acrylic), process for editing the sensor code (setting the machine name), goals for installation, process for getting from the Fusion360 file to an assembled shelter, process for uploading data to the web site, calibration, setting up the web server (database, password file), next steps list, and so on.


Process for cutting the shelter from the 20180521 drawing:
1. Requires (from Home Depot, for instance) 3 sheets of 11x14 .093" thick acrylic per shelter.  (aside: The shelter design has slot widths designed for that thickness; you could change the "t" (thickness) parameter of the .f3d drawing and recompute and it SHOULD work, but no guarantees -- I've had problems adjusting .f3d parameters in the past.)
2. From the .f3d file, make a drawing (select visible components, D size sheet l:1 scale) of the parts, deleting the legend and border, precise layout not critical.
3.  Export the drawing into a PDF, since that's what Adobe Illustrator can read
4.  In Illustrator, change the line weight to .001pt and the line colors to red; also, right-click on the unwanted rectangular border, choose "release clipping mask", and then delete the border.
5.  In Illustrator, make 3 new drawings, 11x14, and distribute copies of the (eight) shelter components among the 3 drawings.  I ended up needing to make two changes to settings in the print dialog:  placement in the upper left corner and no auto-rotate.
6.  From Illustrator, do the laser cutting (print to the Trotec, and follow the normal instructions; I used 90% power and 0.70 speed with Auto PPI).


Assembling the shelter:
First, attach the Arduino and the Plantower to the centerboard with twist-ties through the holes.  Then slot together the outside and base of the shelter and drop in the centerboard with the sensor.  Put the lid in place and use two more twist-ties through the small top holes to hold the lid down, which is the step which latches everything together firmly.  Finally, slide the door into place.


Process for loading code onto the sensor:
Upload the code using the Arduino IDE.  One edit will be necessary for each sensor:  set the SENSOR_NAME in the code around line 40 (of code version 15, which is current as of July 2018). The name is included in the sensor output file and used for tagging the data in the database.  The name is also used in generating the file name for the sensor's data files (a new datafile is created every time the sensor is started:  a sensor named PARK will have data files named PARK_001.csv, PARK_002.csv, etc).  The second edit to remember is that any time the sensor code is modified, the SOFTWARE_VERSION should be changed; this will help separate out any data handling changes which need to be made in the server code and may be required for correct analysis of the data.
(need note here about setting up Arduino IDE, installing the Teensy loader, setting the "Port" to connect, using the console to check that the sensor is running properly, etc.)


Process for uploading data to the database, as of 7/2018:
1. Unplug the sensor from its power source.
2. Remove the micro-SD card from the Arduino.  On the micro-SD card will be files with names like PARK_001.csv, one for each time the sensor has been powered up.  You'll need to upload (one by one) any of these which haven't already been uploaded; you may want to copy them to your local computer for convenience.
3. Navigate a web browser to http://spatialreasoning.com/particulates and choose "Upload".
4.  On the upload page, first "browse" to find the .csv file from the sensor's micro-SD card, then "Upload PM2.5 Data" to actually do the upload.  This will take at least several seconds, with no immediate visual feedback; please be patient.
5.  When the upload finishes, the web page will display a graph of the readings, and will be ready to upload another file if you have multiple to upload.
6.  You likely will want to remove the .csv files from the micro-SD card to avoid future confusion about whether the files have been uploaded already.  If you do so, the next time the sensor is started up it will restart the data file numbering from 001.
7.  Replace the micro-SD card in the sensor.
8.  Plug the sensor in again.
