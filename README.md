# HW2: Socket Programming and Data Visualization

![Alt text](docs/SampleFigure.png?raw=true)

This codebase contains 2 parts that can be found in the SocketClient folder and the SocketServer respectively.

The SocketClient folder contains the code and config files for the STM32 IoT node. The program can be built using Mbed Studio. The functionality for the client is to connect to wifi and send data to the socket server.

The SocketServer folder contains the code to run a socket server, which can visualize the data sent by the socket client.

## Team member:
b09901052 劉承亞
b09901058 邱奕翔


## How to run 

### Run the socket client
    
1, Open Mbed Studio

2, Import the program from here: https://github.com/ARMmbed/mbed-os-example-sockets

3, Copy the `mbed_app.json` in this repository to `./mbed-os-example-sockets/mbed_app.json`

4, Copy the `targets.json` in this repository to `./mbed-os-example-sockets/mbed-os/targets/targets.json`

5, Copy the `BSP_B-L475E-IOT01` directory into `./mbed-os-example-sockets/`
        
NOTE: Remember to change the following configs:
- In `mbed_app.json`
    ```
    "hostname": {
        "value": "\"xxx.xxx.xxx.xxx\""
    },
    ```
    ```
    "nsapi.default-wifi-ssid": "\"ssid\"",
    "nsapi.default-wifi-password": "\"password\"",
    ```


### Run the socket server

1, Go into the socketserver directory
```
cd ./SocketServer
```

2, Then you can run the server by simply typing 
```
python socket_server.py
``` 

NOTE: Remember to change the `HOST` IP in the code

NOTE: To close the server, you just close the window that pops up.

## Features
- The client automatically tries to reconnect to server, when the data isn't sent successfully.
- The server is multithreaded. One thread (`DataThread`) receives the data, while the main thread keep updating the graph every 100ms.
  

