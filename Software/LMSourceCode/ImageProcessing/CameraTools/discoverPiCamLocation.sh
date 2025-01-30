#!/bin/bash
for((m=0; m<=5; ++m))
do
    media-ctl -d "/dev/media$m" --print-dot | grep imx > discover_media.txt
    awk -F"imx296 " '{print $2}' < discover_media.txt | cut -d- -f1 > discover_device.txt
    echo -n -e "Camera is at /dev/media$m on device number " > discover_result.txt
    cat discover_device.txt >> discover_result.txt
    
    if  grep imx discover_media.txt > /dev/null ;  then  cat discover_result.txt ; break;  fi
done

rm discover_media.txt discover_device.txt discover_result.txt
