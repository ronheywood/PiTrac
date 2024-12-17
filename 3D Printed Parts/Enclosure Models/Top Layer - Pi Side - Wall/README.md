**Top Layer – Pi Side – Wall - Description:**

Please see the main Enclosure Assembly instructions in the Documentation folder for additional information on this part.

This is the upper-right-most portion of the PiTrac Launch Monitor. A [FreeCAD 1.0](https://www.freecad.org/downloads.php) compatible parameterized model is included here along with .stl files. It provides a top ceiling with a minimal port for picking up the monitor. This part of the LM does not house any of the electronic components, it just provides a lid for the monitor.

The model is parameterized for ease of modifications, though the part is complex enough that any changes can sometimes lead to issues in FreeCAD. Modifiable parameters are in the “Master Document” spreadsheet, which is assumed to be in the directory immediately above wherever this part’s FreeCAD file exists.

Please consider supporting this work and the DIY LM project here: <https://ko-fi.com/PiTrac>

**Printing Notes:**

Both PLA or PETG can work for this part, though we recommend PLA. PLA is more likely to have warping issues, but is generally more forgiving, and PETG is more likely to have issues with layer separation or drooping or mis-aligned supports, which are harder to deal with.

Ensure that automatic supports in your slicer do not accidentally create supports within the screw and bolt holes and nut indents. They can be hard to remove. See the Prusa Slicer file’s custom support painting for how we print this part.

Recommended print settings: 4 perimeter layers, 15% infill, gyroid infill patterns, custom-assigned supports (see Prusa Slicer .3mf file for details). Due to potential warping issues (especially in what will be the center (left-most) portion of the print), we recommend using a glue-stick (see [here](https://www.youtube.com/watch?v=vRcz8tuGcJU&pp=ygUJI2xlbXN0aWNr)) and brim assists.