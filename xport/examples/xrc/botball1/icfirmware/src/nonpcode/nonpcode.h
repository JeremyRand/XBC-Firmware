#ifndef NONPCODE_H
#define NONPCODE_H

#include "ICRobot.h"
#include "CInterruptContSingleton.h"
#include "intcont.h"
#include "CBluetoothDevice.h"
//#include "gpioint.h"
#define DG_BASE					0x9ffd200
#define USE_OWN_CLASS

//#include "gpioint.h"

//#include "XBCRobot.h"

#ifdef USE_OWN_CLASS
#define NO_NPSERIAL

//----------------------------------------------
#define HOUSEBOT
//#define HARVESTER
//#define TEST_SERVO_LIMITS
//----------------------------------------------

#define DBQ_BT_BASE							0x9ffd000
#define UART_BASE							0x9ffd000
#define UART_VECTOR							16
#define UART_SET_BT()						*((volatile unsigned short *)UART_BASE) &= ~0x0030; \
											*((volatile unsigned short *)UART_BASE) |= 0x0010
#define UART_SET_RS232()					*((volatile unsigned short *)UART_BASE) &= ~0x0030


#include "gba.h"
#include "regbits.h"
#ifndef NO_NPSERIAL
#include "npserial.h"
#endif /* #ifndef NO_NPSERIAL */

#define GBA_SCREEN_WIDTH		240
#define GBA_SCREEN_HEIGHT		160

#define RGB16(r,g,b)  (((b)<<10)+((g)<<5)+(r))
#define VideoBuffer 	((unsigned short *)0x6000000)


#define _array_size(x) (sizeof (x)/sizeof *(x))

//typedef unsigned char u8;


static ICRobot * nonpcodeparent;
static char * othernpc;

class NonPcode : public IInterrupt //For using interrupts!
{
#ifndef NO_NPSERIAL
	friend class NPSerial;
#endif /* #ifndef NO_NPSERIAL */


//#define USE_PCODE_CAM

#ifdef USE_PCODE_CAM
#include "pcode-cam.h"
#endif


public:
	
	NonPcode();
	virtual ~NonPcode();

	int start(ICRobot * robot = 0);

//Motor Functions for Housebot - by Noah
#ifdef HOUSEBOT
	void left_motors_mrp(int velocity, int delta_pos);
	void right_motors_mrp(int velocity, int delta_pos);
	void go_straight(int velocity, int delta_pos);
	void turn(bool right, int velocity, int delta_pos);
#endif

//Motor Functions for Harvester - by Farz
#ifdef HARVESTER
	void HVlm_mrp(int velocity, int delta_pos);
	void HVrm_mrp(int velocity, int delta_pos);
	void HV_straight(int velocity, int cm);
	void HV_turn(bool right, int velocity, int degrees);
	void HV_grab_leaves();
	void HV_LA_lower_hand();
	void HV_LA_open();
	void HV_LA_close();
	void HV_raise_leaves();
	void HV_grab_water();
	void HV_release_water();
	void HV_dump_leaves();
	void HV_move_house();
	void HV_RA_raise_hand();
	void HV_RA_lower_hand();

#endif

	//ChangeValue Functions 
	void changeValue(char name[],int *val,int inc);

	void startOtherNPC(ICRobot * robot = 0);

	void Interrupt(unsigned char vector);
	void Register (IInterrupt * pint, unsigned char vector);
	void UnRegister (unsigned char vector);

	static void setMLTranslatorNPC(ICRobot * robotTranslator)
	{
		nonpcodeparent = robotTranslator;
	}

	void newNPC()
	{
#ifndef NO_NPSERIAL
		NPSerial npsrl = NPSerial();
		othernpc = npsrl.GetNewNPC();
#endif /* #ifndef NO_NPSERIAL */
		//othernpc = the_npc;
	}

protected:

	// Each bit in this register reflects the state of corresponding pins.  
	// If it is input, the bit value reflects the inputted state on the corresponding pin
	// If it is output, the corresponding pin state reflects the bit value.
	volatile unsigned short *m_data;

