#include "Arduino.h"
#include "LoraNode.h"
#include "QList.h"

//Todo: Jeder Node kann immer nur einem Gateway zugeordnet sein.
//Bisher ist diese Zuordnung per #define fest im Code verankert.
//Spaeter koennte diese Zuordnung per Eeprom-Konfiguration festgelegt werden.
#define  DEVICE_LORA_GATEWAY            (DEVICE_CLASS_INFRASTRUCTURE | DEVICE_TYPE_GATEWAY  | 0x0001) //LoRa Gateway

#define LORA_RESP_WAITTIME              40  //Zeit in ms zwischen Empfang eines Lora Command Frames und dem Versenden des ACK Frames

typedef struct _tagCallbackElem
{
  uint32_t  radioId;
  void *    cbFunction;
  uint8_t   cmdId;
}CALLBACK_ELEM;

LoraNode::LoraNode(int spiSelect, int reset, int busy, int interrupt) : SX126x(spiSelect,reset,busy,interrupt)
{ 
	timer									= LORA_TIMER_INACTIVE;
	frameLen              = 0;
	onLORA_ACTOR_REQ 			= NULL;
	onLORA_ACTOR_REQ 			= NULL; 
 	onLORA_SENSOR_REQ 		= NULL;
 	onLORA_EEPWRITE_REQ 	= NULL;						
	onLORA_EEPREAD_REQ 		= NULL;					
	onLORA_PROPERTY_REQ 	= NULL;					

#ifndef LORANODE_NO_ENCRYPT
	encryptDecryptState 	= 0;
#endif
	return;
}

LoraNode::~LoraNode()
{
}

void LoraNode::begin(uint8_t taskIntervalInMs, uint8_t spreadingFactor, uint32_t frequencyInHz, int8_t txPowerInDbm) 
{
	taskInterval 							= taskIntervalInMs; 
	currentTransactionContext = 0;
	SX126x::begin(spreadingFactor, frequencyInHz, txPowerInDbm); 
}

void LoraNode::LoRaConfig(uint8_t spreadingFactor, uint8_t bandwidth, uint8_t codingRate, uint16_t preambleLength, uint8_t payloadLen, bool crcOn, bool invertIrq) 
{
	SX126x::LoRaConfig(spreadingFactor, bandwidth, codingRate, preambleLength, payloadLen, crcOn, invertIrq);
}

void LoraNode::RegisterCallback(uint32_t radioId, LORACMDID cmdId, void* cb)
{
  CALLBACK_ELEM cbElem;
  cbElem.radioId = radioId;
  cbElem.cmdId = cmdId;
  cbElem.cbFunction = cb;
  
  cbList.push_back(cbElem);
}

void LoraNode::Send_LORA_ACTOR_RESP(uint8_t status)
{
	ACTOR_RESP data;
	data.status = status;
	QueueResponse(LORA_ACTOR_RESP, (void*)&data, sizeof(data));
}

void LoraNode::Send_LORA_SENSOR_RESP(SENSOR_RESP *pResp)
{
  uint8_t len = sizeof(pResp->nSensors);
  len += pResp->nSensors * sizeof(SENSOR_ELEM);
	QueueResponse(LORA_EEPREAD_RESP, pResp, len);
}	
						
void LoraNode::Send_LORA_EEPWRITE_RESP(void)
{
	QueueResponse(LORA_EEPREAD_RESP, NULL, 0);
}	
					
void LoraNode::Send_LORA_EEPREAD_RESP(void)
{
	QueueResponse(LORA_EEPREAD_RESP, NULL, 0);
}		
			
void LoraNode::Send_LORA_PROPERTY_RESP(void)
{
	QueueResponse(LORA_EEPREAD_RESP, NULL, 0);
}

