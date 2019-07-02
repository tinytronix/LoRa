#include <LoraNode.h>


//-------------------------------------------------------------------------------------
//
// Defines
//
//-------------------------------------------------------------------------------------
#define TASK_TIME_MS                    25
#define TIME_SECONDS(x)                 ((x) * (1000/TASK_TIME_MS))
#define TIME_MILLISECONDS(x)            ((x) / TASK_TIME_MS)


#define RF_FREQUENCY                    433000000 // Hz  center frequency
#define TX_OUTPUT_POWER                 22        // dBm tx output power
/* Langsame Datenrate
#define LORA_BANDWIDTH                  4         // bandwidth=125khz  0:250kHZ,1:125kHZ,2:62kHZ,3:20kHZ.... look for radio line 392                                                               
#define LORA_SPREADING_FACTOR           7         // spreading factor=11 [SF5..SF12]
#define LORA_CODINGRATE                 4         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
*/
#define LORA_BANDWIDTH                  6         // bandwidth=125khz  0:250kHZ,1:125kHZ,2:62kHZ,3:20kHZ.... look for radio line 392                                                               
#define LORA_SPREADING_FACTOR           5         // spreading factor=11 [SF5..SF12]
#define LORA_CODINGRATE                 4         // [1: 4/5,2: 4/6,3: 4/7, 4: 4/8]

#define LORA_PREAMBLE_LENGTH            8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT             0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON      false     // variable data payload
#define LORA_IQ_INVERSION_ON            false
#define LORA_PAYLOADLENGTH              0         // 0: variable receive length 
                                                  // 1..255 payloadlength
                                                  

//-------------------------------------------------------------------------------------
//
// globale Typdefinitionen
//
//-------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//-------------------------------------------------------------------------------------                                                
LoraNode                lora(PD5,               //Port-Pin Output: SPI select
                             PD6,               //Port-Pin Output: Reset 
                             PD7,               //Port-Pin Input:  Busy
                             PB0                //Port-Pin Input:  Interrupt DIO1 
                             );



//-------------------------------------------------------------------------------------
//
// globale Objekte
//
//------------------------------------------------------------------------------------- 
void setup() 
{
  Serial.begin(9600);
  delay(500);
  
  Serial.println("\n");
  Serial.print("LoRa Node");
 
  lora.begin(TASK_TIME_MS,
             SX126X_PACKET_TYPE_LORA,
             RF_FREQUENCY,              //frequency in Hz
             TX_OUTPUT_POWER);          //tx power in dBm

  //for encryption #define LORANODE_NO_ENCRYPT before including LoraGateway.h
  //cipher = {0x00,0x00,0x00};
  //lora.EncryptDecryptKey(&cipher);
  
  lora.LoRaConfig(LORA_SPREADING_FACTOR, 
                    LORA_BANDWIDTH, 
                    LORA_CODINGRATE, 
                    LORA_PREAMBLE_LENGTH, 
                    LORA_PAYLOADLENGTH, 
                    true,                 //crcOn  
                    false                 //invertIrq
                    );

 lora.RegisterCallback(0x12345678,              //Lora node ID 
                       LORA_ACTOR_REQ,          //Lora command id
                       (void*)onLORA_ACTOR_REQ);//callback function for command LORA_ACTOR_REQ
}


//-------------------------------------------------------------------------------------
//
// onLORA_ACTOR_REQ
//
//------------------------------------------------------------------------------------- 
void onLORA_ACTOR_REQ(uint16_t id, uint8_t action)
{
  Serial.println("onLORA_ACTOR_REQ");
  lora.Send_LORA_ACTOR_RESP(1);
}


//-------------------------------------------------------------------------------------
//
// loop
//
//------------------------------------------------------------------------------------- 
void loop() 
{
  lora.Service();
  delay(TASK_TIME_MS);
}