	// Each bit in this register determines whether the corresponding pin is input or output.
	// If the bit value is 0, the corresponding pin is input.
	// If the bit value is 1, the corresponding pin is output.
	volatile unsigned short *m_dataDir;

	// Each bit in this register determines whether the corresponding pin will trigger an interrupt.
	// If the bit value is 0, the corresponding pin will not trigger an interrupt.
	// If the bit value is 1, the corresponding pin will trigger an interrupt on each rising or falling edge.
	// An interrupt is only generated for pins configured as input.   
	volatile unsigned short *m_intMask;

	// Each bit in this register determines which edge (rising or falling) the corresponding bit will trigger an interrupt.
	// If the bit value is 0, the corresponding pin will trigger on each negative edge of the corresponding pin.
	// If the bit value is 1, the corresponding pin will trigger on each positive edge of the corresponding pin.
	// An interrupt is only generated for pins that are unmasked and configured as input.
	volatile unsigned short *m_intEdge;

	// Each bit in this register reflects whether the corresponding pin has generated an interrupt.
	// If the bit value is 1, the corresponding pin has triggered an interrupt.
	// If the bit value is 0, the corresponding pin has not triggered an interrupt.
	// This register is intended to be read from the interrupt service routine.
	// This register is automatically reset by the interrupt controller (m_pIntCont). 
	volatile unsigned short *m_intStatus;

	
	void DigitalPinPower(int pin, bool on);

#include "botball.h"

	typedef void ((NonPcode::*func)(int));
	func m_pins[16];
	IInterrupt * m_vectors[40];
	CInterruptContSingleton  * interrupts;

	CSimpTimer timer;
	int v_mseconds;

	//CPrintBuffer * m_text;
	long unsigned int mseconds();

	void pprintf(char * fmt, ...);

	void RegisterDigital(void (NonPcode::*func)(int), unsigned char pin, bool risingEdge = true);
	void print_pin(int pin);
	void killswitch(int pin)
	{
		die();
	}

	void die()
	{
		//ao();
		pprintf("Dying\n");
		pprintf("Better hope Motors are dead!!!\n");

		disable_servos();
		//sleep(1.0);
		pprintf("Uhhh Servos dead?\n");
		for(int x=0;x<8;x++) DigitalPinPower(x,false);
		pprintf("digital pins dead\n");

		pprintf("Playing dead noise\n");
#ifdef HOUSEBOT
		callml1(1540,8);
		sleep(1.0);
#else
		callml1(1540,5);
		sleep(1.0);
#endif
		

		//while(callml1(1542,0));

		pprintf("killed cpu:");
		asm("swi 0x26\n\t");
		
		//ao();
		//disable_servos():
	}

