// UART2.c
// Revised UART2 driver modeled after UART5.c for TM4C123
// PD7 = U2Tx, PD6 = U2Rx (PD7 requires unlocking)
// Author: Modified to mimic UART5.c

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define UART2_FR_TXFF         0x00000020  // UART Transmit FIFO Full
#define UART2_FR_RXFE         0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8      0x00000060  // 8-bit word length
#define UART_LCRH_FEN         0x00000010  // Enable FIFOs
#define UART_CTL_UARTEN       0x00000001  // UART Enable
#define CR                    0x0D
#define LF                    0x0A

//*****************************************************************************
// UART2_Init
// Initializes UART2 for 8-bit, 1-stop, no parity, FIFOs enabled.
// Configures PD7 (Tx) and PD6 (Rx). Baud rate is computed assuming 80 MHz sysclk.
//*****************************************************************************
void UART2_Init(uint32_t baud) {
  uint32_t brd, remainder;
  
  // Enable UART2 clock (bit 2) and wait until it is ready.
  SYSCTL_RCGCUART_R |= 0x04;
  while ((SYSCTL_PRUART_R & 0x04) == 0) {};
  
  // Enable clock for Port D (bit 3) and wait until ready.
  SYSCTL_RCGCGPIO_R |= 0x08;
  while ((SYSCTL_PRGPIO_R & 0x08) == 0) {};
  
  // Unlock PD7 for reconfiguration (PD7 is a locked NMI pin) and allow changes on PD7 and PD6.
  GPIO_PORTD_LOCK_R = 0x4C4F434B;      // Unlock Port D
  GPIO_PORTD_CR_R |= 0xC0;             // Commit PD6 and PD7
  
  // Disable analog functionality on PD6 and PD7.
  GPIO_PORTD_AMSEL_R &= ~0xC0;
  
  // Enable alternate function on PD6 (U2Rx) and PD7 (U2Tx).
  GPIO_PORTD_AFSEL_R |= 0xC0;
  
  // Set port control to assign PD6 and PD7 to UART2.
  // Clear previous PCTL settings for these pins and set to 0x1 for UART.
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R & 0x00FFFFFF) | 0x11000000;
  
  // Enable digital functionality on PD6 and PD7.
  GPIO_PORTD_DEN_R |= 0xC0;
  
  // Disable UART2 while configuring.
  UART2_CTL_R &= ~UART_CTL_UARTEN;
  
  // Calculate baud rate divisor:
  //   BRD = System Clock / (16 * baud)
  brd = 80000000 / (16 * baud);
  remainder = 80000000 % (16 * baud);
  UART2_IBRD_R = brd;
  UART2_FBRD_R = ((remainder * 64) + (baud / 2)) / baud;
  
  // Set line control for 8-bit, no parity, 1 stop bit, and enable FIFOs.
  UART2_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
  
  // Clear FIFO interrupt level fields (optional if not using interrupts).
  UART2_IFLS_R &= ~0x3F;
  
  // Enable UART2.
  UART2_CTL_R |= UART_CTL_UARTEN;
}

//*****************************************************************************
// UART2_OutChar
// Busy-wait until space exists in the FIFO, then output a character.
//*****************************************************************************
void UART2_OutChar(char data) {
  while ((UART2_FR_R & UART2_FR_TXFF) != 0) {} // Wait for room in FIFO
  UART2_DR_R = data;
}

//*****************************************************************************
// UART2_OutString
// Outputs a NULL-terminated string via UART2.
//*****************************************************************************
void UART2_OutString(char *pt) {
  while (*pt) {
    UART2_OutChar(*pt);
    pt++;
  }
}

//*****************************************************************************
// UART2_InChar
// Waits for and receives a character from UART2.
//*****************************************************************************
char UART2_InChar(void) {
  while ((UART2_FR_R & UART2_FR_RXFE) != 0) {} // Wait for input
  return (char)(UART2_DR_R & 0xFF);
}

//*****************************************************************************
// UART2_Out_CRLF
// Outputs CR and LF to start a new line on the terminal.
//*****************************************************************************
void UART2_Out_CRLF(void) {
  UART2_OutChar(CR);
  UART2_OutChar(LF);
}
