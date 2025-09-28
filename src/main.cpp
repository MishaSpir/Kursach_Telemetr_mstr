//эта программа  записывает конфигурацию в радиомодуль E32
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include  <cstdlib>

#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/vector.h>

#include "E32_dop.cpp"
  
#define BAUD_RATE (9600)
#define M0	GPIO0
#define M1	GPIO1

uint8_t data_buffer;	
Circular_buffer b;

volatile uint32_t tiks = 0;
uint32_t last_time = 0;

void sys_tick_handler(void){ //функция обработчик-прерываний systick
	tiks++;
}

void systick_setup(void){
systick_set_frequency(1000,72'000'000);
systick_counter_enable();
systick_interrupt_enable();

}

//Функия задержки в микросекундах
void delay_us(uint32_t us) {
	uint32_t start = systick_get_value();
	// Расчет тактов для указанного времени (72 тактов = 1 мкс при 72 МГц)
	uint32_t ticks = us * 72;
	while ((start - systick_get_value()) < ticks) {
		// Ждем, пока не пройдет нужное количество тактов
		}
	}

//  Функция задержки в миллисекундах
void delay_ms(uint32_t ms) {
	while (ms--) {
		delay_us(1000); // 1000 мкс = 1 мс
	}
}



static void clock_setup(void){
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	// rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_USART1);

}

static void gpio_setup(void){
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, M0); //для M0
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, M1); //для M1
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO2); //для светодиода	
}

static void uart2_setup(void){
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART2, BAUD_RATE);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	//активируем перрывания по приёму данных в UART2
  	USART_CR1(USART2) |= USART_CR1_RXNEIE; 
	nvic_enable_irq(NVIC_USART2_IRQ);

	usart_enable(USART2);
}

static void uart1_setup(void){

/* Setup GPIO pin GPIO_USART1_TX. */
gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
gpio_set_mode(GPIOA, GPIO_MODE_INPUT,GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_RX);
/* Setup UART parameters. */
usart_set_baudrate(USART1, BAUD_RATE);
usart_set_databits(USART1, 8);
usart_set_stopbits(USART1, USART_STOPBITS_1);
usart_set_mode(USART1, USART_MODE_TX);
usart_set_parity(USART1, USART_PARITY_NONE);
usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
usart_enable(USART1);

}


  

  

void usart2_isr(void)
{
	if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART2) & USART_SR_RXNE) != 0)) {
	
		// Чтение USART_DR автоматически очищает флаг USART_SR_RXNE
		b.put( static_cast<uint8_t>(usart_recv(USART2)));
		//активировать прерывание по готовности передатчика UART 
	}


}


  

int main(void){
	clock_setup();
	gpio_setup();
	uart2_setup();
	uart1_setup();
	systick_setup();
	//C0 00 00 18 06 44



	// char data_amount[2];
	uint8_t str[6];
	uint8_t byte_data;

	gpio_set(GPIOB,M0);
	gpio_set(GPIOB,M1);
	delay_ms(2000); 
	uint8_t str_tx[]={0xC0,0x00,0x00,0x1A,0x06,0x44};
	uart2_write(str_tx,6);
	delay_ms(200); 
	uint8_t str2_tx[]={0xC1, 0xC1, 0xC1};
	uart2_write(str2_tx,3);
	
	
	
	

/* Blink the LED (PC12) on the board with every transmitted byte. */

	while (1) {
		gpio_toggle(GPIOB, GPIO2); //без переключения светодиоода ничего не работает 
	
		//если индексы чтения и записи в кольцевом буфере совпадают
		if(!b.empty()){
			// usart_send_blocking(USART2,b.get());
			byte_data = b.get();//временно, по сути ничё не делает(надо изменить настройки класса)
			// usart_send_blocking(USART2,' ');
			// itoa(b.count,data_amount,10);
			// usart_send_blocking(USART2,data_amount[0]);
			
			//если в буфере накопилось 6 байт 
			if(b.count>=6){
				for(int i=0; i<6; ++i){
					str[i]=b.buf[6-b.wr_idx+i];//тут есть беда: если я начну записывать с конца буфера и перейду в начало, то...
				}
				for(int i=0; i<6; ++i){
					usart_send_blocking(USART1,str[i]);
				}
				b.wr_idx =0;
				b.rd_idx =0;
				b.full_ = false;
				gpio_clear(GPIOB,M0);
				gpio_clear(GPIOB,M1);
				// usart_send_blocking(USART1,'/n');
			}
			
		
			
		}
		
		
			
			
		






	}

  

return 0;

}