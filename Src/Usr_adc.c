
#include "main.h"


uint32_t ubUserButtonPressed = 0U;

/* Variables for ADC conversion data */
uint16_t uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE; /* ADC group regular conversion data */

/* Variables for ADC conversion data computation to physical values */
uint16_t uhADCxConvertedData_Voltage_mVolt = 0U;  /* Value of voltage calculated from ADC conversion data (unit: mV) */

/* Variable to report status of ADC group regular unitary conversion          */
/*  0: ADC group regular unitary conversion is not completed                  */
/*  1: ADC group regular unitary conversion is completed                      */
/*  2: ADC group regular unitary conversion has not been started yet          */
/*     (initial state)                                                        */
uint8_t ubAdcGrpRegularUnitaryConvStatus = 2U; /* Variable set into ADC interruption callback */

/* Private function prototypes -----------------------------------------------*/
void     SystemClock_Config(void);
void     Configure_ADC(void);
void     Activate_ADC(void);
void     ConversionStartPoll_ADC_GrpRegular(void);
void     LED_Init(void);
void beepInit(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	uint16_t cnt1 = 0;
	uint16_t cnt2 = 0;
	/* Configure the system clock to 16 MHz */
	SystemClock_Config();

	/* Initialize LED4 */
	LED_Init();
	beepInit();

	/* Configure ADC */
	/* Note: This function configures the ADC but does not enable it.           */
	/*       To enable it, use function "Activate_ADC()".                       */
	/*       This is intended to optimize power consumption:                    */
	/*       1. ADC configuration can be done once at the beginning             */
	/*          (ADC disabled, minimal power consumption)                       */
	/*       2. ADC enable (higher power consumption) can be done just before   */
	/*          ADC conversions needed.                                         */
	/*          Then, possible to perform successive "Activate_ADC()",          */
	/*          "Deactivate_ADC()", ..., without having to set again            */
	/*          ADC configuration.                                              */
	Configure_ADC();

	/* Activate ADC */
	/* Perform ADC activation procedure to make it ready to convert. */
	Activate_ADC();

	/* Reset status variable of ADC unitary conversion before performing        */
	/* a new ADC conversion start.                                              */
	/* Note: Optionally, for this example purpose, check ADC unitary            */
	/*       conversion status before starting another ADC conversion.          */

	if (ubAdcGrpRegularUnitaryConvStatus != 0)
	{
		ubAdcGrpRegularUnitaryConvStatus = 0;
	}

	/* Init variable containing ADC conversion data */
	uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE;

	/* Perform ADC group regular conversion start, poll for conversion          */
	/* completion.                                                              */
	ConversionStartPoll_ADC_GrpRegular();

	/* Retrieve ADC conversion data */
	/* (data scale corresponds to ADC resolution: 12 bits) */
	uhADCxConvertedData = LL_ADC_REG_ReadConversionData12(ADC1);

	/* Computation of ADC conversions raw data to physical values               */
	/* using LL ADC driver helper macro.                                        */
	uhADCxConvertedData_Voltage_mVolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, uhADCxConvertedData, LL_ADC_RESOLUTION_12B);

	/* Update status variable of ADC unitary conversion */
	ubAdcGrpRegularUnitaryConvStatus = 1;

	/* Infinite loop */
	while (1)
	{
		/* Note: At this step, ADC is performing ADC conversions continuously,    */
		/*       indefinitely (ADC continuous mode enabled in this example).      */
		/*       Main program reads frequently ADC conversion data                */
		/*       (without waiting for end of each conversion: software reads data */
		/*       when main program execution pointer is available and can let     */
		/*       some ADC conversions data unread and overwritten by newer data,  */
		/*       overrun IT is disabled and overrun flag is ignored).             */
		/*       and stores it into the same variable.                            */

		/* Retrieve ADC conversion data */
		uhADCxConvertedData = LL_ADC_REG_ReadConversionData12(ADC1);

		/* Computation of ADC conversions raw data to physical values             */
		/* using LL ADC driver helper macro.                                      */
		uhADCxConvertedData_Voltage_mVolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, uhADCxConvertedData, LL_ADC_RESOLUTION_12B);
		if(uhADCxConvertedData_Voltage_mVolt > 1200)
		{
			cnt1++;
			cnt2 = 0;
			if(cnt1 >= 1000)
			{
				cnt1 = 0;
				LL_GPIO_SetOutputPin(LED_PORT, LED_GREEN_PIN);
				LL_GPIO_ResetOutputPin(LED_PORT, LED_RED_PIN);
				LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
			}
		}
		else
		{
			cnt2++;
			cnt1 = 0;
			if(cnt2 >= 1000)
			{
				cnt2 = 0;
				LL_GPIO_SetOutputPin(LED_PORT, LED_RED_PIN);
				LL_GPIO_ResetOutputPin(LED_PORT, LED_GREEN_PIN);
				LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1);
			}
		}

	}

	/* Note: ADC conversion data is stored into variable                        */
	/*       "uhADCxConvertedData".                                             */
	/*       (for debug: see variable content into watch window).               */

	/* Note: ADC conversion data are computed to physical values                */
	/*       into variable "uhADCxConvertedData_Voltage_mVolt"                  */
	/*       using ADC LL driver helper macro "__LL_ADC_CALC_DATA_TO_VOLTAGE()".*/
	/*       (for debug: see variable content into watch window).               */

}

