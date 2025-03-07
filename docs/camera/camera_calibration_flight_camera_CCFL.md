---
title: Calibrate Focal Point Length
layout: home
parent: Flight Camera (bottom)
nav_order: 2.2
---

## Calibrate Camera Focal Length 
1. Calibration requires that the LM be turned on. When you do so, first check to make sure that the LED strobe is not on. You can tell if it is on because the LEDs will glow a dull red (even for IR LEDs, there’s a little light on the high-frequency end of the visible spectrum).  
2. Next we will calibrate the focal length of the Flight camera by using the known diameter of a golf ball and the diameter of the ball as seen by the camera in terms of pixels.  
    1. Use the “Determine the camera’s focal length” process for Tee camera, below  
3. Run `runCam2BallCalibration.sh` script and take the resulting average focal length.  Make sure the ball is well lit with a good contrasting color behind it (use the preview function if necessary)  
    1. Certain types of black felt also show up black behind the ball and create great contrast.  
    2. If you get errors, such as “Attempted to draw mask area outside image”, check to ensure the golf ball is in the middle of the camera image.  
    3. Check for the images in the `logging.kLinuxBaseImageLoggingDir` directory. Can also try `--logging_level=trace` instead of info in the script.  
    4. After the final average value has been computed, you may have to hit Ctrl-C to stop the program  
    5. Also, the `Calibrated focal length` measurement that is output by the program should remain pretty close to the same number from measurement to measurement.  So, for example (ignore the “mm”),   
    1. `[2024-12-05 15:50:42.048685] (0x0000007fb6135040) [info]` Calibrated focal length for distance `0.550000` and Radius: `67.788940` is **6.094049**mm.  
    2. `[2024-12-05 15:50:42.048996] (0x0000007fb6135040) [info]` Focal Length = **6.094049**.  
    3. `[2024-12-05 15:50:43.441564] (0x0000007fb6135040) [info]` Calibrated focal length for distance `0.550000` and Radius: `67.803833` is `6.095388mm`  
    4. If the value is varying largely, the lighting may not be sufficient.  
4. Enter this value into the JSON file, e.g., `"kCamera2FocalLength": 6.155454` (or whatever the value is).  

----