#include <stdio.h>
#include "pico/stdlib.h"
#include "Hardware/pwm.h"

#define  rot_a 11
static int Brightness = 0;
static int last_Brightness = 0;

void rotary_clock_interrupt_handler(uint gio, uint32_t event){
    if(gpio_get(rot_a)){
        if(Brightness > 0) Brightness -= 50;
    }else{
        if(Brightness < 1000) Brightness +=50;
    }
    if(Brightness < 0){
        Brightness = 0;
    }else if(Brightness > 1000){
        Brightness = 999;
    }
}

int main() {
    stdio_init_all();
    bool led = false;

    const uint led_1 = 26;
    const uint led_2 = 21;
    const uint led_3 = 20;

    const uint rotary_sw = 12;
    const uint rotary_A = 10;
    const uint rotary_B = 11;

    gpio_init(rotary_sw);
    gpio_set_dir(rotary_sw, GPIO_IN);
    gpio_pull_up(rotary_sw);

    gpio_init(rotary_A);
    gpio_set_dir(rotary_A, GPIO_IN);

    gpio_init(rotary_B);
    gpio_set_dir(rotary_B, GPIO_IN);


    uint slice_num = pwm_gpio_to_slice_num(led_1);
    uint channel  = pwm_gpio_to_channel(led_1);
    pwm_set_enabled(slice_num, false);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);
    pwm_config_set_wrap(&config, 999);
    pwm_init(slice_num, &config, false);
    pwm_set_chan_level(slice_num, channel, Brightness);
    gpio_set_function(led_1, GPIO_FUNC_PWM);
    pwm_set_enabled(slice_num, true);

    uint slice_num2 = pwm_gpio_to_slice_num(led_2);
    uint channel2 = pwm_gpio_to_channel(led_2);
    pwm_set_enabled(slice_num2, false);
    pwm_init(slice_num2,&config,false);
    pwm_set_chan_level(slice_num2, channel2, Brightness);
    gpio_set_function(led_2, GPIO_FUNC_PWM);
    pwm_set_enabled(slice_num2, true);

    uint slice_num3 = pwm_gpio_to_slice_num(led_3);
    uint channel3 = pwm_gpio_to_channel(led_3);
    pwm_set_enabled(slice_num3, false);
    pwm_init(slice_num3,&config,false);
    pwm_set_chan_level(slice_num3, channel3, Brightness);
    gpio_set_function(led_3, GPIO_FUNC_PWM);
    pwm_set_enabled(slice_num3, true);

    gpio_set_irq_enabled_with_callback(rotary_A, GPIO_IRQ_EDGE_RISE, true, &rotary_clock_interrupt_handler);
    while(true) {
        if(!gpio_get(rotary_sw)){
            led = !led;
            while(!gpio_get(rotary_sw)){ sleep_ms(300);}
            if(led && last_Brightness == 0){
                Brightness = 500;
            }else if(led){
                Brightness = last_Brightness;
            }else if(Brightness == 0){
                Brightness = 500;
                led = !led;
            }else {
                last_Brightness = Brightness;
                Brightness = 0;
            }
            pwm_set_chan_level(slice_num, channel, Brightness);
            pwm_set_chan_level(slice_num2, channel2, Brightness);
            pwm_set_chan_level(slice_num3, channel3, Brightness);
        }
        if(led){
            last_Brightness = Brightness;
            pwm_set_chan_level(slice_num, channel, Brightness);
            pwm_set_chan_level(slice_num2, channel2, Brightness);
            pwm_set_chan_level(slice_num3, channel3, Brightness);
        }
    }
    return 0;
}
