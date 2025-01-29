#!/bin/bash
#
# Takes numPics pictures, each about 1 second apart
# The script uses the libcamera-still utility, and assumes that the Pi GS Camera is in use (IMX296)
#
if [[ $# -lt 2 ]];  then  echo Format: "$0" baseFName numPics; exit;  fi
sleep 5
for((m=1; m<=${2}; ++m))
do
	echo '====================== GET READY - Taking next picture in 2 seconds ======================='
	sleep 2
	libcamera-still --timeout 1000 --gain 1.0 --nopreview --tuning-file=/usr/share/libcamera/ipa/rpi/pisp/imx296.json --shutter 10000 --width 1456 --height 1088 --denoise cdn_off -o ${1}${m}.png
done
