#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "uart.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define STRLEN 256


// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_NAME uart1
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

    const char send[] = "AT+MODE=LWOTAA\r\n";
    //const char send2[] = "AT\r\n";
    //const char send3[] = "AT+VER\r\n";
    //const char send4[] = "AT+KEY=APPKEY,”ad50014f5e3b35a03ef7cf0909b5b975”\r\n";
    const char send4[] = "AT+KEY=APPKEY,\"AD50014F5E3B35A03EF7CF0909B5B975\"\r\n";
    const char send5[] = "AT+CLASS=A\r\n";
    const char send6[] = "AT+PORT=8\r\n";
    const char send7[] = "AT+JOIN\r\n";
    const char send8[] = "AT+MSG=\"Hello World\"\r\n";
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
                uint64_t start = time_us_64();
                while(time_us_64() - start < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("MODE: %s\n", str);

                    state = 5;

                break;
            case 5:
                uart_send(UART_NR, send4);
                sleep_ms(500);
                uint64_t start2 = time_us_64();
                while(time_us_64() - start2 < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("KEY: %s\n", str);

                state = 6;

            case 6:
                uart_send(UART_NR, send5);
                sleep_ms(500);
                uint64_t start3 = time_us_64();
                while(time_us_64() - start3 < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("CLASS: %s\n", str);

                state = 7;
                break;
            case 7:
                uart_send(UART_NR, send6);
                sleep_ms(500);
                uint64_t start4 = time_us_64();
                while(time_us_64() - start4 < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("CLASS: %s\n", str);

                state = 8;
                break;

            case 8:
                uart_send(UART_NR, send7);
                sleep_ms(8500);
                uint64_t start5 = time_us_64();
                while(time_us_64() - start5 < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("CLASS: %s\n", str);

                state = 9;
                break;
            case 9:
                uart_send(UART_NR, send8);
                sleep_ms(4000);
                uint64_t start6 = time_us_64();
                while(time_us_64() - start6 < 500000){
                    pos  = uart_read(UART_NR, (uint8_t *)str, STRLEN);
                    if(pos){
                        break;
                    }
                }
                str[pos] = '\0';
                printf("CLASS: %s\n", str);

                state = 1;
                break;
        }
    }

}

