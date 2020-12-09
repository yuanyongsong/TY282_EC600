#include "Usr_main.h"

u32	BatVoltage_Adc;			//电池电压ADC采样值

#define VDDA_APPLI                       (3300U)


void Adc_init(void)
{
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);

	/* Turn on ADC1 as peripheral */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC);

	/* Clock selection */
	LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV4);
	/* ADC Calibration */
	if (LL_ADC_IsEnabled(ADC1)) {
			LL_ADC_Disable(ADC1);
	}
	while (LL_ADC_IsEnabled(ADC1));
	LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);
	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1));
	/* ADC Setting */
	LL_ADC_Enable(ADC1);
	LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
	LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
	LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);
	LL_ADC_SetSamplingTimeCommonChannels(ADC1,LL_ADC_SAMPLINGTIME_COMMON_1,LL_ADC_SAMPLINGTIME_39CYCLES_5);
	LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
	LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_8);
	LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
	LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED);
	/* Enable ADC conversion */
	

}

u16 Adc_Value_Get(void)
{
	uint16_t val = 0;
	uint16_t voltage = 0;

	LL_ADC_REG_StartConversion(ADC1);
//	while(!LL_ADC_IsActiveFlag_EOC(ADC1));
	delay_ms(5);
	val = LL_ADC_REG_ReadConversionData12(ADC1);
	voltage = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, val, LL_ADC_RESOLUTION_12B);
	return voltage;
}


void Close_ADC(void)
{
	LL_ADC_Disable(ADC1);//关闭ADC
	LL_ADC_ClearFlag_ADRDY(ADC1);//清除ADRDY标志
}

void Open_ADC(void)
{
  if (LL_ADC_IsEnabled(ADC1) == 0)//检测ADC1是否开启
  {
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);//禁用ADC1深断电模式

    LL_ADC_EnableInternalRegulator(ADC1);//开启ADC1内部电压调节器
    delay_us(20);
    LL_ADC_StartCalibration(ADC1);//运行ADC自动校正
    while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0){}//等待ADC校准结束
    delay_us(20);
    LL_ADC_Enable(ADC1);//开启ADC
    while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0){}//等待ADC准备好
  }
}
