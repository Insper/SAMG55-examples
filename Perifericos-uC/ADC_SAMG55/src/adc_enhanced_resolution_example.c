


/*
ADC_Pin = PA18

*/







#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "asf.h"




volatile bool is_conversion_done = false;

volatile uint32_t g_ul_value = 0;


void ADC_init(void);


static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}


static void configure_tc_trigger(void)
{
	uint32_t ul_div = 0;
	uint32_t ul_tc_clks = 0;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_TC0);

	pmc_disable_pck(PMC_PCK_3);
	pmc_switch_pck_to_sclk(PMC_PCK_3, PMC_PCK_PRES(0));
	pmc_enable_pck(PMC_PCK_3);


	/* TIOA configuration */
	ioport_set_pin_mode(PIN_TC0_TIOA0, PIN_TC0_TIOA0_MUX);
	ioport_disable_pin(PIN_TC0_TIOA0);

	/* Configure TC for a 1Hz frequency and trigger on RC compare. */
	tc_find_mck_divisor(1, ul_sysclk, &ul_div, &ul_tc_clks, ul_sysclk);
	tc_init(TC0, 0, ul_tc_clks | TC_CMR_CPCTRG | TC_CMR_WAVE |
			TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
	TC0->TC_CHANNEL[0].TC_RA = (ul_sysclk / ul_div) / 2;
	TC0->TC_CHANNEL[0].TC_RC = (ul_sysclk / ul_div) / 1;

	/* Start the Timer. */
	tc_start(TC0, 0);
}







static void adc_end_conversion(void)
{
	g_ul_value = adc_channel_get_value(ADC, ADC_CHANNEL_1);
	is_conversion_done = true;
}


void ADC_init(void){
	
		adc_enable();

		adc_select_clock_source_mck(ADC);

		struct adc_config adc_cfg;

		adc_get_config_defaults(&adc_cfg);

		adc_init(ADC, &adc_cfg);
		adc_channel_enable(ADC, ADC_CHANNEL_1);

		adc_set_trigger(ADC, ADC_TRIG_TIO_CH_0);

		adc_set_callback(ADC, ADC_INTERRUPT_EOC_1,
		adc_end_conversion, 1);

		adc_set_resolution(ADC, ADC_12_BITS);
		adc_start_calibration(ADC);

		configure_tc_trigger();
	
}

int main(void)
{

	sysclk_init();
	board_init();

	configure_console();


	ADC_init();

	while (1) {
		if (is_conversion_done == true) {
			printf("%d\n", (int)g_ul_value);
			is_conversion_done = false;
		}
	}
}
