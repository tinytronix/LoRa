# LoRa
An Arduino Library for peer-to-peer LoRa data communication for home automation purpose.

This library sits on top of my [SX126x](https://github.com/tinytronix/SX126x) LoRa driver and implements a C++ class for a Lora gateway and a C++ class for multiple Lora nodes. Both classes implement their part of a communication protocol to let both talk to each other. 

It can be used to set up a private LoRa network for e.g. home automation infrastructure which consists of one gateway and arbitrary nodes like sensors, actors and so on.

# Features
- low protocol overhead (only 7 bytes)
- 32Bit radio id for each LoRa node
- end to end message transaction handle
- up to 121 Bytes of payload
- receive confirmation
- message retransmission if confirmation fails
- optional encryption (decide at compile time)
- callback interface 
- tx power adaption
- round trip (request -> confirmation) ~70ms at SF5, 125kHz

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
