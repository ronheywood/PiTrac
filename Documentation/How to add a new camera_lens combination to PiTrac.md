**How to add a new camera/lens combination to PiTrac**

This document describes how to add a new camera type and/or a new lens type to the PiTrac source code so that the system will behave in a (hopefully) optimal manner with that equipment.  For our example, we will be adding a new combination of an Innomaker GS camera with a 3.6 focal length M12 lens.

1. Make sure you have your own github “fork”.  A fork will allow you to make changes and then make a push request (PR) to have those changes checked into the main github repository so that everyone can benefit.  
2. Check out the latest source code in your fork.   Usually, just go to wherever your forked source code resides and execute:   git pull  
3. Edit the file camera\_hardware.h and add a new camera/lens combination with a new (and next-highest) ordinal number (emphasized part is new).  
   1.   
   2.             enum CameraModel {  
   3.                 PiCam13 \= 1,  
   4.                 PiCam2 \= 2,  
   5.                 PiHQCam6mmWideLens \= 3,  
   6.                 PiGSCam6mmWideLens \= 4,  
   7.                 PiGSCam3\_6mmLens \= 5,  
   8.                 **InnoMakerIMX296GS3\_6mmM12Lens \= 6,**  
   9.                 kUnknown \= 100  
   10.             };  
4. Add a corresponding line in the camera\_hardware.cpp file:  
   1.   
   2.         std::map\<std::string, int\> camera\_table \=  
   3.         { { "1", CameraModel::PiCam13 },  
   4.             { "2", CameraModel::PiCam2 },  
   5.             { "3", CameraModel::PiHQCam6mmWideLens },  
   6.             { "4", CameraModel::PiGSCam6mmWideLens },  
   7.             { "5", CameraModel::PiGSCam3\_6mmLens },  
   8.             **{ "6", CameraModel::InnoMakerIMX296GS3\_6mmM12Lens },**  
   9.             { "100", CameraModel::kUnknown },  
   10.         };  
5. Add whatever code is necessary to setup the camera’s member parameter values in the camera\_hardware.cpp file’s CameraHardware::init\_camera\_parameters method.  Typically, this will be the focal length, resolution, whether it is a monochrome camera, and importantly, the expected\_ball\_radius\_pixels\_at\_40cm\_ value.  That value can be determined by running rpicam-still of a ball at that distance from the camera.  This value helps the system to pick a reasonable range of ball radii when searching for ball circles.    
6. Most of the code will probably be taken care of by a similar camera, so you can just piggy-back on that code by adding your camera as a potential alternative in an OR statement:  
   1.         // This section deals with the common characteristics of some of the cameras  
   2.         if (model \== PiGSCam6mmWideLens **|| model \== InnoMakerIMX296GS3\_6mmM12Lens**) {  
7. Otherwise, add your specifics prior to the common code:  
   1.         **if (model \== InnoMakerIMX296GS3\_6mmM12Lens) {**  
   2.             **focal\_length\_ \= 3.6f;**  
   3.             **horizontalFoV\_ \= 70.0f;**  
   4.             **verticalFoV\_ \= 70.0f;**  
   5.             **is\_mono\_camera\_ \= true;**  
   6.             **expected\_ball\_radius\_pixels\_at\_40cm\_ \= 57;**  
   7.         **}**  
8. Recompile (ninja) and then setup your environment variables (typically in .basrc or .zshrc) to reference the new ordinal number, e.g.:  
   1. PITRAC\_SLOT1\_CAMERA\_TYPE=6  
   2. PITRAC\_SLOT2\_CAMERA\_TYPE=6

