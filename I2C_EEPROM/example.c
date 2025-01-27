//
// Created by iamna on 20/11/2023.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0x50

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    while (1) {

        // sending data:
        // Memory size: 2^15= 32768
        uint16_t address = 0x55;
        uint16_t address2 = 0x56;
        uint16_t address3 = 0x57;
        uint8_t address_buffer[2];
        address_buffer[0] = address >> 8; address_buffer[1] = address;
        i2c_write_blocking(i2c0, DEVADDR, address_buffer, 2, true);
        uint8_t data = 1;
        i2c_write_blocking(i2c0, DEVADDR, (const uint8_t *) data, 1, false);
        sleep_ms(10);

        // reading data:
        i2c_write_blocking(i2c0, DEVADDR, address_buffer, 2, true);
        uint8_t received_data;
        received_data = i2c_read_blocking(i2c0, DEVADDR, (const uint8_t *) data, 1, false);
        printf("Reading data from I2C: %d\n", received_data);
        sleep_ms(1000);


    }
    return 0;
}

/*
 //
// Created by iamna on 20/11/2023.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0x50

void eeprom_write_byte(uint16_t addr, uint8_t value) {
    uint8_t data[3];
    data[0] = (addr >> 8) & 0xFF;  // High byte of address
    data[1] = addr & 0xFF;  // Low byte of address
    data[2] = value;
    i2c_write_blocking(i2c0, DEVADDR, data, 3, false);
    sleep_ms(5);  // Wait for internal write to complete
}

uint8_t eeprom_read_byte(uint16_t addr) {
    uint8_t data[2];
    data[0] = (addr >> 8) & 0xFF;  // High byte of address
    data[1] = addr & 0xFF;  // Low byte of address
    i2c_write_blocking(i2c0, DEVADDR, data, 2, false);  // Send address
    uint8_t value;
    i2c_read_blocking(i2c0, DEVADDR, &value, 1, false);  // Read value
    return value;
}

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    // Test write and read
    eeprom_write_byte(0, 0xA5);
    eeprom_write_byte(1, 0xBC);
    uint8_t value0 = eeprom_read_byte(0);
    uint8_t value1 = eeprom_read_byte(1);
    printf("Data at address 0: %02X\n", value0);
    printf("Data at address 1: %02X\n", value1);

    return 0;
}
 */

/*
 //
// Created by iamna on 20/11/2023.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17

#define DEVADDR 0x50

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

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    // Test write and read
    uint8_t write_values[2] = {0xA5, 0xBC};
    eeprom_write_bytes(0, write_values, 2);
    uint8_t read_values[2];
    eeprom_read_bytes(0, read_values, 2);
    printf("Data at address 0: %02X\n", read_values[0]);
    printf("Data at address 1: %02X\n", read_values[1]);

    return 0;
}
 */

/*
 #include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17
#define DEVADDR 0x50
#define EEPROM_SIZE 32768  // Replace with your EEPROM size
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
    if (press) {
        return true;
    } else {
        return false;
    }
}

int main() {
    stdio_init_all();
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
    printf("LED 0 state: %d\n", (ls.state >> 0) & 1);
    printf("LED 1 state: %d\n", (ls.state >> 1) & 1);
    printf("LED 2 state: %d\n", (ls.state >> 2) & 1);
    if (!led_state_is_valid(&ls)) {
        set_led_state(&ls, 0x02);  // Middle LED on, others off
    }
    printf("LED state: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (ls.state >> i) & 1);
    }
    printf("\n");

    uint64_t start_time = time_us_64();

    while (1) {
        if (pressed(SW0_PIN)) {
            ls.state ^= 0x01;  // Toggle left LED
            sleep_ms(200);  // Debounce
        }

        if (pressed(SW1_PIN)) {
            ls.state ^= 0x02;  // Toggle middle LED
            sleep_ms(200);  // Debounce
        }

        if (pressed(SW2_PIN)) {
            ls.state ^= 0x04;  // Toggle right LED
            sleep_ms(200);  // Debounce
        }

        gpio_put(LED0_PIN, ls.state & 0x01);
        gpio_put(LED1_PIN, ls.state & 0x02);
        gpio_put(LED2_PIN, ls.state & 0x04);

        if (time_us_64() - start_time >= 1000000) {  // Every second
            start_time = time_us_64();
            eeprom_write_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));
        }
    }

    return 0;
}
 */