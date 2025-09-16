//Мигаем светодиодом
#include <libopencm3/stm32/rcc.h> //rcc.h - reset and clock control
#include <libopencm3/stm32/gpio.h> //inputs outputs

int main() {
    rcc_periph_clock_enable(RCC_GPIOD);

    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE ,GPIO15);

    while (true) {

        gpio_toggle(GPIOD,GPIO15);
        for(volatile uint32_t i =0; i < 2'000'000; i +=2);

    }
}