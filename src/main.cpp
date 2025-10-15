//эта программа реализации прасинга
#include <libopencm3/cm3/nvic.h>
#include "../inc/LiquidCrystalSTM.hpp"
#include "../inc/Pars.hpp"
#include "../inc/setup.hpp"


uint8_t pkg_is_begin = 0;
uint8_t pkg_is_received = 0;

uint8_t data_buffer;	
Circular_buffer b;
uint8_t ch;

void pasr_bufer_clear(void);
void pars_buffer_uart_print(void);
enum State {
    idle, // = 0
    data, // = 1
    fin, // = 2
    err // = 3
};
	

State state{idle};


//БУФФЕР ПАРСИНГА
//строка 0 --- первое рапарсированное число
//строка 1 ---второе распарсированное число
const uint8_t COL_SIZE =4;
const uint8_t ROW_SIZE =5;
char buf[ROW_SIZE][COL_SIZE];int m = 0;int k =0;

//временные переменные
volatile uint32_t tiks = 0;
uint32_t last_time = 0;


//функция обработчик-прерываний systick. Срабатывает каждую 1 мс. 
//Настройка частоты в systick_setup в файле setup.cpp
void sys_tick_handler(void){ 
	tiks++;
}

  

void usart2_isr(void)
{
	if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART2) & USART_SR_RXNE) != 0)) {
		// Чтение USART_DR автоматически очищает флаг USART_SR_RXNE
		//Только когда в UART придёт символ $ и пакет пришел, тогда позволим заполнять буфер
		if(usart_recv(USART2)=='$' ){pkg_is_begin=1;
		}
		if(pkg_is_begin ){
			b.put( static_cast<uint8_t>(usart_recv(USART2)));

				if(!b.empty()){
					ch = b.get();
					usart_send_blocking(USART1,ch);
					//КОНЕЧНЫЙ АВТОМАТ
					switch(state){
						case idle:
							if(ch == '$'){state = data;
							}// Принят маркер, переходим к приёму данных
							else {}
						break;
						case data:

							if(ch == '*'){state = fin;pkg_is_begin=0;pkg_is_received = 1;}//Принят терминатор, заканчиваем приём данных
							else if (ch == '$'){state = err;m = 0;
							k = 0;

							}
							else if(ch!=',') {buf[k+1][m] = ch;m++; m %= COL_SIZE;
								
							}
							else {k+=1; 
								if(k>=ROW_SIZE){k=0;}
								m = 0;
								
						}//Принят разделитель, принимаем новой число int 
						break;
						case fin:
							m = 0;
							k = 0;
							if (ch == '$'){state = data;} 
						else {state = err;} 
						break;
						case err:
								
						k=0;m=0;
						break;
						// default:
					}
				}	
				
			}	


		}
		
	}






  

