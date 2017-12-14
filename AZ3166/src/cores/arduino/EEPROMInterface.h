// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef _EEPROM_INTERFACE_
#define _EEPROM_INTERFACE_

#include "mbed.h"

#define WIFI_SSID_ZONE_IDX      0x03
#define WIFI_PWD_ZONE_IDX       0X0A
#define AZ_IOT_HUB_ZONE_IDX     0X05

#define WIFI_SSID_MAX_LEN       32
#define WIFI_PWD_MAX_LEN        64
#define AZ_IOT_HUB_MAX_LEN      200
#define EEPROM_DEFAULT_LEN		200

/**
 * \brief Write/Read data to/from EEPROM of STSAFE_A100 through I2C interface.
 * 
 */
class EEPROMInterface
{
public:
	EEPROMInterface();
	~EEPROMInterface();
    
	int write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int read(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
	void enableHostSecureChannel();
private:
	void *handle = NULL;
	FLASH_AdvOBProgramInitTypeDef pAdvOBInit;
	bool PCROPCheck(int sector);
	bool checkZoneSize(int dataZoneIndex, int &size, bool write);
	bool isHostSecureChannelEnabled();
	int writeWithEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int writeWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int readWithEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
	int readWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
};

#endif /* _EEPROM_INTERFACE_ */
