#ifndef _LORACOMMON_H
#define _LORACOMMON_H

#define LORANODE_NO_ENCRYPT						  //Kommentar entfernen, wenn keine Verschluesselung genwuenscht ist

    
#define LORA_SECONDS(x)             		((x) * (1000/taskInterval))
#define LORA_MILLISECONDS(x)        		((x) / taskInterval)
#define LORA_TIMER_INACTIVE							0xFFFF


#define LORA_STATUS_SUCCESS                     0
#define LORA_STATUS_CMD_NOTSUPPORTED            1
#define LORA_STATUS_CHANNEL_NOTSUPPORTED        3
#define LORA_STATUS_DEVICE_NOTSUPPORTED         4
#define LORA_STATUS_DEVICE_NOTCONNECTED         5
#define LORA_STATUS_BAD_DEVICEID                6
#define LORA_STATUS_OUTDATED                    7


typedef enum _tagLORAMDID
{
	//Requests immer ungerade, Responses immer gerade Zahl
	//Diese Festlegung wird benutzt, um Gateway-Funktionen und Node-Funktionen 
	//auseinander zu halten
	LORA_ACTOR_REQ				= 0x01,
	LORA_ACTOR_RESP				= 0x02,
	LORA_SENSOR_REQ				= 0x03,
	LORA_SENSOR_RESP			= 0x04,
	LORA_EEPWRITE_REQ			= 0x05,
	LORA_EEPWRITE_RESP		= 0x06,
	LORA_EEPREAD_REQ			= 0x07,
	LORA_EEPREAD_RESP			= 0x08,
	LORA_PROPERTY_REQ			= 0x09,
	LORA_PROPERTY_RESP		= 0x0A
}LORACMDID;

//GW
typedef struct _tagLORAHEAD
{
  uint32_t radioId;                              //id des Empfaengers 
  uint8_t  transactionContext;                   //wird vom Gateway fuer jeden Request neu vergeben und muss zurueckgeschickt werden, falls eine Antwort erwartet wird
  uint8_t  cmdId;                                //LORACMD, LORAACK, LORABRIDGE etc.
  uint8_t  status;                               //ACK etc                                                        
}LORAHEAD;


typedef struct _tagACTOR_REQ
{
  uint16_t			id; 	
	uint8_t				action; 	                            //0:off  1:on  2:pulseUp  3:pulseDown                          					                                                       
}ACTOR_REQ;


typedef struct _tagACTOR_RESP
{
	uint8_t				status; 	                            					                                                       
}ACTOR_RESP;

typedef struct _tagSENSOR_REQ
{
	uint8_t				startId;
  uint8_t       nSensors;   //Anzahl der abzufragenden Sensoren	                            					                                                       
}SENSOR_REQ;

typedef struct _tagSENSOR_ELEM
{
  int8_t        id;
  int16_t       value;
}SENSOR_ELEM;

typedef struct _tagSENSOR_RESP
{
	uint8_t				nSensors;
  SENSOR_ELEM   sensor[6];      //1..6 Sensorwerte					                                                       
}SENSOR_RESP;


typedef struct _tagFRAME
{
  LORAHEAD 			head;
  union 
  {	
  	uint8_t			data;
  	ACTOR_REQ		actorReq;
  	ACTOR_RESP	actorResp;
    SENSOR_REQ  sensorReq;
    SENSOR_RESP sensorResp;
  };                      					                                                       
}LORAFRAME;


typedef struct _tagCipherKey
{
	uint8_t key[3];
}CIPHERKEY;

#endif //_LORACOMMON_H
