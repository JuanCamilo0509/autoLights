# Automatic light module
This project aims to be a plug and play wall switch based on the esp8266 board and using MQTT protocol for communications.

```
.
├── Hardware
│   └── autoLightPCB(Kicad)
├── README.md
└── software
    ├── lightModule(esp8266)
    └── mosquittoServer
        └── mosquitto.conf
```

This project has two main directories:
    - Software: Two main sub-directories, the light-Module where the code for the esp is maintain, and the "mosquitto-Server" with the ".conf" file for the server. 
    - Hardware: In this repository you can find the PCB layout for the board and the 3d model for the case.

## Future features. 
- HTTP Server on SoftAP mode when the variables aren't on the esp memory to set them up: (SSID, WiFi password, topic, button check).
- Get a prototype working on a switch.
- 3d model case for the switch.
- Better documentation for all the sections.
