# LoRa
Arduino Library for Lora Gateway and Lora Node devices using my SX126x Arduino driver library

This library implements a Lora gateway and a Lora node class. It can be used to set up e.g. a
home automation infrastructure.

The C++ gateway class includes encryption and message repeating in case there is no answer.
On the node side tere is a callback interface.

In general all communication is initiated by the gateway. The nodes can only send data if they
were adressed by the gateway. 
