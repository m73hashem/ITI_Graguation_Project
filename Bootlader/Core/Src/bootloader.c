
/**************************include section start****************************/
#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "usart.h"
#include "crc.h"
#include "bootloader.h"

/**************************include section end******************************/
static void Bootloader_Send_ACK(uint8_t Replay_Len);
static void Bootloader_Send_NACK(void);
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);
static uint8_t Host_Address_Verification(uint32_t Jump_Address);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer);
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len);
static void bootloader_jump_to_user_app(void);
static uint8_t HOST_BUFFER[BL_HOST_BUFFER_RX_LENGTH];


/***
 * @brief -> this function is used after flashing a new firmware (application)
 * 			to jump from bootloader main into the application resethandler (entry of the application startupcode)
 */
static void bootloader_jump_to_user_app(void){
	/* Value of the main stack pointer of our main application */
	/*not required for now*/
	//uint32_t MSP_Value = *((volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS);

	/* Reset Handler definition function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4));

	/* Fetch the reset handler address of the user application */
	pMainApp ResetHandler_Address = (pMainApp)MainAppAddr;
	/* Set Main Stack Pointer (it is not needed step the hardware on once you shift the ivt it init the msp by default after reset)*/
	//__set_MSP(MSP_Value);

	/* DeInitialize / Disable of modules */
	HAL_RCC_DeInit(); /* DeInitialize the RCC clock configuration to the default reset state. */
	/* Disable Maskable Interrupt */

	/* Jump to Application Reset Handler */
	ResetHandler_Address();
}
/*
 *@brief this function is used to send strings to host from BOOTLOADER through uart
 *@param string the string i want to print
 *@retval  HAL_StatusTypeDef return status (OK,ERROR,BUSY..etc)

 */
Std_ReturnType BOOTLOADER_PRINT(uint8_t * string,... )
{
	Std_ReturnType Status=HAL_ERROR;
	/*create a local buffer to save the string */
	uint8_t Local_Buff[100]={0};
	va_list args;
	va_start(args,string);
	/*save the string into a local buffer*/
	vsprintf((char *)Local_Buff,(char *)string,args);
	/*uart send this string*/
	Status=HAL_UART_Transmit(DEBUGGING_UART,(uint8_t *)Local_Buff,sizeof(Local_Buff),HAL_MAX_DELAY);
	va_end(args);

	return Status;
}
/***********************************************************************************************/
/***
 * @brief -> this function is used to receive command from (HOST) (EARSEFLASH-WRITEONFLASH-GETBLVERSION)
 * @retval ->
 */
HAL_StatusTypeDef BOOTLOADER_RECEIVE_COMMAND(void)
{
	memset(HOST_BUFFER ,0,100 ) ;
	uint8_t Data_Length = 0;
	uint8_t COMMAND=0;
	BL_Status Status = BL_NACK;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	/*receiving the incoming frame length(first byte of the frame always the length of the rest of frame) */
	HAL_Status = HAL_UART_Receive(COMMUNICATION_UART, HOST_BUFFER, 1, HAL_MAX_DELAY);
	/*check if uart receiving frame length process is oki*/
	if(HAL_Status != HAL_OK){
		HAL_Status = HAL_ERROR;
	}
	else{
		/*extract the rest frame length into a external variable*/
		Data_Length = HOST_BUFFER[0];

#if BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE
		BOOTLOADER_PRINT("You Received %d\r\n",Data_Length);
#endif
		HAL_Status = HAL_UART_Receive(COMMUNICATION_UART, &HOST_BUFFER[1], Data_Length, HAL_MAX_DELAY);
		if(HAL_Status != HAL_OK){

#if BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE

			BOOTLOADER_PRINT("Bootloader Received Wrong Frame. \r\n");
#endif
			Status = HAL_ERROR;
		}
		else{
			/* Read the command packet received from the HOST */
			COMMAND=HOST_BUFFER[1];
			switch(COMMAND)
			{

			case BOOT_LOADER_GET_VERSION:
#if BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE
				Status=BOOTLOADER_PRINT("You Received Get Version 0x%x\r\n",COMMAND);
#endif
				BOOTLOADER_SEND_VERSION_INFO(HOST_BUFFER);
				break;
			case BOOT_LOADER_GO_TO_ADDR_CMD:
				Bootloader_Jump_To_Address(HOST_BUFFER);
				Status = BL_OK;
				break;
			case BOOT_LOADER_FLASH_ERASE_CMD:
				Bootloader_Erase_Flash(HOST_BUFFER);
				Status = BL_OK;
				break;
			case BOOT_LOADER_MEM_WRITE_CMD:
				Bootloader_Memory_Write(HOST_BUFFER);
				Status = BL_OK;
				break;
			default:
				BOOTLOADER_PRINT((uint8_t*)"Invalid command code received from host !! \r\n");
				break;

			}
		}
	}

	return Status;
}
/***
 * @brief this function is used to send some basic info about bootloader
 *  	  1-MAJOR_VERSION  2-MINOR_VERSION  3-VENDOR_ID or the chip id
 */
