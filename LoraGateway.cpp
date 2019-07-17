#include "Arduino.h"
#include "LoraGateway.h"



#define LORA_REPEAT_INTERVAL            120       //nach dieser Zeit in ms wird das Komamndo wiederholt, wenn keine Antwort kommt

#define LORA_PROTOCOL_STATE_TRANSMIT    1         //Kommando ist zum Senden bereit
#define LORA_PROTOCOL_STATE_WAITACK     2         //Kommando wartet auf Bestaetigung (nur wenn ACK Flag gesetzt ist)
#define LORA_PROTOCOL_STATE_IDLE        3         //derzeit laeuft keine Transaktion
#define LORA_PROTOCOL_STATE_ERROR       4         //Kommando nicht erfolgreich (ACK oder Response nicht emfangen)
#define LORA_PROTOCOL_STATE_RXDONE      5         //Kommando wurde von der Gegenseite beantwortet

LoraGateway::LoraGateway(int spiSelect, int reset, int busy, int interrupt) : SX126x(spiSelect,reset,busy,interrupt)
{
  timer  = LORA_TIMER_INACTIVE;	

#ifndef LORANODE_NO_ENCRYPT
  encryptDecryptState 	= 0;
#endif
  return;
}

LoraGateway::~LoraGateway()
{
}

void LoraGateway::begin(uint8_t taskIntervalInMs, uint8_t spreadingFactor, uint32_t frequencyInHz, int8_t txPowerInDbm) 
{
  taskInterval	            = taskIntervalInMs; 
  currTransaction.state     = LORA_PROTOCOL_STATE_IDLE;
  transactionCtxFactory     = 0;
  SX126x::begin(spreadingFactor, frequencyInHz, txPowerInDbm); 
}

void LoraGateway::LoRaConfig(uint8_t spreadingFactor, uint8_t bandwidth, uint8_t codingRate, uint16_t preambleLength, uint8_t payloadLen, bool crcOn, bool invertIrq) 
{
  SX126x::LoRaConfig(spreadingFactor, bandwidth, codingRate, preambleLength, payloadLen, crcOn, invertIrq);
}
	
void LoraGateway::ReceiveResp(void)
{
  uint8_t rxLen = 0;
 
  if ( true == ReceiveMode() )
  { 
    rxLen = SX126x::Receive((uint8_t*)&currTransaction.loraFrameRx, sizeof(LORAFRAME));
    if ( currTransaction.state < LORA_PROTOCOL_STATE_IDLE )
    {
      if ( (rxLen > sizeof(LORAHEAD)) && (rxLen <= sizeof(LORAFRAME)) )
      {
        currTransaction.frameLenRx = rxLen;
#ifndef LORANODE_NO_ENCRYPT
	EncryptDecrypt((uint8_t*)&currTransaction.loraFrameRx, (uint8_t*)&currTransaction.loraFrameRx, rxLen);
#endif
        if ( currTransaction.loraFrameRx.head.transactionContext == currTransaction.loraFrameTx.head.transactionContext )
        {
          Serial.println("WAITACK -> RXDONE");
          currTransaction.state = LORA_PROTOCOL_STATE_RXDONE;
        }
      }
    }
  }
}

uint8_t LoraGateway::GetResponseData(uint8_t** pRespData)
{
  if ( currTransaction.state == LORA_PROTOCOL_STATE_RXDONE )
  {
    *pRespData = &currTransaction.loraFrameRx.data;
    return (currTransaction.frameLenRx - sizeof(LORAHEAD));
  }
  else
  {
    *pRespData = NULL;
    return 0;
  }
}

