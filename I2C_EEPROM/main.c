//
// Created by iamna on 21/11/2023.
//

/*
 Improve Exercise 1 by adding a persistent log that stores messages to EEPROM. When the program starts it
writes “Boot” to the log and every time when state or LEDs is changed the state change message, as
described in Exercise 1, is also written to the log.
The log must have the following properties:
• Log starts from address 0 in the EEPROM.
• First two kilobytes (2048 bytes) of EEPROM are used for the log.
• Log is persistent, after power up writing to log continues from location where it left off last time.
The log is erased only when it is full or by user command.
• Each log entry is reserved 64 bytes.
o First entry is at address 0, second at address 64, third at address 128, etc.
o Log can contain up to 32 entries.
• A log entry consists of a string that contains maximum 61 characters, a terminating null character
(zero) and two-byte CRC that is used to validate the integrity of the data. A maximum length log
entry uses all 64 bytes. A shorter entry will not use all reserved bytes. The string must contain at
least one character.
• When a log entry is written to the log, the string is written to the log including the terminating zero.
Immediately after the terminating zero follows a 16-bit CRC, MSB first followed by LSB.
o Entry is written to the first unused (invalid) location that is available.
o If the log is full then the log is erased first and then entry is written to address 0.
• User can read the content of the log by typing read and pressing enter.
o Program starts reading and validating log entries starting from address zero. If a valid string
is found it is printed and program reads string from the next location.
o A string is valid if the first character is not zero, there is a zero in the string before index 62,
and the string passes the CRC validation.
o Printing stops when an invalid string is encountered or the end log are is reached.
• User can erase the log by typing erase and pressing enter.
o Erasing is done by writing a zero at the first byte of every log entry.
Figure 1 Structure of log in EEPROM
Use following code for CRC-calculation
(adapted from: https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum )
uint16_t crc16(const uint8_t *data_p, size_t length) {
 uint8_t x;
 uint16_t crc = 0xFFFF;
 while (length--) {
 x = crc >> 8 ^ *data_p++;
 x ^= x >> 4;
 crc = (crc << 8) ^ ((uint16_t) (x << 12)) ^ ((uint16_t) (x << 5)) ^ ((uint16_t) x);
 }
 return crc;
}
The property of CRC is such that when a CRC that is calculated over a number of data bytes is placed
immediately after the bytes and the CRC is calculated over the data bytes plus the CRC-bytes the result is
zero, provided that the data has not been modified after CRC was calculated.
For example:
uint8_t buffer[10] = { 51, 32, 93, 84, 75, 16, 17, 28 };
uint16_t crc = crc16(buffer, 8);
// put CRC after data
buffer[8] = (uint8_t) (crc >> 8);
buffer[9] = (uint8_t) crc;
// validate data
if(crc16(buffer, 10) != 0) {
 printf("Error\n");
}
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <string.h>

#define I2C0_SDA_PIN 16
#define I2C0_SCL_PIN 17
#define DEVADDR 0x50
#define EEPROM_SIZE 32767  // Replace with your EEPROM size
#define LOG_ENTRY_ADD_MAX 2048  // Size of the log area in EEPROM
#define LOG_ENTRY_SIZE 64
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

typedef struct log_entry {
    char message[64];
} log_entry;

uint16_t address = 0;

uint16_t crc16(const uint8_t *data_p, size_t length) {
    uint8_t x;
    uint16_t crc = 0xFFFF;
    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
    }
    return crc;
}

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

void erase_log(){
    uint8_t data[LOG_ENTRY_SIZE] = {0};
    uint8_t read_data[LOG_ENTRY_SIZE];
    while (address < LOG_ENTRY_ADD_MAX){
        eeprom_write_bytes(address, data, LOG_ENTRY_SIZE);
        address += LOG_ENTRY_SIZE;
    }
    address = 0;
}

void write_log(log_entry *le){
    uint8_t data[LOG_ENTRY_SIZE]; // 64 bytes
    size_t le_size = strlen(le->message);
    for(int i = 0; i < le_size; i++){
        data[i] = le->message[i];
    }
    if(le_size > 0 && le_size < 62) {
        data[le_size] = '\0';
        uint16_t crc = crc16(data, 62);
        data[le_size + 1] = (uint8_t)(crc >> 8);
        data[le_size + 2] = (uint8_t) crc;
        uint8_t read_data[LOG_ENTRY_SIZE];
        if(address < LOG_ENTRY_ADD_MAX){
            eeprom_read_bytes(address, read_data, LOG_ENTRY_SIZE);
            printf("addr: %d\n", address);
            if(read_data[0] == 0){
                eeprom_write_bytes(address, data, LOG_ENTRY_SIZE);

            }
            address += LOG_ENTRY_SIZE;
        }
        else{
            printf("delete\n");
            address = 0;
            erase_log();
        }
    }

}

void read_log(){
    uint8_t data[LOG_ENTRY_SIZE];
    uint16_t addr = 0;
    uint8_t read_data[LOG_ENTRY_SIZE];
    printf("LOG:\n");
    while (addr < LOG_ENTRY_ADD_MAX){
        eeprom_read_bytes(addr, read_data, LOG_ENTRY_SIZE);
        if(read_data[0] != 0) {
            if (strchr(read_data, '\0') && strchr(read_data, '\0') < &read_data[62]){
                if(crc16(read_data, 64) != 0    ) {
                    printf("%s\n", read_data);
                }
            }
        }
        //printf("%s\n", read_data);
        addr += LOG_ENTRY_SIZE;
    }
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
uint64_t get_elapsed_seconds(uint64_t start_time) {
    uint64_t current_time = time_us_64();
    uint64_t elapsed_time = current_time - start_time;
    uint64_t elapsed_seconds = elapsed_time / 1e6;
    return elapsed_seconds;
}
//read state to initialize address
void read_address(){
    uint8_t data[LOG_ENTRY_SIZE];
    uint8_t read_data[LOG_ENTRY_SIZE];
    while (address < LOG_ENTRY_ADD_MAX){
        eeprom_read_bytes(address, read_data, LOG_ENTRY_SIZE);
        if(read_data[0] != 0) {
            if (strchr(read_data, '\0') && strchr(read_data, '\0') < &read_data[62]){
                if(crc16(read_data, 64) != 0    ) {
                    address += LOG_ENTRY_SIZE;
                }
            }
        }
        else{
            break;
        }
    }
}

int main() {
    stdio_init_all();
    uint64_t start_time = time_us_64();
    uart_is_readable(uart0);
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
    log_entry le;

    eeprom_read_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));

    // write Boot to log



    if (!led_state_is_valid(&ls)) {
        print_state(&ls);
        print_time(start_time);
        set_led_state(&ls, 0x02);  // Middle LED on, others off

    } else {
        gpio_put(LED0_PIN, ls.state & 0x01);
        gpio_put(LED1_PIN, ls.state & 0x02);
        gpio_put(LED2_PIN, ls.state & 0x04);
        print_state(&ls);
        print_time(start_time);
    }

    bool led_state_changed = false;

    read_address();
    char input[256];
    //erase_log();
    strcpy(le.message, "Boot");
    write_log(&le);
    //read_log();
    int i = 0;
    char command[50];
    int command_index = 0;
    while (1) {

        if (pressed(SW0_PIN)) { // LED 2
            // ls.state ^= 0x01;  // Toggle left LED
            // ls.not_state = ~ls.state;
            uint64_t elapsed_seconds = get_elapsed_seconds(start_time);
            led_state_changed = true;
            if(!(ls.state & 0x01)){
                set_led_state(&ls, ls.state | 0x01);
                sprintf(le.message, "LOG: LED 1 State: ON, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 1 State: ON");
            }else{
                set_led_state(&ls, ls.state & ~0x01);
                sprintf(le.message, "LOG: LED 1 State: OFF, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 1 State: OFF");
            }
        }

        if (pressed(SW1_PIN)) { //LED 2
            // ls.state ^= 0x02;  // Toggle middle LED
            //ls.not_state = ~ls.state;
            uint64_t elapsed_seconds = get_elapsed_seconds(start_time);
            led_state_changed = true;
            if(!(ls.state & 0x02)){
                set_led_state(&ls, ls.state | 0x02);
                sprintf(le.message, "LOG: LED 2 State: ON, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 2 State: ON");
            }else{
                set_led_state(&ls, ls.state & ~0x02);
                sprintf(le.message, "LOG: LED 2 State: OFF, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 2 State: OFF");
            }
        }

        if (pressed(SW2_PIN)) { // LED 3
            //ls.state ^= 0x04;  // Toggle right LED
            //ls.not_state = ~ls.state;
            uint64_t elapsed_seconds = get_elapsed_seconds(start_time);
            led_state_changed = true;
            if(!(ls.state & 0x04)){
                set_led_state(&ls, ls.state |0x04);
                sprintf(le.message, "LOG: LED 3 State: ON, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 3 State: ON");
            }else{
                set_led_state(&ls, ls.state & ~0x04);
                sprintf(le.message, "LOG: LED 3 State: OFF, Timestamp: %llu seconds", elapsed_seconds);
                //strcpy(le.message, "LOG: LED 3 State: OFF");
            }
        }
        if(led_state_changed){
            gpio_put(LED0_PIN, ls.state & 0x01);
            gpio_put(LED1_PIN, ls.state & 0x02);
            gpio_put(LED2_PIN, ls.state & 0x04);
            eeprom_write_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));
            write_log(&le);
            read_log();
            print_state(&ls);
            print_time(start_time);
            led_state_changed = false;

        }
        while (uart_is_readable(uart0)) {
            char input_char = uart_getc(uart0);
            if (input_char == '\n' || command_index >= 50 - 1) {
                command[command_index] = '\0';
                if (strcmp(command, "read") == 0) {
                    read_log();
                } else if (strcmp(command, "erase") == 0) {
                    erase_log();
                    erase_log();
                } else{
                    printf("Invalid command\n");
                }
                command_index = 0;
            } else {
                command[command_index++] = input_char;
            }
        }


        /*
        if(fgets(input, 256, stdin)){
            if(strcmp(input, "read\n") == 0){
                read_log();
            }else if(strcmp(input, "erase\n") == 0){
                erase_log();
            }
        }
        */
        //eeprom_write_bytes(EEPROM_SIZE - sizeof(ledstate), (uint8_t*)&ls, sizeof(ledstate));


    }

    return 0;
}