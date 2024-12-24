**PiTrac Start-Up Documentation**

If you are at this point, you should have PiTrac compiled on both Pi’s, your enclosure built and your cameras calibrated.  If that’s not quite done yet, see the [START HERE](https://github.com/jamespilgrim/PiTrac/blob/main/Documentation/PiTrac%20%E2%80%93%20START%20HERE.md) document to determine where to go first.

**Start-up and Sanity Checks:**

1. Ensure that each account that you will run PiTrac in has the PITRAC\_ROOT set to the directory above the “ImageProcessing” directory where the meson.build file exists.  
   1. Typically, export PITRAC\_ROOT=/Dev/PiTrac/Software  
2. Ensure the golf\_sim\_config.json file is correctly set up.  If not sure, follow the Configuration File documentation.  
3. Check that the executable at least runs by itself:  
   1. cd $PITRAC\_ROOT/ImageProcessing  
   2. build/pitrac\_lm \--help  
   3. (the executable should show the command-line parameters)

**Check the Strobe Light and Camera Triggering:**

1. Problems can sometimes exist in the pathway from the Pi 1 to the Pi 2 Camera and the Strobe Assembly (though the Connector Board).  For example, sometimes the ribbon cable may be loose or mis-connected from the Pi 1 to the Connector Board.  So a few initial checks are good to perform before going further…  
2. Position the PiTrac so that you can see into the IR LEDs through the LED lens (the small array of square LEDs should be visible)  
3. Run a strobe-light test  
   1. cd $PITRAC\_ROOT/ImageProcessing  
   2. ./RunScripts/runPulseTest.sh  
   3. The above script will periodically send a series of short “on” pulses to the LED strobe light.  Due to the IR wavelengths used by the LED (at least the one on the parts list), you should be able to see very short groups of dark-reddish pulses in the LED lens.  
      1. **NOTE** \- Just in case, look at the LED from at least a couple feet away, especially if you are using a higher-power LED.  
4. If you can’t see the pulsing, double-check the output of the runPulseTest script as well as the connections from the Pi 1 near the top of the LM to the Connector Board.  
   1. If you can’t see the red pulses, check first to see if the little red LED on the Connector Board is pulsing.  If it is, but there is no strobe-light pulses, it’s likely one or more of the following problems:  
      1. The power-supply to the Connector Board is not connected correctly  
      2. The wiring from the output of the Connector Board to the LED strobe assembly not connected correctly  
5. Camera 2 Shutter Triggering  
   1. When the system is running normally, the shutter for Camera 2 is triggered by a pulse from the Pi 1\.  The correct functioning of this signal pathway should be confirmed before starting the system in full.  
   2. To check the triggering, we will set the Pi 2 camera in an “external” triggering mode, where its shutter is controlled by the XTR signal that is sent to the camera from Pi 1 through the connector board.  
   3. On the Pi 2, cd $PITRAC\_ROOT/ImageProcessing  
   4. ./CameraTools/setCameraTriggerExternal.sh  
   5. .//RunScripts/runCam2Still.sh  
   6. Normally, the camera would not take a picture because it is waiting for a signal from the Pi 1\.  
   7. On the Pi 1, do:  
      1. /RunScripts/runPulseTest.sh  
   8. As soon as the Pi 1 script starts sending pulses to the Camera 2 (as well as pulses to the LED strobe array), the Camera 2 program that is running should take a picture.  
   9. If having problems, you can also just run the system’s rpicam-hello program, which should hang and not do anything while external triggering is set, but then should take pictures as soon as the runPulseTest.sh script is run  
   10. Finally, return the triggering to internal on the Pi 2:  
       1. $PITRAC\_ROOT/CameraTools/setCameraTriggerInternal.sh  
   11. 

  