/**
  * @brief  Configure ADC (ADC instance: ADC1) and GPIO used by ADC channels.
  * @note   In case re-use of this function outside of this example:
  *         This function includes checks of ADC hardware constraints before
  *         executing some configuration functions.
  *         - In this example, all these checks are not necessary but are
  *           implemented anyway to show the best practice usages
  *           corresponding to reference manual procedure.
  *           (On some STM32 series, setting of ADC features are not
  *           conditioned to ADC state. However, in order to be compliant with
  *           other STM32 series and to show the best practice usages,
  *           ADC state is checked anyway with same constraints).
  *           Software can be optimized by removing some of these checks,
  *           if they are not relevant considering previous settings and actions
  *           in user application.
  *         - If ADC is not in the appropriate state to modify some parameters,
  *           the setting of these parameters is bypassed without error
  *           reporting:
  *           it can be the expected behavior in case of recall of this
  *           function to update only a few parameters (which update fullfills
  *           the ADC state).
  *           Otherwise, it is up to the user to set the appropriate error
  *           reporting in user application.
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
void Configure_ADC(void)
{
#if (USE_TIMEOUT == 1)
	uint32_t Timeout = 0U; /* Variable used for timeout management */
#endif /* USE_TIMEOUT */

	/*## Configuration of GPIO used by ADC channels ############################*/

	/* Note: On this STM32 device, ADC1 channel 4 is mapped on GPIO pin PA.04 */

	/* Enable GPIO Clock */
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

	/* Configure GPIO in analog mode to be used as ADC input */
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ANALOG);
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_11, LL_GPIO_MODE_ANALOG);
	/*## Configuration of NVIC #################################################*/
	/* Configure NVIC to enable ADC1 interruptions */
