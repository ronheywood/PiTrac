**Base Layer – Power Side – Wall - Description:**

Please see the main Enclosure Assembly instructions in the Documentation folder for additional information on this part.

This is the lower-left-most portion of the PiTrac Launch Monitor. A [FreeCAD 1.0](https://www.freecad.org/downloads.php) compatible parameterized model is included here along with .stl files. It provides a base for the Power-Side (left) portion of the LM, including an area to place a power strip and all of the required power supplies. This part of the LM may also store a small network switch to allow for a hard-line connect of the LM. A nut-insert area is provided on the bottom, but is not currently used.

**NOTE:** This part was created by taking the right-side base-layer, modifying a few things, and then creating a re-scaled, basically reversed, object. We have not figured out how to create the mirror-image part without still retaining the original part. For that reason, please ignore the right-hand-side (Pi-side) part when printing.

The model is parameterized for ease of modifications, though the part is complex enough that any changes can sometimes lead to issues in FreeCAD. Modifiable parameters are in the “Master Document” spreadsheet, which is assumed to be in the directory immediately above wherever this part’s FreeCAD file exists.

Please consider supporting this work and the DIY LM project here: <https://ko-fi.com/PiTrac>

**Printing Notes:**

Print only the body named “Base Layer - Wall - Power Side Scale”. The other body is the base part from which this mirrored-image Power-side part is created.

Ensure that automatic supports in your slicer do not accidentally create supports within the screw and bolt holes and nut indents. They can be hard to remove. See the Prusa Slicer file’s custom support painting for how we print this part.

The two base parts (this one included) are the most difficult to print in the entire PiTrac project. Both PLA or PETG can work for this part, though we recommend PLA. PLA is more likely to have warping issues, but is generally more forgiving, and PETG is more likely to have issues with layer separation or drooping or mis-aligned supports, which are harder to deal with.

Recommended print settings: 4 perimeter layers (less if the bolts are not expected to be tightened very much), 15% infill, gyroid infill patterns, custom-assigned supports (see Prusa Slicer .3mf file for details). Due to potential warping issues (especially in what will be the center (left-most) portion of the print), we recommend using a glue-stick (see here) and brim assists.