void BOOTLOADER_SEND_VERSION_INFO(uint8_t *HOST_BUFF)
{
	/*the boot loader version inforamtion*/
	uint8_t BL_Version[4] = {MAJOR_VERSION, MINOR_VERSION,VENDOR_ID};
	uint32_t Host_CRC32=0;
	/*calculate total frame length to calculate crc*/
	uint8_t Host_CMD_Packet_Len=HOST_BUFF[0] + 1;
	/*get the crc calculated by the host*/
	Host_CRC32 = *((uint32_t *)((HOST_BUFF + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/*calling the bootloader crc calculator api and then compare it to the received crc from host*/
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&HOST_BUFF[0] , Host_CMD_Packet_Len - 4, Host_CRC32))
	{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Passed \r\n");
#endif
		/*sending acknowledge and send to host that bootloader information is 4 bytes so receive the 4 bytes*/
		Bootloader_Send_ACK(4);
		Bootloader_Send_Data_To_Host((uint8_t *)(&BL_Version[0]), 4);
		/*debugging info through uart2 if you want to insure that bl Functioning on required way*/
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE)
		BOOTLOADER_PRINT("BOOTLOADER MAJOR VERSION IS-> %u",BL_Version[0]);
		BOOTLOADER_PRINT("BOOTLOADER MINOR VERSION IS-> %u",BL_Version[1]);
		BOOTLOADER_PRINT("VENDOR ID IS -> 0x%x",VENDOR_ID);
#endif

	}
	else
	{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
	/*it is here for testing after flashing purpose*/
	bootloader_jump_to_user_app();
}


/****************************************************************************************************/
/***
 * @brief this function is used to write data on the flash
 * @param *Host_Buffer -> pointer to the frame array which contain the data that is gonna be flashed
 */
void Bootloader_Memory_Write(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	/*creating a variable receive the start address that we gonna start flash from it*/
	uint32_t HOST_Address = 0;
	/*the length of payload in one time
	 * for example if hex file 1000 byte and we could flash up to 128 byte per calling
	 * so we gonna need 1000/128 = so gonna call the flash function 8 times
	 * and last time payload length may be less than 128 so we should know the length of data
	 * to extract it from data buffer */

	uint8_t Payload_Len = 0;
	/*status that the sended address is valid place to flash a firmware in it*/
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	/*status that writing on flash is done on required way*/
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;


#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
	BOOTLOADER_PRINT("Write data into different memories of the MCU \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("HOST_Address = 0x%X \r\n", HOST_Address);
#endif
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[6];
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(HOST_Address);
		if(ADDRESS_IS_VALID == Address_Verification)
		{
			/* Write the payload to the Flash memory */
			Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);
			if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status)
			{
				/*(like ACK) Report payload write passed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
				BOOTLOADER_PRINT("Payload Valid \r\n");
#endif
			}
			else
			{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
				BOOTLOADER_PRINT("Payload InValid \r\n");
#endif
				/* Report payload write failed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
			}
		}
		else
		{
			/* Report address verification failed */
			Address_Verification = ADDRESS_IS_INVALID;
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
	}
	else
	{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}
}
/*****************************************************************************************************/
/**
 * @brief -> this function is used to jump at aspecific address (like functions on application)
 */
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint32_t HOST_Jump_Address = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
	BOOTLOADER_PRINT("Jump bootloader to specified address \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(1);
		/* Extract the address form the HOST packet */
		HOST_Jump_Address = *((uint32_t *)&Host_Buffer[2]);
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(HOST_Jump_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
			BOOTLOADER_PRINT("Address verification succeeded \r\n");
#endif
			/* Report address verification succeeded */
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
			/* Prepare the address to jump */
			Jump_Ptr Jump_Address = (Jump_Ptr)(HOST_Jump_Address + 1);
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
			BOOTLOADER_PRINT("Jump to : 0x%X \r\n", Jump_Address);
#endif
			Jump_Address();
		}
		else{
			/* Report address verification failed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
	}
	else{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}


/****************************************************************************************************/
/***
 * @brief this function is used to check if crc received by host is the same crc
 * 		  that calulate on the received frame
 */
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	/*calculated crc value on received data*/
	uint32_t MCU_CRC_Calculated = 0;
	/*counter to loop on the frame received bytes*/
	uint8_t Data_Counter = 0;
	/*a buffer to extract the data from the array frame*/
	uint32_t Data_Buffer = 0;
	/* Calculate CRC32 */
	for(Data_Counter = 0; Data_Counter < Data_Len; Data_Counter++){
		Data_Buffer = (uint32_t)pData[Data_Counter];
		/*calling the CRC api to calculate the crc on the received frame */
		MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, &Data_Buffer, 1);
	}
	/* Reset the CRC Calculation Unit */
	__HAL_CRC_DR_RESET(CRC_ENGINE_OBJ);
	/* Compare the Host CRC and Calculated CRC */
	if(MCU_CRC_Calculated == Host_CRC){
		CRC_Status = CRC_VERIFICATION_PASSED;
	}
	else{
		CRC_Status = CRC_VERIFICATION_FAILED;
	}

	return CRC_Status;
}

/****************************************************************************************************/
/***
 * @brief this function is used to send ack to host and the replied length if
 *  	 host asked a data like( get version info)
 *@param Replay_Len-> thelength of data that host asked bootloader for it
 */
static void Bootloader_Send_ACK(uint8_t Replay_Len){
	uint8_t Ack_Value[2] = {0};
	Ack_Value[0] = CBL_SEND_ACK;
	Ack_Value[1] = Replay_Len;
	HAL_UART_Transmit(COMMUNICATION_UART, (uint8_t *)Ack_Value, 2, HAL_MAX_DELAY);
}
/***
 * @brief this function is used to send not acknowlegment to the host
 */
static void Bootloader_Send_NACK(void){
	uint8_t Ack_Value = CBL_SEND_NACK;
	HAL_UART_Transmit(COMMUNICATION_UART, &Ack_Value, 1, HAL_MAX_DELAY);
}
/***
 * @brief this function is used to send data to the host through uart
 * @param Host_Buffer -> the data that bootloader gonna send
 * @param Data_Len -> the length of this data
 */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len){
	HAL_UART_Transmit(COMMUNICATION_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}

/***************************************************************************/
/***
 * @brief this function is used to verify that the address that host want bootloader
 * 		  to flash firmware on it is valid (this function is used to verify flash only
 * 		  you could add sram address interval if you want to execute from flash
 * 		  also used to check if address that host asked to jump is valid or not
 * @param  Jump_Address -> the address that host send it
 */
static uint8_t Host_Address_Verification(uint32_t Jump_Address){
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	/*check if the address is located in flash intervals*/
	if((Jump_Address >= FLASH_BASE) && (Jump_Address <= STM32F401_FLASH_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}

/***************************************************************************/
/***
 * @brief this function is a helper function to (Bootloader_Erase_Flash() )used to erase the flash
 * @param Sector_Number -> start erase from this sector
 * @param Number_Of_Sectors-> how many sectors you want to erase
 * @retval Sector_Validity_Status-> return the status of erasing operation
 */
static uint8_t Perform_Flash_Erase(uint8_t Sector_Numebr, uint8_t Number_Of_Sectors){
	uint8_t Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;	/*a struct gonna take the erasing information like Mass erase or sector Erase.
	 * Bank number , start secotr , number of sectors*/
	FLASH_EraseInitTypeDef pEraseInit;
	/*a variable used to verify number of sector sended by host
	 * =  (number of sectors - startsector)*/

	uint8_t Remaining_Sectors = 0;
	/*flag status of sector erasing errors*/
	uint32_t SectorError = 0;
	/* check if Number Of sectors is out of range
	 * incase host send that he want to erase 12 sector
	 * and my flash has only 8 sector so it is out of range*/

	if(Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER){
		Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	}
	else{
		/*check if start sector is less than the maximum flash sector number or the host asked to erase the whole flash*/
		if((Sector_Numebr <= (CBL_FLASH_MAX_SECTOR_NUMBER - 1)) || (CBL_FLASH_MASS_ERASE == Sector_Numebr)){
			/* Check if user needs Mass erase */
			if(CBL_FLASH_MASS_ERASE == Sector_Numebr){
				/* Flash Mass erase activation */
				pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
				BOOTLOADER_PRINT("Flash Mass erase activation \r\n");
#endif
			}
			else{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
				BOOTLOADER_PRINT("User needs Sector erase \r\n");
#endif
				/* User needs Sector erase */
				/*calculate the remaining sectors and check if it is less of flash sector number*/

				Remaining_Sectors = CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Numebr;
				if(Number_Of_Sectors > Remaining_Sectors){
					/*if the remaining sector is less than the whole flash sectors
					 * so make the number of sectors gonna be erased is the remaining sectors*/
					Number_Of_Sectors = Remaining_Sectors;
				}
				else { /* Nothing */ }

				pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS; /* Sectors erase only */
				pEraseInit.Sector = Sector_Numebr;        /* Initial FLASH sector to erase when Mass erase is disabled */
				pEraseInit.NbSectors = Number_Of_Sectors; /* Number of sectors to be erased. */
			}

			pEraseInit.Banks = FLASH_BANK_1; /* Bank 1  */
			pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; /* Device operating range: 2.7V to 3.6V */

			/* Unlock the FLASH control register access */
			HAL_Status = HAL_FLASH_Unlock();
			/* Perform a mass erase or erase the specified FLASH memory sectors */
			HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
			if(HAL_SUCCESSFUL_ERASE == SectorError){
				Sector_Validity_Status = SUCCESSFUL_ERASE;

			}
			else{
				Sector_Validity_Status = UNSUCCESSFUL_ERASE;
				HAL_Status=HAL_ERROR;
			}
			/* Locks the FLASH control register access */
			HAL_Status = HAL_FLASH_Lock();

		}
		else{
			Sector_Validity_Status = UNSUCCESSFUL_ERASE;

		}
	}
	return Sector_Validity_Status;
}
/***********************************************************************/
/***
 * @brief this function is used to erase flash
 * @param ->Host_Buffer -> the array that contatin the received command frame from host
 *
 */
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint8_t Erase_Status = 0;

#if (BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE)
	BOOTLOADER_PRINT("Mass erase or sector erase of the user flash \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUGGING_MODE_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Perform Mass erase or sector erase of the user flash */
		/*Host_Buffer[2]->Start_Sector , Host_Buffer[3]-> number of sectors*/
		Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		if(SUCCESSFUL_ERASE == Erase_Status){
			/* Report erase Passed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
			BOOTLOADER_PRINT("Successful Erase \r\n");
#endif
		}
		else{
			/* Report erase failed */
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
			BOOTLOADER_PRINT("Erase request failed !!\r\n");
#endif
		}
	}
	else{
#if (BOOT_LOADER_DEBUGGING_MODE == DEBUG_INFO_ENABLE)
		BOOTLOADER_PRINT("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}
}
/********************************************************************************/
/***
 * @brief this function is a helper function for (Bootloader_Memory_Write()) used to write on flash
 * @param Host_Payload-> the data that host asked to write it on flash
 * @param Payload_Start_Address-> where the host asked bootloader to write this data
 * @param Payload_Len-> the length of this data
 * @retval Flash_Payload_Write_Status-> the status of the writing operation
 */
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len){
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_Counter = 0;

	/* Unlock the FLASH control register access */
	HAL_Status = HAL_FLASH_Unlock();

	if(HAL_Status != HAL_OK){
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else{
		/*looping to write the whole payload (128 byte , 64byte , 256 byte depend on configuration of flash api (HAL_FLASH_Program())down there*/
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++){
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Payload_Start_Address + Payload_Counter, Host_Payload[Payload_Counter]);
			if(HAL_Status != HAL_OK){
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
				break;
			}
			else{
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
			}
		}
	}

	if((FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) && (HAL_OK == HAL_Status)){
		/* Locks the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock();
		if(HAL_Status != HAL_OK){
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
		}
		else{
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
		}
	}
	else{
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}

	return Flash_Payload_Write_Status;
}


