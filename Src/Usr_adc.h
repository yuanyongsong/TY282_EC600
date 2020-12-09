#ifndef __USR_ADC_H
#define __USR_ADC_H

void Adc_init(void);
u16 Adc_Value_Get(void);
void Close_ADC(void);
void Open_ADC(void);
extern u32	BatVoltage_Adc;
#endif
