# LoRa
Arduino Library for Lora Gateway and Lora Node devices for my home automation.

This library sits on top of my SX126x LoRa driver and implements a Lora gateway, a Lora node and the communication
protocol to let both talk to each other. It can be used to set up a private LoRa network for e.g. home automation infrastructure which consists of one gateway and arbitrary nodes like sensors, actors and so on.

The communication protocol includes encryption and message repeating in case there is no answer from the node.
On the node side there is a callback interface. So data input from gateway ends up in a node callback.
This callback delivers the gateway data as function parameter. Please see the example for more information!

In general all communication is initiated by the gateway (data REQUEST). The nodes can only send (data RESPONSE) if they
were adressed by the gateway. 

## Prerequisites
- Install the SX126x Lora driver into Arduino IDE (https://github.com/tinytronix/SX126x)
- Install SpritzCipher into Arduino IDE if you want Encryption (https://github.com/abderraouf-adjal/ArduinoSpritzCipher)
- Install QList into Arduino IDE (https://github.com/SloCompTech/QList)

## Where to start
Please refer to the gateway and node example!

## FAQ
Q: Is this thing LoraWAN compatible? <br>
A: No. You may use the SX126x driver and the hardware as a base for a LoRaWAN device but this projects does not support LoRaWAN.
The gateway and the node software implement an completely different (home brewed) communication protocol.<br>
