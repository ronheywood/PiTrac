---
title: Terminology
layout: home
parent: Home
nav_order: 1.2
---

### Terminology 

<table>
  <tr>
    <th>Device</th>
    <th>Description</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>Flight Camera</td>
    <td>This is the modified camera that will be installed on the first floor of the enclosure. Bottom camera.</td>
    <td> 
    - This camera is attached to Core Pi.<br>
    - Previously known as camera 2. In scripts, may be referred as such.
    </td>
  </tr>
   <tr>
    <td>Tee Camera (As in, Tee off)</td>
    <td>This is an unmodified camera that will be installed on the top floor of the enclosure. Top camera</td>
    <td> 
    - This camera is attached to Tee Pi.<br>
    - Previously known as camera 1. In scripts, may be referred as such.
    </td>
  </tr>
     <tr>
    <td>Tee Pi </td>
    <td>This Raspberry Pi that is used for the shared directory--Samba Server. It is recommended that you use a faster Pi (e.g. Pi5) for this Pi. Installed on the top floor of the enclosure.</td>
    <td> 
    - Connected to the Tee Camera (The top floor Camera).<br>
    </td>
  </tr>
       <tr>
    <td>Flight Pi </td>
    <td>Flight Pi - used for messaging running ActiveMQ and Apache Tomee. Installed on the bottom floor of the enclosure.</td>
    <td> 
    - Connected to the Flight camera (the modified camera).<br>
    </td>
  </tr>
</table>

### PiTrac 
<a href="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQSr4Eu550wjRoRfLHclEgkAARA1St3Qvlx-dvPuE9Aupqc?width=1024">
<img src="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQSr4Eu550wjRoRfLHclEgkAARA1St3Qvlx-dvPuE9Aupqc?width=1024" alt="PiTrac">
</a>

### Diagram
<a href="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQRJJ03UsL98QbZK4ppRKmdsAQ_Gr50rl9IbFNyvEkk1Wyg?width=1030&height=894">
<img src="https://1drv.ms/i/c/35c1f51c7fbc7aba/IQRJJ03UsL98QbZK4ppRKmdsAQ_Gr50rl9IbFNyvEkk1Wyg?width=1030&height=894" alt="PiTrac Diagram">
</a>