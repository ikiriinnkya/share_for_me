
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "millis.h"
#include "debugPrint.h"
#include "i2cmaster.h"
#include "VL53L0X.h"
#include "vl53l0xExample.h"
#define pi 3.14
#define chokkei 76.0
#define haba 180.0
#define maai 95.0
#define rotate_hosei_r 0.95
#define rotate_hosei_l 0.95
#define LINEIKICHI 140
#define SUBERIHOSEI 1.00
#define IKI_ICHIGIMEHOSEI_R 0
#define sbi(PORT,BIT) PORT |=_BV(BIT);
#define cbi(PORT,BIT) PORT |=_BV(BIT);
int global_linecount = 0;
int global_lineflag = 0;
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
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		if(stepper_inr_count <= step/4){
	        save1 = Map(stepper_inr_count,0,step/4,0,100);
			OCR2A = 160 - save1;
		}
		else if(stepper_inr_count <= step*3/4){
		    OCR2A = 60;
		}
		else{
			save1 = Map(stepper_inr_count,step*3/4,step,100,0);
			OCR2A = 160 - save1;
		}
	}
    TIMSK2 = 0;
	stepper_inr_count = 0;
}
void inr_little_forward(float mm){
	PORTD = 0;
	int step = ((200.0*mm)/(pi*chokkei))*SUBERIHOSEI*2;
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		OCR2A = 160;
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
	TIMSK2 = 2;OCR2A = 160;
	while(step >= stepper_inr_count){
		OCR2A = 160;
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
		if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3))break;
		
		if(bit_is_set(PINC,PINC0)){
			_delay_ms(500);left_only_f(10);_delay_ms(500);inr_back(10);_delay_ms(500);
		}
		if(bit_is_set(PINC,PINC3)){
			_delay_ms(500);right_only_f(10);_delay_ms(500);inr_back(10);_delay_ms(500);
		}
		if(bit_is_set(PINC,PINC0) && bit_is_set(PINC,PINC1) && bit_is_set(PINC,PINC2) && bit_is_set(PINC,PINC3))break;
		else inr_little_forward(5);
	}

}
void yuru_ichigime(){
	while(1){
		if(bit_is_set(PINC,PINC0) || bit_is_set(PINC,PINC3))break;

		else inr_little_forward(10);
	}
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
	for(int i=2149;i>=1062;i--){
	  OCR1A = i;
	  _delay_ms(1);
	}
	_delay_ms(1000);
	for(int i=0;i<2000;i++){
	  PORTD = 0b00010000;
	  _delay_ms(0.3);
	  PORTD = 0b00000000;
	  _delay_ms(0.7);	
	}
	for(int i = 1062;i<=2149;i++){
	  PORTD = 0b00010000;
	  _delay_ms(0.3);
	  PORTD = 0b00000000;
	  _delay_ms(0.7);
	  OCR1A = i;
	}
	PORTD = 0b00000000;
	_delay_ms(1000);
}

int decide_ballcolor(){
	float r=0,y=0,b=0;
	PORTB = 0b00000100;
	for(int i=0;i<5;i++){
		r+=analoginput_3();
		_delay_ms(500);
	}
	r/=5;
	r-=42;
	//debug_dec(r);
	PORTB = 0b01000000;
	for(int i=0;i<5;i++){
		y+=analoginput_3();
		_delay_ms(500);
	}
	y/=5;
	y+=9;
	//debug_dec(y);
	PORTB = 0b10000000;
	for(int i=0;i<5;i++){
		b+=analoginput_3();
		_delay_ms(500);
	}
	b/=5;
	b+=29;
	//debug_dec(b);
	PORTB = 0b00000000;
	if(r>y && r>b)return 0;//r
	else if(y>r && y>b)return 1;//b
	else if(b>y && b>r)return 2;//g
	return 0;		
}




void ballseach(){
	uint32_t filter;
	int distance = 0;
	int kaisu = 15;
	int stocks[kaisu];		
	int forwardcount=0;
	statInfo_t xTraStats;
	initVL53L0X(1);
	setMeasurementTimingBudget( 500UL );
	int count = 0;
	while(1){
	inr_little_rotate_right(70);
	_delay_ms(500);
	for(int i=10;i<=140;i+=10){
		for(int j=0;j<kaisu;j++){
			readRangeSingleMillimeters(&xTraStats);
			stocks[j] = xTraStats.rawDistance;
		}
	qsort(stocks,kaisu,sizeof(int),compare_int);
	filter = 0;
	for(int k=kaisu - 3;k<kaisu;k++) filter+=stocks[k];
		if(filter/3 <= 180){
				count = i-10;
				goto approach;
		}
		inr_little_rotate_left(10);
	}
	_delay_ms(500);
	inr_little_rotate_right(70);_delay_ms(100);
	inr_little_forward(100);
	forwardcount++;
	}
	
	approach:;
    _delay_ms(500);

if(filter/3 > maai+20){
	inr_little_forward(filter/3 - maai - 20);distance += (filter/3 - maai - 20);
}
else{
	inr_back(maai - filter/3 + 20);distance -= (maai - filter/3 + 20); 
}
	int hosei1,hosei2;
    inr_little_rotate_left(5);
	readRangeSingleMillimeters(&xTraStats);hosei1 = xTraStats.rawDistance;
	_delay_ms(500);
	inr_little_rotate_left(5);
	readRangeSingleMillimeters(&xTraStats);hosei2 = xTraStats.rawDistance;
	_delay_ms(500);
	if(hosei1 <=hosei2)inr_little_rotate_right(5);
	_delay_ms(500);
	
	if(min(hosei1,hosei2) < maai){
		inr_back(maai - min(hosei1,hosei2));distance -= (maai - min(hosei1,hosei2));
	}
	else{
		inr_little_forward(min(hosei1,hosei2) - maai);distance += (min(hosei1,hosei2) - maai);
	}
	
	
	ballcatch();
	if(count <= 70){
		inr_rotate_left(70 - count);//inr_rotate_right(10);
	}
	else{
		inr_rotate_right(count - 70);//rotate_right(10);
	}
	_delay_ms(200);
	if(distance >= 0 && distance <= 50)inr_little_forward(50 - distance + 20);
	else inr_little_forward(50 - distance + 20);
	_delay_ms(500);
	inr_rotate_left(90);
	_delay_ms(500);
	inr_back(50);
	_delay_ms(500);
	ichigime();
	_delay_ms(500);
	inr_back(50);
	_delay_ms(500);
	inr_rotate_left(90);
	_delay_ms(500);
	ichigime();
	_delay_ms(500);		
}


