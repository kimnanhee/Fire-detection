#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <sensor.h>

int main(void)
{
	DDRF = 0x00; // 가스, 불꽃, 온도센서 입력핀
    DDRC = 0xFF; // 부저, 팬모터, 릴레이 출력핀
	
    while (1) 
    {
		int temp_state = temp_sensor_read();
		int gas_state = gas_sensor_read();
		int fire_state = fire_sensor_read();
		
		_delay_ms(500); // 0.5초마다 측정
    }
}

