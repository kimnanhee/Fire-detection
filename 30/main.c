/*
A : 4비트 LCD
PB5 : 서보모터
PC0 : 부저
PC1 : 팬모터
PC2 : 릴레이
PF0 : 가스 센서
PF1 : 불꽃 센서
PF2 : 온도 센서
*/
#define F_CPU 16000000UL
#define BAUDRATE(x) ((F_CPU/16/x)-1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "sensor.h"
#include "uart.h"
#include "lcd.h"
#include "servo.h"

char uart_arr[5]; // uart 수신 문자열
int uart_i = 0, uart_finish = 0;
int uart_state = 0;

int main(void)
{
	DDRA = 0xFF; // 4bit LCD사용
	PORTA = 0x00;
	DDRB = 0xFF; // 서보 모터 연결
	PORTB = 0x00;
	DDRC = 0xFF; // 부저, 팬모터, 릴레이 출력핀
	PORTC = 0x04; // 평상시 포트 출력
	DDRF = 0x00; // 가스, 불꽃, 온도센서 입력핀
    
	int state = 0; // 상황을 저장
	
	uart_init(BAUDRATE(9600)); // baudrate 속도 설정
	sei();
	
	LCD_init();
	SERVO_init();
	
	float temp = 0, gas = 0;
	int temp_state = 0, gas_state = 0, fire_state = 0;
	char buff[50]; // uart 송신 문자열
	
	memset(uart_arr, 0, 5); // uart 수신 문자열 초기화
	
    while (1) 
    {
		if(uart_finish)
		{
			uart_i = 0;
			uart_finish = 0;
			
			if(strcmp(uart_arr, "auto")==0) uart_state = 0; // 자동 모드
			else if(strcmp(uart_arr, "sudo")==0) uart_state = 1; // 수동 모드
			else if(strcmp(uart_arr, "fanm")==0) uart_state = 10;
			else if(strcmp(uart_arr, "serm")==0) uart_state = 11;
			else if(strcmp(uart_arr, "rela")==0) uart_state = 12;
			
			memset(uart_arr, 0, 5); // uart 수신 문자열 초기화
		}
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
		
		sprintf(buff, "temp : %3.1f, gas : %.1f, fire : %d     ", temp, gas, fire_state);
		uart_string(buff);
		
		sprintf(buff, "%.1fC  %.1f%%  %d", temp, gas*20, fire_state);
		LCD_setcursor(0, 0);
		LCD_wString(buff);
		
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
		if(state == 3) SERVO_OFF();
		else SERVO_ON();
		
		_delay_ms(500); // 0.5초마다 측정
    }
}

ISR(USART0_RX_vect) // uart에 들어온 값이 있을 때 실행
{
	unsigned char re = UDR0; // UDR0에 레지스터에 데이터가 저장이 된다.
	uart_arr[uart_i++] = re;
	if(uart_i == 4) uart_finish = 1;
}