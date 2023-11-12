 
#include "stdint.h"
#include "stm32f4xx_hal_def.h"
/*********************macro decleration section start*********************************/
/*received data buffer length*/
#define BL_HOST_BUFFER_RX_LENGTH     200
/*debugging mode using uart2 */
#define DEBUGGING_MODE_ENABLE      1U
#define DEBUGGING_MODE_DISABLE     2U

/*********************boot loader configuration start*********************************/
#define BOOT_LOADER_DEBUGGING_MODE  DEBUGGING_MODE_DISABLE
/*using two uart one for communicating with host and the uart2
 * is used to debugging the bootloader operations */
#define DEBUGGING_UART     	   &huart2
#define COMMUNICATION_UART     &huart1


//get version data
#define DBGMCU_IDCODE  (*((uint32_t*)(0xE0042000)))

#define MAJOR_VERSION           1
#define MINOR_VERSION           1
#define VENDOR_ID    ((uint16_t)(DBGMCU_IDCODE&0x00000FFF))



/* CRC_VERIFICATION */
#define CRC_ENGINE_OBJ               &hcrc

#define CRC_TYPE_SIZE_BYTE           4

#define CRC_VERIFICATION_FAILED      0x00
#define CRC_VERIFICATION_PASSED      0x01

#define CBL_SEND_NACK                0xAB
#define CBL_SEND_ACK                 0xCD


/* CBL_MEM_WRITE_CMD */
#define FLASH_PAYLOAD_WRITE_FAILED   0x00
#define FLASH_PAYLOAD_WRITE_PASSED   0x01

#define FLASH_LOCK_WRITE_FAILED      0x00
#define FLASH_LOCK_WRITE_PASSED      0x01



#define STM32F401_FLASH_SIZE         (256 * 1024)
#define FLASH_BASE                   0x08000000UL
#define FLASH_SECTOR2_BASE_ADDRESS   0x08008000UL
#define STM32F401_FLASH_END          (FLASH_BASE + STM32F401_FLASH_SIZE)

/*check jumping or writing address*/
#define ADDRESS_IS_INVALID           0x00
#define ADDRESS_IS_VALID             0x01

/* CBL_FLASH_ERASE_CMD */
#define CBL_FLASH_MAX_SECTOR_NUMBER  8
#define CBL_FLASH_MASS_ERASE         0xFF

#define INVALID_SECTOR_NUMBER        0x00
#define VALID_SECTOR_NUMBER          0x01
#define UNSUCCESSFUL_ERASE           0x02
#define SUCCESSFUL_ERASE             0x03

#define HAL_SUCCESSFUL_ERASE         0xFFFFFFFFU
/*********************boot loader configuration ends*********************************/

/*********************macro decleration section ends*********************************/

/*********************data type section start*********************************/
typedef enum
{
BOOT_LOADER_GET_VERSION =0x10,
BOOT_LOADER_GO_TO_ADDR_CMD=0x14,
BOOT_LOADER_FLASH_ERASE_CMD=0x15,
BOOT_LOADER_MEM_WRITE_CMD=0x16,
}BL_COM_T;

typedef enum{
	BL_NACK = 0,
	BL_OK
}BL_Status;

typedef void (*pMainApp)(void);
typedef void (*Jump_Ptr)(void);

	/*********************data type section end**********************************/

typedef HAL_StatusTypeDef Std_ReturnType;

	/********************function decleration section start**********************/
HAL_StatusTypeDef BOOTLOADER_PRINT(uint8_t * string,... );
HAL_StatusTypeDef BOOTLOADER_RECEIVE_COMMAND(void);

void Bootloader_Memory_Write(uint8_t *Host_Buffer);
void BOOTLOADER_SEND_VERSION_INFO(uint8_t *HOST_BUFF);
/*********************function decleration section start***********************/
