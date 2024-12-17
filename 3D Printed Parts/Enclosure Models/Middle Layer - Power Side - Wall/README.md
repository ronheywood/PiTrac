**Middle Layer – Power Side – Wall - Description:**

Please see the main Enclosure Assembly instructions in the Documentation folder for additional information on this part and the required mounting hardware and other assemblies.

This is the middle-left-most portion of the PiTrac Launch Monitor. A [FreeCAD 1.0](https://www.freecad.org/downloads.php) compatible parameterized model is included here along with .stl files. It provides a walled enclosure for a floor that provides a wall between the power-side bay and the Pi-side.

**NOTE:** This part was created by taking the right-side middle-layer wall, modifying a few things, and then creating a re-scaled, basically reversed, object. We have not figured out how to create the mirror-image part without still retaining the original part. For that reason, please ignore the right-hand-side (Pi-side) part when printing.

**NOTE:** The associated floor mounts **_under_** the lower pads of this wall component.

The model is parameterized for ease of modifications, though the part is complex enough that any changes can sometimes lead to issues in FreeCAD. Modifiable parameters are in the “Master Document” spreadsheet, which is assumed to be in the directory immediately above wherever this part’s FreeCAD file exists.

Please consider supporting this work and the DIY LM project here: <https://ko-fi.com/PiTrac>

**Printing Notes:**

Both PLA or PETG can work for this part, though we recommend PLA. PLA is more likely to have warping issues, but is generally more forgiving, and PETG is more likely to have issues with layer separation or drooping or mis-aligned supports, which are harder to deal with.

Ensure that automatic supports in your slicer do not accidentally create supports within the screw and bolt holes and nut indents. They can be hard to remove. See the Prusa Slicer file’s custom support painting for how we print this part. Custom removal of any supports in the vertical holes coming up from the bottom of the bottom side mount risers is critical.

The screw holes under the lower-floor mounting pads can sometimes be fouled by the supports under those pads. Carefully remove the supports to provide sufficient access to the holes.

Recommended print settings: 4 perimeter layers (less if the bolts are not expected to be tightened very much), 15% infill, rectilinear infill patterns, 7mm brim, custom-assigned supports (see Prusa Slicer .3mf file for details). Due to potential warping issues and floor-pull-away issues, we recommend using an ST16 glue-stick (see [here](https://www.youtube.com/watch?v=vRcz8tuGcJU&pp=ygUJI2xlbXN0aWNr)) and brim assists. The contact area with the printing bed is pretty small, so anything that helps adhere the part to the bed during printing is helpful.