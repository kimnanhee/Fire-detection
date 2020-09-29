#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <sensor.h>

int main(void)
{
	DDRF = 0x00; // 가스, 불꽃, 온도센서 입력핀
    DDRC = 0xFF; // 부저, 팬모터, 릴레이 출력핀
	PORTC = 0x04; // 평상시 포트 출력
	int state = 0;
	
    while (1) 
    {
		int temp_state = temp_sensor_read();
		int gas_state = gas_sensor_read();
		int fire_state = fire_sensor_read();
		
		if(temp_state==1 && gas_state==1 && fire_state==1) state = 4; // 화재
		else if(temp_state==1 && gas_state==0 && fire_state==0) state = 1; // 온도
		else if(temp_state==0 && gas_state==1 && fire_state==0) state = 2; // 가스
		else if(temp_state==0 && gas_state==0 && fire_state==1) state = 3; // 불꽃
		else state = 0; // 평상
		
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
			SERVO();
			break;
			
			case 4: // 화재
			PORTC = 0x01;
			break;
		}
		_delay_ms(500); // 0.5초마다 측정
    }
}