//  NVIC_SetPriority(ADC1_IRQn, 0);
//  NVIC_EnableIRQ(ADC1_IRQn);

	/*## Configuration of ADC ##################################################*/

	/*## Configuration of ADC hierarchical scope: common to several ADC ########*/

	/* Enable ADC clock (core clock) */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC);

	/* Note: Hardware constraint (refer to description of the functions         */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of these features is conditioned to   */
	/*       ADC state:                                                         */
	/*       All ADC instances of the ADC common group must be disabled.        */
	/* Note: In this example, all these checks are not necessary but are        */
	/*       implemented anyway to show the best practice usages                */
	/*       corresponding to reference manual procedure.                       */
	/*       Software can be optimized by removing some of these checks, if     */
	/*       they are not relevant considering previous settings and actions    */
	/*       in user application.                                               */
	if(__LL_ADC_IS_ENABLED_ALL_COMMON_INSTANCE() == 0)
	{
		/* Note: Call of the functions below are commented because they are       */
		/*       useless in this example:                                         */
		/*       setting corresponding to default configuration from reset state. */

		/* Set ADC clock (conversion clock) common to several ADC instances */
		/* Note: On this STM32 serie, ADC common clock asynchonous prescaler      */
		/*       is applied to each ADC instance if ADC instance clock is         */
		/*       set to clock source asynchronous                                 */
		/*       (refer to function "LL_ADC_SetClock()" below).                   */
		// LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_ASYNC_DIV1);

		/* Set ADC measurement path to internal channels */
		// LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);


		/*## Configuration of ADC hierarchical scope: multimode ####################*/

		/* Note: Feature not available on this STM32 serie */

	}


	/*## Configuration of ADC hierarchical scope: ADC instance #################*/

	/* Note: Hardware constraint (refer to description of the functions         */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of these features is conditioned to   */
	/*       ADC state:                                                         */
	/*       ADC must be disabled.                                              */
	if (LL_ADC_IsEnabled(ADC1) == 0)
	{
		/* Note: Call of the functions below are commented because they are       */
		/*       useless in this example:                                         */
		/*       setting corresponding to default configuration from reset state. */

		/* Set ADC clock (conversion clock) */
		LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV4);

		/* Set ADC data resolution */
		// LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);

		/* Set ADC conversion data alignment */
		// LL_ADC_SetResolution(ADC1, LL_ADC_DATA_ALIGN_RIGHT);

		/* Set ADC low power mode */
		// LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);

		/* Set ADC channels sampling time */
		/* Note: On this STM32 serie, sampling time is common to groups           */
		/*       of severals channels within ADC instance.                        */
		/*       Therefore, groups of sampling sampling times are configured      */
		/*       here under ADC instance scope.                                   */
		/*       Then, sampling time of channels are configured below             */
		/*       among group of sampling times available, at channel scope.       */
		LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_COMMON_1, LL_ADC_SAMPLINGTIME_39CYCLES_5);

	}


	/*## Configuration of ADC hierarchical scope: ADC group regular ############*/

	/* Note: Hardware constraint (refer to description of the functions         */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of these features is conditioned to   */
	/*       ADC state:                                                         */
	/*       ADC must be disabled or enabled without conversion on going        */
	/*       on group regular.                                                  */
	if ((LL_ADC_IsEnabled(ADC1) == 0)               ||
	    (LL_ADC_REG_IsConversionOngoing(ADC1) == 0)   )
	{
		/* Set ADC group regular trigger source */
		LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

		/* Set ADC group regular trigger polarity */
		// LL_ADC_REG_SetTriggerEdge(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

		/* Set ADC group regular continuous mode */
		LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);

		/* Set ADC group regular conversion data transfer */
		// LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);

		/* Set ADC group regular overrun behavior */
		LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_PRESERVED);

		/* Set ADC group regular sequencer */
		/* Note: On this STM32 serie, ADC group regular sequencer has             */
		/*       two settings:                                                    */
		/*       - Sequencer configured to fully configurable:                    */
		/*         sequencer length and each rank                                 */
		/*         affectation to a channel are configurable.                     */
		/*         Channels selection is limited to channels 0 to 14.             */
		/*       - Sequencer configured to not fully configurable:                */
		/*         sequencer length and each rank affectation to a channel        */
		/*         are fixed by channel HW number.                                */
		/*         Channels selection is not limited (channels 0 to 18).          */
		/*       Refer to description of function                                 */
		/*       "LL_ADC_REG_SetSequencerConfigurable()".                         */

		/* Clear flag ADC channel configuration ready */
		LL_ADC_ClearFlag_CCRDY(ADC1);

		/* Set ADC group regular sequencer configuration flexibility */
		LL_ADC_REG_SetSequencerConfigurable(ADC1, LL_ADC_REG_SEQ_CONFIGURABLE);

		/* Poll for ADC channel configuration ready */
#if (USE_TIMEOUT == 1)
		Timeout = ADC_CHANNEL_CONF_RDY_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

		while (LL_ADC_IsActiveFlag_CCRDY(ADC1) == 0)
		{
#if (USE_TIMEOUT == 1)
			/* Check Systick counter flag to decrement the time-out value */
			if (LL_SYSTICK_IsActiveCounterFlag())
			{
				if(Timeout-- == 0)
				{
					/* Time-out occurred. Set LED to blinking mode */
					LED_Blinking(LED_BLINK_ERROR);
				}
			}
#endif /* USE_TIMEOUT */
		}
		/* Clear flag ADC channel configuration ready */
		LL_ADC_ClearFlag_CCRDY(ADC1);

		/* Set ADC group regular sequencer length and scan direction */
		LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);

		/* Poll for ADC channel configuration ready */
