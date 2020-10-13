/*
A : 4비트 LCD
PB5 : 서보모터
PC0 : 부저
PC1 : 팬모터
PC2 : 릴레이
PD0 : 온도 센서(DHT11)
PF0 : 불꽃 센서
PF1 : 가스 센서
*/
#define F_CPU 16000000UL
#define BAUDRATE(x) ((F_CPU/16/x)-1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sensor.h"
#include "uart.h"
#include "lcd.h"
#include "servo.h"

char uart_arr[5]; // uart 수신 문자열
int uart_state = 0;
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
	DDRD = 0x00; // 온도 센서
	DDRF = 0x00; // 불꽃, 가스센서 입력핀
	
	uart_init(BAUDRATE(9600)); // baudrate 속도 설정
	sei();
	
	LCD_init();
	SERVO_init();
	
	float temp, fire, gas;
	char buff[100]; // uart 송신 문자열
	
	memset(uart_arr, 0, 5); // uart 수신 문자열 초기화
	
    while (1) 
    {
		if(uart_finish) 
			uart_check(); // 수신 완료일때, 내용 확인
		
		temp = temp_sensor_read();
		gas = gas_sensor_read();
		fire = fire_sensor_read();
		fire = fire * 999.0 / 1023.0;
		
		sprintf(buff, "temp : %2d.%1d, gas : %d.%d, fire : %3d ", (int)temp, ((int)(temp*10)%10), (int)gas,((int)(gas*10)%10), (int)fire);
		uart_string(buff);
		
		LCD_setcursor(0, 0);
		sprintf(buff, "%2d.%d", (int)temp,((int)(temp*10)%10)); // LCD 온도 출력
		LCD_wString(buff);
		LCD_data(0b11011111);
		sprintf(buff, "C %d.%d%% %3d ", (int)gas, ((int)(gas*10)%10), (int)fire); // LCD 가스, 불꽃 출력
		LCD_wString(buff);
		
		if(mode == 0) // 자동모드일때만 상태 변경
		{
			if(fire < 700)  // 화재발생
			{
				fanm_s = 0; // 팬 끄기
				serm_s = 0; // 서보모터 끄기(가스 차단)
				rela_s = 0; // 릴레이 끄기(전기 차단)
				buzz_s = 1; // 부저 동작
				
				sprintf(buff, "fire fire"); // 블루투스로 화재 신호 전송
				uart_string(buff);
				
				LCD_setcursor(1, 0);
				LCD_wString(buff);
			}
			else if(temp > 30 || gas > 1.5) // 팬 모터만 작동
			{
				fanm_s = 1; // 팬 동작
				serm_s = 1; // 서보모터 동작(가스 통함)
				rela_s = 1; // 릴레이 동작(전기 통함)
				buzz_s = 0; // 부저 끄기
				
				sprintf(buff, "fan on          ");
				LCD_setcursor(1, 0);
				LCD_wString(buff);
			}
			else // 평상시
			{
				fanm_s = 0; // 팬 끄기
				serm_s = 1; // 서보모터 동작(가스 통함)
				rela_s = 1; // 릴레이 동작(전기 통함)
				buzz_s = 0; // 부저 끄기
				
				sprintf(buff, "                ");
				LCD_setcursor(1, 0);
				LCD_wString(buff);
			}
		}
		
		PORTC = 0x00;
		if(buzz_s == 1) PORTC |= 0x01; // PC0 on
		else;
		if(fanm_s == 1) PORTC |= 0x02; // PC1 on
		else;
		if(rela_s == 1) PORTC |= 0x04; // PC2 on
		else;
		if(serm_s == 1) SERVO_ON();
		else SERVO_OFF();
		// PORTC = ((buzz_s == 1) << 0) | ((fanm_s ==  1) << 1) | ((rela_s == 1) << 2);
		_delay_ms(3000); // 3초마다 측정
    }
}

ISR(USART0_RX_vect) // uart에 들어온 값이 있을 때 실행
{
	unsigned char re = UDR0; // UDR0에 레지스터에 데이터가 저장이 된다.
	
	if(re == '\x02') uart_state = 1; // 시작 문자열
	else if(re == '\x03') uart_state = 0, uart_finish = 1; // 종료 문자열
	
	else if(uart_state) uart_arr[uart_i++] = re;
}