#ifndef STM32G0xx_LL_FLASH_H
#define STM32G0xx_LL_FLASH_H

#include "stm32g0xx.h"

#define LL_OK     1
#define LL_ERROR  0

#define FLASH_FLAG_BSY             FLASH_SR_BSY1            /*!< FLASH Busy flag                           */ 
#define FLASH_FLAG_PGERR           FLASH_SR_PGAERR          /*!< FLASH Programming error flag    */
#define FLASH_FLAG_WRPERR          FLASH_SR_WRPERR         /*!< FLASH Write protected error flag          */
#define FLASH_FLAG_EOP             FLASH_SR_EOP            /*!< FLASH End of Operation flag               */

#define FLASH_TYPEERASE_PAGES          FLASH_CR_PER         /*!< FLASH_CR_PER          */
//#define FLASH_TYPEERASE_MASSERASE      FLASH_CR_MER            /*!< MASSERASE              */

#define FLASH_KEY1                      0x45670123U   /*!< Flash key1 */
#define FLASH_KEY2                      0xCDEF89ABU   /*!< Flash key2: used with FLASH_KEY1
                                                           to unlock the FLASH registers access */
#define FLASH_OPTKEY1                   0x08192A3BU   /*!< Flash option byte key1 */
#define FLASH_OPTKEY2                   0x4C5D6E7FU   /*!< Flash option byte key2: used with FLASH_OPTKEY1
                                                           to allow option bytes operations */

#define FLASH_PAGE_SIZE          0x800U         //STM32G070RB一个扇区为2k，共63个扇区

typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} LL_StatusTypeDef;

typedef enum {
	ProgaraType_DATA64,
	ProgaraType_DATA32,
	ProgaraType_DATA16
}ProgaramDataType;

typedef enum {\
	FLASH_Lock=1U,Flash_Unlock=!FLASH_Lock\
}FlashStates;

  /* Set the OBL_Launch bit to launch the option byte loading */
__STATIC_INLINE void LL_FLASH_SET_OBL_Launch(void)
{
  SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);
}
__STATIC_INLINE void LL_Flash_Lock(void)
{
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}


  /* @brief  Set flash erase type.
  * @param  FLASH_TYPEERASE specifies the FLASH flags to clear.
  *          This parameter can be any combination of the following values:
  *            @arg @ref FLASH_TYPEERASE_PAGES         PAGES Erase
  *            @arg @ref FLASH_TYPEERASE_MASSERASE      FLASH Write protected error flag 
  * @retval none*/

__STATIC_INLINE void LL_FLASH_SetTypeErase(uint32_t FLASH_TYPEERASE)
{
  SET_BIT(FLASH->CR, FLASH_TYPEERASE);
}

  /* @brief  Set flash erase ADDR.
  *          This parameter can be any combination of the following values:
  *            @arg @ref EraseADDR         uint32_t value
  * @retval none*/

__STATIC_INLINE void LL_FLASH_StartErase(void)
{
  SET_BIT(FLASH->CR, FLASH_CR_STRT);
}

  /* @brief  Clear the specified FLASH flag.
  * @param  __FLAG__ specifies the FLASH flags to clear.
  *          This parameter can be any combination of the following values:
  *            @arg @ref FLASH_FLAG_EOP         FLASH End of Operation flag 
  *            @arg @ref FLASH_FLAG_WRPERR      FLASH Write protected error flag 
  *            @arg @ref FLASH_FLAG_PGERR       FLASH Programming error flag
  * @retval none*/

__STATIC_INLINE void LL_FLASH_ClearFlag(uint32_t STATE_FLAG)
{
  WRITE_REG(FLASH->SR, STATE_FLAG);
}

  /*get bit flash bsy*/
__STATIC_INLINE uint32_t LL_FLASH_IsActiveFlag_BSY(void)
{
  return (READ_BIT(FLASH->SR, FLASH_SR_BSY1) == (FLASH_SR_BSY1));
}
/*get end of operation bilt*/
__STATIC_INLINE uint32_t LL_FLASH_IsActiveFlag_EOP(void)
{
  return (READ_BIT(FLASH->SR, FLASH_SR_EOP) == (FLASH_SR_EOP));
}
/*clear end of operation bilt*/
__STATIC_INLINE void LL_FLASH_ClearFlag_EOP(void)
{
  SET_BIT(FLASH->SR, FLASH_SR_EOP);//EOP bit Set clear
}
  /* @brief  Set flash erase type.
  * @param  FLASH_TYPEERASE specifies the FLASH flags to clear.
  *          This parameter can be any combination of the following values:
  *            @arg @ref FLASH_TYPEERASE_PAGES         PAGES Erase
  *            @arg @ref FLASH_TYPEERASE_MASSERASE      FLASH Write protected error flag 
  * @retval none*/
__STATIC_INLINE void LL_FLASH_DisenableErase(uint32_t FLASH_TYPEERASE)
{
  CLEAR_BIT(FLASH->CR, FLASH_TYPEERASE);
}

/*EnableProgram*/
__STATIC_INLINE void LL_FLASH_EnableProgram(void)
{
  SET_BIT(FLASH->CR,FLASH_CR_PG);
}
/*DisenableProgram*/
__STATIC_INLINE void LL_FLASH_DisenableProgram(void)
{
  CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
}
/*read flash's states of lock or unlock*/
__STATIC_INLINE FlashStates LL_FLASH_LockState(void)
{
	return (FlashStates)(READ_BIT(FLASH->CR,FLASH_CR_LOCK));
}
/*set key for flash*/
__STATIC_INLINE void LL_FLASH_SetKey(uint32_t key)
{
	WRITE_REG(FLASH->KEYR,key);
}


void LL_Flash_Unlock(void);
ErrorStatus LL_Flash_PageErase(uint32_t page_addr);
ErrorStatus LL_FLASH_Program(ProgaramDataType ProgramType,uint32_t flash_addr,uint64_t data);
#endif

