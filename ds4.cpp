#include <linux/joystick.h>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include "button_def.h"

#define BUTTON_DATA_MAX 100
#define STICK_DATA_MAX 100

#define CB 20

int fd = open( "/dev/input/js0", O_RDONLY );

std::string filename = "bridge.txt";

unsigned int  ButtonData[BUTTON_DATA_MAX];
signed int     StickData[STICK_DATA_MAX];

int cb_counter = 0;

int main(int argc, char** argv){

	for(;;){
		  for(int i=0;i < BUTTON_DATA_MAX;i++) ButtonData[i] = 0;
		  struct js_event  event;
		  if( read( fd, &event, sizeof(struct js_event) ) >= sizeof(struct js_event) ){
		      switch( event.type & 0x7f ){
		      case JS_EVENT_BUTTON:
		          if( event.number < BUTTON_DATA_MAX ){
			      ButtonData[event.number] = event.value;
		          }
		          break;
		      case JS_EVENT_AXIS:
		          if( event.number < STICK_DATA_MAX ){
		              StickData[ event.number ]= event.value;
		          }
		          break;
		      } 
		  }


	if(StickData[KARSOL_R_L] == 32767){
		if(cb_counter > CB){
                	std::cout << "KARSOL_R" << std::endl;
                	std::ofstream writing_file;
                	writing_file.open(filename, std::ios::trunc);
                	writing_file << "MOVE_RIGHT" <<  std::endl;
			cb_counter = 0;
		}
		else cb_counter++;
	}
	else if(StickData[KARSOL_R_L] == -32767){
		if(cb_counter > CB){
                	std::cout << "KARSOL_L" << std::endl;
                	std::ofstream writing_file;
                	writing_file.open(filename, std::ios::trunc);
                	writing_file << "MOVE_LEFT" <<  std::endl;
			cb_counter = 0;
		}
		else cb_counter++;
	}
        else if(StickData[KARSOL_U_D] == -32767){
		if(cb_counter > CB){
                	std::cout << "KARSOL_U" << std::endl;
                	std::ofstream writing_file;
                	writing_file.open(filename, std::ios::trunc);
                	writing_file << "MOVE_FORWARD" <<  std::endl;
			cb_counter = 0;
		}
		else cb_counter++;
        }
        else if(StickData[KARSOL_U_D] == 32767){
		if(cb_counter > CB){
                	std::cout << "KARSOL_D" << std::endl;
                	std::ofstream writing_file;
                	writing_file.open(filename, std::ios::trunc);
                	writing_file << "MOVE_BACK" <<  std::endl;
			cb_counter = 0;
		}
		else cb_counter++;
        }
	else{
		std::ofstream writing_file;
		writing_file.open(filename, std::ios::trunc);
		writing_file << "MOVE_STOP" << std::endl;
	}

	
        if(ButtonData[R1] == 1){
                std::cout << "R1" << std::endl;
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "ROTATE_RIGHT" <<  std::endl;
        }
        else if(ButtonData[L1] == 1){
                std::cout << "L1" << std::endl;
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "ROTATE_LEFT" <<  std::endl;
        }
        else{
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "ROTATE_STOP" <<  std::endl;
        }


	if(ButtonData[MARU] == 1){
                std::cout << "MARU" << std::endl;
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "PICKUP" <<  std::endl;
	}
	else if(ButtonData[SANKAKU] == 1){
                std::cout << "SANKAKU" << std::endl;
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "PASS" <<  std::endl;
	}
	else{
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "NON_BUTTON" <<  std::endl;
	}


	if(ButtonData[OPTION] == 1){
                std::cout << "KICK" << std::endl;
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "KICK" <<  std::endl;
	}
	else{
                std::ofstream writing_file;
                writing_file.open(filename, std::ios::app);
                writing_file << "NON_KICK" <<  std::endl;
	}
	sleep(0.1);

	}
	close( fd );
}
 
