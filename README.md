# LoRa
Arduino Library for Lora Gateway and Lora Node devices for my home automation.

This library sits on top of my SX126x LoRa driver and implements a Lora gateway and a Lora node class. 
It can be used to set up e.g. a home automation infrastructure which consists of one gateway and arbitrary 
nodes like sensors, actors and so on.

The C++ gateway class includes encryption and message repeating in case there is no answer from the node.
On the node side there is a callback interface. So data input from gateway ends up in a node callback.
This callback delivers the gateway data as function parameter. Please see the example for more information!

In general all communication is initiated by the gateway. The nodes can only send data if they
were adressed by the gateway. 

## Prerequisites
- Install the SX126x Lora driver into Arduino IDE (https://github.com/tinytronix/SX126x)
- Install SpritzCipher into Arduino IDE if you want Encryption (https://github.com/abderraouf-adjal/ArduinoSpritzCipher)
- Install QList into Arduino IDE (https://github.com/SloCompTech/QList)

## Where to start
Please refer to the gateway and node example!
