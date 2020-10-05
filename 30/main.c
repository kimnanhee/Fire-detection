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
#include <stdio.h>
#include <string.h>

#include "sensor.h"
#include "uart.h"
#include "lcd.h"
#include "servo.h"

char uart_arr[5]; // uart 수신 문자열
int uart_i = 0, uart_finish = 0; // uart 수신 완료 확인 변수
int mode = 0; // 자동, 수동
int fanm_s = 0, serm_s = 1, rela_s = 1, buzz_s = 0; // 팬모터, 서보모터, 릴레이, 부저 on/off 상태

void uart_check() // 수신 완료일때, 내용 확인하는 함수
{
	uart_i = 0;
	uart_finish = 0;
	
	if(strcmp(uart_arr, "auto")==0) mode = 0; // 자동 모드
	else if(strcmp(uart_arr, "sudo")==0) mode = 1; // 수동 모드
	
	if(mode == 1) // 수동 모드일 때
	{
		if(strcmp(uart_arr, "fanm")==0) fanm_s = !fanm_s; // 팬 모터 버튼 클릭
		else if(strcmp(uart_arr, "serm")==0) serm_s = !serm_s; // 서보 보터 버튼 클릭
		else if(strcmp(uart_arr, "rela")==0) rela_s = !rela_s; // 릴레이 버튼 클릭
	}
	memset(uart_arr, 0, 5); // uart 수신 문자열 초기화
}

int main(void)
{
	DDRA = 0xFF; // 4bit LCD사용
	PORTA = 0x00;
	DDRB = 0xFF; // 서보 모터 연결
	PORTB = 0x00;
	DDRC = 0xFF; // 부저, 팬모터, 릴레이 출력핀
	PORTC = 0x04; // 평상시 포트 출력
	DDRF = 0x00; // 가스, 불꽃, 온도센서 입력핀
	
	uart_init(BAUDRATE(9600)); // baudrate 속도 설정
	sei();
	
	LCD_init();
	SERVO_init();
	
	float temp, gas;
	int temp_state = 0, gas_state = 0, fire_state = 0;
	char buff[50]; // uart 송신 문자열
	
	memset(uart_arr, 0, 5); // uart 수신 문자열 초기화
	
    while (1) 
    {
		if(uart_finish == 1) uart_check(); // 수신 완료일때, 내용 확인
		
		temp = temp_sensor_read();
		if(temp > 30.0) temp_state = 1; // 30도 이상일때
		else temp_state = 0;
		_delay_ms(100);
		
		gas = gas_sensor_read();
		if(temp > 3.0) gas_state = 1; // 3.0이상일때
		else gas_state = 0;
		_delay_ms(100);
		
		fire_state = fire_sensor_read();
		_delay_ms(100);
		
		sprintf(buff, "temp : %3.1f, gas : %.1f, fire : %d     ", temp, gas, fire_state);
		uart_string(buff);
		
		sprintf(buff, "%.1fC  %.1f%%  %d", temp, gas*20, fire_state);
		LCD_command(0x01);
		_delay_ms(2);
		LCD_setcursor(0, 0);
		LCD_wString(buff);
		
		if(fire_state==1) // 화재발생
		{
			fanm_s = 0; // 팬 끄기
			serm_s = 0; // 서보모터 끄기(가스 차단)
			rela_s = 0; // 릴레이 끄기(전기 차단)
			buzz_s = 1; // 부저 동작
			
			sprintf(buff, "fire fire fire %d", 0); // 블루투스로 화재 신호 전송
			uart_string(buff);
		}
		else if(temp_state==1 || gas_state==1) // 팬 모터만 작동
		{
			fanm_s = 1; // 팬 동작
			serm_s = 1; // 서보모터 동작(가스 통함)
			rela_s = 1; // 릴레이 동작(전기 통함)
			buzz_s = 0; // 부저 끄기
		}
		else // 평상시
		{
			fanm_s = 0; // 팬 끄기
			serm_s = 1; // 서보모터 동작(가스 통함)
			rela_s = 1; // 릴레이 동작(전기 통함)
			buzz_s = 0; // 부저 끄기
		}
		
		PORTC = 0x00;
		if(buzz_s == 1) PORTC |= 0x01; // PC0 on
		else;
		if(fanm_s == 1) PORTC |= 0x02; // PC1 on
		else;
		if(rela_s == 1) PORTC |= 0x04;
		else;
		if(serm_s == 1) SERVO_ON();
		else SERVO_OFF();
		
		_delay_ms(500); // 0.5초마다 측정
    }
}

ISR(USART0_RX_vect) // uart에 들어온 값이 있을 때 실행
{
	unsigned char re = UDR0; // UDR0에 레지스터에 데이터가 저장이 된다.
	uart_arr[uart_i++] = re;
	if(uart_i == 4) uart_finish = 1;
}