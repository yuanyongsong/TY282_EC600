#ifndef USR_UART_H
#define USR_UART_H

#include "usr_main.h"

#define UART1_BUF_LEN 3000       
#define UART2_BUF_LEN 1000
#define UART3_BUF_LEN 100
#define UART4_BUF_LEN 100

#define AT_PORT USART1

extern unsigned char Uart1RecCnt;  
extern unsigned char Uart2RecCnt;  
extern unsigned char Uart3RecCnt;  
extern unsigned char Uart4RecCnt;  
extern unsigned short Uart1Index;
extern unsigned short Uart2Index;
extern char Uart1Buf[UART1_BUF_LEN];   
extern char Uart2Buf[UART2_BUF_LEN];   
extern char Uart4Buf[UART4_BUF_LEN];


void UART_Init(void);
void UART_AtInit(void);
void UART_Send(USART_TypeDef *USARTx, uint8_t *data,uint16_t dataleng);

void UART_Handle(void);
void At_Receive(void);

#endif

