**PiTrac \- Raspberry Pi Setup and Configuration**

These instructions are targeted toward folks who don’t have a lot of experience building software systems in the Pi Operating System and who could benefit from more step-by-step direction. Someone who’s familiar with using tools like meson and ninja to build software can likely skip over many of these steps. However, the instructions contain a number of idiosyncratic steps and configuration requirements that are particular to PiTrac.

These instructions start with a Raspberry Pi with nothing on it, and are meant to describe all the steps to get from that point to a working, compiled version of PiTrac.  PiTrac currently requires two Raspberry Pi’s, so the majority of these instructions will have to be repeated twice.  Because the ‘smaller’ Pi system that connects to Camera 2 is the only Pi that handles the Tomcat/Tomee web-based GUI for the system, there are a few more steps for that system.

**Necessary & Recommended Components:**

- A raspberry Pi 4 and/or 5 with at least 4 Gig of memory  
- A Micro SD card with at least 64Gig  
- Especially if you are planning on using the larger Pi 5 as a development environment, an NVMe hat with an NVMe drive is a great investment.  
- Power supply for the Pi(s)  
- Network cabling if using the Pi’s wired ethernet port (recommended \- some of the packages we’ll be downloading are large and slow over WiFi)  
- Monitor, keyboard and mouse to connect to the Pi (recommended, but can also run the Pi ‘headless’ without any direct connections  
- Especially if running headless, a Mac, PC, or other Linux machine that you will use to interact with the Pi, along with a terminal tool to login to the Pi, such as Putty.  
- Visual Studio (optional) for your PC or MAC  
  - Most of the PiTrac system runs not only in the Pi, but can also (mostly) run from a Visual Studio platform on a PC or Mac.  The more comfortable graphical  programming environment in VS is great for testing and debugging and coding new features.  
- A separate file server, such as a NAS is highly recommended if you expect to be making changes to the PiTrac system.  Pi’s are a little fragile, so keeping the PiTrac files elsewhere in a safer environment (and then mounting those files on the Pi remotely) is a good practice.  
  - We typically have a separate server that we use both from the Pi and also from a PC running Visual Studio that is used to help debugging.  
  - It’s wise to think of the Pi as a temporary, write-only device that could be erased at any time.

**Standard Setup:**

1. Create a secure, static-safe environment to run your Pi’s on.  3D-Printing the two Pi-Side “Floors” from the plans on github is one way to provide this environment, and you’ll need to print them at some point anyway.  
2. Raspian O/S and Pi Initialization  
   1. Start with the Pi powered off (unplugged).  Have a Cat5/6 cable that is connected to your local network plugged in if possible.  
   2. On a PC, connect a Micro SD card via USB to use to boot the Pi for the first time.  
      1. Use a 64GB card so we have room to expand  
   3. Install and run the [RPi Imager utility](https://www.raspberrypi.com/software/)  
   4. Select Pi 4 or 5 for the device depending on what you have, for Operating System choose the 64-bit OS.  Make sure the “Storage” is pointing to the MicroSD card (and not something like your hard-drive\!), as it will be overwritten entirely.  Hit NEXT.  
   5. Answer “EDIT SETTING” when asked if you want to apply customisations  
      1. If you are American, ignore the clearly-incorrect spelling of “customization.” ;)  
      2. In the GENERAL tab,:  
         1. Select a hostname that will easily distinguish between the two Pi’s in the system, such as rsp01, rsp02, etc.  
         2. Add a \<PiTracUser\> username that will be used to compile and run PiTrac, and that can log into your NAS, if you’re using a server (recommended).  E.g., “pitrac” as a user (or just use “pi”)  
            1. This will be the username and password necessary to log into the system for the first time, so double-check you’ve verified both.  
            2. Use the actual user name whenever you see \<PiTracUser\> below  
         3. Make sure the wireless LAN credentials are setup in case you can’t connect a hard line  
      3. In the SERVICES tab,   
         1. Enable SSH and use password authentication  
   6. After setting up the customizations, select YES to “apply OS customisation settings” to the card setup and start the write process.  Should take about 20 minutes.  
   7. Once the SD Card is written and verified, if you have a keyboard, mouse, and monitor, hook those up first.  This can all be done via a remote login, but it’s nice to be able to have a full user setup from the beginning if there are any issues.  
   8. Insert the Micro SD card into the Pi and start up the Pi by plugging in the power (don’t insert or disconnect the SD card when the Pi is on\!)..  
      1. The first bootup takes a while.  Better to be able to monitor it with an attached monitor if possible.  
3. Log into the Pi using whatever credentials you expect to use to run PiTrac (the \<PiTracUserName\>  
   1. If running headless, remotely login using putty or a ssh tool of your choice  
      1. Logging in from whatever computer you are reading this setup document on will make it easy to copy-paste from this document into files on the Pi  
      2. For example,   
         1. putty rsp02 \-l \<username\>    (the boot image should already allow putty)  
   2. If running directly with a monitor and keyboard, click on updates icon near top-right to make sure everything is up to date  
      1. Install everything to get up to date  
   3. Or, equivalently, do the following from the command line:  
      1. sudo apt \-y update  
      2. sudo apt \-y upgrade  
4. Remotely login (to be able to paste from this setup document)  
   1. putty rsp01 \-l \<username\>    (the boot image should already allow putty)  
   2. Then, follow the instructions below…  
5. If necessary, make sure that \<PiTracUserName\> has sudo privileges  
6. To Install an NVME Board on the Pi  \[Optional\]:  
   1. If you have a SSD drive, best to get it up and booting now  
   2. See also the instructions here, which will work in most cases: [https://wiki.geekworm.com/NVMe\_SSD\_boot\_with\_the\_Raspberry\_Pi\_5](https://wiki.geekworm.com/NVMe_SSD_boot_with_the_Raspberry_Pi_5)  
      Although the instructions below should work as well.  
   3. With the Pi off, Install the NVMe Board and NVMe SSD drive per instructions of whatever board you are using.  
   4. Power up and Enable the PCIe interface (your instructions may differ):  
      1. cd /boot/firmware/  
      2. sudo cp config.txt config.txt.ORIGINAL  
      3. By default the PCIe connector is not enabled.   
      4. To enable it you should add the following option into /boot/firmware/config.txt before the last “\[all\]” at the end of the file and reboot (sudo reboot now):  
         1. \# Enable the PCIe External Connector.  
         2. dtparam=pciex1  
            1. A more memorable alias for pciex1 exists, so you can alternatively add dtparam=nvme to the /boot/firmware/config.txt file.  
   5. After the reboot, we will image the NVMe drive  
      1. First, ***if using a non-HAT+ adapter***, add on first non-commented line of /boot/firmware/config.txt:   PCIE\_PROBE=1  (see instructions for you device)  
      2. Change BOOT\_ORDER to BOOT\_ORDER=0xf416 (to boot off NVM first), OR \- better yet,   
         1. sudo raspi-config  
         2. Go to the Advanced Options /s Boot Order  
         3. Select whatever order you want, usually NVMe card first   
      3. Shutdown, remove power to the PReboot \- after, a lsblk command should show something like this (see underlined line):  
         1. pitrac@rsp05:\~ $ lsblk  
         2. NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS  
         3. mmcblk0     179:0    0  29.7G  0 disk  
         4. |-mmcblk0p1 179:1    0   512M  0 part /boot/firmware  
         5. \`-mmcblk0p2 179:2    0  29.2G  0 part /  
         6. nvme0n1     259:0    0 238.5G  0 disk  
      4. At this point, the NVMe drive should be accessible, and we will make a copy (image) of the bootup Micro SD card onto the SSD drive  
      5. From the Pi Graphical Desktop, Applications \=\>Accessories \=\>SD Card Copier on the main screen, run the SD Card Copier program, and copy the OS to the NVME ssd.  There’s no need to select the separate UUID option.  
         1. If running headless, see the internet for other ways to image the SSD  
      6. Power down, remove power, then remove the SSD card  
      7. When you turn the power on, the Pi should reboot from the SSD drive, and it should be pretty quick\!

7. Setup mounting of a remote NAS drive (or similar) to use for development so that you can’t lose everything if the Pi has an issue.    
   1. The remote drive will store the development environment, though you can obviously set up the PiTrac not to need a separate drive once you have everything working.  However, it’s really a good idea to have the development and test environment on a different computer than on the individual Pi’s.  
   2. There are many ways to automatically mount a removable drive to a Pi.  The following is just one way that assumes you have a NAS with NFS services enabled and with a shareable drive that the Pi can read/write to.  
      1. NOTE:  If this Pi will be anywhere in a public network, obviously do not include your password in the fstab\!  
   3. sudo mkdir /mnt/PiTracShare  
   4. cd /etc  
   5. sudo cp fstab fstab.original  
   6. sudo chmod 600 /etc/fstab   \[to try protect any passwords in the file\]  
   7. sudo vi fstab  
      1. If using NFS:  
         1. \<NAS IP Address\>:/\<NAS Shared Drive Name\> /mnt/PiTracShare nfs \_netdev,auto 0 0  
         2. For example:  
            1. 10.0.0.100:/NAS\_Share\_Drive /mnt/PiTracShare nfs \_netdev,auto 0 0  
      2. If using CIFS:  
         1. Add the following to /etc/fstab after the last non-comment line, replacing RELATIVE PWD and other things in \[\]’s with the real pwd and info  
            1. //\<NAS IP Address\>:/\<NAS Shared Drive Name\> /mnt/PiTracShare cifs username=\[PiTracUserName\],password=\[PWD\],workgroup=WORKGROUP,users,exec,auto,rw,file\_mode=0777,dir\_mode=0777,user\_xattr 0 0  
   8. sudo systemctl daemon-reload  
   9. sudo mount \-a  
      1. If there’s an error, make sure the password is correct  
      2. ls \-al /mnt/PiTracShare    should show any files there  
8. Setup Samba server (to allow the two Pi’s to share a folder between themselves)  
   1. Need to allow the Pi’s to serve out directories to the other Pi to share information like debugging pictures from one Pi to the other  
   2. See [https://pimylifeup.com/raspberry-pi-samba/](https://pimylifeup.com/raspberry-pi-samba/) for the basics  
   3. We suggest the faster Pi 5 (or whatever will be connected to Camera 1\) be Pi from which the shared directory is shared.  
   4. Something like:  
      1. sudo apt-get install samba samba-common-bin  
      2. sudo systemctl restart smbd  
      3. sudo systemctl status smbd  
         1. Should show “active (running)”  
      4. mount \-t cifs  
      5. sudo vi /etc/samba/smb.conf   and add the following lines at the bottom  
         1. \[LM\_Shares\]  
         2. path \= /home/\<PiTracUser\>/LM\_Shares  
         3. writeable=Yes  
         4. create mask=0777  
         5. directory mask=0777  
         6. public=no  
      6. sudo smbpasswd \-a \<PiTracUsername\>  
         1. Enter the same password you used for the PiTracUsername  
      7. sudo systemctl restart smbd  
9. Setup ssh to make it easier to login securely and quickly (w/o a pwed) \[optional, but really useful\]  
   1. This step assumes your PiTrac is secure in your own network and that the machine you use to log in is not used by others (given that this helps automate remote logins)  
   2. If not already, remotely log into the Pi from the machine where you’re reading this document  
   3. Create an ssh file  
      1. install \-d \-m 700 \~/.ssh  
   4. Install putty on the remote (non-Pi Mac/PC) machine that you’ll use to log in  
   5. Use the puttygen utility to generate a public key.  This is just a long text string or two  
   6.  vi \~/.ssh/authorized\_keys and paste in the public key for putty  
      1. (or can just use the mount to get a copy of the file from another Pi)  
      2. The key would have been generated using puttygen  
      3. The file should simply have each key (no spaces\!)  preceded on the same line with “ssh-rsa ”  
   7. sudo chmod 644 \~/.ssh/authorized\_keys  
   8.   
10. If you don’t already have your development world setup the way you want it, we suggest trying now some of the environments/tools at the bottom of these instructions labeled “**Nice-to-Haves for an easy-to-use development environment**”  
11. Git and GitHub  
    1. If the project will be hosted on a shared drive, and you 100% control of that drive and it’s not public, then let github know that we’re all family here.  On the pi and on whatever computer you log in from, do::  
       1. git config \--global \--add safe.directory "\*"  
       2. Otherwise, Git desktop and Visual Studio often have problems  
12. Configure the clock to not vary (as our timing is based on it\!)  
    1. cd /boot/firmware  
    2. sudo cp config.txt config.txt.ORIGINAL  
    3. Sudo vi config.txt:  
    4. For Pi 4 & 5:  
       1. Set force\_turbo=1   in /boot/firmware/config.txt  
       2. E.g.,   
       3. \# PiTrac MOD \-  force fast clock even at low load  
       4. force\_turbo=1  
    5. For Pi 5 also add  
       1. arm\_boost=1 in /boot/firmware/config.txt   
       2.   
13. Install and build OpenCV \- for both python and C++    
    1. Latest version as of this writing (late 2024\) is 4.10  
    2. See e.g., [https://itslinuxfoss.com/install-opencv-debian/](https://itslinuxfoss.com/install-opencv-debian/) for more information on installing  
    3. Increase swap space other requirements to have around 6 Gig.  For a 4 Gig or larger Pi, you can skip this.  
       1. See [https://qengineering.eu/install-opencv-4.5-on-raspberry-64-os.html](https://qengineering.eu/install-opencv-4.5-on-raspberry-64-os.html)  
          1. \# enlarge the boundary (CONF\_MAXSWAP)  
          2. $ sudo nano /sbin/dphys-swapfile  
          3. \# give the required memory size (CONF\_SWAPSIZE)  
          4. $ sudo nano /etc/dphys-swapfile  
          5. \# reboot afterwards  
          6. $ sudo reboot  
       2. See also https://docs.opencv.org/3.4/d7/d9f/tutorial\_linux\_install.html  
    4. Compile OpenCV  
       1. mkdir \~/Dev  
       2. cd Dev    (this is where we will compile the packages PiTrac needs)  
       3. See [https://qengineering.eu/install-opencv-4.5-on-raspberry-64-os.html](https://qengineering.eu/install-opencv-4.5-on-raspberry-64-os.html)  
       4. You can use the fully-automated script, though you might learn more if you follow the steps in the guide.  
          1. The script in named OpenCV-4-10-0.sh and is available as described in the above URL.  
          2. In the OpenCV-4-10-0.sh script, it’s useful for testing to change the following before running:  
             1.  INSTALL\_C\_EXAMPLES=ON   
          3. Run the script and review the output to make sure there were no errors.  The script takes quite a while to run on some Pi’s.  
       5. Ensure the script run the sudo make install step at the end after the script runs  
14. Install Boost (a set of utilities that PiTrac uses)  
    1. Install the current version of the boost development environment  
       1. sudo apt-get install libboost1.74-all  
    2. Create a boost.pc file to tell meson how to find boost files when PiTrac is compiled  
       1. sudo vi  /usr/share/pkgconfig/boost.pc    and in it place:  
          1. \# Package Information for pkg-config  
          2.   
          3. \# Path to where Boost is installed  
          4. prefix=/usr  
          5. \# Path to where libraries are  
          6. libdir=${prefix}/lib  
          7. \# Path to where include files are  
          8. includedir=${prefix}/boost  
          9.   
          10. Name: Boost  
          11. Description: Boost provides free peer-reviewed portable C++ source libraries  
          12. Version: 1.74.0   \# ← OR WHATEVER VERSION YOU DOWNLOAD  
          13. Libs: \-L${libdir} \-lboost\_filesystem \-lboost\_system \-lboost\_timer \-lboost\_log \-lboost\_chrono \-lboost\_regex \-lboost\_thread \-lboost\_program\_options  
          14. Cflags: \-isystem ${includedir}  
    3. Finally, because of a problem when compiling boost under C++20 (which PiTrac uses), add “\#include \<utility\> as the last include before the line that says “name space boost” in the awaitable.hpp file at /usr/include/boost/asio/awaitable.hpp”  
       1. This is a hack, but work for now.

15. Install and build lgpio (this is a library to work with the GPIO pins of the Pi)  
    1. cd \~/Dev   
    2. wget http://abyz.me.uk/lg/lg.zip  
    3. unzip lg.zip  
    4. cd lg  
    5. make  
    6. sudo make install  
    7. Create a /usr/local/lib/pkgconfig/lgpio.pc containing the following:  
       1. \# Package Information for pkg-config  
       2.   
       3. prefix=/usr/local  
       4. exec\_prefix=${prefix}  
       5. libdir=${exec\_prefix}/lib  
       6. includedir=${prefix}/include/  
       7.   
       8. Name: lgpio  
       9. Description: Open Source GPIO library  
       10. Version: 1.0.0  
       11. Libs: ${exec\_prefix}/lib/liblgpio.so   
       12. Cflags: \-I${includedir}  
16. Install and build libcamera (for c++ camera control)  
    1. Install Prerequisites  
       1. sudo apt-get install \-y libevent-dev  
       2. sudo apt install \-y pybind11-dev  
       3. sudo apt \-y install doxygen  
       4. sudo apt install \-y python3-graphviz  
       5. sudo apt install \-y python3-sphinx  
       6. sudo apt install **\-y** python3-yaml python3-ply  
       7. sudo apt install \-y libavdevice-dev  
       8. sudo apt install \-y qtbase5-dev libqt5core5a libqt5gui5 libqt5widgets5  
    2. NEW \- MARCH 7 2024 \- Use [https://www.raspberrypi.com/documentation/computers/camera\_software.html\#building-libcamera-and-rpicam-apps](https://www.raspberrypi.com/documentation/computers/camera_software.html#building-libcamera-and-rpicam-apps)  
       1. BUT \-   
          1. Perform the git clone at \~/Dev where we’ve been building the other software  
          2. Do not install boost dev as a prerequisite- we built it already above  
          3. When done (after the install step), do sudo ldconfig to refresh the shared libraries  
          4. On the Pi 4 (if it has less than 6GB memory), add “-j 2” at the end of the ninja \-C build command to limit the amount of memory used during the build.  
17. Build rpicam-apps:  
    1. See [https://www.raspberrypi.com/documentation/computers/camera\_software.html\#building-libcamera-and-rpicam-apps](https://www.raspberrypi.com/documentation/computers/camera_software.html#building-libcamera-and-rpicam-apps)  
       1. BUT, we will add `-Denable_opencv=true` to the meson build step because we have installed OpenCV and will wish to use OpenCV-based post-processing stages  
       2. Also, we don’t need to re-install most of the prerequisites listed in the Pi website.  Just do:  
          1. sudo apt install \-y libexif-dev  
    2. cd \~Dev  
    3. git clone https://github.com/raspberrypi/rpicam-apps.git  
    4. cd rpicam-apps  
    5. meson setup build \-Denable\_libav=enabled \-Denable\_drm=enabled \-Denable\_egl=enabled \-Denable\_qt=enabled \-Denable\_opencv=enabled \-Denable\_tflite=disabled \-Denable\_hailo=disabled  
    6. meson compile \-C build  
    7. sudo meson install \-C build  
    8. sudo ldconfig \# this is only necessary on the first build  
18. Install recent java (for activeMQ)  
    1. sudo apt install openjdk-17-jdk openjdk-17-jre  
19. Install msgpack  
    1. Info at:  [https://github.com/msgpack/msgpack-c/wiki/v1\_1\_cpp\_packer\#sbuffer](https://github.com/msgpack/msgpack-c/wiki/v1_1_cpp_packer#sbuffer)   
    2. cd \~/Dev  
    3. git clone [https://github.com/msgpack/msgpack-c.git](https://github.com/msgpack/msgpack-c.git)  
    4. For some reason, the above does not grab all the necessary files. So, go here: [https://github.com/msgpack/msgpack-c/tree/cpp\_master](https://github.com/msgpack/msgpack-c/tree/cpp_master) and click on “Code” and down load the zip file into the \~/Dev directory  
    5. unzip /mnt/PiTracShare/dev/tmp/msgpack-c-cpp\_master.zip  
    6. cd msgpack-c-cpp\_master  
    7. cmake \-DMSGPACK\_CXX20=ON .  
    8. sudo cmake \--build . \--target install  
    9. sudo /sbin/ldconfig  
    10. TBD \- DO WE NEED TO DO THIS?  I DIDN”T  
    11. Also, need to create our own .pc file:  
        1. \# Package Information for pkg-config  
        2.   
        3. prefix=/usr/local  
        4. exec\_prefix=${prefix}  
        5. libdir=${exec\_prefix}/lib  
        6. includedir=${prefix}/msgpack/adaptor/include/  
        7.   
        8. Name: msgpack-cxx  
        9. Description: Open Source GPIO library  
        10. Version: 1.0.0  
        11. Cflags: \-I${includedir}  
20. Install ActiveMQ Broker (need only do on the Pi 2 system, as it is the only system that will be running the broker ?)   
    1. [https://activemq.apache.org/version-5-getting-started.html\#installation-procedure-for-unix](https://activemq.apache.org/version-5-getting-started.html#installation-procedure-for-unix)   
    2. Get Apache Pre-Reqs  
       1. sudo apt-get install libapr1-dev  
       2. sudo apt install libcppunit-dev  
       3. sudo apt \-y install doxygen  
       4. sudo apt \-y install e2fsprogs  
       5. sudo apt \-y install maven  
    3. cd activemq-cpp-library-3.9.5  
    4. ./autogen.sh  
    5. ./configure  
    6. make  
    7. sudo make install  
21. Install ActiveMQ C++ CMS messaging system (on both Pi’s)  
    1. Pre-requisites:  
       1. sudo apt \-y install libtool  
       2. sudo apt-get \-y install libssl-dev  
       3. sudo apt-get \-y install libapr1-dev  
       4. sudo apt install \-y libcppunit-dev  
       5. sudo apt-get install \-y autoconf  
    2. Download [activemq-cpp-library-3.9.5-src.tar.gz](http://www.apache.org/dyn/closer.lua/activemq/activemq-cpp/3.9.5/activemq-cpp-library-3.9.5-src.tar.gz)  
    3. [https://activemq.apache.org/components/cms/developers/building](https://activemq.apache.org/components/cms/developers/building)   
       1. [https://activemq.apache.org/components/classic/download/](https://activemq.apache.org/components/classic/download/) has the source code zip file that you will want to download  
       2. cd \~/Dev  
       3. gunzip /mnt/PiTracShare/tmp/activemq-cpp-library-3.9.5-src.tar.gz (or wherever you put the .gz zip file)  
       4. export MAVEN\_OPTS=-Xmx1024M  
       5.  mvn \-Dtest=false \-Dsurefire.failIfNoSpecifiedTests=false clean install  
    4. May also able to do:  
       1. git clone [https://gitbox.apache.org/repos/asf/activemq-cpp.git](https://gitbox.apache.org/repos/asf/activemq-cpp.git) if the version is new enough  
    5. cd activemq-cpp-library/activemq-cpp-library  
    6. ./autogen.sh  
    7. ./configure  
    8. make  
    9. sudo make install  
22. Install maven for building servlets on Tomcat/Tomee  
    1. sudo apt install maven  
23. Install tomee on the cam2 system  
    1. Use the “Plume” version that supports JMS  
    2. Make sure we have java 17+  
    3. [https://tomee.apache.org/download.html](https://tomee.apache.org/download.html)  
    4. cd /opt  
    5. unzip /mnt/PiTracShare/dev/tmp/apache-tomee-9.1.2-plume.zip  
    6. sudo mv apache-tomee-plume-9.1.2 tomee  
    7. sudo chmod \-R 755 tomee  
    8. Vi conf/tomcat-users.xml and add:  
       1. \<role rolename="tomcat"/\>  
       2. \<role rolename="admin-gui"/\>  
       3. \<role rolename="manager-gui"/\>  
       4. \<user username="tomcat" password="tomcat" roles="tomcat,admin-gui,manager-gui"/\>  
    9. Add a systemctl daemon script to /etc/systemd/system/tomee.service  
       1. \[Unit\]  
       2. Description=Apache TomEE  
       3. After=network.target  
       4.   
       5. \[Service\]  
       6. User=root  
       7. Type=forking  
       8. \#Environment=JAVA\_HOME=/usr/lib/jvm/default-java  
       9. Environment=JAVA\_HOME=/usr/lib/jvm/java-1.17.0-openjdk-arm64  
       10. Environment=CATALINA\_PID=/opt/tomee/temp/tomee.pid  
       11. Environment=CATALINA\_HOME=/opt/tomee  
       12. Environment=CATALINA\_BASE=/opt/tomee  
       13. Environment=CATALINA\_OPTS='-server'  
       14. Environment=JAVA\_OPTS='-Djava.awt.headless=true'  
       15. ExecStart=/opt/tomee/bin/startup.sh  
       16. ExecStop=/opt/tomee/bin/shutdown.sh  
       17. KillSignal=SIGCONT  
       18.   
       19. \[Install\]  
       20. WantedBy=multi-user.target  
    10. Update /opt/tomee/webapps/manager/META-INF/context.xml to allow “.\*” instead of just 127.0….  Replace the whole regex string  
    11. Add a new document base/root to allow access to the shared mounted drive:    
        1. Edit conf/server.xml and just before the /Host\> put:  
        2. NOTE \_ THIS IS NOT CORRECT ANY MORE \_ FIX\!  
        3. \<Context docBase="/mnt/PiTracShare/dev/GolfSim/LM/Images" path="/golfsim/Images" /\>  
    12. Allow symbolic linking.  In conf/context.xml, add before the end:  
        1. \<Resources allowLinking="true" /\>  
    13. Install the systemctl:    
        1. sudo systemctl daemon-reload  
        2. sudo systemctl enable tomee  
        3. sudo systemctl start tomee  
        4. sudo systemctl status tomee.service  
        5. Touch the PID file so that it’s there when things start the first time.  
24. Setup Git credentials  
    1. FIRST, I tried , [https://joaopmatias.medium.com/quick-setup-for-git-credentials-and-signed-com/gpiomits-98087c1cea9f](https://joaopmatias.medium.com/quick-setup-for-git-credentials-and-signed-commits-98087c1cea9f)  is a start, but then I used gh instead of credential manager  
    2. Note that my personal git token is at /mnt/PiTracShare/git\_personal\_token.txt  
    3. Instead, use the “gh” CLI:  
       1.  sudo mkdir \-p \-m 755 /etc/apt/keyrings  
       2. wget \-qO- https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg \> /dev/null \\\\n  
       3. echo "deb \[arch=$(dpkg \--print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg\] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list \> /dev/null  
       4. sudo apt install gh \-y  
       5. gh auth login   (will ask for the personal token)  
       6. gh repo clone https://github.com/jamespilgrim/LaunchMonitor.git  
25. Setup  for streaming (just trying to see a picture on the screen\!)  
    1. See [https://qengineering.eu/install-gstreamer-1.18-on-raspberry-pi-4.html](https://qengineering.eu/install-gstreamer-1.18-on-raspberry-pi-4.html)  
       1. sudo apt-get install libgstreamer1.0-dev \\  
       2.      libgstreamer-plugins-base1.0-dev \\  
       3.      libgstreamer-plugins-bad1.0-dev \\  
       4.      gstreamer1.0-plugins-ugly \\  
       5.      gstreamer1.0-tools \\  
       6.      gstreamer1.0-gl \\  
       7.      Gstreamer1.0-gtk3  
26. 1,$ s/\\.\\.\\/build\\/golf\_sim\_test/$PITRAC\_ROOT\\/ImageProcessing\\/build\\/pitrac\_lm/g  
27. Install other Launch Monitor dependencies  
    1. Formatting library because the currently-packaged gcc12.2 in debian doesn’t have the c++20 format capability yet  
       1. **`sudo apt`** `install libfmt-dev`  
28. **Build Launch Monitor\!**  
    1. Prerequisites:  
       1. Setup the PITRAC\_ROOT environment variable to point to the “LM” directory of the PiTrac build.  That is one directory “up” from the directory that has the meson.build file in it.  
          1. E.g., include in your .zshrc or .bashrc or whatever shell you use:  
             1. export PITRAC\_ROOT=/mnt/PiTracShare/GolfSim/LM  
             2.   
       2. sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers  
       3. Add extended timeout to  rpi\_apps.yaml file so that even if an external trigger doesn’t fire for a really long time, the libcamera library won’t time-out:  
          1. (**NOTE** for Pi 5, use /usr/share/libcamera/pipeline/rpi/pisp instead of /usr/share/libcamera/pipeline/rpi/vc4, below)  
          2. cd  /usr/share/libcamera/pipeline/rpi/vc4  
          3. sudo cp  rpi\_apps.yaml  rpi\_apps.yaml.ORIGINAL  
          4. In /usr/share/libcamera/pipeline/rpi/vc4/rpi\_apps.yaml, at the end of the pipeline section, add the following (including the last comma\!)  
             1. \# Custom timeout value (in ms) for camera to use. This overrides  
             2. \# the value computed by the pipeline handler based on frame  
             3. \# durations.  
             4. \#  
             5. \# Set this value to 0 to use the pipeline handler computed  
             6. \# timeout value.  
             7. \#  
             8. "camera\_timeout\_value\_ms": 1000000,  
       4. Get the latest imx296\_noir.json into /usr/share/libcamera/ipa/rpi/pisp  
    2. Edit the meson.build file and ensure that the following line is commented out if compiling on a Pi 4 (and is commented out if compiling on a Pi 5):  
       1. add\_global\_arguments('-DPITRAC\_COMPILING\_ON\_PI\_4', language : 'cpp')  
       2. This requirement should go away once we have the latest rpicam-app code integrated into the system.  
    3. meson setup build \-Denable\_libav=true \-Denable\_drm=true \-Denable\_egl=true \-Denable\_qt=true \-Denable\_opencv=true \-Denable\_tflite=false  
    4. ninja \-C build

**Nice-to-Haves for an easy-to-use development environment**

1. The following steps are only for someone who’s a little new to linux and doesn’t already have a development environment setup the way they like it.  The following are just a few tools that (for the authors of the PiTrac project) seem to make things a little more efficient.  
2. Z-shell and OhMyZa  
   1. Connect to your raspberry Pi with SSH  
   2. Install zsh :   
      1. sudo apt-get update && sudo apt-get install zsh  
      2. Edit your passwd configuration file to tell which shell to use for user pi :   
         1. sudo vi /etc/passwd and change /bin/bash to /bin/zsh for the \<PiTracUserName\> (usually the last line)  
         2. logout  
   3. Reconnect to your raspberry, and   
      1. If on login Zsh asks about the .z files, select 0 to create an empty .zshrc  
      2. check that zsh is the shell with echo $0.  
   4. Install OhMyZsh :   
      1. sh \-c "$(wget https://raw.githubusercontent.com/robbyrussell/oh-my-zsh/master/tools/install.sh \-O \-)"  
   5. Disconnect from your instance and reconnect/re-login to it.  
   6. Turn off  any zsh processing for any large git directories (otherwise,  it will make “cd” into a git directory freeze)  
      1. cd \<whatever directory you plan to build PiTrac in\>  
      2. git config \--add oh-my-zsh.hide-status 1   
      3. git config \--add oh-my-zsh.hide-dirty 1  
      4. NOTE \- you may have to do this later if you don’t have the directory picked out yet  
3. Install neovim  
   1. Install NeoVim:  
      1. sudo apt-get install neovim  
         2. sudo apt-get install python3-neovim  
   2. Install vundle into NVIM, not vim:  
      1.   
      2. git clone [https://github.com/VundleVim/Vundle.vim.git](https://github.com/VundleVim/Vundle.vim.git) \~/.config/nvim/bundle/Vundle.vim  
      3. Edit .\~/.config/nvim/bundle/Vundle.vim/autoload/vundle.vim and change   
         1. Change $HOME/.vim/bundle (should be near the end of the file) to $HOME/.config/nvim/bundle.  
   3. Configure neovim \- Vundle does not work perfectly with nvim unless you make some changes  
      1. Note these comments:  
         1. [https://gist.github.com/lujiacn/520e3e8abfd1c1b39c30399222766ee8](https://gist.github.com/lujiacn/520e3e8abfd1c1b39c30399222766ee8)   
         2. [https://superuser.com/questions/1405420/i-really-need-help-installing-vundle-for-neovim](https://superuser.com/questions/1405420/i-really-need-help-installing-vundle-for-neovim)   
      2. Create the file /home/\<PiTracUserName\>/.config/nvim/init.vim and add:  
         1. set nocompatible              " be iMproved, required  
         2. filetype off                  " required  
         3.   
         4. " set the runtime path to include Vundle and initialize  
         5. set rtp+=\~/.config/nvim/bundle/Vundle.vim  
         6.   
         7. call vundle\#begin()            " required  
         8. Plugin 'VundleVim/Vundle.vim'  " required  
         9.   
         10. " \===================  
         11. " my plugins here  
         12. " \===================  
         13. Plugin 'scrooloose/nerdtree'                            
         14. Plugin 'valloric/youcompleteme'   
         15.   
         16. " Plugin 'dracula/vim'  
         17. " \===================  
         18. " end of plugins  
         19. " \===================  
         20. call vundle\#end()               " required  
         21. filetype plugin indent on       " required  
      3. Add any other plugins you want.  The above example establishes these two  
         1. Plugin 'scrooloose/nerdtree'  
         2. Plugin 'valloric/youcompleteme'  
      4. Then in vim, type :PluginInstall     in order to install the plug ins  
      5. (Ignore anything on the net that refers to .vimrc \- that’s not applicable if using nvim.

Example .zshrc file:  
\# If you come from bash you might have to change your $PATH.  
export PATH=.:/$HOME/bin:/usr/local/bin:$PATH

\# Path to your oh-my-zsh installation.  
export ZSH="$HOME/.oh-my-zsh"

\# Set name of the theme to load \--- if set to "random", it will  
\# load a random theme each time oh-my-zsh is loaded, in which case,  
\# to know which specific one was loaded, run: echo $RANDOM\_THEME  
\# See https://github.com/ohmyzsh/ohmyzsh/wiki/Themes  
ZSH\_THEME="robbyrussell"

\# Set list of themes to pick from when loading at random  
\# Setting this variable when ZSH\_THEME=random will cause zsh to load  
\# a theme from this variable instead of looking in $ZSH/themes/  
\# If set to an empty array, this variable will have no effect.  
\# ZSH\_THEME\_RANDOM\_CANDIDATES=( "robbyrussell" "agnoster" )

\# Uncomment the following line to use case-sensitive completion.  
\# CASE\_SENSITIVE="true"

\# Uncomment the following line to use hyphen-insensitive completion.  
\# Case-sensitive completion must be off. \_ and \- will be interchangeable.  
\# HYPHEN\_INSENSITIVE="true"

\# Uncomment one of the following lines to change the auto-update behavior  
\# zstyle ':omz:update' mode disabled  \# disable automatic updates  
\# zstyle ':omz:update' mode auto      \# update automatically without asking  
\# zstyle ':omz:update' mode reminder  \# just remind me to update when it's time

\# Uncomment the following line to change how often to auto-update (in days).  
\# zstyle ':omz:update' frequency 13

\# Uncomment the following line if pasting URLs and other text is messed up.  
\# DISABLE\_MAGIC\_FUNCTIONS="true"

\# Uncomment the following line to disable colors in ls.  
\# DISABLE\_LS\_COLORS="true"

\# Uncomment the following line to disable auto-setting terminal title.  
\# DISABLE\_AUTO\_TITLE="true"

\# Uncomment the following line to enable command auto-correction.  
\# ENABLE\_CORRECTION="true"

\# Uncomment the following line to display red dots whilst waiting for completion.  
\# You can also set it to another string to have that shown instead of the default red dots.  
\# e.g. COMPLETION\_WAITING\_DOTS="%F{yellow}waiting...%f"  
\# Caution: this setting can cause issues with multiline prompts in zsh \< 5.7.1 (see \#5765)  
\# COMPLETION\_WAITING\_DOTS="true"

\# Uncomment the following line if you want to disable marking untracked files  
\# under VCS as dirty. This makes repository status check for large repositories  
\# much, much faster.  
\# DISABLE\_UNTRACKED\_FILES\_DIRTY="true"

\# Uncomment the following line if you want to change the command execution time  
\# stamp shown in the history command output.  
\# You can set one of the optional three formats:  
\# "mm/dd/yyyy"|"dd.mm.yyyy"|"yyyy-mm-dd"  
\# or set a custom format using the strftime function format specifications,  
\# see 'man strftime' for details.  
\# HIST\_STAMPS="mm/dd/yyyy"

\# Would you like to use another custom folder than $ZSH/custom?  
\# ZSH\_CUSTOM=/path/to/new-custom-folder

\# Which plugins would you like to load?  
\# Standard plugins can be found in $ZSH/plugins/  
\# Custom plugins may be added to $ZSH\_CUSTOM/plugins/  
\# Example format: plugins=(rails git textmate ruby lighthouse)  
\# Add wisely, as too many plugins slow down shell startup.  
plugins=(git)

source $ZSH/oh-my-zsh.sh

\# User configuration

\# export MANPATH="/usr/local/man:$MANPATH"

\# You may need to manually set your language environment  
\# export LANG=en\_US.UTF-8

\# Preferred editor for local and remote sessions  
\# if \[\[ \-n $SSH\_CONNECTION \]\]; then  
\#   export EDITOR='vim'  
\# else  
\#   export EDITOR='mvim'  
\# fi

\# Compilation flags  
\# export ARCHFLAGS="-arch x86\_64"

\# Set personal aliases, overriding those provided by oh-my-zsh libs,  
\# plugins, and themes. Aliases can be placed here, though oh-my-zsh  
\# users are encouraged to define aliases within the ZSH\_CUSTOM folder.  
\# For a full list of active aliases, run \`alias\`.  
\#  
\# Example aliases  
\# alias zshconfig="mate \~/.zshrc"  
\# alias ohmyzsh="mate \~/.oh-my-zsh"  
\#

alias cdlm='cd  /mnt/PiTracShare/GolfSim/LM/ImageProcessing'  
alias cdlml='cd  \~/Dev/LaunchMonitor/ImageProcessing'  
alias cdlc='cd  /mnt/PiTracShare/GolfSim/LibCamTest'  
alias cdserv='cd /usr/share/tomcat9-examples/examples/WEB-INF/classes/async'  
alias ll='ls \-al'  
function findtext() {  
        grep \-rni "$1" .  
}

\# Some useful navigation aliases

alias pushdd="pushd \\$PWD \> /dev/null"  
alias cd='pushdd;cd'  
alias ssh='ssh \-A'  
alias soc='source \~/.bashrc'  
\#below to go back to a previous directory (or more)  
alias popdd='popd \>/dev/null'  
alias cd.='popdd'  
alias cd..='popdd;popdd'  
alias cd...='popdd;popdd;popdd'  
alias cd....='popdd;popdd;popdd;popdd'  
\#below to remove directories from the stack only (do not 'cd' anywhere)  
alias .cd='popd \-n \+0'  
alias ..cd='popd \-n \+0;popd \-n \+0;popd \-n \+0'

\# Enable vi mode  
bindkey \-v \# Enable vi mode

