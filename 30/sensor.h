#ifndef SENSOR_H_
#define SENSOR_H_

int temp_sensor_read() // 온도가 일정 수준을 넘으면 1을 반환
{
	char key = PINF & 0x02;
	if(key == 0x02) return 1;
}
int gas_sensor_read() // 가스이 일정 수준을 넘으면 1을 반환
{
	char key = PINF & 0x02;
	if(key == 0x02) return 1;
}
int fire_sensor_read() // 불꽃이 감지되면 1을 반환
{
	char key = PINF & 0x02;
	if(key == 0x02) return 1;
}

#endif /* SENSOR_H_ */