#if (USE_TIMEOUT == 1)
		Timeout = ADC_CHANNEL_CONF_RDY_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

		while (LL_ADC_IsActiveFlag_CCRDY(ADC1) == 0)
		{
#if (USE_TIMEOUT == 1)
			/* Check Systick counter flag to decrement the time-out value */
			if (LL_SYSTICK_IsActiveCounterFlag())
			{
				if(Timeout-- == 0)
				{
					/* Time-out occurred. Set LED to blinking mode */
					LED_Blinking(LED_BLINK_ERROR);
				}
			}
#endif /* USE_TIMEOUT */
		}
		/* Clear flag ADC channel configuration ready */
		LL_ADC_ClearFlag_CCRDY(ADC1);

		/* Set ADC group regular sequencer discontinuous mode */
		// LL_ADC_REG_SetSequencerDiscont(ADC1, LL_ADC_REG_SEQ_DISCONT_DISABLE);

		/* Set ADC group regular sequence: channel on the selected sequence rank. */
		LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_1);

		/* Poll for ADC channel configuration ready */
#if (USE_TIMEOUT == 1)
		Timeout = ADC_CHANNEL_CONF_RDY_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

		while (LL_ADC_IsActiveFlag_CCRDY(ADC1) == 0)
		{
#if (USE_TIMEOUT == 1)
			/* Check Systick counter flag to decrement the time-out value */
			if (LL_SYSTICK_IsActiveCounterFlag())
			{
				if(Timeout-- == 0)
				{
					/* Time-out occurred. Set LED to blinking mode */
					LED_Blinking(LED_BLINK_ERROR);
				}
			}
#endif /* USE_TIMEOUT */
		}
		/* Clear flag ADC channel configuration ready */
		LL_ADC_ClearFlag_CCRDY(ADC1);
	}


	/*## Configuration of ADC hierarchical scope: ADC group injected ###########*/

	/* Note: Feature not available on this STM32 serie */


	/*## Configuration of ADC hierarchical scope: channels #####################*/

	/* Note: Hardware constraint (refer to description of the functions         */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of these features is conditioned to   */
	/*       ADC state:                                                         */
	/*       ADC must be disabled or enabled without conversion on going        */
	/*       on either groups regular or injected.                              */
	if ((LL_ADC_IsEnabled(ADC1) == 0)               ||
	    (LL_ADC_REG_IsConversionOngoing(ADC1) == 0)   )
	{
		/* Set ADC channels sampling time */
		/* Note: On this STM32 serie, sampling time is common to groups           */
		/*       of severals channels within ADC instance.                        */
		/*       See sampling time configured above, at ADC instance scope.       */
		LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_COMMON_1);

	}


	/*## Configuration of ADC transversal scope: analog watchdog ###############*/

	/* Set ADC analog watchdog channels to be monitored */
	// LL_ADC_SetAnalogWDMonitChannels(ADC1, LL_ADC_AWD1, LL_ADC_AWD_DISABLE);

	/* Set ADC analog watchdog thresholds */
	// LL_ADC_ConfigAnalogWDThresholds(ADC1, LL_ADC_AWD1, 0xFFF, 0x000);


	/*## Configuration of ADC transversal scope: oversampling ##################*/

	/* Set ADC oversampling scope */
	// LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);

	/* Set ADC oversampling parameters */
	// LL_ADC_ConfigOverSamplingRatioShift(ADC1, LL_ADC_OVS_RATIO_2, LL_ADC_OVS_SHIFT_NONE);


	/*## Configuration of ADC interruptions ####################################*/
	/* Note: In this example, no ADC interruption enabled */

	/* Note: In this example, overrun interruption is disabled to fit with      */
	/*       the standard use case of ADC in continuous mode:                   */
	/*       to not have to read each ADC conversion data. SW can let           */
	/*       some ADC conversions data unread and overwritten by newer data)    */

}

/**
  * @brief  Perform ADC activation procedure to make it ready to convert
  *         (ADC instance: ADC1).
  * @note   Operations:
  *         - ADC instance
  *           - Run ADC self calibration
  *           - Enable ADC
  *         - ADC group regular
  *           none: ADC conversion start-stop to be performed
  *                 after this function
  *         - ADC group injected
  *           Feature not available                                  (feature not available on this STM32 serie)
  * @param  None
  * @retval None
  */
