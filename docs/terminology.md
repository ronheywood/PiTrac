---
title: Terminology
layout: home
parent: Home
nav_order: 1.2
---

### Terminology 

- Top Camera (Position Camera)
Bottom Camera (Flight Camera)

- Core Pi (aka Raspberry Pi 1, Top Pi) // Desc: Where pitrac_lm, Samba would run?
Messaging Pi (aka Raspberry Pi 2, Bottom Pi) // Desc: Where ActiveMQ and Tomee would run?

- Flight Camera - This is the modified camera that will be installed on the first floor of the enclosure.
    - This camera is attached to Core Pi.
- Tee Camera - This is an unmodified camera that will be installed on the top floor of the enclosure.
    - This camera is installed

- Tee Pi - This Raspberry Pi that is used for the shared directory--Samba Server. It is recommended that you use a faster Pi (e.g. Pi5) for this Pi. Installed on the top floor of the enclosure.
    - Connected to the Tee off Camera (The top floor Camera)
- Flight Pi - used for messaging running ActiveMQ and Apache Tomee. Installed on the bottom floor of the enclosure.
    - Connected to the Flight camera (the modified camera).

### PiTrac 
<a href="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQSr4Eu550wjRoRfLHclEgkAARA1St3Qvlx-dvPuE9Aupqc?width=1024">
<img src="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQSr4Eu550wjRoRfLHclEgkAARA1St3Qvlx-dvPuE9Aupqc?width=1024" alt="PiTrac">
</a>

### Diagram
<a href="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQRJJ03UsL98QbZK4ppRKmdsAQ_Gr50rl9IbFNyvEkk1Wyg?width=1030&height=894">
<img src="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQRJJ03UsL98QbZK4ppRKmdsAQ_Gr50rl9IbFNyvEkk1Wyg?width=1030&height=894" alt="PiTrac Diagram">
</a>