#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

// ------------------ PRZYCISKI ------------------
// ------------------ AKTYWNY 0 ------------------

#define USER_BTN_GPIO GPIOC
#define USER_BTN_PIN 13

#define LEFT_BTN_GPIO GPIOB
#define LEFT_BTN_PIN 3
#define RIGHT_BTN_GPIO GPIOB
#define RIGHT_BTN_PIN 4
#define UP_BTN_GPIO GPIOB
#define UP_BTN_PIN 5
#define DOWN_BTN_GPIO GPIOB
#define DOWN_BTN_PIN 6
#define ACTION_BTN_GPIO GPIOB
#define ACTION_BTN_PIN 10

// ------------------ AKTYWNY 1 ------------------

#define AT_BTN_GPIO GPIOA
#define AT_BTN_PIN 0

// ------------------ DIODY ------------------

#define RED_LED_GPIO GPIOA
#define GREEN_LED_GPIO GPIOA
#define BLUE_LED_GPIO GPIOB
#define GREEN2_LED_GPIO GPIOA
#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define BLUE_LED_PIN 0
#define GREEN2_LED_PIN 5

#define RedLEDon() \
    RED_LED_GPIO->BSRR = 1 << (RED_LED_PIN + 16)
#define RedLEDoff() \
    RED_LED_GPIO->BSRR = 1 << RED_LED_PIN

#define GreenLEDon() \
    GREEN_LED_GPIO->BSRR = 1 << (GREEN_LED_PIN + 16)
#define GreenLEDoff() \
    GREEN_LED_GPIO->BSRR = 1 << GREEN_LED_PIN

#define BlueLEDon() \
    BLUE_LED_GPIO->BSRR = 1 << (BLUE_LED_PIN + 16)
#define BlueLEDoff() \
    BLUE_LED_GPIO->BSRR = 1 << BLUE_LED_PIN

#define Green2LEDon() \
    GREEN2_LED_GPIO->BSRR = 1 << GREEN2_LED_PIN
#define Green2LEDoff() \
    GREEN2_LED_GPIO->BSRR = 1 << (GREEN2_LED_PIN + 16)


// ------------------ USART ------------------

#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE
#define USART_WordLength_8b 0x0000
#define USART_Parity_No 0x0000
#define USART_FlowControl_None 0x0000
#define USART_StopBits_1 0x0000     // Liczba bitów przerwy między komunikatami.
#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ             // Zmienne do ustalenia prędkości przesyłania danych.
#define BAUD 9600U

// ------------------ USART ------------------


#define BUFFOR_SIZE 10000
#define COMMANDS_SIZE 12

static char buffor[BUFFOR_SIZE];
static unsigned ready_commands[BUFFOR_SIZE];
static unsigned it_commands = 0;
static unsigned it_end = 0;
static unsigned it = 0;

const char* commands[] = {"LR1", "LR0", "LRT", "LG1", "LG0", "LGT", "LB1", "LB0", "LBT", "Lg1", "Lg0", "LgT"};


struct cyclic_buffer {
    char* start;
    char* end;
    char* head;
    char* tail;
    unsigned size;
};

typedef struct cyclic_buffer cyclic_buffer;

void init(cyclic_buffer* c) {
    static char storage[BUFFOR_SIZE];  
    c->size = BUFFOR_SIZE;
    c->start = storage;
    c->end = c->start + c->size;
    c->head = c->start;
    c->tail = c->start;
}

bool is_empty(cyclic_buffer* c) {
    if(c->tail == c->head) {
        return true;
    }
    return false;
}

char* next_in_buffer(cyclic_buffer* c, char* pointer) {
    if(pointer + 1 == c->end) {
        return c->start;
    }
    return pointer + 1;
}

void push_back(cyclic_buffer* c, const char* s) {
    while(*s != '\0') {
        *(c->head) = *s;
        s++;
        char* next = next_in_buffer(c, c->head);
        if(next == c->tail) {
            break;
        }
        c->head = next;
    }
}

char pop_front(cyclic_buffer* c) {
    char rez = *(c->tail);
    c->tail = next_in_buffer(c, c->tail);
    return rez;
}

