#ifndef SENSOR_H_
#define SENSOR_H_

void ADC_set() // ADC 설정
{
	ADCSRA = (1 << ADEN); // ADC enable
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 16Mhz/128 = 125Khz
}

uint16_t ADC_read(uint8_t channel) // channel에 해당하는 ADC값 반환
{
	ADMUX = (1 << REFS0); // ADC0, 5V에 0~1023 설정
	ADMUX |= channel; // ADC핀 설정
	
	ADCSRA |= (1 << ADSC); // ADC 변환 시작
	while(ADCSRA & (1 << ADSC)); // ADC 변환 완료
	
	return ADCW;  // Return converted value
}

int temp_sensor_read() // 온도가 일정 수준을 넘으면 1을 반환
{
	ADC_set();
	int value = ADC_read(0);
	
	float temp = (float)value * 5.0 / 1024 * 100; // ~`C 단위의 온도
	
	if(temp > 36) return 1; // 36도 이상일때 1 반환
	else return 0;
}

int gas_sensor_read() // 가스이 일정 수준을 넘으면 1을 반환
{
	ADC_set();
	int value = ADC_read(2);
	
	if(value > 500) return 1;
	else return 0;
}

int fire_sensor_read() // 불꽃이 감지되면 1을 반환
{
	char key = PINF & 0x02;
	if(key == 0x02) return 1;
	else return 0;
}

#endif /* SENSOR_H_ */