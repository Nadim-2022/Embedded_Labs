#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define IN1 2
#define IN2 3
#define IN3 6
#define IN4 13
#define ADC_1 28

static int toStart = 0;

static const uint8_t clockwise[8][4] = {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,1,0},
        {0,0,1,1},
        {0,0,0,1},
        {1,0,0,1}
};
uint8_t counterClockwise[8][4] = {
        {1,0,0,1},
        {0,0,0,1},
        {0,0,1,1},
        {0,0,1,0},
        {0,1,1,0},
        {0,1,0,0},
        {1,1,0,0},
        {1,0,0,0}
};
/*
uint8_t counterClockwise[8][4] = {
        {0,0,0,1},
        {0,0,1,1},
        {0,0,1,0},
        {0,1,1,0},
        {0,1,0,0},
        {1,1,0,0},
        {1,0,0,0},
        {1,0,0,1}
};
 */
void init(){
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(IN3);
    gpio_init(IN4);
    gpio_init(ADC_1);
    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);
    gpio_set_dir(ADC_1, GPIO_IN);
    gpio_pull_up(ADC_1);

}

int calibarate(){
    int stepCount = 0;
    while(gpio_get(ADC_1) == 1){
        for (int i = toStart; i < 8; i++) {
            gpio_put(IN1, clockwise[i][0]);
            gpio_put(IN2, clockwise[i][1]);
            gpio_put(IN3, clockwise[i][2]);
            gpio_put(IN4, clockwise[i][3]);
            sleep_ms(1);
            toStart++;
            if (toStart == 8){
                toStart = 0;
            }
            if(gpio_get(ADC_1) == 0){
                break;
            }
        }
    }
    printf("Calibaration start\n");
    int count = 0;
    while (count < 3){
        while (gpio_get(ADC_1) == 0) {
            for (int i = toStart; i < 8; i++) {
                gpio_put(IN1, clockwise[i][0]);
                gpio_put(IN2, clockwise[i][1]);
                gpio_put(IN3, clockwise[i][2]);
                gpio_put(IN4, clockwise[i][3]);
                sleep_ms(1);
                stepCount++;
                toStart++;
                if (toStart == 8){
                    toStart = 0;
                }
                if(gpio_get(ADC_1) == 1){
                    break;
                }
            }
        }
        while (gpio_get(ADC_1) == 1) {
            for (int i = toStart; i < 8; i++) {
                gpio_put(IN1, clockwise[i][0]);
                gpio_put(IN2, clockwise[i][1]);
                gpio_put(IN3, clockwise[i][2]);
                gpio_put(IN4, clockwise[i][3]);
                sleep_ms(1);
                stepCount++;
                toStart++;
                if (toStart == 8){
                    toStart = 0;
                }
                if(gpio_get(ADC_1) == 0){
                    break;
                }
            }
        }
        count++;
    }
    return stepCount;
}
int run(int toStep){
    int stepCount = 0;
    while (stepCount < toStep){
        for (int i = toStart; i < 8; i++) {
            gpio_put(IN1, clockwise[i][0]);
            gpio_put(IN2, clockwise[i][1]);
            gpio_put(IN3, clockwise[i][2]);
            gpio_put(IN4, clockwise[i][3]);
            sleep_ms(1);
            stepCount++;
            toStart++;
            if (toStart == 8){
                toStart = 0;
            }
            if (stepCount == toStep){
                break;
            }
        }

    }
    return stepCount;
}
int main() {
    stdio_init_all();
    init();
    sleep_ms(2000);
    printf("Device is on..\n");;
    int count = 0;
    int steps = 0;
    //int step = (steps/3) + 24 ;
    int step = 0;
    int currentStep = 0;
    int toStep = 0;
    char command[50];
    int commandIndex = 0;
    bool calibarated = false;
    int steps_to_calibarate_from = 0;
    while(1){
        // printf("Steps: %d\n", steps);
        // If command is status print the is calibarated or not and number of steps
        // If command is calibarate then calibarate
        //If command is run + number of steps then run
        while(uart_is_readable(uart0)){
            char c = uart_getc(uart0);
            if(c == '\n'){
                command[commandIndex] = '\0';
                commandIndex = 0;
                printf("Command: %s\n", command);
                if(strcmp(command, "status") == 0){
                    if(steps > 0){
                        printf("Number of Steps: %d\n", steps);
                    } else {
                        printf("Number of Steps: Not Available\n");
                    }
                    if(calibarated){
                        printf("Calibarated: Yes\n");
                    } else {
                        printf("Calibarated: No\n");
                    }
                }
                if(strcmp(command, "calib") == 0){
                    steps = calibarate()/3;
                    step = steps/8;
                    currentStep = 0;
                    toStep = 0;
                    calibarated = true;
                }
                if(strncmp(command, "run", 3) == 0){
                    if(calibarated) {
                        int runSteps = atoi(command + 3);
                        printf("Run steps: %d\n", runSteps);
                        if (runSteps >= 1 && runSteps <= 8) {
                            if (steps > 0) {
                                toStep = (runSteps * step);
                                if (toStep > steps) {
                                    toStep = steps;
                                }
                                currentStep =  run(toStep);
                                printf("Current step: %d\n", currentStep);
                            }
                        } else if (runSteps == 0) {
                            toStep = step * 8;
                            currentStep = run(toStep);
                            printf("Current step: %d\n", currentStep);
                        } else {
                            printf("Invalid number of steps\n");
                        }
                    }
                    else {
                        printf("Calibarate First\n");
                    }
                }
            } else {
                command[commandIndex] = c;
                commandIndex++;
            }
        }

        //printf("Count: %d\n", count);




    }

    return 0;
}
