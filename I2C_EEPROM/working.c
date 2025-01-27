//
// Created by iamna on 21/11/2023.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17
#define DEVADDR 0x50
#define EEPROM_SIZE 32767  // Replace with your EEPROM size
#define LED0_PIN 22  // Replace with your LED pins
#define LED1_PIN 21
#define LED2_PIN 20
#define SW0_PIN 9  // Replace with your switch pins
#define SW1_PIN 8
#define SW2_PIN 7

typedef struct ledstate {
    uint8_t state;
    uint8_t not_state;
} ledstate;


void set_led_state(ledstate *ls, uint8_t value) {
    ls->state = value;
    ls->not_state = ~value;
}

bool led_state_is_valid(ledstate *ls) {
    return ls->state == (uint8_t) ~ls->not_state;
}

void eeprom_write_bytes(uint16_t addr, uint8_t* values, uint16_t length) {
    uint8_t data[length + 2];
    data[0] = (addr >> 8) & 0xFF;  // High byte of address
    data[1] = addr & 0xFF;  // Low byte of address
    for (uint16_t i = 0; i < length; i++) {
        data[i + 2] = values[i];
    }
    i2c_write_blocking(i2c0, DEVADDR, data, length + 2, false);
    sleep_ms(100);  // Wait for internal write to complete
}

void eeprom_read_bytes(uint16_t addr, uint8_t* values, uint16_t length) {
    uint8_t data[2];
    data[0] = (addr >> 8) & 0xFF;  // High byte of address
    data[1] = addr & 0xFF;  // Low byte of address
    i2c_write_blocking(i2c0, DEVADDR, data, 2, true);  // Send address
    i2c_read_blocking(i2c0, DEVADDR, values, length, false);  // Read values
}


/*
void eeprom_read_bytes(uint16_t addr, uint8_t* values, uint16_t length) {
    uint8_t data[2];
    data[0] = (addr >> 8) & 0xFF;  // High byte of address
    data[1] = addr & 0xFF;  // Low byte of address
    int result = i2c_write_blocking(i2c0, DEVADDR, data, 2, true);  // Send address
    if (result != PICO_ERROR_NONE) {
        printf("Error writing to EEPROM: %d\n", result);
        return;
    }
    result = i2c_read_blocking(i2c0, DEVADDR, values, length, false);  // Read values
    if (result != PICO_ERROR_NONE) {
        printf("Error reading from EEPROM: %d\n", result);
        return;
    }
}
 */
void print_state(ledstate *ls){
    printf("LED 1 State: %s\n", (ls->state & 0x01) ? "ON" : "OFF");
    printf("LED 2 State: %s\n", (ls->state & 0x02) ? "ON" : "OFF");
    printf("LED 3 State: %s\n", (ls->state & 0x04) ? "ON" : "OFF");
}
void print_time(uint64_t start_time){
    uint64_t current_time = time_us_64();
    uint64_t elapsed_time = current_time - start_time;
    uint64_t elapsed_seconds = elapsed_time / 1e6;

    printf("Time elapsed: %llu seconds\n", elapsed_seconds);
}
bool pressed(uint pin) {
    int press = 0;
    int release = 0;
    while ((press <3 && release < 3)){
        if (!gpio_get(pin)) {
            press++;
            release = 0;
        } else {
            release++;
            press = 0;
        }
        sleep_ms(10);
    }
    while (!gpio_get(pin)) {
        sleep_ms(10);
    }
    if (press) {
        return true;
    } else {
        return false;
    }
}

int main() {
    stdio_init_all();
    uint64_t start_time = time_us_64();
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    gpio_init(LED0_PIN);
    gpio_init(LED1_PIN);
    gpio_init(LED2_PIN);
    gpio_set_dir(LED0_PIN, GPIO_OUT);
    gpio_set_dir(LED1_PIN, GPIO_OUT);
    gpio_set_dir(LED2_PIN, GPIO_OUT);

    //gpio_init(SW0_PIN);
    //gpio_init(SW1_PIN);
    //gpio_init(SW2_PIN);
    gpio_set_function(SW0_PIN, GPIO_FUNC_SIO);
    gpio_set_function(SW1_PIN, GPIO_FUNC_SIO);
    gpio_set_function(SW2_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(SW0_PIN, GPIO_IN);
    gpio_set_dir(SW1_PIN, GPIO_IN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
    gpio_pull_up(SW0_PIN);
    gpio_pull_up(SW1_PIN);
    gpio_pull_up(SW2_PIN);

    ledstate ls;

    eeprom_read_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));

    if (!led_state_is_valid(&ls)) {
        print_state(&ls);
        print_time(start_time);
        set_led_state(&ls, 0x02);  // Middle LED on, others off

    } else {
        print_state(&ls);
        print_time(start_time);
    }

    bool led_state_changed = false;

    while (1) {

        if (pressed(SW0_PIN)) {
            ls.state ^= 0x01;  // Toggle left LED
            ls.not_state = ~ls.state;
            led_state_changed = true;
            sleep_ms(50);  // Debounce
        }

        if (pressed(SW1_PIN)) {
            ls.state ^= 0x02;  // Toggle middle LED
            ls.not_state = ~ls.state;
            led_state_changed = true;
            sleep_ms(50);  // Debounce
        }

        if (pressed(SW2_PIN)) {
            ls.state ^= 0x04;  // Toggle right LED
            ls.not_state = ~ls.state;
            led_state_changed = true;
            sleep_ms(50);  // Debounce
        }
        if(led_state_changed){
            eeprom_write_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));
            print_state(&ls);
            print_time(start_time);
            led_state_changed = false;
        }
        gpio_put(LED0_PIN, ls.state & 0x01);
        gpio_put(LED1_PIN, ls.state & 0x02);
        gpio_put(LED2_PIN, ls.state & 0x04);


        //eeprom_write_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));


    }

    return 0;
}