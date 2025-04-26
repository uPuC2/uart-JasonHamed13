/* ===========  UART.c  =========== */
#include <avr/io.h>
#include "UART.h"

#define F_CPU 16000000UL

const uint8_t _ansi_colors[] = {33, 32, 34};

/* --- guardamos la configuracion de paridad de cada UART --- */
static uint8_t _uart_parity[4] = {0};

void UART_Ini(uint8_t com, uint32_t baudrate,
              uint8_t size, uint8_t parity, uint8_t stop)
{
    uint16_t ubrr = (F_CPU / (16 * baudrate)) - 1;
    _uart_parity[com] = parity;          /* 0-ninguna, 1-impar, 2-par */

    switch(com) {
    case 0:
        UBRR0H = ubrr >> 8;  UBRR0L = ubrr;
        UCSR0B = (1<<RXEN0)|(1<<TXEN0);
        UCSR0C = ((size==8)<<UCSZ01)|((size>=8)<<UCSZ00);
        if(parity==1) UCSR0C |= (1<<UPM01)|(1<<UPM00);
        else if(parity==2) UCSR0C |= (1<<UPM01);
        if(stop==2) UCSR0C |= (1<<USBS0);
        break;

    case 2:
        UBRR2H = ubrr >> 8;  UBRR2L = ubrr;
        UCSR2B = (1<<RXEN2)|(1<<TXEN2);
        UCSR2C = ((size==8)<<UCSZ21)|((size>=8)<<UCSZ20);
        if(parity==1) UCSR2C |= (1<<UPM21)|(1<<UPM20);
        else if(parity==2) UCSR2C |= (1<<UPM21);
        if(stop==2) UCSR2C |= (1<<USBS2);
        break;

    case 3:
        UBRR3H = ubrr >> 8;  UBRR3L = ubrr;
        UCSR3B = (1<<RXEN3)|(1<<TXEN3);
        UCSR3C = ((size==8)<<UCSZ31)|((size>=8)<<UCSZ30);
        if(parity==1) UCSR3C |= (1<<UPM31)|(1<<UPM30);
        else if(parity==2) UCSR3C |= (1<<UPM31);
        if(stop==2) UCSR3C |= (1<<USBS3);
        break;
    }
}

/* --- envio de un byte --- */
static inline void _uart_tx(uint8_t com, char data)
{
    switch(com){
    case 0: while(!(UCSR0A & (1<<UDRE0))); UDR0 = data; break;
    case 2: while(!(UCSR2A & (1<<UDRE2))); UDR2 = data; break;
    case 3: while(!(UCSR3A & (1<<UDRE3))); UDR3 = data; break;
    }
}

void UART_putchar(uint8_t com, char data) { _uart_tx(com,data); }

/* ----------  _puts()  ------------ */
void UART_puts(uint8_t com, char *s)
{
    while (*s) {
        if (*s == '\n')                    /* LF -> CR LF */
            _uart_tx(com, '\r');

        _uart_tx(com, *s++);               /* envia el byte tal cual   */
    }
}
/* -------------------------------------------- */

uint8_t UART_available(uint8_t com){
    switch(com){
    case 0: return (UCSR0A & (1<<RXC0));
    case 2: return (UCSR2A & (1<<RXC2));
    case 3: return (UCSR3A & (1<<RXC3));
    default: return 0;
    }
}

char UART_getchar(uint8_t com){
    while(!UART_available(com));
    switch(com){
    case 0: return UDR0;
    case 2: return UDR2;
    case 3: return UDR3;
    default: return 0;
    }
}

void UART_gets(uint8_t com, char *str)
{
    uint8_t idx = 0;
    for(;;){
        char c = UART_getchar(com);
        if(c=='\r'||c=='\n'){ str[idx]='\0'; UART_puts(com,"\r\n"); break; }
        else if((c==8||c==127) && idx){ idx--; str[idx]='\0'; UART_puts(com,"\b \b"); }
        else if(idx<19 && ((c>='0'&&c<='9')||c=='.')){ str[idx++]=c; _uart_tx(com,c); }
    }
}

void UART_clrscr(uint8_t com){ UART_puts(com, "\x1B[2J\x1B[H"); }

void UART_setColor(uint8_t com, uint8_t color){
    char buf[8];
    itoa(_ansi_colors[color], buf, 10);
    UART_puts(com, "\x1B["); UART_puts(com, buf); _uart_tx(com,'m');
}

void UART_gotoxy(uint8_t com,uint8_t x,uint8_t y){
    char buf[8];
    UART_puts(com, "\x1B["); itoa(y,buf,10); UART_puts(com,buf);
    _uart_tx(com,';');       itoa(x,buf,10); UART_puts(com,buf);
    _uart_tx(com,'H');
}

/* -------------  itoa / atoi ------------- */
void itoa(uint16_t v,char* str,uint8_t base){
    char tmp[17]; uint8_t i=0;
    if(base<2) base=10;
    do{
        uint16_t r = v % base;
        tmp[i++] = (r<10)?('0'+r):('A'+r-10);
        v/=base;
    }while(v && i<sizeof(tmp));
    uint8_t j=0; while(i) str[j++]=tmp[--i]; str[j]='\0';
}
uint16_t atoi(char *s){
    uint16_t v=0;
    while(*s && *s!='.'){
        if(*s>='0'&&*s<='9') v=v*10+(*s-'0'); else break; s++;
    }
    return v;
}
