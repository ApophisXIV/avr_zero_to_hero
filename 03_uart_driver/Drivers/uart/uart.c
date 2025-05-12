#include "uart.h"
#include "../../board.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/setbaud.h>

static int UART_write_byte(int write_byte, FILE *stream);
static int UART_read_byte(FILE *stream);

static FILE UART_stdin  = FDEV_SETUP_STREAM(NULL, UART_read_byte, _FDEV_SETUP_READ);
static FILE UART_stdout = FDEV_SETUP_STREAM(UART_write_byte, NULL, _FDEV_SETUP_WRITE);

void UART_init(void) {
    UBRR0H = UBRRH_VALUE;    // Mirar setbaud.h
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);      // Enable RX and TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);    // 8 data bits, no parity, 1 stop bit

    stdin  = &UART_stdin;
    stdout = &UART_stdout;
}

// void UART_IT_init(void) {
//     UART_init();
//     UCSR0B |= (1 << RXCIE0);    // Enable RX interrupt
//     UCSR0B |= (1 << TXCIE0);    // Enable TX interrupt
// }

bool UART_is_available(void) {
    return UCSR0A & (1 << RXC0);
}

static int UART_write_byte(int write_byte, FILE *stream) {
    loop_until_bit_is_set(UCSR0A, UDRE0);    // Wait for empty transmit buffer
    UDR0 = write_byte;                       // write one byte to UART0
    return 0;
}

static int UART_read_byte(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);    // Wait for incoming data
    return UDR0;                            // read one byte from UART0
}

ISR(USART_TX_vect) {
}

ISR(USART_RX_vect) {
}