void LoraNode::QueueResponse(LORACMDID cmdId, void* pData, uint16_t len)
{
	if ( !pData || (0 == len) )
		return;
	
  frameLen = sizeof(LORAHEAD) + len;
  //Todo: Jeder Node kann immer nur einem Gateway zugeordnet sein.
  //Bisher ist diese Zuordnung per #define fest im Code verankert.
  //Spaeter koennte diese Zuordnung per Eeprom-Konfiguration festgelegt werden.
	dataFrame.head.radioId = DEVICE_LORA_GATEWAY;
	dataFrame.head.cmdId = cmdId;
	dataFrame.head.transactionContext = currentTransactionContext;
  memcpy(&dataFrame.data, pData, len);
	timer = LORA_MILLISECONDS(LORA_RESP_WAITTIME);
}

void LoraNode::ReceiveReq(void)
{
  CALLBACK_ELEM cbElem;
	uint8_t rxLen = 0;
  bool found = false;

  if ( true == ReceiveMode() )
  {  
    rxLen = SX126x::Receive((uint8_t*)&dataFrame, sizeof(LORAFRAME));
    if ( (rxLen >= 4) && (rxLen <= sizeof(LORAFRAME)) )
    {
#ifndef LORANODE_NO_ENCRYPT
			EncryptDecrypt((uint8_t*)&dataFrame, (uint8_t*)&dataFrame, rxLen);
#endif
      
      for(int i=0;i<cbList.size();i++)
      {
        cbElem = cbList.at(i);
        if ( cbElem.radioId == dataFrame.head.radioId )
          if ( cbElem.cmdId == dataFrame.head.cmdId )
          { 
            found = true;
            break;
          }
      }
   
    	if ( found )
    	{
    		currentTransactionContext = dataFrame.head.transactionContext;
 
    		switch ( dataFrame.head.cmdId )
    		{	
	    		case LORA_ACTOR_REQ:
      			((cbLORA_ACTOR_REQ)cbElem.cbFunction)(dataFrame.actorReq.id, dataFrame.actorReq.action);
	    			break;
	    		
	    		case LORA_SENSOR_REQ:
						((cbLORA_SENSOR_REQ)cbElem.cbFunction)(dataFrame.sensorReq.startId, dataFrame.sensorReq.nSensors);
	    			break;
	    			
	    		case LORA_EEPWRITE_REQ:
						((cbLORA_EEPWRITE_REQ)cbElem.cbFunction)();
	    			break;
	    			
	    		case LORA_EEPREAD_REQ:
            ((cbLORA_EEPREAD_REQ)cbElem.cbFunction)();
	    			break;
	    			
	    		case LORA_PROPERTY_REQ:
	    			((cbLORA_PROPERTY_REQ)cbElem.cbFunction)();
	    			break;
	    		
					default:
	    			break;
    		}
    	}
    }
  }
}

void LoraNode::Send(void)
{	
#ifndef LORANODE_NO_ENCRYPT
	LORAFRAME d;
#endif

	if ( timer == 0 )
	{	
#ifndef LORANODE_NO_ENCRYPT
		EncryptDecrypt((uint8_t*)&dataFrame, (uint8_t*)&d, frameLen);
		SX126x::Send((uint8_t*)&d, frameLen, SX126x_TXMODE_ASYNC);
#else
		SX126x::Send((uint8_t*)&dataFrame, frameLen, SX126x_TXMODE_ASYNC);
#endif
		timer = LORA_TIMER_INACTIVE;
	}
}

void LoraNode::Timer(void)
{	
	if ( (timer != LORA_TIMER_INACTIVE) && (timer > 0) )
		timer--;
}

#ifndef LORANODE_NO_ENCRYPT
void  LoraNode::EncryptDecrypt(uint8_t* input, uint8_t* output, uint16_t len)
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

void LoraNode::EncryptDecryptKey(CIPHERKEY * key)
{
	encryptDecryptKey = *key;
	spritz_setup(&encryptDecryptCtx, (uint8_t*)&encryptDecryptKey, sizeof(encryptDecryptKey));
	encryptDecryptState = 2;
}
#endif
void LoraNode::Service(void) 
{
#ifndef LORANODE_NO_ENCRYPT
	if ( encryptDecryptState == 1 )
	{
		spritz_setup(&encryptDecryptCtx, (uint8_t*)&encryptDecryptKey, sizeof(encryptDecryptKey));
		encryptDecryptState = 2;
	}
#endif
	Timer();
	ReceiveReq();
	Send();
}