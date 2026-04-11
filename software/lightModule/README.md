# Light Module
How to use 
Configure the project

```
make menuconfig
```

- Under configuration set:
    - Broker URL: The URL of the broker to connect to.
    - Topic URL: Topic where the ESP should be focus on.
    - Wifi SSID: The name of your network.
    - Wifi PassWord: Password for the Wifi Network.

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:
```
make flash monitor
```
(To exit the serial monitor, type Ctrl-].)


## Features
- Publish and subscription to a topic.
    - GPIO0 publish to the topic through a interrupt routine that toggles the stead of the light. 
    - GPIO2 is the output pin for the signal to change the light setup.