int main(void){
	clock_setup();
	gpio_setup();
	uart2_setup();//для связи с радиомодулем
	uart1_setup();//для вывода на комп данных с радиомодуля
	systick_setup();
	lcd_timer2_setup();//таймер 2 занят функциями задержки LCD библиотеки

	gpio_set(GPIOB,M0);
	gpio_set(GPIOB,M1);
	delay_ms(2000); 
	uint8_t str_tx[]={0xC0,0x00,0x00,0x1A,0x06,0x44};
	uart2_write(str_tx,6);
	delay_ms(200); 
	gpio_clear(GPIOB,M0);
	gpio_clear(GPIOB,M1);
	delay_ms(200); 

	uint8_t EraseSymbolForLCD[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00};

	LiquidCrystal lcd(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
	lcd.begin(20, 4,0x00);
	uint8_t str_init[] = "E32_radio_reciver";
	for(unsigned int i = 0; i< sizeof(str_init)/sizeof(str_init[0]); i++){
		lcd.write(str_init[i]);
		delay_ms(80);
	}	
	delay_ms(2000);
	//C0 00 00 18 06 44











	while (1) {
		gpio_toggle(GPIOB, GPIO2); //без переключения светодиоода ничего не работает 
		if(pkg_is_received){
			pkg_is_received =0;
			
		}
		//если индексы чтения и записи в кольцевом буфере совпадают
		// if(!b.empty()){
		
		// 	if(b.readBytes(str,10)){//если пришло больше чем 8байт
		// 		state = idle;
		// 		for(int i=0; i<10; ++i){
		// 			usart_send_blocking(USART1,str[i]);
					
					
		// 		}

			
		// 		gpio_clear(GPIOB,M0);
		// 		gpio_clear(GPIOB,M1);
				
		// 		//КОНЕЧНЫЙ АВТОМАТ
		// 		for(const char ch: str){//прогоняет по всей строке
		// 			switch(state){
		// 				case idle:
		// 					if(ch == '$'){state = data;
		// 					}// Принят маркер, переходим к приёму данных
		// 					else {}
		// 				break;
		// 				case data:

		// 					if(ch == '*'){state = fin;pkg_is_begin=0;}//Принят терминатор, заканчиваем приём данных
		// 					else if (ch == '$'){state = err;m = 0;
		// 					k = 0;

		// 					}
		// 					else if(ch!=',') {buf[k+1][m] = ch;m++; m %= COL_SIZE;
								
		// 					}
		// 					else {k+=1; 
		// 						if(k>=ROW_SIZE){k=0;}
		// 						m = 0;
		// 						// usart_send_blocking(USART1,(uint16_t)k);
		// 						// usart_send_blocking(USART1,'\t');
		// 				// usart_send_blocking(USART1,buf[2][0]);
		// 				// usart_send_blocking(USART1,buf[2][1]);
		// 				// usart_send_blocking(USART1,buf[2][2]);
		// 				// usart_send_blocking(USART1,buf[2][3]);
		// 				}//Принят разделитель, принимаем новой число int 
		// 				break;
		// 				case fin:
		// 					m = 0;
		// 					k = 0;
		// 					if (ch == '$'){state = data;} 
		// 				else {state = err;} 
		// 				break;
		// 				case err:
								
		// 				k=0;m=0;
		// 				break;
		// 				// default:
		// 			}
		// 		}
				
		// 	}	
			

	
			
		// }

	if(tiks-last_time>=1000){
			last_time = tiks;
			   lcd.createChar(0, EraseSymbolForLCD);
			 lcd.clear();
			 lcd.write(buf[1][0]);
			 lcd.write(buf[1][1]);
			 lcd.write(buf[1][2]);
			 lcd.write(buf[1][3]);
			 lcd.setCursor(0,5);
			 lcd.write(buf[2][0]);
			 lcd.write(buf[2][1]);
			 lcd.write(buf[2][2]);
			 lcd.setCursor(0,9);
			 lcd.write(buf[3][0]);
			 lcd.write(buf[3][1]);
			 lcd.write(buf[3][2]);	

	
			 pars_buffer_uart_print();
	

	
	

	// //ПАРСИНГ ИНТОВ
	// int int_val[2];
	// char str_from_int [2][4];
	// for(int i=0; i<2; i++){
	// 	for(int j=0; j<4; j++){
	// 	str_from_int[i][j]=0;
	// 	}
	// }
	// //формируем из строки число int
	// int_val[0] = atoi(buf[1]);//buf[1]указатель на строку1 ---перове число
	// int_val[1] = atoi(buf[2]);//buf[2]указатель на строку2 ---второе число
	// //формируем из int строку(для вывода в UART)
	// itoa(int_val[0],str_from_int[0],10);
	// itoa(int_val[1],str_from_int[1],10);
	

	// //если число меньше чем длина массива, в данном случае чем 4,
	// 	//тогда itoa поставит '\0' на незаполненные места
	// // usart_send_blocking(USART1,str_from_int[0][0]);
	// // usart_send_blocking(USART1,str_from_int[0][1]);
	// // usart_send_blocking(USART1,str_from_int[0][2]);
	// // usart_send_blocking(USART1,str_from_int[0][3]);
	// // usart_send_blocking(USART1,'\t');
	
	// // usart_send_blocking(USART1,str_from_int[1][0]);
	// // usart_send_blocking(USART1,str_from_int[1][1]);
	// // usart_send_blocking(USART1,str_from_int[1][2]);
	// // usart_send_blocking(USART1,str_from_int[1][3]);
	// // usart_send_blocking(USART1,'\t');

		}


		
		
		
			
			
		






	}

  

return 0;

}


void pasr_bufer_clear(void){ //функция очистки буфера парсинга
	for(int i = 0; i<ROW_SIZE; i++){
		for(int j =0; j< COL_SIZE; j++){
			buf[i][j]='\0';
		}
	}
}
void pars_buffer_uart_print(void){//функция вывода ВСЕГО содержимого буфера на UART
	usart_send_blocking(USART1,'\t');	
			for(int i = 1; i<ROW_SIZE; i++){
				for(int j =0; j< COL_SIZE; j++){
					usart_send_blocking(USART1,buf[i][j]);
				}
				usart_send_blocking(USART1,'\t');
			}usart_send_blocking(USART1,'\n');
}