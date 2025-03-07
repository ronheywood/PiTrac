---
title: Initial Setup
layout: home
parent: Calibration
nav_order: 1
---

## Camera Initial Setup

1. Use a lens-cleaning cloth and appropriate cleaning liquid to make sure the lens are spotless.  
2. If possible, make sure the Pi computers are hard-wired into the network and the network switch (if any) is turned on.  
3. Before doing anything, log into the system remotely and perform a sanity check to make sure the camera is up and running correctly.  For example, take a picture using:  
    - `PiTracCameraTools/lcGS.sh test.jpg // TODO missing` 
    - So long as a picture is taken, things are probably operating correctly.  
4. Attach a keyboard, mouse and monitor to whatever Pi Camera is being calibrated using the front and side Pi Access ports on the monitor’s walls.    
    - If your Pi is set up to display a terminal clearly on a different computer (such as using VNC viewer), you may be able to avoid this connection.  
5. If the monitor does not come up with an image after the mouse is wriggled, it may be that the system was booted at a time that the monitor was not present.  In that case, reboot.  

6. Place a small piece of transparent tape onto your monitor screen, with a dot on it so that the dot is positioned to be at the exact center of the screen.  This dot will be referred to as the “marker dot”, and its role is simply to indicate where the center of the image is.  If this dot is at the same point as some feature that the camera sees, that feature is in the center of the camera’s view. Some people will use a remote terminal program like VNC Viewer instead of connecting a monitor directly to the Pi.  In this case, just make sure that you monitor aspect ratio and any window borders will not skew the true center calibration spot.  

7. Loosen the thumb screws on the lenses (if necessary) and rotate the lens to the full-open aperture position.  For the 6mm lens in the parts list, the aperture screw is the one furthest from the camera.  Tighten the aperture screw back down and loosen the focus screw. 

----