void Activate_ADC(void)
{
	__IO uint32_t wait_loop_index = 0U;
	__IO uint32_t backup_setting_adc_dma_transfer = 0U;
#if (USE_TIMEOUT == 1)
	uint32_t Timeout = 0U; /* Variable used for timeout management */
#endif /* USE_TIMEOUT */

	/*## Operation on ADC hierarchical scope: ADC instance #####################*/

	/* Note: Hardware constraint (refer to description of the functions         */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of these features is conditioned to   */
	/*       ADC state:                                                         */
	/*       ADC must be disabled.                                              */
	/* Note: In this example, all these checks are not necessary but are        */
	/*       implemented anyway to show the best practice usages                */
	/*       corresponding to reference manual procedure.                       */
	/*       Software can be optimized by removing some of these checks, if     */
	/*       they are not relevant considering previous settings and actions    */
	/*       in user application.                                               */
	if (LL_ADC_IsEnabled(ADC1) == 0)
	{
		/* Enable ADC internal voltage regulator */
		LL_ADC_EnableInternalRegulator(ADC1);

		/* Delay for ADC internal voltage regulator stabilization.                */
		/* Compute number of CPU cycles to wait for, from delay in us.            */
		/* Note: Variable divided by 2 to compensate partially                    */
		/*       CPU processing cycles (depends on compilation optimization).     */
		/* Note: If system core clock frequency is below 200kHz, wait time        */
		/*       is only a few CPU processing cycles.                             */
		wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
		while(wait_loop_index != 0)
		{
			wait_loop_index--;
		}

		/* Disable ADC DMA transfer request during calibration */
		/* Note: Specificity of this STM32 serie: Calibration factor is           */
		/*       available in data register and also transfered by DMA.           */
		/*       To not insert ADC calibration factor among ADC conversion data   */
		/*       in DMA destination address, DMA transfer must be disabled during */
		/*       calibration.                                                     */
		backup_setting_adc_dma_transfer = LL_ADC_REG_GetDMATransfer(ADC1);
		LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);

		/* Run ADC self calibration */
		LL_ADC_StartCalibration(ADC1);

		/* Poll for ADC effectively calibrated */
#if (USE_TIMEOUT == 1)
		Timeout = ADC_CALIBRATION_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

		while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0)
		{
#if (USE_TIMEOUT == 1)
			/* Check Systick counter flag to decrement the time-out value */
			if (LL_SYSTICK_IsActiveCounterFlag())
			{
				if(Timeout-- == 0)
				{
					/* Time-out occurred. Set LED to blinking mode */
					LED_Blinking(LED_BLINK_ERROR);
				}
			}
#endif /* USE_TIMEOUT */
		}

		/* Restore ADC DMA transfer request after calibration */
		LL_ADC_REG_SetDMATransfer(ADC1, backup_setting_adc_dma_transfer);

		/* Delay between ADC end of calibration and ADC enable.                   */
		/* Note: Variable divided by 2 to compensate partially                    */
		/*       CPU processing cycles (depends on compilation optimization).     */
		wait_loop_index = (ADC_DELAY_CALIB_ENABLE_CPU_CYCLES >> 1);
		while(wait_loop_index != 0)
		{
			wait_loop_index--;
		}

		/* Enable ADC */
		LL_ADC_Enable(ADC1);

		/* Poll for ADC ready to convert */
#if (USE_TIMEOUT == 1)
		Timeout = ADC_ENABLE_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

		while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0)
		{
#if (USE_TIMEOUT == 1)
			/* Check Systick counter flag to decrement the time-out value */
			if (LL_SYSTICK_IsActiveCounterFlag())
			{
				if(Timeout-- == 0)
				{
					/* Time-out occurred. Set LED to blinking mode */
					LED_Blinking(LED_BLINK_ERROR);
				}
			}
#endif /* USE_TIMEOUT */
		}

		/* Note: ADC flag ADRDY is not cleared here to be able to check ADC       */
		/*       status afterwards.                                               */
		/*       This flag should be cleared at ADC Deactivation, before a new    */
		/*       ADC activation, using function "LL_ADC_ClearFlag_ADRDY()".       */
	}

	/*## Operation on ADC hierarchical scope: ADC group regular ################*/
	/* Note: No operation on ADC group regular performed here.                  */
	/*       ADC group regular conversions to be performed after this function  */
	/*       using function:                                                    */
	/*       "LL_ADC_REG_StartConversion();"                                    */

	/*## Operation on ADC hierarchical scope: ADC group injected ###############*/
	/* Note: Feature not available on this STM32 serie */

}

/**
  * @brief  Perform ADC group regular conversion start, poll for conversion
  *         completion.
  *         (ADC instance: ADC1).
  * @note   This function does not perform ADC group regular conversion stop:
  *         intended to be used with ADC in single mode, trigger SW start
  *         (only 1 ADC conversion done at each trigger, no conversion stop
  *         needed).
  *         In case of continuous mode or conversion trigger set to
  *         external trigger, ADC group regular conversion stop must be added.
  * @param  None
  * @retval None
  */
