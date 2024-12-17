**Errata and Future Hopes for the DIY LM**

**Errata:**

* The LM is not enabled for left-handed golfers.  This is embarrassing, but we simply haven’t had the time to complete this capability. And we have no left-handed friend golfers to test it.  In addition, the Camera 1 has to be pointed the other direction to make this work (at least currently).  Repositioning is not easy.  Better cameras and faster processing and triggering of Camera 2 in the future may not need this side-tilt at all, and would make it much easier to be non-handist..  
* Very low, worm-burner shots are often not analyzed correctly, because the system loses the ball in the hitting mat background visual noise.  
* Highly-angled shots that end up with the ball very close to or relatively far from the LM are often not processed correctly, as the ball is not well-focused.

**Future Hopes:**

* Enclosure  
  * Easier-to-join enclosure halves.  It’s hard to get to the bolts to tighten the halves together  
  * The supports for the lower-power-side floor’s overlap joint are a pain to remove.  Maybe print at a different angle?  
  * The seams on the 3D printed enclosure are unsightly.  Would be great to be able to print in a way that makes it look a little more professional.  Maybe more lap joints?  
  * Access to the Pi’s is difficult even with the ports..   
  * The inter-floor screws currently overhang into the layer interiors, which might present a safety issue.  Perhaps add a little bump on the side of the inner wall that the screws can end into.  
  * Removing the supports on the power-side floor lap joint is painful.  Is there a better design or way to print?  
* Cameras  
  * Calibration is too difficult and takes too much time.  How to improve?  
  * As new, higher-resolution global shutters come onto the market, it will be great to integrate them into the system.   Maybe we won’t need the angled Camera 1 in order to watch the teed-up ball\!  
  * The field of focus is too narrow.  A better lens might help.  
* Strobe:  
  * The 12V power supply for the LEDs seems like overkill given the short pulses that PiTrac uses.  Do we even need a constant-current  supply?  Could we just use a cheap 12V AC adapter for a couple dollars?    
  * Could we over-drive the LEDs by a few volts to get brighter pulses?  This would help with a number of things, including picture sharpness for fast balls.  Most LED’s can handle higher voltage for short periods of time without significantly shortening their lifespans.  
* Hardware:  
  * Must the Connector Board really have its own 5V power source?  Is there a better circuit that will protect the Pi’s from the 12V source and each other?  Can we run the board from the 12V supply?  
  * A fail-safe (temperature based? Dead-man timing?) for the strobe light to make sure it never stays on would be a great safety feature.  
* Performance  
  * The time between when the ball is hit and when the picture-taking and strobe-pulsing starts is too long.  A faster FPS camera would help, of course.