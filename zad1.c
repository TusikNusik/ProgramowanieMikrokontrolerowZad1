#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#define NO_BUTTON 20

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

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
// Nie uzywac funkcji z wykladu bo sa blokujace, napisac wlasna implementacje ze sladow 18, 19.

// Aby wypisywac na wyjscie musze orzystac z programu posredniczacego minicom, wywolanie minicom i trzeba go skonfigurowac w pliku
// ktory jest w katalogu domowym.

// TODO: zaimplementowac bufor nadawczy, odbiorczy w tym nadawczy cykliczny, a takze pattern matching na instrukcje odbierane.

#define BUFFOR_SIZE 1000

struct cyclic_buffer {
    char* start;
    char* end;
    char* head;
    char* tail;
    unsigned size;
};

typedef struct cyclic_buffer cyclic_buffer;

void init(cyclic_buffer* c) {
    c->size = BUFFOR_SIZE;
    c->start = (char*)malloc(sizeof(char) * BUFFOR_SIZE);
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
        *s++;
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
    return r
}

bool left_pressed(cyclic_buffer* c, bool was_pressed) {
    if((LEFT_BTN_GPIO->IDR >> LEFT_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("LEFT RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("LEFT PRESSED\n");
    }
    return true;
}

bool right_pressed(cyclic_buffer* c, bool was_pressed) {
    if((RIGHT_BTN_GPIO->IDR >> RIGHT_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("RIGHT RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("RIGHT PRESSED\n");
    }
    return true;
}

bool up_pressed(cyclic_buffer* c, bool was_pressed) {
    if((UP_BTN_GPIO->IDR >> UP_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("UP RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("UP PRESSED\n");
    }
    return true;
}

bool down_pressed(cyclic_buffer* c, bool was_pressed) {
    if((DOWN_BTN_GPIO->IDR >> DOWN_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("DOWN RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("DOWN PRESSED\n");
    }
    return true;
}

bool action_pressed(cyclic_buffer* c, bool was_pressed) {
    if((ACTION_BTN_GPIO->IDR >> ACTION_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("FIRE RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("FIRE PRESSED\n");
    }
    return true;
}

bool user_pressed(cyclic_buffer* c, bool was_pressed) {
    if((USER_BTN_GPIO->IDR >> USER_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("USER RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("USER PRESSED\n");
    }
    return true;
}

bool mode_pressed(cyclic_buffer* c, bool was_pressed) {
    if(!(AT_BTN_GPIO->IDR >> AT_BTN_PIN) & 1) {
        if(was_pressed) {
            push_back("MODE RELEASED");
        }
        return false;
    }
    if(!was_pressed) {
        push_back("MODE PRESSED\n");
    }
    return true;
}


unsigned check_buttons(cyclic_buffer* c, bool x) {
    if(left_pressed(c, x)) return LEFT_BTN_PIN;
    if(right_pressed(c, x)) return RIGHT_BTN_PIN;
    if(up_pressed(c, x)) return UP_BTN_PIN;
    if(down_pressed(c, x)) return DOWN_BTN_PIN;
    if(mode_pressed(c, x)) return AT_BTN_PIN;
    if(user_pressed(c, x)) return USER_BTN_PIN;
    if(action_pressed(c, x)) return ACTION_BTN_PIN;

    return NO_BUTTON;
}

void handle_buttons(cyclic_buffer* c, unsigned* active_button) {
    if(*active_button == NO_BUTTON) {
        *active_button = check_buttons(c, false);
    }
    else {
        if(*active_button == LEFT_BTN_PIN) {
            if(left_pressed(c, true))
                return;
            else
                *active_button = NO_BUTTON;
        }
    }
}


void send_byte(char x) {
    
    while(!(USART2->SR & USART_SR_TXE));
    USART2->DR = x;
    
    
}

bool handle_input(unsigned it) {
    if(USART2->SR & USART_SR_RXNE) {
        buffor[it] = USART2->DR;
        it++;
        return true;
    }
    return false;
}

int main() {

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
    RCC_AHB1ENR_GPIOBEN;

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


    
    unsigned it = 0;
    unsigned active_button = 20;
    bool is_active = false;

    for(;;) {
        //if(handle_input(it)) {
        //    continue;
        //}
        
        handle_buttons(&active_button);

    }


    return 0;
}
