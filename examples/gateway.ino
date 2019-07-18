#include <LoraGateway.h>

/*
*
* This example can be used for the lora gateway:
*
*/

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
// globale Objekte
//
//-------------------------------------------------------------------------------------                                          
LoraGateway             lora(PD5,               //Port-Pin Output: SPI select
                             PD6,               //Port-Pin Output: Reset 
                             PD7,               //Port-Pin Input:  Busy
                             PB0                //Port-Pin Input:  Interrupt DIO1 
                             );

CIPHERKEY               cipher;

uint8_t                 data;
uint8_t*                pRxData;
uint8_t                 rxLen;
uint8_t                 txTimer;

//-------------------------------------------------------------------------------------
//
// setup
//
//-------------------------------------------------------------------------------------
void setup() 
{
  rxLen = 0;
  txTimer = 0;
  Serial.begin(9600);
  delay(500);
  
  Serial.println("\n");
  Serial.print("LoRa Gateway");
  
  lora.begin(TASK_TIME_MS,              //important to maintain RX TX timing in the Service Function
             SX126X_PACKET_TYPE_LORA,
             RF_FREQUENCY,              //frequency in Hz
             TX_OUTPUT_POWER);          //tx power in dBm

  //for encryption #define LORANODE_NO_ENCRYPT before including LoraGateway.h
  //cipher = {0x00, 0x00, 0x00};
  //lora.EncryptDecryptKey(&cipher);
  
  lora.LoRaConfig(LORA_SPREADING_FACTOR, 
                  LORA_BANDWIDTH, 
                  LORA_CODINGRATE, 
                  LORA_PREAMBLE_LENGTH, 
                  LORA_PAYLOADLENGTH, 
                  true,                 //crcOn  
                  false                 //invertIrq
                  );
}


//-------------------------------------------------------------------------------------
//
// main loop
//
//-------------------------------------------------------------------------------------
void loop() 
{
  if ( txTimer == 0 )
  {
    ACTOR_REQ req;
  
    req.id = 3;       //Actor id
    req.action = 1;   //Actor action

    //triggers callback onLORA_ACTOR_REQ(uint16_t id, uint8_t action)
    //on client node side
    lora.Send(0x12345678,                   //ID of Lora Node device 
              LORA_ACTOR_REQ,               //Command ID
              (uint8_t*)&req,               //pointer to data
              (uint8_t)sizeof(ACTOR_REQ));  //length

              
    txTimer = TIME_SECONDS(1);
  }
  else
  {
    txTime--;
  }
   
  lora.Service();
  
  rxLen = lora.GetResponseData(&pRxData);
  if ( rxLen )
    Serial.println("Response: LORA_ACTOR_RESP");
    
  delay(TASK_TIME_MS);
}
