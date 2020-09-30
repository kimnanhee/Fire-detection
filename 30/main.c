#define F_CPU 16000000UL
#define BAUDRATE(x) ((F_CPU/16/x)-1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "sensor.h"
#include "uart.h"
#include "lcd.h"

int main(void)
{
	DDRA = 0xFF; // 4bit LCD사용
	PORTA = 0x00;
	DDRF = 0x00; // 가스, 불꽃, 온도센서 입력핀
    DDRC = 0xFF; // 부저, 팬모터, 릴레이 출력핀
	PORTC = 0x04; // 평상시 포트 출력
	int state = 0;
	
	uart_init(BAUDRATE(9600)); // baudrate 설정
	sei();
	
	LCD_init();
	
	float temp = 0, gas = 0;
	int temp_state = 0, gas_state = 0, fire_state = 0;
	
    while (1) 
    {
		float temp = temp_sensor_read();
		if(temp > 30.0) temp_state = 1; // 30도 이상일때
		else temp_state = 0;
		_delay_ms(100);
		
		float gas = gas_sensor_read();
		if(temp > 3.0) gas_state = 1; // 3.0이상일때
		else gas_state = 0;
		_delay_ms(100);
		
		int fire_state = fire_sensor_read();
		_delay_ms(100);
		
		if(temp_state==0 && gas_state==0 && fire_state==0) state = 0; // 평상
		else if(temp_state==1 && gas_state==1 && fire_state==1) state = 4; // 화재
		else if(temp_state==1 && gas_state==0 && fire_state==0) state = 1; // 온도
		else if(temp_state==0 && gas_state==1 && fire_state==0) state = 2; // 가스
		else if(temp_state==0 && gas_state==0 && fire_state==1) state = 3; // 불꽃
		else state = 5;
		
		switch(state)
		{
			case 0: // 평상
			PORTC = 0x04;
			break; 
			
			case 1: // 온도 or 가스 이상
			case 2:
			PORTC = 0x06;
			break;
			
			case 3: // 불꽃 이상
			PORTC = 0x04;
			break;
			
			case 4: // 화재
			PORTC = 0x01;
			break;
		}
		_delay_ms(500); // 0.5초마다 측정
    }
}

ISR(USART0_RX_vect) // uart에 들어온 값이 있을 때 실행
{
	unsigned char buff = UDR0; // UDR0에 레지스터에 데이터가 저장이 된다.
	uart_write(buff - 'a' + 'A'); // 소문자->대문자로 전송
}