	void set_digital_interrupts(unsigned char vector = 21)
	{
		IInterrupt * temp = interrupts->GetRegistered(vector);
		if(temp) Register(temp,vector);
		interrupts->Register(this,vector);
		interrupts->Unmask(vector);
	}
/*
	float seconds();
	void msleep(long msec);
	void sleep(float seconds);
	void inline defer();
	int callml1(int command, int input, void * pointer = 0);
	int callml2(int command, int input, int input2, void * pointer = 0, void * pointer2 = 0);
	int callml3(int command, int input, int input2, int input3, void * pointer = 0);
	int button_state();
	int check_button(int button) ;
	int a_button();
	int b_button();
	int up_button();
	int down_button();
	int right_button();
	int left_button();
	int r_button();
	int l_button();
	int any_button();
	int choose_button();
	int escape_button();
	int button_transit(int button, int prev_state, int curr_state);
	int button_hit(int button, int prev_state, int curr_state);
	int button_held(int button, int prev_state, int curr_state);
	int button_released(int button, int prev_state, int curr_state);
	void display_clear() ;
	void display_set_xy(int x, int y) ;
	void display_get_xy(int * x, int * y) ;
	void display_clear_at(int x, int y, int width);
	void beeper_on();
	void beeper_off();
	void set_beeper_pitch(float frequency);
	void tone(float frequency, float length);
	void beep();
	int digital(int port);
	int extra_digital(int port);
	int analog12(int port);
	int analog(int port);
	void enable_servos();
	void disable_servos();
	void set_servo_position(int servo, int pos);
	int get_servo_position(int servo);
	long get_motor_position_counter(int motor);
	void set_motor_position_counter(int motor, long value);
	void clear_motor_position_counter(int motor);
	void mav(int motor, int velocity);
	void move_at_velocity(int motor, int velocity);
	void mtp(short motor, short velocity, int goal_pos);
	void move_to_position(short motor, short velocity, int goal_pos);
	void move_relative_position(int motor, int velocity, long delta_pos);
	void mrp(int motor, int velocity, long delta_pos);
	void freeze(int motor);
	void set_pid_gains(int x, int y, int z);
	void set_motion_acceleration(int motor, int acceleration);
	int get_motion_acceleration(int motor);
	int get_motor_done(int motor);
	void bmd(int motor);
	void block_motor_done(int motor);
	void setpwm(int motor, int pwm);
	void fd(int motor);
	void bk(int motor);
	void motor(int port, int speed);
	int _scale_speed(int speed);
	void off(int motor);
	void alloff();
	void ao();
	int enable_encoder(int port);
	void disable_encoder(int port);
	void reset_encoder(int port);
	int read_encoder(int port);
	int enable_extra_encoder(int port);
	void disable_extra_encoder(int port);
	void reset_extra_encoder(int port);
	int read_extra_encoder(int port);
	int scale_sonar(int microsecs);
	int sonar(int port);
	int set_sonar_port(int port);
	void send_sonar_ping(int port);
	int get_sonar_ping_time(int port);
	int get_scaled_sonar_value(int port);
	float power_level();
	int random(int x);
	void seed_random(long seed);
	void hog_processor();
	void set_green_led(int value);
	void set_red_led(int value);
	void set_digital_output_value(int port, int value);
*/
		//libxbc.ic:

