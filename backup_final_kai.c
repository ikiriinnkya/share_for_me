#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "millis.h"
#include "debugPrint.h"
#include "i2cmaster.h"
#include "VL53L0X.h"
#include "vl53l0xExample.h"
#define pi 3.14
#define chokkei 76.0
#define haba 180.0
#define maai 100.0
#define rotate_hosei_r 0.95
#define rotate_hosei_l 0.95
#define LINEIKICHI 140
#define SUBERIHOSEI 1.00
#define IKI_ICHIGIMEHOSEI_R 0
#define SERVO_UP 2199
#define SERVO_DOWN 1062
#define ICHIGIMEHOSEI 2.5
#define sbi(PORT,BIT) PORT |=_BV(BIT);
#define cbi(PORT,BIT) PORT |=_BV(BIT);
int global_linecount = 0;
int global_lineflag = 0;
int global_ballthrough = 0;
volatile uint64_t stepper1_inr_count = 0;
volatile uint64_t stepper2_inr_count = 0;
volatile uint64_t stepper_inr_count = 0;

ISR(TIMER1_COMPA_vect){
	PORTD ^= (1<<6);
	stepper1_inr_count++;
}

//ISR(TIMER2_COMPA_vect){
	//PORTD ^= (1<<3);
	//stepper2_inr_count++;
//}