bool left_pressed(cyclic_buffer* c, bool was_pressed) {
    if((LEFT_BTN_GPIO->IDR >> LEFT_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "LEFT RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "LEFT PRESSED\r\n");
    }
    return true;
}

bool right_pressed(cyclic_buffer* c, bool was_pressed) {
    if((RIGHT_BTN_GPIO->IDR >> RIGHT_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "RIGHT RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "RIGHT PRESSED\r\n");
    }
    return true;
}

bool up_pressed(cyclic_buffer* c, bool was_pressed) {
    if((UP_BTN_GPIO->IDR >> UP_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "UP RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "UP PRESSED\r\n");
    }
    return true;
}

bool down_pressed(cyclic_buffer* c, bool was_pressed) {
    if((DOWN_BTN_GPIO->IDR >> DOWN_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "DOWN RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "DOWN PRESSED\r\n");
    }
    return true;
}

bool action_pressed(cyclic_buffer* c, bool was_pressed) {
    if((ACTION_BTN_GPIO->IDR >> ACTION_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "FIRE RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "FIRE PRESSED\r\n");
    }
    return true;
}

bool user_pressed(cyclic_buffer* c, bool was_pressed) {
    if((USER_BTN_GPIO->IDR >> USER_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back(c, "USER RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "USER PRESSED\r\n");
    }
    return true;
}

bool mode_pressed(cyclic_buffer* c, bool was_pressed) {
    if(!((AT_BTN_GPIO->IDR >> AT_BTN_PIN) & 1)) {
        if(was_pressed) {
            push_back(c, "MODE RELEASED\r\n");
        }
        return false;
    }
    if(!was_pressed) {
        push_back(c, "MODE PRESSED\r\n");
    }
    return true;
}


void check_buttons(cyclic_buffer* c, bool* b_pressed) {
    if(left_pressed(c, b_pressed[LEFT_BTN_PIN])) { b_pressed[LEFT_BTN_PIN] = true; } else { b_pressed[LEFT_BTN_PIN] = false; }
    if(right_pressed(c, b_pressed[RIGHT_BTN_PIN])) { b_pressed[RIGHT_BTN_PIN] = true; } else { b_pressed[RIGHT_BTN_PIN] = false; }
    if(up_pressed(c, b_pressed[UP_BTN_PIN])) { b_pressed[UP_BTN_PIN] = true; } else { b_pressed[UP_BTN_PIN] = false; }
    if(down_pressed(c, b_pressed[DOWN_BTN_PIN])) { b_pressed[DOWN_BTN_PIN] = true; } else { b_pressed[DOWN_BTN_PIN] = false; }
    if(mode_pressed(c, b_pressed[AT_BTN_PIN])) { b_pressed[AT_BTN_PIN] = true; } else { b_pressed[AT_BTN_PIN] = false; }
    if(user_pressed(c, b_pressed[USER_BTN_PIN])) { b_pressed[USER_BTN_PIN] = true; } else { b_pressed[USER_BTN_PIN] = false; }
    if(action_pressed(c, b_pressed[ACTION_BTN_PIN])) { b_pressed[ACTION_BTN_PIN] = true; } else { b_pressed[ACTION_BTN_PIN] = false; }
}

bool led_command_ready() {
    if(ready_commands[it_commands] != 0) {
        return true;
    }
    return false;
}


void led_command_execute() {
    if(ready_commands[it_commands] == 1)    RedLEDon(); 
    if(ready_commands[it_commands] == 2)    RedLEDoff(); 
    if(ready_commands[it_commands] == 4)    GreenLEDon(); 
    if(ready_commands[it_commands] == 5)    GreenLEDoff();
    if(ready_commands[it_commands] == 7)    BlueLEDon();
    if(ready_commands[it_commands] == 8)    BlueLEDoff();
    if(ready_commands[it_commands] == 10)   Green2LEDon();
    if(ready_commands[it_commands] == 11)   Green2LEDoff();
    
    if(ready_commands[it_commands] == 3) {
        if(RED_LED_GPIO->IDR & (1 << (RED_LED_PIN)))    RedLEDon();
        else    RedLEDoff();
    }
    if(ready_commands[it_commands] == 6) {
        if(GREEN_LED_GPIO->IDR & (1 << (GREEN_LED_PIN)))    GreenLEDon();
        else    GreenLEDoff();
    }
    if(ready_commands[it_commands] == 9) {
        if(BLUE_LED_GPIO->IDR & (1 << (BLUE_LED_PIN)))    BlueLEDon();
        else    BlueLEDoff();
    }
    if(ready_commands[it_commands] == 12) {
        if(GREEN2_LED_GPIO->IDR & (1 << (GREEN2_LED_PIN)))    Green2LEDoff();
        else    Green2LEDon();
    }

    it_commands++;
}

void delay_command(unsigned x) {
    ready_commands[it_end] = x;
    it_end++;
}

void led_command_check() {
    for(unsigned i = 0; i < COMMANDS_SIZE; i++) {
        if(strstr(buffor, commands[i])) {
            delay_command(i + 1);
            it = 0;
            buffor[0] = '\0';
            return;
        }
    }
}

void buffor_add(char c) {
    if(c == '\n' || c =='\r' || c == '\0') {
        led_command_check();
    }
    else {
        if(it < BUFFOR_SIZE - 1) {
            buffor[it] = c;
            buffor[it + 1] = '\0';
            it ++;
        }
        else {
            it = 0;
            buffor[0] = '\0';
        }
    }
}

void send_byte(cyclic_buffer* c) {
    if(USART2->SR & USART_SR_TXE) {
        USART2->DR = pop_front(c);
    }
}

bool handle_input() {
    if(USART2->SR & USART_SR_RXNE) {
        char c = USART2->DR;
        buffor_add(c);
        return true;
    }
    return false;
}


int main() {
    
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
    RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOafConfigure(GPIOA,
        2,
        GPIO_OType_PP,
        GPIO_Fast_Speed,
        GPIO_PuPd_NOPULL,
        GPIO_AF_USART2);
    
    GPIOafConfigure(GPIOA,
        3,
        GPIO_OType_PP,
        GPIO_Fast_Speed,
        GPIO_PuPd_UP,
        GPIO_AF_USART2); 

    USART2->CR1 = USART_Mode_Rx_Tx | USART_WordLength_8b | USART_Parity_No;
    USART2->CR2 = USART_StopBits_1;
    USART2->CR3 = USART_FlowControl_None;
    USART2->BRR = (PCLK1_HZ + (BAUD / 2U)) / BAUD;

    USART2->CR1 |= USART_Enable;

    __NOP();
    RedLEDoff();
    GreenLEDoff();
    BlueLEDoff();
    Green2LEDoff();

    GPIOoutConfigure(RED_LED_GPIO,
        RED_LED_PIN,
        GPIO_OType_PP,
        GPIO_Low_Speed,
        GPIO_PuPd_NOPULL);

    GPIOoutConfigure(GREEN_LED_GPIO,
        GREEN_LED_PIN,
        GPIO_OType_PP,
        GPIO_Low_Speed,
        GPIO_PuPd_NOPULL);
    
    GPIOoutConfigure(BLUE_LED_GPIO,
        BLUE_LED_PIN,
        GPIO_OType_PP,
        GPIO_Low_Speed,
        GPIO_PuPd_NOPULL);
    
    GPIOoutConfigure(GREEN2_LED_GPIO,
        GREEN2_LED_PIN,
        GPIO_OType_PP,
        GPIO_Low_Speed,
        GPIO_PuPd_NOPULL);


    memset(ready_commands, 0, sizeof(ready_commands));
    cyclic_buffer c;
    bool buttons_pressed[20];
    memset(buttons_pressed, false, sizeof(buttons_pressed));


    init(&c);

    for(;;) {
        if(handle_input(&c)) {
            continue;
        }

        if(led_command_ready()) {
            led_command_execute();
            continue;
        }
        
        check_buttons(&c, buttons_pressed);

        if(!is_empty(&c)) {
            send_byte(&c);
        }
    }


    return 0;
}