	//static const int motor_gain_array = {600, 0, 750};

/////////////////////////////////////////////////////////////
// Time

/*  returns time since reset or reset_system_time in seconds */
float seconds()
{
    return ((float) mseconds()) / 1000.;
}

void msleep(long msec)
{
    long end_time= mseconds() + msec;
    
    while (1) {
        /* if the following test doesn't execute at least once a second,
    msleep may not halt */
        long done= mseconds()-end_time;
        if (done >= 0L && done <= 1000L) break;
        defer();
    }   
}

void sleep(float seconds)
{
    msleep((long)(seconds * 1000.));
}

void inline defer()
{
	asm("nop;nop;nop;nop;nop;");
}

/////////////////////////////////////////////////////////////
// Button Functions

// Button bit mask definitions
#define A_BTN   0x0001
#define B_BTN   0x0002
#define SEL_BTN 0x0004
#define STRT_BTN 0x0008
#define RIGHT_BTN  0x0010
#define LEFT_BTN  0x0020
#define UP_BTN   0x0040
#define DOWN_BTN  0x0080
#define R_BTN   0x0100
#define L_BTN   0x0200

// ALL_BTNS is a mask including all the buttons that IC has access to.
// This is equivalent to : (A_BTN | B_BTN | RIGHT_BTN | LEFT_BTN |
// UP_BTN | DOWN_BTN | R_BTN | L_BTN)
#define ALL_BTNS  0x03F3

// The following are aliases to provide a uniform interface across
// disparate targets for choosing and escaping from menus
#define CHOOSE_BTN A_BTN
#define ESCAPE_BTN B_BTN

// button_state returns a bit vector of the state of all the buttons.
// To tell if a particular button is pressed you bitwise and the
// result of this function withe one of the bit mask definitions for
// each button listed above.  A 0 means a given button is pressed, and
// non-zero means that it is not.  The check_button function makes
// this more convenient by doing the and and inverting the result so
// that you get a 1 if pressed and and 0 if not.

int callml1(int command, int input, void * pointer = 0)
{
	return nonpcodeparent->CallML1Translator(command,input,pointer?pointer:0);
}

int callml2(int command, int input, int input2, void * pointer = 0, void * pointer2 = 0)
{
	return nonpcodeparent->CallML2Translator(command,input,input2,pointer,pointer2);
}

int callml3(int command, int input, int input2, int input3, void * pointer = 0)
{
	return nonpcodeparent->CallML3Translator(command,input,input2,input3,pointer?pointer:0);
}

int button_state()
{
    return callml1(113, 0);
}

int check_button(int button) 
{
    return(!(button_state() & button));
}

int a_button()
{
    return check_button(A_BTN);
}

int b_button()
{
    return check_button(B_BTN);
}

int start_button()
{
    return check_button(STRT_BTN);
}

int select_button()
{
    return check_button(SEL_BTN);
}

int up_button()
{
    return check_button(UP_BTN);
}

int down_button()
{
    return check_button(DOWN_BTN);
}

int right_button()
{
    return check_button(RIGHT_BTN);
}

int left_button()
{
    return check_button(LEFT_BTN);
}

int r_button()
{
    return check_button(R_BTN);
}

int l_button()
{
    return check_button(L_BTN);
}

int any_button()
{
    return !((button_state() & ALL_BTNS) == ALL_BTNS);
}

// The following are aliases to provide a uniform interface across
// disparate targets for choosing and escaping from menus
int choose_button()
{
    return a_button();
}
int escape_button()
{
    return b_button();
}


// The following functions take a button mask and two button states,
// previous and current, and returns whether the state of the
// requested button did the following:
//   transit:   state changed, either by being hit or released
//   hit:       was not pressed previously, but is currently
//   held:      was pressed during both states
//   released:  was pressed previously, but is not currently

int button_transit(int button, int prev_state, int curr_state)
{
    return ((curr_state ^ prev_state) & button);
}

int button_hit(int button, int prev_state, int curr_state)
{
    return ((~curr_state & prev_state) & button);
}

int button_held(int button, int prev_state, int curr_state)
{
    return (~(curr_state | prev_state) & button);
}

int button_released(int button, int prev_state, int curr_state)
{
    return  (curr_state & ~prev_state) & button;
}


/////////////////////////////////////////////////////////////
// Display Functions

void display_clear() 
{
    callml1(120, 0);
}

void display_set_xy(int x, int y) 
{
    callml2(221, x, y);
}

void display_get_xy(void * x, void * y) 
{
    callml2(222, 0, 0, x, y);
}

void display_clear_at(int x, int y, int width)
{
    // Clear the part of the display starting at (x,y) for width chars
    callml3(323, x, y, width);
}

/////////////////////////////////////////////////////////////
// Tone Functions

void beeper_on()
{
    callml1(140, 1);
}

void beeper_off()
{
    callml1(140, 0);
}

void set_beeper_pitch(float frequency)
{
    callml1(141, (int)frequency);
}

/*  1/2 cycle delay in .5us goes in 0x26 and 0x27  */ 
void tone(float frequency, float length)
{
    set_beeper_pitch(frequency);
    beeper_on();
    sleep(length);
    beeper_off();
}

void beep()
{
    tone(500., .1);
}    

/////////////////////////////////////////////////////////////
// Sensor Functions
volatile int digital(int port)
{
    if(port < 16 && port > 7) return callml1(100, (port-8)*2);
    pprintf("Digital must be between 8 and 15\n");
    return -1;
}

int extra_digital(int port)
{
    if(port < 24 && port > 15) return callml1(100, ((port-16)*2)+1);
    pprintf("Extra digital must be between 16 and 23\n");
    return -1;
}

int analog12(int port)
{
    if(port < 8 && port >= 0) return callml1(101, port);
    pprintf("Analog sensors must be between 0 and 7\n");
    return -1;
}

// 8-bit analog for HB compatibility
int analog(int port)
{
    if(port < 8 && port >= 0) return callml1(103, port);
    pprintf("Analog sensors must be between 0 and 7\n");
    return -1;
}

/////////////////////////////////////////////////////////////
// Servo Functions
void enable_servos()
{
    callml1(1050, 0);
}

void disable_servos()
{
    callml1(1051, 0);
}

void reset_servos()
{
	for(short a=0;a<4;a++)
		callml2(2050, a, 128);
}

void set_servo_position(int servo, int pos)
{
    if(servo < 0 || servo > 3)
      {
        pprintf("Servo index must be between 0 and 3\n");
        return;
    }
    if (pos < 0 || pos > 255)
      {
        pprintf("Servo position must be between 0 and 255\n");
        return;
    }
    callml2(2050, 3-servo, pos);
}

int get_servo_position(int servo)
{
    if(servo < 0 || servo > 3)
      {
        pprintf("Servo index must be between 0 and 3\n");
        return -1;
    }
    return callml1(1052, 3-servo);
}

/////////////////////////////////////////////////////////////
// Motor Functions

//////////////////////////
// XBC-specific motor functions

// WARNING: These only work with firmware version 1.04 or later

// Recommended default acceleration for motor position and velocity
// commands.  Value used can be changed by calling
// set_motion_acceleration.  The actual default is set by the firmware
// on bootup.  Use get_motion_acceleration to determine what value is
// currenlty being used

// Acceleration used by the freeze command
#define FREEZE_ACCEL  5000

// Maximum allowable velocity
#define MAX_VEL   2000

#define BLACK_MOTOR 0
#define SILVER_MOTOR 1
#define WHITE_MOTOR 2

long get_motor_position_counter(int motor)
{
    long m;
    callml2(202, motor, 0, &m);
    return m;
}

void set_motor_position_counter(int motor, long value)
{
    callml2(203, motor, 0, &value);
}

void clear_motor_position_counter(int motor)
{
    long value=0L;
    callml2(203, motor, 0, &value);
}




void mav(int motor, int velocity)
{ move_at_velocity(motor, velocity); }

void move_at_velocity(int motor, int velocity)
{
    if(velocity > MAX_VEL || velocity < -MAX_VEL)
      {
        pprintf("Velocity must be in range -%d <= v <= %d\n", 
               MAX_VEL, MAX_VEL);        
        return;
    }
    callml2(208, motor, velocity);
}

// Move motor to goal_pos at given velocity.  The amount actually
// moved will be goal_pos - get_motor_position().
void mtp(short motor, short velocity, int goal_pos)
{ move_to_position(motor, velocity, goal_pos); }

void move_to_position(short motor, short velocity, int goal_pos)
{ 
    if(velocity > MAX_VEL || velocity < -MAX_VEL)
      {
        pprintf("Velocity must be in range -%d <= v <= %d\n", 
               MAX_VEL, MAX_VEL);        
        return;
    }
    callml3(304, motor, velocity, 0, &goal_pos);
}

// Move delta_pos relative to the current position of the motor.  The
// final motor tick will be get_motor_position() at the time of the
// call + delta_pos.
void move_relative_position(int motor, int velocity, long delta_pos)
{ 
    if(velocity > MAX_VEL || velocity < -MAX_VEL)
      {
        pprintf("Velocity must be in range -%d <= v <= %d\n", 
               MAX_VEL, MAX_VEL);        
        return;
    }
    callml3(305, motor, velocity, 0, &delta_pos);
}
void mrp(int motor, int velocity, long delta_pos)
{
    return(move_relative_position(motor, velocity, delta_pos));
}

//Turns off or actively holds the motor in position depending  on the situation -- but it may drift
void freeze(int motor)
{
    setpwm(motor,0); //added to make freeze work if motor was running
    callml2(209, motor, FREEZE_ACCEL);
}

void set_pid_gains(int x, int y, int z)
{
    callml3(301, x, y, z);
}

//Added Motor Function to support multiple motor types

/*void set_array_gains(int array_index)
{
    set_pid_gains(motor_gain_array[array_index][0],motor_gain_array[array_index][1],motor_gain_array[array_index][2]);
}

void mtp_x(int motor, int velocity, long goal_pos, int motor_type)
{
    set_array_gains(motor_type);
    mtp(motor, velocity, goal_pos);
}

void mrp_x(int motor, int velocity, long delta_pos, int motor_type)
{
    set_array_gains(motor_type);
    mrp(motor, velocity, delta_pos);
}

void mav_x(int motor, int velocity, int motor_type)
{
    set_array_gains(motor_type);
    mav(motor, velocity);
}*/

void set_motion_acceleration(int motor, int acceleration)
{
    callml2(210, motor, acceleration);
}

int get_motion_acceleration(int motor)
{
    return(callml1(161, motor));
}

int get_motor_done(int motor)
{
    return(callml1(162,motor));
}

void bmd(int motor)
{
    //loop doing nothing while motor position move is in progress
    while(!(callml1(162,motor)));
}

void block_motor_done(int motor)
{
    //loop doing nothing while motor position move is in progress
    while(!(callml1(162,motor)));
}

void setpwm(int motor, int pwm)
{
	callml2(201, motor, pwm);
}

//////////////////////////
// Classic pwm motor functions, scaled for XBC and Blk gear motors
/***************************************************************** */
/*                                                                */
/* MOTOR PRIMITIVES                                               */
/*                                                                */
/*   fd(n) sets motor n to full on in the green direction    */
/*   bk(n) sets motor n to full on in the red direction      */
/*   motor(n, s) sets motor n on at speed s;               */
/*     s= 100 is full on green,                  */
/*     s= -100 is full on red,                   */
/*     s= 0 is off                               */
/*   off(n)  turns off motor n      */
/*                                                                */
/*   alloff() turns off all motors                      */
/*   ao()  turns off all motors                      */
/*                                                                */
/*                                                                */
/*   motors are numbered 0 through 3.                          */
/***************************************************************** */

void fd(int motor)
{
    //    move_at_velocity(motor, MAX_VEL);
    setpwm(motor,100);
}

void bk(int motor)
{ 
    setpwm(motor,-100);
    //    move_at_velocity(motor, -MAX_VEL);
}

void motor(int port, int speed)
{
	if(speed) speed=_scale_speed(speed);
	setpwm(port,speed);
}

int _scale_speed(int speed)
{
    int dir=1;
	if (speed<0) {
        dir=-1;
        speed=-speed;
    }
	if (speed > 0){
        if (speed > 20)
		{
          //speed=speed/2+50;
		  speed/=2;
		  speed+=50;
		}else{
			speed*=3;
        }
    }
    if(speed>100)speed=100;
	speed*=dir;
    return (speed);
}



void off(int motor)
{
    setpwm(motor, 0);
}

void alloff()
{
    callml1(160, 0);
}

void ao()
{
    callml1(160, 0);
}

//////////////////////////
// Encoder functions (HB compatible)
// Since there are 16 digital ports, 
// users can actually have 16 encoders.
// For botball, we chop the top 8 off so
// we can have compatible sensors for all
// boards.
// To enable encoders for all 16 ports,
// make each function body look like this:
// if(port <= 16 && !!port)
// {
//     return callml(<func#>, port);
// }


int enable_encoder(int port)
{
    if(port > 7 && port < 16)
      {
        return callml1(170, (port-8)*2);
    }
    pprintf("Encoder port must be between 8 and 15\n");
    return 0;
}

void disable_encoder(int port)
{
    if(port > 7 && port < 16)
      {
        callml1(171, (port-8)*2);
        return;
    }
    pprintf("Encoder port must be between 8 and 15\n");
}

void reset_encoder(int port)
{
    if(port > 7 && port < 16)
      {
        callml1(172, (port-8)*2);
        return;
    }
    pprintf("Encoder port must be between 8 and 15\n");
}

int read_encoder(int port)
{
    if(port > 7 && port < 16)
      {
        return callml1(173, (port-8)*2);
    }
    pprintf("Encoder port must be between 8 and 15\n");
    return 0;
}

int enable_extra_encoder(int port)
{
    if(port > 15 && port < 24)
      {
        return callml1(170, ((port-16)*2)+1);
    }
    pprintf("Extra encoder port must be between 16 and 23\n");
    
    return 0;
}

void disable_extra_encoder(int port)
{
    if(port > 15 && port < 24)
      {
        callml1(171, ((port-16)*2)+1);
        return;
    }
    pprintf("Extra encoder port must be between 16 and 23\n");
}

void reset_extra_encoder(int port)
{
    if(port > 15 && port < 24)
      {
        callml1(172, ((port-16)*2)+1);
        return;
    }
    pprintf("Extra encoder port must be between 16 and 23\n");
}

int read_extra_encoder(int port)
{
    if(port > 15 && port < 24)
      {
        return callml1(173, ((port-16)*2)+1);
    }
    pprintf("Extra encoder port must be between 16 and 23\n");
    return 0;
}

//////////////////////////
// Sonar functions (HB compatible)
// This is a blocking sonar call,
// just like the handyboard version.
// The port number is the lower of the
// two digital ports used for the sonar
// sensor, and will always be used as the 
// input.
// The call will block until the reply
// happens, or up to 50ms, which ever
// happens first. The callml function
// simply returns the number of 
// microseconds for the round trip of
// the signal. 
// This is passed to the scaling function
// scale_sonar, which then turns the value
// into something similar to the handyboard
// value.

int scale_sonar(int microsecs)
{
    int mm;
    mm=(microsecs/100)*17;
    if(mm>4000 || mm<0) mm=32767;
    return mm;
}

int sonar(int port)
{
    if(port > 7 && port < 16) {
        return scale_sonar(callml1(177, ((port-8)<<1)));
    }        
    pprintf("Sonar port must be between 8 and 15\n");
	return 0;
}

//////////////////////////
// Sonar functions (XBC specific)
// This is a bank of non-blocking 
// sonar functions for the XBC.

int set_sonar_port(int port)
{
    if(port > 7 && port < 16)  return callml1(174, ((port-8)*2));
    pprintf("Sonar port must be between 8 and 15\n");
	return 0;
}

void send_sonar_ping(int port)
{
    if(port > 7 && port < 16) callml1(175, ((port-8)*2));
	else pprintf("Sonar port must be between 8 and 15\n");
}

int get_sonar_ping_time(int port)
{
    if(port > 7 && port < 16) return callml1(176, ((port-8)*2));
    pprintf("Sonar port must be between 8 and 15\n");
	return 0;
}

int get_scaled_sonar_value(int port)
{
    if(port > 7 && port < 16) return scale_sonar(callml1(176, ((port-8)*2)));
    pprintf("Sonar port must be between 8 and 15\n");
	return 0;
}

//////////////////////////
// Battery power functions

float power_level()
{
    float p;
    callml1(102, 0, &p);
    return p;
}

//////////////////////////
// Random functions

int random(int x)
{
    return callml1(192, x);
}

void seed_random(long seed)
{
    callml1(193, (int)&seed);
}

//////////////////////////
// Misc functions

void hog_processor()
{
    callml1(190,0);
}

void set_green_led(int value)
{
    callml1(195, value);
}

void set_red_led(int value)
{
    callml1(194, value);
}

void set_digital_output_value(int port, int value)
{
    if(value < 0) value = 0;
    if(value > 1) value = 1;
    if(port < 16 && port > 7) callml2(230, (port-8)*2, value);
    else if(port < 24 && port > 15) callml2(230, ((port-16)*2)+1,value);
    else pprintf("Digital must be between 8 and 23\n");
}


	
};

#endif


#endif
