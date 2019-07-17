#ifndef _LORAGATEWAY_H
#define _LORAGATEWAY_H
#include "LoraCommon.h"
#include <SX126x.h>
#include <SpritzCipher.h>


typedef struct _tagLORATRANSACTION
{
  LORAFRAME       	loraFrameTx;
  LORAFRAME       	loraFrameRx;
  uint16_t              timeout;
  uint16_t              state;
  uint8_t               nRepeat;
  uint8_t               frameLenTx;
  uint8_t               frameLenRx;
  uint8_t               usrContext;
}LORATRANSACTION;


class LoraGateway : private SX126x
{
  public:
    LoraGateway(int spiSelectPin, int resetPin, int busyPin, int interruptPin);
    ~LoraGateway();
    
    void begin                  (uint8_t taskIntervalInMs, uint8_t spreadingFactor, uint32_t frequencyInHz, int8_t txPowerInDbm);  
    void LoRaConfig             (uint8_t spreadingFactor, uint8_t bandwidth, uint8_t codingRate, uint16_t preambleLength, uint8_t payloadLen, bool crcOn, bool invertIrq);
    void Service                (void);
    void Send                   (uint32_t radioId, uint8_t cmdId, uint8_t *data, uint8_t len);
    uint8_t GetResponseData     (uint8_t **pRespData);
#ifndef LORANODE_NO_ENCRYPT
    void EncryptDecryptKey	(CIPHERKEY * key);
#endif
	
  private:
    LORATRANSACTION		currTransaction;
    uint8_t                     transactionCtxFactory;

    uint16_t			timer;
    uint8_t		    	taskInterval;  //intervall in dem die Funktion Service aufgerufen wird, wichtig zur Berechnugn der Timer
#ifndef LORANODE_NO_ENCRYPT
    CIPHERKEY			encryptDecryptKey;
    uint8_t			encryptDecryptState;		//0: no encryption, 1: setup required, 2: encrypt prepared
    spritz_ctx			encryptDecryptCtx;
#endif

    void ReceiveResp		(void);		
    void Timer			(void);
    void ProcessTransaction	(void);
#ifndef LORANODE_NO_ENCRYPT
    void EncryptDecrypt		(uint8_t* input, uint8_t* output, uint16_t len);
#endif
};
#endif //_LORAGATEWAY_H
