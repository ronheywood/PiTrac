**PiTrac configuration and the golf\_sim\_config.json file**

**NOTE \- THIS DOCUMENT IS STILL UNDER CONSTRUCTION**

Before running PiTrac, it needs to be configured.  Configuration includes things like the IP addresses (or names if you have name resolution working) of the two Pi’s.  It can also include various logging and debugging mode selection.

At this point, a full description of each of the \>100 configuration values in the golf\_sim\_config.json file is beyond the scope of the document.  If you want to know what a particular parameter does, it’s best to search the code for that parameter name, find which variable receives its value (usually similarly-named) and then figure out where in the code that variable is used.

However, there are a few configuration values that will need to be set before PiTrac can be run.  They are:

* “logging”:"kLinuxBaseImageLoggingDir":  "/home/\<PiTracUserName\>/LM\_Shares/Images/"  
  * PiTracUserName should be set to pi or whatever that user name is.  Do not use the shortcut “\~” symbol  
  * This directory is where most of the debugging images will be sent   
*  "ipc\_interface":"kWebActiveMQHostAddress": "tcp://10.0.0.65:61616",  
  * This should be the IP address of the Pi 2 system, on which the ActiveMQ broker is running

Correctly setting the above values is usually enough to run the launch monitor.

Here are some additional configuration values that often come in handy.

 "logging": {  
                        "kLogIntermediateExposureImagesToFile": 1,  
                        "kLogIntermediateSpinImagesToFile": 0,  
                        "kLogWebserverImagesToFile": 1,  
                        "kLogDiagnosticImagesToUniqueFiles": 1,  