ISR(TIMER2_COMPA_vect){
	PORTD ^= (9<<3);
	//if(bit_is_set(PORTD,PORTD3)){
		//cbi(PORTD,PORTD3);
	//}
	//else sbi(PORTD,PORTD3);
	//if(bit_is_set(PORTD,PORTD6)){
		//cbi(PORTD,PORTD6);
	//}
	//else sbi(PORTD,PORTD6);
	stepper_inr_count++;
}
void timer1_servo_to_moter(){
	TCCR1B = 0b00001000;
	OCR1A = 0;
	TCCR1A = 0b00000000;//port disconnect CTC Top = OCR1A;
	//TCCR1B = 0b00001101;//bunnsyuu1024
	TCCR1B = 0b00001100;//bunnsyuu256
	TIMSK1 = 0b00000010;
	TCNT1 = 0;
	OCR1A = 41;//slow	
}   
void steper1_set_speeder(int ocr1a){
	TCCR1B = 0b00001000;
	OCR1A = 0;
	TCCR1A = 0b00000000;//port disconnect CTC Top = OCR1A;
	//TCCR1B = 0b00001101;//bunnsyuu1024
	TCCR1B = 0b00001100;
	TIMSK1 = 0b00000010;
	TCNT1 = 0;
	ICR1 = 0;
	OCR1A = ocr1a;
}
void timer1_moter_to_servo(){
	TCCR1B = 0b00001000;
	OCR1A = 0;
	TIMSK1 = 0b00000000;
    TCCR1A = 0b10000010;//port connect servo
    TCCR1B = 0b00011010;//bunnsyuu 8
    ICR1 = 19999;//8MHz 20ms
	OCR1A = 2149;
}
float Map(float value, float start1, float stop1, float start2, float stop2)
{
	if(value < start1)value = start1;
	if(value > stop1)value = stop1;
	return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

int abs(int a){
	if(a>=0) return a;
	else return -a;
}
int min(int a,int b){
	if(a<=b)return b;
	else return a;
}
int compare_int(const void *a,const void *b){
	return *(int*)a - *(int*)b;
} 
void forward(float mm){
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI;
	for(int i=0;i<step;i++){
		PORTD = 0b01001000;
		_delay_ms(4);
		PORTD = 0b00000000;
		_delay_ms(4);
	}	
}
void inr_forward(float mm){
	PORTD = 0;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	int save1 = 0;
	TIMSK2 = 2;OCR2A = 180;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
			save1 = Map(stepper_inr_count,0,step/4,0,pow(120,1.7));
			OCR2A = 180 - pow(save1,0.58);
		}
		else if(stepper_inr_count <= step*3/4){
			OCR2A = 60;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,pow(120,1.7),0);
			OCR2A = 180 - pow(save1,0.58);
		}
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_high_forward(float mm){
	PORTD = 0;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	int save1 = 0;
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
	        save1 = Map(stepper_inr_count,0,step/4,0,pow(120,1.7));
			OCR2A = 160 - pow(save1,0.58);
		}
		else if(stepper_inr_count <= step*3/4){
		    OCR2A = 40;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,pow(120,1.7),0);
			OCR2A = 160 - pow(save1,0.58);
		}
	}
    TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_little_forward(float mm){
	PORTD = 0;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	TIMSK2 = 2;OCR2A = 180;
	while(step >= stepper_inr_count){
		OCR2A = 180;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}

void back(float mm){
	for(int i=0;i<((200*mm)/(pi*chokkei))*SUBERIHOSEI;i++){
		PORTD = 0b11101000;
		_delay_ms(4);
		PORTD = 0b10100000;
		_delay_ms(4);
	}
}
void inr_back(float mm){
	PORTD = 0b10100000;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	TIMSK2 = 2;OCR2A = 200;
	while(step >= stepper_inr_count){
		OCR2A = 200;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_smart_back(float mm){
	PORTD = 0b10100000;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	int save1 = 0;
	TIMSK2 = 2;OCR2A = 240;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
			save1 = Map(stepper_inr_count,0,step/4,0,120);
			OCR2A = 240 - save1;
		}
		else if(stepper_inr_count <= step*3/4){
			OCR2A = 120;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,120,0);
			OCR2A = 240 - save1;
		}
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}
void left_only_f(int count){
	for(int i=1;i<=count;i++){
		PORTD = 0b00001000;
		_delay_ms(4);
		PORTD = 0b00000000;
		_delay_ms(4);
	}
}
void left_only_b(int count){
	for(int i=1;i<=count;i++){
		PORTD = 0b00101000;
		_delay_ms(4);
		PORTD = 0b00100000;
		_delay_ms(4);
	}
}
void rotate_left(float degrees){
	int step = (haba/chokkei)*0.56*degrees*rotate_hosei_l;
	for(int i = 0;i<step;i++){
		PORTD = 0b01101000;
		_delay_ms(4);
		PORTD = 0b00100000;
		_delay_ms(4);
	}
}
void inr_rotate_left(float degrees){
	PORTD = 0b00100000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_l;
	int save1 = 0;
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
			save1 = Map(stepper_inr_count,0,step/4,0,60);
			OCR2A = 160 - save1;
		}
		else if(stepper_inr_count <= step*3/4){
			OCR2A = 100;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,60,0);
			OCR2A = 160 - save1;
		}
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}