void ballemit(){
	for(int i=2149;i>=1099;i--){
		OCR1A = i;
		_delay_ms(1);
	}
	_delay_ms(1000);
	for(int i=1099;i<=2149;i++){
		OCR1A = i;
		_delay_ms(1);
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
}
void position_to_senter(){
	inr_back(50);
	_delay_ms(500);
	inr_rotate_right(90);
	_delay_ms(500);
	ichigime();
	_delay_ms(500);
	//inr_little_forward(15);
	//_delay_ms(500);
	inr_rotate_left(90);
	_delay_ms(500);
	inr_little_forward(50);
	_delay_ms(100);
}
void iki_ichigimeset(){
	ichigime();
	_delay_ms(200);
	inr_back(50);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(200);
	ichigime();
	_delay_ms(200);
	inr_back(50);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);	
}
void kaeri_ichigimeset(){
	ichigime();
	_delay_ms(500);
	inr_back(50);
	_delay_ms(500);
	inr_rotate_left(90);
	_delay_ms(500);
	ichigime();
	_delay_ms(500);
	inr_back(50);
	_delay_ms(500);
	inr_rotate_right(90);
	_delay_ms(100);
}
void rline_to_ball(){
	inr_back(50);
	_delay_ms(500);
	ichigime();
	inr_forward(610);
	iki_ichigimeset();
	inr_forward(740);
	_delay_ms(500);
	inr_rotate_right(90);
	_delay_ms(500);
	inr_forward(280);
	iki_ichigimeset();
	inr_forward(840);
	_delay_ms(500);
	inr_rotate_right(90);
	_delay_ms(500);
	inr_forward(230);
	ichigime();
	_delay_ms(500);
	position_to_senter();
}
void yline_to_ball(){
	inr_back(50);
	_delay_ms(100);
	iki_ichigimeset();
	inr_forward(750);
	inr_rotate_right(90);
	inr_forward(280);
	iki_ichigimeset();
	inr_forward(840);
	inr_rotate_right(90);
	inr_forward(200);
	ichigime();
	position_to_senter();
}
void bline_to_ball(){
	inr_forward(730);
	_delay_ms(200);
	inr_rotate_right(90);
	_delay_ms(500);
	inr_forward(230);
	ichigime();
	_delay_ms(200);
	position_to_senter();	
}
void ball_to_bline(){
	inr_forward(330);
	_delay_ms(200);
	inr_rotate_left(90);
	_delay_ms(200);
	inr_forward(610);
	kaeri_ichigimeset();
	ballemit();//hidari ni magaccau
	inr_rotate_right(180);
	_delay_ms(200);
}
void init(void) {
	debugInit();
	//--------------------------------------------------
	// GPIOs
	//--------------------------------------------------
	CBI( UCSR0B, RXEN0 );// Disable UART RX
	DDRD = 0b11111111;
	DDRB = 0b11111111;
	DDRC = 0b00000000;
	// Enable weak pullups on I2C lines
	PORTC = (1<<PIN_I2C_SCL) | (1<<PIN_I2C_SDA);
    TCCR1A = 0b10000010;//servo
	TCCR1B = 0b00011010;
	ICR1 = 19999;//8MHz 20ms
	OCR1A = 2149;//999~2149
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
	//rline_to_ball();
	//inr_rotate_left(90);
	//inr_rotate_right(90);
	ballseach();
	ball_to_bline();
	bline_to_ball();
	//right_only_b(10);
	//left_only_b(10);
	//ichigime();
	//inr_rotate_right(360);
	//yline_to_ball();
	//inr_rotate_left(90);
	//TIMSK2 = 2;
	//OCR2A = 60;
	//timer1_servo_to_moter();
	//steper1_set_speeder(60);
	//while(1);
	//inr_forward(600);
	//inr_rotate_left(90);
      //start_ball();
	 //linetrace_ver2(2);
	 //ballseach();
	 //_delay_ms(1000);
	 //forward(100);
	 //linetrace_ver2(2);
	 //rotate_left(180);
	 //linetrace_debug();
	 //timer_rotate_right(90);
	 //_delay_ms(3000);
	 //timer_rotate_left(90);
	 return 0;
}
