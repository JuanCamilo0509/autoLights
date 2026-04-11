# Automatic light module
This project aims to be a plug and play wall switch based on the esp8266 board and using MQTT protocol for communications.

.
├── Hardware
│   └── autoLightPCB(Kicad)
├── README.md
└── software
    ├── lightModule(esp8266)
    └── mosquittoServer
        └── mosquitto.conf

This project has two main directories:
    - Software: Two main sub-directories, the lightModule where the code for the esp is maintain, and the mosquittoServer with the .conf file for the server. 
    - Hardware: In this repo you can find the PCB layout for the board and the 3d model for the case.