void ConversionStartPoll_ADC_GrpRegular(void)
{
#if (USE_TIMEOUT == 1)
	uint32_t Timeout = 0U; /* Variable used for timeout management */
#endif /* USE_TIMEOUT */

	/* Start ADC group regular conversion */
	/* Note: Hardware constraint (refer to description of the function          */
	/*       below):                                                            */
	/*       On this STM32 serie, setting of this feature is conditioned to     */
	/*       ADC state:                                                         */
	/*       ADC must be enabled without conversion on going on group regular,  */
	/*       without ADC disable command on going.                              */
	/* Note: In this example, all these checks are not necessary but are        */
	/*       implemented anyway to show the best practice usages                */
	/*       corresponding to reference manual procedure.                       */
	/*       Software can be optimized by removing some of these checks, if     */
	/*       they are not relevant considering previous settings and actions    */
	/*       in user application.                                               */
	if ((LL_ADC_IsEnabled(ADC1) == 1)               &&
	    (LL_ADC_IsDisableOngoing(ADC1) == 0)        &&
	    (LL_ADC_REG_IsConversionOngoing(ADC1) == 0)   )
	{
		LL_ADC_REG_StartConversion(ADC1);
	}


#if (USE_TIMEOUT == 1)
	Timeout = ADC_UNITARY_CONVERSION_TIMEOUT_MS;
#endif /* USE_TIMEOUT */

	while (LL_ADC_IsActiveFlag_EOC(ADC1) == 0)
	{
#if (USE_TIMEOUT == 1)
		/* Check Systick counter flag to decrement the time-out value */
		if (LL_SYSTICK_IsActiveCounterFlag())
		{
			if(Timeout-- == 0)
			{
				/* Time-out occurred. Set LED to blinking mode */
				LED_Blinking(LED_BLINK_SLOW);
			}
		}
#endif /* USE_TIMEOUT */
	}

	/* Clear flag ADC group regular end of unitary conversion */
	/* Note: This action is not needed here, because flag ADC group regular   */
	/*       end of unitary conversion is cleared automatically when          */
	/*       software reads conversion data from ADC data register.           */
	/*       Nevertheless, this action is done anyway to show how to clear    */
	/*       this flag, needed if conversion data is not always read          */
	/*       or if group injected end of unitary conversion is used (for      */
	/*       devices with group injected available).                          */
	LL_ADC_ClearFlag_EOC(ADC1);

}

/**
  * @brief  Initialize LED4.
  * @param  None
  * @retval None
  */
void LED_Init(void)
{
	/* Enable the LED4 Clock */
	LED_GPIO_CLK_ENABLE();

	/* Configure IO in output push-pull mode to drive external LED4 */
	LL_GPIO_SetPinMode(LED_PORT, LED_RED_PIN, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(LED_PORT, LED_GREEN_PIN, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_ResetOutputPin(LED_PORT, LED_RED_PIN);
	LL_GPIO_ResetOutputPin(LED_PORT, LED_GREEN_PIN);
}
void beepInit(void)
{
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
}



/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = HSI
  *            SYSCLK(Hz)                     = 16000000
  *            HCLK(Hz)                       = 16000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI clock division factor      = 1
  *            HSI Frequency(Hz)              = 16000000
  *            Flash Latency(WS)              = 0
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
	/* Set AHB prescaler*/
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

	/* Set APB1 prescaler*/
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	/* HSI configuration and activation */
	LL_RCC_SetHSIDiv(LL_RCC_HSI_DIV_1);
	LL_RCC_HSI_Enable();
	while(LL_RCC_HSI_IsReady() != 1)
	{
	};

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
	{
	};

	/* Set systick to 1ms in using frequency set to 16MHz */
	LL_Init1msTick(16000000);

	/* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
	LL_SetSystemCoreClock(16000000);
}

/******************************************************************************/
/*   USER IRQ HANDLER TREATMENT                                               */
/******************************************************************************/


/**
  * @brief  ADC group regular overrun interruption callback
  * @note   This function is executed when ADC group regular
  *         overrun error occurs.
  * @retval None
  */
void AdcGrpRegularOverrunError_Callback(void)
{
	/* Note: Disable ADC interruption that caused this error before entering in */
	/*       infinite loop below.                                               */

	/* Disable ADC group regular overrun interruption */
	LL_ADC_DisableIT_OVR(ADC1);

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
