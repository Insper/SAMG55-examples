#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOA
#define LED_PIO_ID   ID_PIOA
#define LED_IDX      6
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  2
#define BUT_IDX_MASK (1 << BUT_IDX)

volatile bool led0_Status = true;

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}


static void ativaLEDBlink()
{
	
		led0_Status = !led0_Status;
		if (!led0_Status) pio_set(LED_PIO, LED_IDX_MASK);
	

}



static void but_Handler(uint32_t id, uint32_t mask)
{
	if (!pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)) {
		ativaLEDBlink();
	}
}





static void configure_buttons(void)
{

pmc_enable_periph_clk(LED_PIO_ID);
pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);


pmc_enable_periph_clk(BUT_PIO_ID);

pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);


pio_handler_set(BUT_PIO,
BUT_PIO_ID,
BUT_IDX_MASK,
PIO_IT_FALL_EDGE,
but_Handler);

pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
pio_get_interrupt_status(BUT_PIO);

NVIC_EnableIRQ(BUT_PIO_ID);
NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4

}





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



int main(void)
{
	
sysclk_init();
board_init();

ssd1306_init();
ssd1306_display_on();

configure_console();
configure_buttons();






	while (1) {
		if (led0_Status) {
			pin_toggle(LED_PIO, LED_IDX_MASK);
			ssd1306_clear();
			ssd1306_set_column_address(40);
			ssd1306_set_page_address(2);
			ssd1306_write_text("LED Blink ON");
			printf("LED BLink ON\n");
			}else {
			ssd1306_clear();
			ssd1306_set_column_address(40);
			ssd1306_set_page_address(3);
			ssd1306_write_text("LED Blink Off");
			printf("LED BLink Off\n");
		}
		delay_ms(500);
	}

}