void LoraGateway::ProcessTransaction(void)
{
#ifndef LORANODE_NO_ENCRYPT
	LORAFRAME d;
#endif
  switch ( currTransaction.state ) 
  {
    case LORA_PROTOCOL_STATE_TRANSMIT:
#ifndef LORANODE_NO_ENCRYPT
      EncryptDecrypt((uint8_t*)&currTransaction.loraFrameTx, (uint8_t*)&d, currTransaction.frameLenTx);
      if ( SX126x::Send((uint8_t*)&d, currTransaction.frameLenTx, SX126x_TXMODE_ASYNC) )
#else
      if ( SX126x::Send((uint8_t*)&currTransaction.loraFrameTx, currTransaction.frameLenTx, SX126x_TXMODE_ASYNC) )
#endif
      {
        if ( currTransaction.timeout > 0 )
        {
          Serial.println("TRANSMIT -> WAITACK");
          currTransaction.state = LORA_PROTOCOL_STATE_WAITACK;
        }
        else
        {
          Serial.println("TRANSMIT -> IDLE");
          currTransaction.state = LORA_PROTOCOL_STATE_IDLE;
        }
      }
      break;

    case LORA_PROTOCOL_STATE_WAITACK:
      if ( currTransaction.timeout > 0 )
      {
        currTransaction.timeout--;
      }
      else if ( currTransaction.nRepeat > 0 )
      {
        currTransaction.nRepeat--;
        currTransaction.timeout  = LORA_MILLISECONDS(LORA_REPEAT_INTERVAL);
        Serial.println("WAITACK -> TRANSMIT");      
        currTransaction.state = LORA_PROTOCOL_STATE_TRANSMIT;
      }
      else
      { 
        Serial.println("WAITACK -> ERROR ");
        currTransaction.state = LORA_PROTOCOL_STATE_ERROR;
      }   
      break;

    case LORA_PROTOCOL_STATE_ERROR:
      break;
      
    default:
      break;
  }
}

void LoraGateway::Timer(void)
{	
  if ( (timer != LORA_TIMER_INACTIVE) && (timer > 0) )
    timer--;
}

#ifndef LORANODE_NO_ENCRYPT
void  LoraGateway::EncryptDecrypt(uint8_t* input, uint8_t* output, uint16_t len)
{
  if ( encryptDecryptState == 0 )
  {
    memcpy(output, input, len);
    return;
  }
		
  if ( encryptDecryptState == 1 )
  {
    //wenn wir hier sind wuerde das bedeuten, dass in einem Taskzyklus 2x gesendet 
    //oder 2x empfangen oder sowohl empfangen als auch gesendet wurde. 
    //Das kann eigentlich nicht sein, denn die Statemachines im Gateway und den Endgeraeten
    //lassen das nicht zu.
    spritz_setup(&encryptDecryptCtx, (uint8_t*)&encryptDecryptKey, sizeof(encryptDecryptKey));
    encryptDecryptState = 2;
  }
  
  encryptDecryptState = 1;
  spritz_crypt(&encryptDecryptCtx, input, len, output);
}

void LoraGateway::EncryptDecryptKey(CIPHERKEY * key)
{
  encryptDecryptKey = *key;
  spritz_setup(&encryptDecryptCtx, (uint8_t*)&encryptDecryptKey, sizeof(encryptDecryptKey));
  encryptDecryptState = 2;
}
#endif

void LoraGateway::Send(uint32_t radioId, uint8_t cmdId, uint8_t *pData, uint8_t dataLen)
{
  //pruefen, ob eine Transaktion laeuft
  if ( currTransaction.state > LORA_PROTOCOL_STATE_WAITACK )
  {
    //Transaktionsszustand ist IDLE oder ERROR oder RXDONE
    transactionCtxFactory++;
    currTransaction.frameLenTx  = sizeof(LORAHEAD) + dataLen;
    currTransaction.nRepeat     = 3;
    currTransaction.timeout     = LORA_MILLISECONDS(LORA_REPEAT_INTERVAL);
    currTransaction.state       = LORA_PROTOCOL_STATE_TRANSMIT;

    currTransaction.loraFrameTx.head.radioId            = radioId;
    currTransaction.loraFrameTx.head.transactionContext = transactionCtxFactory;
    currTransaction.loraFrameTx.head.cmdId              = cmdId;
    currTransaction.loraFrameTx.head.status             = LORA_STATUS_SUCCESS;
    
    memcpy(&currTransaction.loraFrameTx.data, pData, dataLen);
  }
}

void LoraGateway::Service(void) 
{
#ifndef LORANODE_NO_ENCRYPT
  if ( encryptDecryptState == 1 )
  {
    spritz_setup(&encryptDecryptCtx, (uint8_t*)&encryptDecryptKey, sizeof(encryptDecryptKey));
    encryptDecryptState = 2;
  }
#endif
  Timer();
  ReceiveResp();
  ProcessTransaction();
}
