/*
OCR1A값 설정

-90 : 375
  0 : 500
 90 : 250
 
 PB5에 연결
*/
#ifndef SERVO_H_
#define SERVO_H_

void SERVO_init(void)
{
	TCCR1A = (1 << COM1A1) | (1 << WGM11); // PWM 설정
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); // 분주비 설정
	ICR1 = 4999; // ICR1 set to TOP
	OCR1A = 500; // 0 degree
	TCNT1 = 0x00; // Strating point
}
		
void SERVO_ON() // 가스가 통한다
{
	OCR1A = 250;
	_delay_ms(100);
}

void SERVO_OFF() // 모터를 돌려서 가스를 차단시킨다
{
	OCR1A = 500;
	_delay_ms(100);
}

#endif /* SERVO_H_ */