#ifndef USR_EXFLASH_H
#define USR_EXFLASH_H

#include "usr_main.h"

#define FLASH_SPI	            SPI1
#define BREAK_PIONT_ADDR_0		0x00000000      //断点保存起始地址
#define BREAK_PIONT_ADDR_1		0x00010000      //断点保存起始地址
#define EXFLASH_APP1_ADDR		0x00100000  

#define FLASH_UPG_ADDR	        0x00080000      //远程升级结果存放地址
#define FLASH_FS_ADDR	        0x001f0000
#define FLASH_CHECK_ADDR	    0x001ff000

#define FLASH_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define FLASH_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define FLASH_CMD_WREN           0x06  /*!< Write enable instruction */
#define FLASH_CMD_READ           0x03  /*!< Read from Memory instruction */
#define FLASH_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define FLASH_CMD_RDID           0x9F  /*!< Read identification */
#define FLASH_CMD_SE             0xD8  /*!< Sector Erase instruction */
#define FLASH_CMD_BE             0xC7  /*!< Bulk Erase instruction */

#define FLASH_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define FLASH_DUMMY_BYTE         0xA5
#define FLASH_SPI_PAGESIZE       0x100

#define FLASH_M25P128_ID         0x202018
#define FLASH_M25P64_ID          0x202017
  

#define FLASH_CS_LOW()       LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_15)
#define FLASH_CS_HIGH()      LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_15)   


extern unsigned short Breakpointleng;

void EXFLASH_SpiInit(void);
void EXFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void EXFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void EXFLSAH_SaveBreakPoint(void);
unsigned char EXFLASH_ReadBreakPoint(void);
void EXFLASH_EraseSector(uint32_t SectorAddr);

#endif

