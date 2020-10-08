#ifndef LCD_H_
#define LCD_H_

#define RS_H (PORTA |= 0x01)		// RS=1
#define RS_L (PORTA &= 0xFE)		// RS=0
#define RW_H (PORTA |= 0x02)		// RW=1
#define RW_L (PORTA &= 0xFD)		// RW=0
#define ENABLE (PORTA |= 0x04)		// ENABLE
#define DISABLE (PORTA &= 0xFB)		// DISABLE

void LCD_command(char command){
	PORTA = (command&0xF0);			// send High nibble
	RS_L; RW_L;						// RS=0 RW=0
	ENABLE;							// E1 : L->H
	_delay_us(1);
	DISABLE;						// E1 : H->L	
	PORTA = (command & 0x0F)<<4;	// send Low nibble
	RS_L; RW_L;						// RS=0 RW=0
	ENABLE;							// E1 : L->H
	_delay_us(1);
	DISABLE;						// E1 : H->L
}

void LCD_init(void){
	_delay_ms(40);
	LCD_command(0x28);		// DL=0(4bit) N=1(2Line) F=0(5x7)
	_delay_us(50);
	LCD_command(0x0C);		// LCD ON, Cursor X, Blink X
	_delay_us(50);
	LCD_command(0x01);		// LCD Clear
	_delay_ms(2);
	LCD_command(0x06);		// Entry Mode
	_delay_us(50);
}

void LCD_data(char data){
	_delay_us(100);
	PORTA = (data&0xF0);			// send High nibble
	RS_H; RW_L;						// RS=1 RW=0
	ENABLE;							// E1 : L->H
	_delay_us(1);
	DISABLE;							// E1 : H->L
	PORTA = (data&0x0F)<<4;			// send Low nibble
	RS_H; RW_L;						// RS=1 RW=0
	ENABLE;							// E1 : L->H
	_delay_us(1);
	DISABLE;						// E1 : H->L
}

void LCD_wString(char *str) // 문자열을 출력
{
	while(*str)
		LCD_data(*str++);
}

void LCD_setcursor(char col, char row)
{
	LCD_command(0x80 | col * 0x40 + row);
	_delay_us(40);
}

#endif /* LCD_H_ */