/*
 * I2C_MASTER.cpp
 *
 * Created: 31.07.2024 13:03:52
 * Author : Adrian
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>

#define F_CPU 16000000

#include <util/delay.h>

#define SLA_W 0x08
#define SLA_R 0x08

#define BUFFER_SIZE 11 

char message_buffer[BUFFER_SIZE]; 


#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void USART_Init(unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char) (ubrr >>8);
	UBRR0L = (unsigned char) ubrr;
	
	//Enable transmission and reception
	UCSR0B = (1 << RXEN0) | (1 <<TXEN0);

	//Set frame format:8data, 2 stop bit
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01) | (1 << UPM01);
	
}

void USART_TransmitString(const char* data)
{
	while (*data!= 0x00)
	{
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = *data;
		data++;
	}
}

void USART_Transmitter(char data)
{
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void i2c_init(void)
{
    // Setez SCL la 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;
    // Enable i2c
    TWCR = (1<<TWEN);
}

void i2c_start(void) {
    // Conditia de start
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

void i2c_write(uint8_t data) {
    TWDR = data;
    // Conditia de write
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

void i2c_stop(void) {
    // Conditia de stop
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR & (1 << TWSTO));
}

void i2c_write_string(const char *str) {
    while (*str) {
        i2c_write(*str);
        str++;
    }
}

uint8_t i2c_read_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

uint8_t i2c_read_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

void i2c_read_string(char *buffer, uint8_t length) 
{
	strcpy(buffer,"");
	for (uint8_t i = 0; i < length - 1; i++) 
	{	
		buffer[i] = i2c_read_ack();
	}
	buffer[length - 1] = i2c_read_nack();
	buffer[length] = '\0';

}

//FACE EXACT ACELSI LUCRU CA FUNCIA PRECEDENTA

//void i2c_read(uint8_t sla,char *buffer, uint8_t length)
//{
//	//conditie de start
//	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
//	while ((TWCR & (1<<TWINT)) == 0);
//
//	//SLA+R
//	TWDR = sla | 1;
//
//	TWCR = (1 <<TWINT) |(1 <<TWEN);
//
//	strcpy(buffer,"");
//	for (uint8_t i = 0; i < length - 1; i++)
//	{
//		buffer[i] = i2c_read_ack();
//	}
//	buffer[length - 1] = i2c_read_nack();
//	buffer[length] = '\0';
//	
//	//conditie de stop
//	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
//	while (TWCR & (1 << TWSTO));
//}

int main(void)
{
    i2c_init();
	USART_Init(MYUBRR);
    while (1)
    {
        USART_TransmitString("Tranzactia i2c a inceput. \r\n");
		i2c_start();
		 
		USART_TransmitString("Setup pemtru write. \r\n");
		i2c_write((SLA_W)<<1);
		
		USART_TransmitString("S-a transmis S1. \r\n");  
        i2c_write_string("s1");
		
		USART_TransmitString("Tranzactia i2c a incetat. \r\n");  
        i2c_stop();
       

	    _delay_ms(10);

		USART_TransmitString("Tranzactia i2c a inceput. \r\n");
		i2c_start();
		
		USART_TransmitString("Setup pentru read. \r\n");
		i2c_write((SLA_R << 1) | 1);
		
        i2c_read_string(message_buffer, BUFFER_SIZE);
 
		USART_TransmitString("Tranzactia i2c a incetat. \r\n");
		i2c_stop();
		       
			   
        _delay_ms(1000);

        i2c_start();
		USART_TransmitString("Tranzactia i2c a inceput. \r\n");
		
        i2c_write((SLA_W)<<1);
		USART_TransmitString("Setup pentru write. \r\n");
		
        i2c_write_string("s2");
		USART_TransmitString("S-a transmis S1. \r\n");
		
        i2c_stop();
        USART_TransmitString("Tranzactia i2c a incetat. \r\n"); 
		
        _delay_ms(10);

        i2c_start();
        i2c_write((SLA_R << 1) | 1);
        i2c_read_string(message_buffer, BUFFER_SIZE);
        i2c_stop();
        
        _delay_ms(1000);

        i2c_start();
        i2c_write((SLA_W)<<1);
        i2c_write_string("s3");
        i2c_stop();
        
        _delay_ms(10);

        i2c_start();
        i2c_write((SLA_R << 1) | 1);
        i2c_read_string(message_buffer, BUFFER_SIZE);
        i2c_stop();
        
        _delay_ms(1000);

        i2c_start();
        i2c_write((SLA_W)<<1);
        i2c_write_string("r3");
        i2c_stop();
        
        _delay_ms(10);

        i2c_start();
        i2c_write((SLA_R << 1) | 1);
        i2c_read_string(message_buffer, BUFFER_SIZE);
        i2c_stop();
        
        _delay_ms(1000);

        i2c_start();
        i2c_write((SLA_W)<<1);
        i2c_write_string("r2");
        i2c_stop();
        
        _delay_ms(10);

        i2c_start();
        i2c_write((SLA_R << 1) | 1);
        i2c_read_string(message_buffer, BUFFER_SIZE);
        i2c_stop();
        
        _delay_ms(1000);

        i2c_start();
        i2c_write((SLA_W)<<1);
        i2c_write_string("r1");
        i2c_stop();
        
        _delay_ms(10);

        i2c_start();
        i2c_write((SLA_R << 1) | 1);
        i2c_read_string(message_buffer, BUFFER_SIZE);
        i2c_stop();
        
        _delay_ms(1000);
		
		i2c_start();
		i2c_write((SLA_W)<<1);
		i2c_write_string("s4");
		i2c_stop();
		
		_delay_ms(10);
		
		i2c_start();
		i2c_write((SLA_R << 1) | 1);
		TWDR=0;
		i2c_read_string(message_buffer, BUFFER_SIZE);
		i2c_stop();
		
		_delay_ms(1000);
    }
}