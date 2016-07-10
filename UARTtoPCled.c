/*  using a serial port terminal,   tm4c123 asks user to input an r,g, or b, and changes led color accordingly. */

/*in the future I would like to create a lil ting that displays my ADC value over the UART terminal*/
/*but this ditty is bitchin now that it works.*/
/*a small but very loud error exists right now, puTTy terminal is still diaplaying a large amount of garbage */
/*which could be the result of a mis-configured puTTy.  Works fine when I switch over to coolterm OS X*/ 



#include <lm4f120h5qr.h>
#include <string.h>
#include <stdlib.h>

char readChar(void);
void printChar(char c);
void printString(char * string);
char* readString(char delimiter);

int main(void) 
{
    char c;
    SYSCTL->RCGCUART |= (1<<0);       // 1. UART enable , RCGCUART register (see page 344). (1<<1) would enable RCGC on UART1, etc.
    SYSCTL->RCGCGPIO |= (1<<0);       // 2.GPIO module clock enable (RCGCGPIO register (pg340) Table 23-5 on page 1351.)
    GPIOA->AFSEL = (1<<1)|(1<<0);     // 3. Set the GPIO AFSEL bits for appr pins (pg 671).
        // To determine which GPIOs toconfigure, see Table 23-4 on page 1344
        // 4. Config GPIO current level / slew rate as specified for the mode selected (see pg673, pg 681)
    GPIOA->PCTL = (1<<0)|(1<<4);      // 5. Config PMCn bits/GPIOPCTL_R, assigns the UART signals to pins (pg688 and Tbl 23-5 pg1351).
    GPIOA->DEN = (1<<0)|(1<<1); 
    
    // Find  the Baud-Rate Divisor
    // BRD = 16,000,000 / (16 * 9600) = 104.16666666666666666666666666666666666666666666666666
    // UARTFBRD[DIVFRAC] = integer(0.166667 * 64 + 0.5) = 11

/***********With the BRD values in hand***********/
/***********UART config goes like this:***********/
    UART0->CTL &= ~(1<<0);     // 1. Disable the UART by clearing the UARTEN bit in the UARTCTL register
    UART0->IBRD = 104;         // 2. Write the integer portion of the BRD to the UARTIBRD register  
    UART0->FBRD = 11;          // 3. Write the fractional portion of the BRD to the UARTFBRD register
    UART0->LCRH = (0x3<<5)|(1<<4);  // 4.  UARTLCRH register serial parameters(a value of 0x0000.0060 8-bit, no parity, 1-stop bit)
    UART0->CC = 0x0;          // 5. Configure the UART clock source by writing to the UARTCC register
                             // 6. Optionally, configure the µDMA channel (see “Micro Direct Memory Access (µDMA)” on page 585)
    UART0->CTL = (1<<0)|(1<<8)|(1<<9);  // 7. Enable the UART by setting the UARTEN bit in the UARTCTL register.

/************ Configure LED pins************/
    SYSCTL->RCGCGPIO |= (1<<5); // enable clock on PortF
    GPIOF->DIR = (1<<1)|(1<<2)|(1<<3);  // make LED pins (PF1, PF2, and PF3) outputs
    GPIOF->DEN = (1<<1)|(1<<2)|(1<<3); // enable digital function on LED pins
    GPIOF->DATA &= ~((1<<1)|(1<<2)|(1<<3)); // turn off leds

    while(1)
    {
      printString("Enter \"r\", \"g\", or\"b\":\n\r");
      
        c = readChar();
       // char *string = readString('\r');  //*pos changed for readibility
        printChar(c);
       // printString("You typed: ");
       // printString(string);
        printString("\n\r");
        switch(c)
        {
        case 'r':
          GPIOF->DATA = (1 << 1);   //red led
          break;
        case 'b':
          GPIOF->DATA = (1 << 2);   //blue led
          break;
        case 'g':
          GPIOF->DATA = (1 << 3);   //green led
          break;
        }
        
        //free(string);
    }
}

char readChar(void)  
{
    char c;                             //UART flag register is 0 when tx is empty
    while((UART0->FR & (1<<4)) != 0);   //make sure prev transmission is finished
    c = UART0->DR;                      //waits till user inputs data into reg...               
    return c;                           //c is crested from readChar
}                                       //dumps "c" data into data register 

void printChar(char c)  
{
    while((UART0->FR & (1<<5)) != 0);
    UART0->DR = c;           
}

void printString(char * string)
{
  while(*string)                        //while pointer is not null
  {
    printChar(*(string++));
  }
}

char* readString(char delimiter)
{

  int stringSize = 0;
  char* string = (char*)calloc(10,sizeof(char));
  char c = readChar(); 
  printChar(c);
  
  while(c!=delimiter)
  { 

    *(string+stringSize) = c; // put the new character at the end of the array
    stringSize++;
    
    if((stringSize%10) == 0) // string length has reached multiple of 10
    {
      string = (char*)realloc(string,(stringSize+10)*sizeof(char)); // adjust string size by increasing size by another 10
    }
    
    c = readChar();
    printChar(c); // display the character the user typed
  }

  if(stringSize == 0)
  {
   
    return '\0'; // null car
  }
  return string;
}
