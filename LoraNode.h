#ifndef _LORANODE_H
#define _LORANODE_H
#include "LoraCommon.h"
#include <SX126x.h>
#include "QList.h"

#ifndef LORANODE_NO_ENCRYPT
	#include <SpritzCipher.h>
#endif


#define DEVICE_CLASS_MASK                            0xFF000000
#define DEVICE_TYPE_MASK                             0x00FF0000
#define DEVICE_ADDRESS_MASK                          0x0000FFFF

#define DEVICE_CLASS_CONTROLLER                      0x01000000
#define DEVICE_CLASS_AKTOR                           0x02000000
#define DEVICE_CLASS_SENSOR                          0x03000000
#define DEVICE_CLASS_AKTORSENSOR                     0x04000000
#define DEVICE_CLASS_INFRASTRUCTURE                  0x05000000

#define DEVICE_TYPE_HUT4C                            0x00010000
#define DEVICE_TYPE_HUT6C                            0x00020000
#define DEVICE_TYPE_INWALL                           0x00030000
#define DEVICE_TYPE_BRIDGE                           0x00040000
#define DEVICE_TYPE_GATEWAY                          0x00050000


typedef struct _tagLORATRANSACTION    LORATRANSACTION;
typedef struct _tagCallbackElem       CALLBACK_ELEM;

typedef void(*cbLORA_ACTOR_REQ)				(uint16_t id, uint8_t action);
typedef void(*cbLORA_SENSOR_REQ)			(uint8_t startId, uint8_t nSensors);
typedef void(*cbLORA_EEPWRITE_REQ)		(void);										
typedef void(*cbLORA_EEPREAD_REQ)			(void);											
typedef void(*cbLORA_PROPERTY_REQ)		(void);						


class LoraNode : private SX126x
{
  public:
    LoraNode(int a, int b, int c, int d);
    ~LoraNode();
    
    void begin(uint8_t taskIntervalInMs, uint8_t spreadingFactor, uint32_t frequencyInHz, int8_t txPowerInDbm);  
    void LoRaConfig(uint8_t spreadingFactor, uint8_t bandwidth, uint8_t codingRate, uint16_t preambleLength, uint8_t payloadLen, bool crcOn, bool invertIrq);
		void Service(void);
 		void RegisterCallback(uint32_t radioId, LORACMDID cmdId, void* callback);
		
		void Send_LORA_ACTOR_RESP			(uint8_t status);
		void Send_LORA_SENSOR_RESP		(SENSOR_RESP *pResp);
		void Send_LORA_EEPWRITE_RESP	(void);
		void Send_LORA_EEPREAD_RESP		(void);
		void Send_LORA_PROPERTY_RESP	(void);
#ifndef LORANODE_NO_ENCRYPT
		void 		EncryptDecrypt					(uint8_t* input, uint8_t* output, uint16_t len);
		void 		EncryptDecryptKey				(CIPHERKEY * key);
#endif

 	private:
 		uint32_t 												ownRadioId;
 		LORAFRAME												dataFrame;
    uint8_t                         frameLen;
 		uint8_t													currentTransactionContext;
 		
 		uint16_t												timer;
 		uint8_t													taskInterval;						//intervall in dem die Funktion Service aufgerufen wird, wichtig zur Berechnugn der Timer
#ifndef LORANODE_NO_ENCRYPT
		CIPHERKEY												encryptDecryptKey;
		uint8_t													encryptDecryptState;		//0: no encryption, 1: setup required, 2: encrypt prepared
		spritz_ctx											encryptDecryptCtx;
#endif	
    QList<CALLBACK_ELEM>            cbList;
	
 		cbLORA_ACTOR_REQ								*onLORA_ACTOR_REQ; 
 		cbLORA_SENSOR_REQ								*onLORA_SENSOR_REQ;
 		cbLORA_EEPWRITE_REQ 						*onLORA_EEPWRITE_REQ;
 		cbLORA_EEPREAD_REQ 							*onLORA_EEPREAD_REQ;
 		cbLORA_PROPERTY_REQ 						*onLORA_PROPERTY_REQ;
	  
		void 		QueueResponse						(LORACMDID cmdId, void* pData, uint16_t len);
		void 		Send										(void);
		void 		ReceiveReq							(void);
		void 		Timer										(void);
};
#endif //_LORANODE_H
