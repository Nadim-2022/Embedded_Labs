#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "uart.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define STRLEN 80


// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#endif

#define BAUD_RATE 9600



int main() {
    const uint button = 9;

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);

    stdio_init_all();

    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE);

    const char send[] = "AT+ID=DevEui\r\n";
    const char send2[] = "AT\r\n";
    const char send3[] = "AT+VER\r\n";
    char str[STRLEN];
    int pos = 0;
    int state = 1;
    int count = 0;
    //uart_inst_t uart_01 = UART_NR;
    while(true){
        switch (state) {
            case 1:
                if(!gpio_get(button)){
                    sleep_ms(100);
                    state = 2;
                }
                break;
            case 2:
                do{
                    uart_send(UART_NR, send2);
                    sleep_ms(500);
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    count++;
                    //printf("count: %d\n", count);
                } while (pos == 0 && count < 5);
                if(count > 4){
                    printf("Module not responding\n");
                    //printf("cCount: %d\n", count);
                    count = 0;
                    state = 1;
                }
                else{
                    printf("Connected to LoRa module\n");
                    //printf("Count: %d\n", count);
                    count = 0;
                    state = 3;
                }
                break;
            case 3:
                uart_send(UART_NR, send3);
                sleep_ms(500);

                pos = uart_read(UART_NR, (uint8_t *) str, STRLEN);
                if (pos > 0) {
                    str[pos] = '\0';
                    printf("Version: %s\n", str);
                    state = 4;
                }

                 else{
                    printf("Module stopped responding\n");
                    state = 1;
                }
                break;
            case 4:
                uart_send(UART_NR, send);
                sleep_ms(500);
                /*
                if(uart_is_readable_within_us((uart_inst_t *)UART_NR, 500000)){
                    printf("Data available\n");
                }
                 */
                pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                if(pos > 0){
                    str[pos] = '\0';
                    char str2[18];
                    //char *str2 = malloc(8 * sizeof(char));
                    int j = 18;
                    for(int i = pos; i >= 0 && j >= 0; i--){
                        if(str[i] != ':'){
                            str2[j--] = tolower(str[i]);
                        }
                    }
                    //str2[strlen(str2)-1] = '\0';
                    printf("DevEui: %s\n", str);
                    printf("DevEui: %s\n", str2);
                    //printf("POS: %d\n", pos);
                    //free(str2);
                    state = 1;
                } else{
                    printf("Module stopped responding\n");
                    state = 1;
                }
                break;
        }
    }

}

