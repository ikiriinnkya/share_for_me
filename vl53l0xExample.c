
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "debugPrint.h"
#include "i2cmaster.h"
#include "millis.h"
#include "VL53L0X.h"
#include "vl53l0xExample.h"
#define pi 3.14
#define chokkei 70.0
#define haba 157.0
#define maai 110.0

int global_linecount=0;
int global_lineflag=0;


float Map(float value, float start1, float stop1, float start2, float stop2)
{
	return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

int abs(int a){
	if(a>=0) return a;
	else return -a;
}
void forward(float mm){
	for(int i=0;i<((200.0*mm)/(pi*chokkei));i++){
		PORTD = 0b01001000;
		_delay_ms(3);
		PORTD = 0b00000000;
		_delay_ms(3);
	}	
}
void back(int mm){
	for(int i=0;i<((200*mm)/(pi*chokkei));i++){
		PORTD = 0b11101000;
		_delay_ms(3);
		PORTD = 0b10100000;
		_delay_ms(3);
	}
}
void rotate_left(float degrees){
	int step = (haba/chokkei)*0.56*degrees;
	for(int i = 0;i<step;i++){
		PORTD = 0b01101000;
		_delay_ms(3);
		PORTD = 0b00100000;
		_delay_ms(3);
	}
}

void rotate_right(float degrees){
	int step = (haba/chokkei)*0.56*degrees;
	for(int i = 0;i<step;i++){
		PORTD = 0b11001000;
		_delay_ms(3);
		PORTD = 0b10000000;
		_delay_ms(3);
	}
}

void turn_right(int turn,int count){
	while(count<5){
		count++;
		PORTD = 0b01000000;
		_delay_ms(1.5);
		PORTD = 0b00000000;
		_delay_ms(1.5);
		if(count%turn!=0){
			PORTD = 0b00001000;
			_delay_ms(1.5);
			PORTD = 0b00000000;
			_delay_ms(1.5);
		}
	}
}

void turn_left(int turn,int count){
	while(count<5){
		count++;
		PORTD = 0b00001000;
		_delay_ms(1.5);
		PORTD = 0b00000000;
		_delay_ms(1.5);
		if(count%turn!=0){
			PORTD = 0b01000000;
			_delay_ms(1.5);
			PORTD = 0b00000000;
			_delay_ms(1.5);
		}
	}
}
void servo(int degrees){
	OCR1A = Map(degrees,0,180,999,1999);
}
void chokkaku(){//使わない
	statInfo_t xTraStats;
    initVL53L0X(1);
	setMeasurementTimingBudget( 500 * 1000UL );
	rotate_right(20);
	while(1){	
		rotate_left(20);_delay_ms(10);int save1 = readRangeSingleMillimeters( &xTraStats );debug_str("\nsave1 = ");debug_dec(save1);
		rotate_right(20);_delay_ms(10);int save2 = readRangeSingleMillimeters(&xTraStats);debug_str("\nsave2 = ");debug_dec(save2);
		if(save1 > save2) rotate_right(5);
		else rotate_left(5);
		
		if(save1==save2) break;
	}
}
int sokkyo(){
	statInfo_t xTraStats;
	initVL53L0X(1);
	setMeasurementTimingBudget( 1000UL );
	readRangeSingleMillimeters( &xTraStats );
	return xTraStats.rawDistance;

}
int analoginput_2(){
	ADMUX = 0b00000000;//use pc0
	ADCSRA = 0b10000111;//init
	DIDR0 = 0b00000001;//unuse pc0 digital input
	ADCSRA = 0b10010111;//clear ADIF
	ADCSRA = 0b11010111;//start
	loop_until_bit_is_set(ADCSRA,ADIF);
	_delay_ms(1);
	return ADC;
}
int analoginput_1(){
	ADMUX = 0b00000001;//use pc1
	ADCSRA = 0b10000111;//init
	DIDR0 = 0b00000010;//unuse pc1 digital input
	ADCSRA = 0b10010111;//clear ADIF
	ADCSRA = 0b11010111;//start
	loop_until_bit_is_set(ADCSRA,ADIF);
	_delay_ms(1);
	return ADC;
}
int analoginput_0(){
	ADMUX = 0b00000010;//use pc2
	ADCSRA = 0b10000111;//init
	DIDR0 = 0b00000100;//unuse pc2 digital input
	ADCSRA = 0b10010111;//clear ADIF
	ADCSRA = 0b11010111;//start
	loop_until_bit_is_set(ADCSRA,ADIF);
	_delay_ms(1);
	return ADC;
}
int analoginput_3(){
	ADMUX = 0b00000011;//use pc3
	ADCSRA = 0b10000111;//init
	DIDR0 = 0b00001000;//unuse pc3 digital input
	ADCSRA = 0b10010111;//clear ADIF
	ADCSRA = 0b11010111;//start
	loop_until_bit_is_set(ADCSRA,ADIF);
	_delay_ms(1);
	return ADC;
}
void linetrace(int linenum){
	while(1){
	  if(analoginput_2() >= analoginput_1()){
		  int turn = Map(analoginput_2(),140,1000,10,1);
		  turn_right(turn,0);
	  }
	  if(analoginput_2() < analoginput_1()){
		  int turn=Map(analoginput_1(),140,1000,10,1);
		  turn_left(turn,0);
	  }
	  if(analoginput_0()<500 && analoginput_3()<500 && global_lineflag==0){
		  global_linecount++;global_lineflag=50;
	  }
	  if(global_linecount == linenum){
		  break;
	  }
	  if(global_lineflag>0){
	      global_lineflag--;
	  }
	}
}
void rideline(){
	
}
void ballcatch(){
	
}
void decide_ballcolor(){
	
}
int wall_ball(){
	int save0,save1,save2;
	save0 = sokkyo();
	rotate_right(30);
	_delay_ms(300);
	save1 = sokkyo();
	rotate_left(60);
	_delay_ms(300);
	save2 = sokkyo();
	_delay_ms(300);
	rotate_right(30);
	if(save1 > save0 + 50 && save2 > save0 + 50){
		return 1;//ball
	}
	else{
		return 0;//wall
	}
	
}

void ballseach(){
	int min = 1145;int mini = 0;
	rotate_right(60);
	for(int i=10;i<=120;i+=10){
		rotate_left(10);
		int save = sokkyo();
		_delay_ms(200);
		if(save < min){
			min = save;
			mini = i;
		}
	}
	_delay_ms(100);
	rotate_right(120-mini);
	_delay_ms(100);
	
	int min2 = 1145;int mini2=0;
	rotate_right(10);
	for(int i=5;i<=20;i+=5){
		rotate_left(5);
		int save2 = sokkyo();
		_delay_ms(200);
		if(save2 < min2){
			min2 = save2;
			mini2 = i;
		}
	}	
	_delay_ms(100);
	rotate_right(20-mini2);
	_delay_ms(100);	
	if(min > maai){
		forward(min - maai);
	}
	else{
		 back(maai - min);
	}
	if(wall_ball() == 1){
		ballcatch();
	}	
}
void linetrace_debug(){
	debug_str("an0:");debug_dec(analoginput_0());debug_str("\n");
	debug_str("an1:");debug_dec(analoginput_1());debug_str("\n");
	debug_str("an2:");debug_dec(analoginput_2());debug_str("\n");
	debug_str("an3:");debug_dec(analoginput_3());debug_str("\n");
	_delay_ms(1000);
}
void vl53l0x_example(){
	//statInfo_t xTraStats;
	//initVL53L0X(1);
	//setMeasurementTimingBudget( 500 * 1000UL );		// integrate over 500 ms per measurement
	//readRangeSingleMillimeters( &xTraStats );		// blocks until measurement is finished
	//int save = xTraStats.rawDistance;
	//debug_str("\ndist = ");
	//debug_dec(save);
	//if ( timeoutOccurred() ) {
	//debug_str(" !!! Timeout !!! \n");
	//}
}
void init(void) {
	debugInit();
	//--------------------------------------------------
	// GPIOs
	//--------------------------------------------------
	CBI( UCSR0B, RXEN0 );// Disable UART RX
	DDRD = 0b11111111;
	DDRB = 0b00000010;
	// Enable weak pullups on I2C lines
	PORTC = (1<<PIN_I2C_SCL) | (1<<PIN_I2C_SDA);
    TCCR1A = 0b10000010;
	TCCR1B = 0b00011010;
	ICR1 = 19999;//8MHz 20ms
	OCR1A = 1999;	
	//--------------------------------------------------
	// Init the other modules
	//--------------------------------------------------
	i2c_init();
	initMillis();
	//initVL53L0X(1);
	//setMeasurementTimingBudget( 1UL );
	sei();
}

int main(){
	
	init();
	//debug_dec(sokkyo());
	//debug_str("\n");
	//turn_left(5,0);
	//linetrace(1);
	//linetrace_debug();
	//ballseach();
	//forward(300.0);
	//rotate_left(90.0);
    //linetrace(1);

}
