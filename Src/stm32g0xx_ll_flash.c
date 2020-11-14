#include "stm32g0xx_ll_flash.h"


void LL_FLASH_Program_TwoBtye(uint32_t flash_addr,uint16_t data)
{
	*(__IO uint16_t*)(flash_addr) = data;
}


void LL_FLASH_Program_Word(uint32_t flash_addr,uint32_t data)
{
	*(__IO uint32_t*)(flash_addr) = data;
}



void LL_Flash_Unlock(void)
{
    #if 0
	while (LL_FLASH_IsActiveFlag_BSY())  
	{
	} 

	if (LL_FLASH_LockState()) 
	{ 
		LL_FLASH_SetKey(FLASH_KEY1);
		LL_FLASH_SetKey(FLASH_KEY2);
	}
    #else
    if((FLASH->CR & FLASH_CR_LOCK) != RESET)
    {
        /* Authorize the FLASH Registers access */
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    } 
	if((FLASH->CR & FLASH_CR_OPTLOCK) != RESET)
    {
        /* Authorize the FLASH Registers access */
        FLASH->OPTKEYR = FLASH_OPTKEY1;
        FLASH->OPTKEYR = FLASH_OPTKEY2;
    }     
    #endif
}


//擦除一个扇区内容
ErrorStatus LL_Flash_PageErase(uint32_t Page)
{
    uint32_t tmp;

    while (LL_FLASH_IsActiveFlag_BSY()) 
    { 
    } 

    LL_FLASH_SetTypeErase(FLASH_TYPEERASE_PAGES);   //扇区擦除允许

    tmp = (FLASH->CR & ~FLASH_CR_PNB);              //设置要擦除的扇区为0
   
//    FLASH->CR = (tmp | (FLASH_CR_STRT | (Page <<  FLASH_CR_PNB_Pos)));    //写入要擦除的扇区，并开始扇区擦除

	FLASH->CR = (tmp | (Page <<  FLASH_CR_PNB_Pos));
	FLASH->CR |= FLASH_CR_STRT;

    while (LL_FLASH_IsActiveFlag_BSY()) 
    { 
    } 

    if (LL_FLASH_IsActiveFlag_EOP()) 
    { 
        LL_FLASH_ClearFlag_EOP();
    } 

    LL_FLASH_DisenableErase(FLASH_TYPEERASE_PAGES);     //扇区擦除禁止
    
	return SUCCESS;
}


ErrorStatus LL_FLASH_Program(ProgaramDataType ProgramType,uint32_t flash_addr,uint64_t data)
{
	
	uint8_t index = 0U;
	uint8_t nbiterations = 0U;
	
	if(ProgramType == ProgaraType_DATA16)
		nbiterations = 1U;
	else if(ProgramType == ProgaraType_DATA32)
		nbiterations = 2U;
	else
		nbiterations = 2U;
    while (LL_FLASH_IsActiveFlag_BSY()) 
	{

	}
 //   LL_FLASH_EnableProgram();
    #if 1
	for(index = 0U; index < nbiterations; index++)
	{
//		LL_FLASH_Program_TwoBtye((flash_addr + (2U*index)), (uint16_t)(data >> (16U*index)));
		LL_FLASH_Program_Word((flash_addr + (4U*index)), (uint32_t)(data >> (32U*index)));
	}
    #else
        *(__IO uint32_t*)flash_addr = data;
    #endif
	while (LL_FLASH_IsActiveFlag_BSY()) 
	{
	}

	if (LL_FLASH_IsActiveFlag_EOP())	
	{
		LL_FLASH_ClearFlag_EOP();
	}
//	LL_FLASH_DisenableProgram();
	return SUCCESS;
}
