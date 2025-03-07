---
title: Camera Calibration Confirmation
layout: home
parent: Flight Camera (bottom)
nav_order: 2.3
---

## Camera Calibration Confirmation
1. Keep the ball in the spot it was in when the focal length was calibrated for now, as the location is known.  We want to ensure the system is ‘finding’ the ball at that location.  
2. Run `“runCam2BallLocation.sh”` script on the Pi connected to the camera being calibrated to make sure the angles and distances and processing are correct  
    1. Generally, the IR filter should be removed if the ball does not have good contrast with the background.  
    2. The ball-search location should work in the center: `--search_center_x 723 --search_center_y 544`. If the script doesn’t return values, check the `log_view_final_search_image_for_Hough.png` file.  
    3. Remember, that the distances will be approximately to the center of the ball FROM THE CAMERA, so will be a little shorter than the distance to the LM in the X direction.  
    4. Check to make sure that the X, Y, and Z values (in meters) are correct.  
    5. Also, the “Radius” measurement that is output by the program should remain pretty close to the same number from measurement to measurement.  So, for example,   
    1. `[2024-12-05 15:43:57.884875] (0x0000007f8ea3d040) [info] Found Ball - (X, Y, Z) (in cm): 0.013771, 0.060536, 0.552454`. **Radius: 67.741455**  
    2. `[2024-12-05 15:44:00.758463] (0x0000007f8ea3d040) [info] Found Ball - (X, Y, Z) (in cm): 0.013772, 0.060538, 0.552480`. **Radius: 67.738266**
    3. `[2024-12-05 15:44:03.616166] (0x0000007f8ea3d040) [info] Found Ball - (X, Y, Z) (in cm): 0.013772, 0.060539, 0.552489`. **Radius: 67.737129**  

3. To ensure everything is working correctly, trying moving the ball 10cm up  and/or left or right and/or closer or further from the camera and then ensure that the runCam2BallLocation program responds with correct position information.  Accuracy should be at least within a centimeter for most measurements.

----