void inr_little_rotate_left(float degrees){
	PORTD = 0b00100000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_l;
	TIMSK2 = 2;OCR2A = 200;
	while(step >= stepper_inr_count){
		OCR2A = 200;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_const_rotate_left(float degrees){
	PORTD = 0b00100000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_l;
	TIMSK2 = 2;OCR2A = 200;
	while(step >= stepper_inr_count){
		OCR2A = 200;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}

void right_only_f(int count){
	for(int i=1;i<=count;i++){
		PORTD = 0b01000000;
		_delay_ms(4);
		PORTD = 0b00000000;
		_delay_ms(4);
	}
}
void right_only_b(int count){
	for(int i=1;i<=count;i++){
		PORTD = 0b11000000;
		_delay_ms(4);
		PORTD = 0b10000000;
		_delay_ms(4);
	}
}
void rotate_right(float degrees){
	int step = (haba/chokkei)*0.56*degrees*rotate_hosei_r;
	for(int i = 0;i<step;i++){
		PORTD = 0b11001000;
		_delay_ms(4);
		PORTD = 0b10000000;
		_delay_ms(4);
	}
}
void inr_rotate_right(float degrees){
	PORTD = 0b10000000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_r;
	int save1 = 0;
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
			save1 = Map(stepper_inr_count,0,step/4,0,60);
			OCR2A = 160 - save1;
		}
		else if(stepper_inr_count <= step*3/4){
			OCR2A = 100;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,60,0);
			OCR2A = 160 - save1;
		}
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_little_rotate_right(float degrees){
	PORTD = 0b10000000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_r;
	TIMSK2 = 2;OCR2A = 200;
	while(step >= stepper_inr_count){
		OCR2A = 200;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
}

void inr_const_rotate_right(float degrees){
	PORTD = 0b10000000;
	int step = (haba/chokkei)*0.56*degrees*2*rotate_hosei_r;
	TIMSK2 = 2;OCR2A = 200;
	while(step >= stepper_inr_count){
		OCR2A = 200;
	}
	TIMSK2 = 0;
	stepper_inr_count = 0;
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


void ichigime(){
while(1){
	while(1){
		if(bit_is_set(PINC,PINC0) || bit_is_set(PINC,PINC3))break;
		inr_little_forward(1);
		
	}
	if((bit_is_set(PINC,PINC0) || bit_is_set(PINB,PINB1)) && (bit_is_set(PINC,PINC3) || bit_is_set(PINC,PINC2)))break;
	else if(bit_is_set(PINC,PINC0)){
		while(bit_is_set(PINC,PINC0)){
			right_only_b(5);_delay_ms(300);
		}
	}
	else if(bit_is_set(PINC,PINC3)){
		while(bit_is_set(PINC,PINC3)){
			left_only_b(5);_delay_ms(300);
		}
	}
}
_delay_ms(200);
inr_little_rotate_right(ICHIGIMEHOSEI);
_delay_ms(200);
}
void ichigime_muhosei(){
	while(1){
		while(1){
			if(bit_is_set(PINC,PINC0) || bit_is_set(PINC,PINC3))break;
			inr_little_forward(1);
			
		}
		if((bit_is_set(PINC,PINC0) || bit_is_set(PINB,PINB1)) && (bit_is_set(PINC,PINC3) || bit_is_set(PINC,PINC2)))break;
		else if(bit_is_set(PINC,PINC0)){
			while(bit_is_set(PINC,PINC0)){
				right_only_b(5);_delay_ms(300);
			}
		}
		else if(bit_is_set(PINC,PINC3)){
			while(bit_is_set(PINC,PINC3)){
				left_only_b(5);_delay_ms(300);
			}
		}
	}
	_delay_ms(200);
}
void linetrace(int linenum){
	if(bit_is_set(PINB,PINB1) && bit_is_set(PINC,PINC2));
}
void linetrace_ver2(int linenum){
	timer1_servo_to_moter();
	TIMSK2 = 2;
	while(1){
			OCR2A = Map(analoginput_2(),15,380,40,200);
			//OCR2A = Map(analoginput_2() + analoginput_3(),500,600,40,200);
			steper1_set_speeder(Map(analoginput_1(),15,380,40,200));
			//steper1_set_speeder(Map(analoginput_1() + analoginput_0(),500,600,40,200));
			_delay_ms(1);
		if(analoginput_0() < LINEIKICHI && analoginput_3() < LINEIKICHI && global_lineflag ==0){
			global_linecount++;global_lineflag = 100;
		}

		if(global_linecount == linenum){
			global_linecount = 0;
			break;
		}
		if(global_lineflag>0){
			global_lineflag--;
		}
	}
	timer1_moter_to_servo();
	TIMSK2 = 0;
}

void ballcatch(){
	//TIMSK1 = 2;
	for(int i=SERVO_UP;i>=SERVO_DOWN;i--){
	  OCR1A = i;
	  _delay_ms(0.5);
	}
	for(int i=0;i<1500;i++){
	  PORTD = 0b00010000;
	  _delay_ms(0.3);
	  PORTD = 0b00000000;
	  _delay_ms(0.7);	
	}
	for(int i = SERVO_DOWN;i<=SERVO_UP;i++){
	  PORTD = 0b00010000;
	  _delay_ms(0.15);
	  PORTD = 0b00000000;
	  _delay_ms(0.35);
	  OCR1A = i;
	}
	PORTD = 0b00000000;
	_delay_ms(100);
	//TIMSK1 = 0;
}

int decide_ballcolor(){
	int r,y;
	PORTB = 0b00000100;//r
	_delay_ms(200);
	r = analoginput_1();
	debug_str("\nr = ");
	debug_dec(r);
	PORTB = 0b01000000;//y
	_delay_ms(200);
	y = analoginput_1();
	debug_str("\ny = ");
	debug_dec(y);	
	//PORTB = 0b10000000;//b
	//_delay_ms(200);
	//b = analoginput_1();
	//debug_str("\nb = ");
	//debug_dec(b);
	PORTB = 0b00000000;
	if(r>=250 && y <= 400){
		debug_str("\ncolar = red");
		return 1;
	}	
	else if(y>=500){
		debug_str("\ncolar = yellow");
		return 2;
	}
	else{
		debug_str("\ncolar = blue");
		return 3;
	}
	return 0;
}

void ballseach_ver2(){
	uint32_t filter;
	int distance = 0;
	int kaisu = 5;
	int stocks[kaisu];
	int saisyou;
	//int forwardcount=0;
	statInfo_t xTraStats;
	initVL53L0X(1);
	setMeasurementTimingBudget( 1000UL );
	int count = 0;
	inr_little_forward(global_ballthrough*200);
	_delay_ms(100);
	while(1){
		saisyou = 9999;
		inr_little_rotate_right(70);
		_delay_ms(100);
		for(int i=10;i<=140;i+=10){
			inr_little_rotate_left(10);
			for(int j=0;j<kaisu;j++){
				readRangeSingleMillimeters(&xTraStats);
				stocks[j] = xTraStats.rawDistance;
			}
			qsort(stocks,kaisu,sizeof(int),compare_int);
			filter = 0;
			for(int k=kaisu-3;k<kaisu;k++) filter+=stocks[k];
			if(filter/3 < saisyou){
				saisyou = filter/3;count = i;
			}
		}
		_delay_ms(100);
		if(saisyou < 300){
			inr_rotate_right(140 - count);
			goto approach;
		}
		_delay_ms(100);
		inr_little_rotate_right(72);_delay_ms(100);
		if(global_ballthrough >= 1){
			inr_rotate_left(90);_delay_ms(200);
			inr_back(50);_delay_ms(200);
			ichigime_muhosei();_delay_ms(200);
			inr_rotate_right(90);_delay_ms(200);
		}
		inr_little_forward(200);
		global_ballthrough++;
	}
	
	approach:;
	distance = 0;
	_delay_ms(500);
	if(saisyou < 40){
		inr_back(40);distance -= 40;
	}
	if(saisyou > maai){
		inr_little_forward((saisyou - maai)*4/3);distance += (saisyou - maai)*4/3;
		_delay_ms(500);
		readRangeSingleMillimeters(&xTraStats);
		int karioki = xTraStats.rawDistance;
		if(karioki > maai){
			inr_little_forward(karioki - maai);distance += (karioki - maai);
		}
		else{
			inr_back(maai - karioki);distance -= (maai - karioki);
		}
	}
	else{
		inr_back(maai - saisyou );distance -= (maai - saisyou );
	}
	
	ballcatch();
	//_delay_ms(500);
	if(distance >= 0){
		inr_back(distance);
	}
	else{
		inr_little_forward(-distance);
	}
	if(count <= 70){
		_delay_ms(500);
		inr_rotate_left(70 - count);
	}
	else{
		_delay_ms(500);
		inr_rotate_right(count - 70);
	}
	_delay_ms(100);
    //inr_back(30);
	if(global_ballthrough >= 1){
		inr_back(50);
		_delay_ms(200);
		inr_rotate_left(90);
		_delay_ms(200);
		inr_back(50);
		_delay_ms(200);
		ichigime_muhosei();
		_delay_ms(200);
		inr_rotate_right(90);
		_delay_ms(200);
		inr_back(global_ballthrough*200 -20);
	}
	else inr_back(global_ballthrough*200 + 40);
	_delay_ms(300);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_back(80);
	_delay_ms(200);
	ichigime_muhosei();
	_delay_ms(200);
	inr_back(50);
	_delay_ms(200);
	//inr_rotate_left(90);
	inr_rotate_right(90);
	_delay_ms(200);
	ichigime();
	_delay_ms(200);
	inr_rotate_left(180);
	_delay_ms(100);
}

void ballemit(){
	//TIMSK1 = 2;
	for(int i=SERVO_UP;i>=SERVO_DOWN;i--){
		OCR1A = i;
		_delay_ms(0.5);
	}
	_delay_ms(1000);
	for(int i=SERVO_DOWN;i<=SERVO_UP;i++){
		OCR1A = i;
		_delay_ms(0.5);
	}
	//TIMSK1 = 0;
}
void linetrace_debug(){
	debug_str("an0:");debug_dec(analoginput_0());debug_str("\n");
	debug_str("an1:");debug_dec(analoginput_1());debug_str("\n");
	debug_str("an2:");debug_dec(analoginput_2());debug_str("\n");
	debug_str("an3:");debug_dec(analoginput_3());debug_str("\n");
	_delay_ms(1000);
}
void vl53l0x_example(){
	statInfo_t xTraStats;
	initVL53L0X(1);
	setMeasurementTimingBudget( 500 * 1000UL );		// integrate over 500 ms per measurement
	readRangeSingleMillimeters( &xTraStats );		// blocks until measurement is finished
	int save = xTraStats.rawDistance;
	debug_str("\ndist = ");
	debug_dec(save);
	if ( timeoutOccurred() ) {
	debug_str(" !!! Timeout !!! \n");
	}
}
void start_ball(){
	inr_forward(250);
	ichigime();
	_delay_ms(500);
	inr_rotate_left(90);
	ballemit();
	inr_rotate_right(90);
	_delay_ms(500);
}
void position_to_senter(){
	inr_smart_back(50);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(200);
	ichigime_muhosei();
	_delay_ms(200);
	inr_little_forward(10);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_little_forward(50);
	_delay_ms(100);
}
void iki_ichigimeset(){
	ichigime();
	_delay_ms(100);
	inr_smart_back(50);
	_delay_ms(100);
	inr_rotate_right(90);
	_delay_ms(100);
	ichigime();
	_delay_ms(100);
	inr_smart_back(50);
	_delay_ms(100);
	inr_rotate_left(90);
	_delay_ms(100);	
}
void kaeri_ichigimeset(){
	ichigime();
	_delay_ms(100);
	inr_smart_back(50);
	_delay_ms(100);
	inr_rotate_left(90);
	_delay_ms(100);
	ichigime();
	_delay_ms(100);
	inr_smart_back(50);
	_delay_ms(100);
	inr_rotate_right(90);
	_delay_ms(100);
}
void rline_to_ball(){
	inr_smart_back(50);
	_delay_ms(100);
	ichigime();
	//inr_forward(610);
	//iki_ichigimeset();
	//inr_forward(740);
	inr_forward(1300);
	_delay_ms(100);
	inr_rotate_right(90);
	_delay_ms(100);
	inr_forward(280);
	//iki_ichigimeset();
	ichigime();
	//inr_little_forward(320);//dannsa_kasokusinai
	//inr_forward(520);
	inr_forward(780);
	_delay_ms(100);
	inr_rotate_right(90);
	_delay_ms(100);
	inr_forward(230);
	ichigime_muhosei();
	_delay_ms(100);
	position_to_senter();
}

void yline_to_ball(){
	_delay_ms(100);
	inr_smart_back(50);
	_delay_ms(100);
	//iki_ichigimeset();
	ichigime_muhosei();
	inr_forward(680);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(200);
	inr_forward(280);
	//iki_ichigimeset();
	ichigime();
	inr_forward(780);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(200);
	inr_forward(230);
	ichigime_muhosei();
	position_to_senter();
}
void bline_to_ball(){
	_delay_ms(100);
	inr_forward(770);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(500);
	inr_forward(230);
	ichigime_muhosei();
	_delay_ms(200);
	position_to_senter();	
}
void ball_to_bline(){
	inr_forward(290);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(610);
	//kaeri_ichigimeset();
	ichigime_muhosei();
	ballemit();
	_delay_ms(200);
	inr_rotate_right(180 + ICHIGIMEHOSEI);
	_delay_ms(200);
}
void ball_to_yline(){
	inr_forward(290);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(610);
	//kaeri_ichigimeset();
	ichigime();
	_delay_ms(200);
	inr_forward(370);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(620);
	//kaeri_ichigimeset();
	ichigime_muhosei();
	_delay_ms(200);
	inr_rotate_right(90);
	ballemit();
	_delay_ms(500);
	inr_rotate_right(90);
	_delay_ms(200);
}
void ball_to_rline(){
	inr_forward(290);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(610);
	//kaeri_ichigimeset();
	ichigime();
	_delay_ms(200);
	inr_forward(370);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(620);
	//kaeri_ichigimeset();
	ichigime_muhosei();
	inr_forward(610);
	//inr_forward(1250);
	ichigime_muhosei();
	_delay_ms(200);
	inr_rotate_right(90);
	ballemit();
	inr_rotate_right(90);	
}
void init(void) {
	debugInit();
	//--------------------------------------------------
	// GPIOs
	//--------------------------------------------------
	CBI( UCSR0B, RXEN0 );// Disable UART RX
	DDRD = 0b11111111;
	DDRB = 0b11111110;
	DDRC = 0b00000000;
	// Enable weak pullups on I2C lines
	PORTC = (1<<PIN_I2C_SCL) | (1<<PIN_I2C_SDA);
    TCCR1A = 0b10000010;//servo
	TCCR1B = 0b00011010;
	ICR1 = 19999;//8MHz 20ms
	OCR1A = SERVO_UP;//999~2149
	//TCCR1A = 0b00000100;//port disconnect CTC Top = OCR1A;
	//TCCR1B = 0b00001101;//bunnsyuu1024
	//TIMSK1 = 0b00000000;
	//OCR1A = 11;
		
	TCCR2A = 0b00000010;
	//TCCR2B = 0b00000111;//1024bunn
	TCCR2B = 0b00000110;//256bunn
	TIMSK2 = 0b00000000;//010//only enable when linetrace
	OCR2A = 11;//11~31
	_delay_ms(2000);
	//--------------------------------------------------
	// Init the other 
	//--------------------------------------------------
	i2c_init();
	initMillis();
	//initVL53L0X(1);
	//setMeasurementTimingBudget( 1UL );
	sei();
	
}

int main(){
	init();
	int ballcolor;
	//start_ball();
	rline_to_ball();
	while(1){
		ballseach_ver2();
		ballcolor = decide_ballcolor();
		if(ballcolor == 1){
			ball_to_rline();
			rline_to_ball();
		}
		else if(ballcolor == 2){
			ball_to_yline();
			yline_to_ball();
		}
		else{
			ball_to_bline();
			bline_to_ball();
		}
	}
}
