**Middle Layer – Pi Side – Wall - Description:**

Please see the main Enclosure Assembly instructions in the Documentation folder for additional information on this part and the required mounting hardware and other assemblies.

This is the middle-right-most portion of the PiTrac Launch Monitor. A [FreeCAD 1.0](https://www.freecad.org/downloads.php) compatible parameterized model is included here along with .stl files. It provides a walled enclosure for two different mounting floors, one near the bottom of the enclosure and one near the top. On the lower (which is ultimately the middle) floor are mounted the PiTrac Connector Board and the LED strobe light and lens assembly. On the upper floor are mounted the Pi Camera 1 assembly and the Pi 1 computer.

Note that the lower-floor (with the Strobe) mounts **_under_** the lower pads, while the top-most floor mounts **_on top_** of the top pads.

The model is parameterized for ease of modifications, though the part is complex enough that any changes can sometimes lead to issues in FreeCAD. Modifiable parameters are in the “Master Document” spreadsheet, which is assumed to be in the directory immediately above wherever this part’s FreeCAD file exists.

Please consider supporting this work and the DIY LM project here: <https://ko-fi.com/PiTrac>

**Printing Notes:**

Both PLA or PETG can work for this part, though we recommend PLA. PLA is more likely to have warping issues, but is generally more forgiving, and PETG is more likely to have issues with layer separation or drooping or mis-aligned supports, which are harder to deal with.

Ensure that automatic supports in your slicer do not accidentally create supports within the screw and bolt holes and nut indents. They can be hard to remove. See the Prusa Slicer file’s custom support painting for how we print this part.

The screw holes under the lower-floor mounting pads can sometimes be fouled by the supports under those pads.

Recommended print settings: 4 perimeter layers (less if the bolts are not expected to be tightened very much), 15% infill, rectilinear infill patterns, 7mm brim, custom-assigned supports (see Prusa Slicer .3mf file for details). Due to potential warping issues and floor-pull-away issues, we recommend using an ST16 glue-stick (see [here](https://www.youtube.com/watch?v=vRcz8tuGcJU&pp=ygUJI2xlbXN0aWNr)) and brim assists. The contact area with the printing bed is pretty small, so anything that helps adhere the part to the bed during printing is helpful.