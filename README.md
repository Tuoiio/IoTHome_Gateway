Microcontroller:  STM32F103C8, ESP32
Operating System: FreeRTOS
Hardware: NRF24L01, LCD TFT 2.8 inch, DHT22, Touch Sensor, Module Relay.
Peripherals and Protocol: I/O, SPI, MQTT.
Technical Skill: C
Enviroment development: KeilC, PlatformIO
Description: The system has 2 nodes and 1 gateway. 
Node 1 use the stm32f1 microcontroller and read temperature and humidity data from the DHT22 sensor, every 5 seconds it will send data to node 2 through the NRF24 module.
Node 2 use the STM32F1 microcontroller, when there is a signal from the touch sensor it will change the status of the relay and send the status of the relay to the gateway.
When the node receives data, it checks to see where the data was sent from. If from the node 1, it sends to the gateway. If from the gateway it will change the status of the relay and send back the status of the relay.
Gateway use the ESP32 microcontroller and FreeRTOS operating system. There are 4 main tasks. Task NRF to transmit or receive data with nodes, task LCD TFT is used to select and enter wifi ID and Password, display node parameter and control relay. The wifi task is used to scan and connect to wifi. The MQTT task  is used to transmit or receive data from the HiveMQ sever using the MQTT protocol. 
