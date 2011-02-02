#include "nonpcode.h"

#ifdef USE_OWN_CLASS

#include <stdio.h>
#include <stdarg.h>

//Uncomment below if you wish to use the camera


#ifdef HOUSEBOT
//House Robot definitions
#define LB_MOTOR 2
#define LF_MOTOR 3
#define RB_MOTOR 0
#define RF_MOTOR 1
//#define BACK 9
#endif

#ifdef HARVESTER
//Harvester motor and servo definitions
#define LB_MOTOR 2
#define LF_MOTOR 3
#define RB_MOTOR 0
#define RF_MOTOR 1

#define RA_TW_SRV 0
#define RA_HD_SRV 1
#define LA_TW_SRV 2
#define LA_HD_SRV 3

#define RA_TW_SRV_MIN 99
#define RA_TW_SRV_MAX 255
#define RA_TW_SRV_MIDDLE 128

#define RA_HD_SRV_MIN 17
#define RA_HD_SRV_MAX 232


#define LA_TW_SRV_MIN 4
#define LA_TW_SRV_MAX 170
#define LA_TW_SRV_MIDDLE 130

#define LA_HD_SRV_MIN 92
#define LA_HD_SRV_MAX 221

#endif

#define NEVER (~((unsigned long)0))


void NonPcode::Interrupt(unsigned char vector)
{
	/*switch(vector)
	{
	default:
		
		break;
	}*/
	pprintf("::%d::\n",vector);
	if(m_vectors[vector]!=0) m_vectors[vector]->Interrupt(vector);
	if(vector==CInterruptContSingleton::TIMER3)
	{
		die();
	}
	if(vector==21)
	{
		//digital interrupt
		pprintf("%#06ho\n",*m_intStatus);
		pprintf("%#06ho\n",*m_intMask);
		//send_sonar_ping(SONAR);
	
		//Handle digital pin interrupts
		int i;
		unsigned short status = *m_intStatus;

		for (i=0; i<16 && status; i++)
		{
			if (status&0x0001 && m_pins[i])
			{
				(this->*m_pins[i])(i);
			}
			status >>= 1;
		}
	}
}

void NonPcode::print_pin(int pin)
{
	pprintf("Pin changed status: %d\n",pin);
}

//Motor functions for HOUSEBOT
#ifdef HOUSEBOT
void NonPcode::left_motors_mrp(int velocity, int delta_pos)
{
	mrp(LB_MOTOR, velocity, delta_pos);
	mrp(LF_MOTOR, velocity, delta_pos);
}

void NonPcode::right_motors_mrp(int velocity, int delta_pos)
{
	mrp(RB_MOTOR, velocity, delta_pos);
	mrp(RF_MOTOR, velocity, delta_pos);
}

void NonPcode::go_straight(int velocity, int delta_pos)
{
	left_motors_mrp(velocity, delta_pos);
	right_motors_mrp(velocity, delta_pos);
	bmd(RF_MOTOR);
}

void NonPcode::turn(bool right, int velocity, int delta_pos)
{
	if(right)
	{
		left_motors_mrp(velocity, delta_pos);
		right_motors_mrp(-velocity >>1, -delta_pos >>1);
		bmd(LF_MOTOR);
	}
	else
	{
		left_motors_mrp(-velocity>>1, -delta_pos>>1);
		right_motors_mrp(velocity, delta_pos);
		bmd(RF_MOTOR);
	}
}
#endif

//Motor and Servo functions for HARVESTER
#ifdef HARVESTER
void NonPcode::HVlm_mrp(int velocity, int delta_pos)
{
	mrp(LB_MOTOR, velocity, delta_pos);
	mrp(LF_MOTOR, velocity, delta_pos);
}

void NonPcode::HVrm_mrp(int velocity, int delta_pos)
{
	mrp(RB_MOTOR, velocity, delta_pos);
	mrp(RF_MOTOR, velocity, delta_pos);
}

void NonPcode::HV_straight(int velocity, int cm)
{
	//1000 ticks = 137/16 in.
	//1170 ticks = 23.6 cm
	HVlm_mrp(velocity, (cm * 99)>>1);
	HVrm_mrp(velocity, (cm * 99)>>1);
	bmd(RF_MOTOR);
}

void NonPcode::HV_turn(bool right, int velocity, int degrees)
{
	//Do some math to change degrees into ticks here:
	degrees*=10;
	if(right)
	{
		HVlm_mrp(velocity, degrees);
		HVrm_mrp(-velocity, -degrees);
	}
	else
	{
		HVlm_mrp(-velocity, -degrees);
		HVrm_mrp(velocity, degrees);
	}
	bmd(RF_MOTOR);
}

void NonPcode::HV_grab_leaves()
{
	int srv_pos;
	do
	{
		srv_pos = get_servo_position(LA_HD_SRV);
		set_servo_position(LA_HD_SRV,srv_pos-5);
		sleep(.05);
	}while(srv_pos>LA_HD_SRV_MIN);
	set_servo_position(LA_HD_SRV, LA_HD_SRV_MIN-5);//Closes Left Arm Hand
	sleep(0.3);
	HV_raise_leaves();
	sleep(0.3);
}

void NonPcode::HV_LA_lower_hand()
{
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIN);//Lowers Left Arm Hand
	sleep(0.3);
}

void NonPcode::HV_LA_open()
{
	set_servo_position(LA_HD_SRV, LA_HD_SRV_MAX);//Opens Left Arm Hand
	sleep(.3);
}

void NonPcode::HV_LA_close()
{
	set_servo_position(LA_HD_SRV, LA_HD_SRV_MIN);//Closes Left Arm Hand
	sleep(.3);
}

void NonPcode::HV_raise_leaves()
{
	//Raises arm slower than normal.
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIN+5);//Raises Left Arm Hand a bit
	sleep(0.15);
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIN+10);//Raises Left Arm Hand more
	sleep(0.15);
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIN+15);//Raises Left Arm Hand more
	sleep(0.15);
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIN+20);//Raises Left Arm Hand more
	sleep(0.15);
}

void NonPcode::HV_grab_water()
{
	set_servo_position(LA_HD_SRV, 128);//Closes Left Arm Hand Slightly
}

void NonPcode::HV_release_water()
{
	set_servo_position(LA_HD_SRV, LA_HD_SRV_MAX);//Opens Left Arm Hand completely
}

void NonPcode::HV_dump_leaves()
{
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MIDDLE);//Twists Left Arm to a vertical position
	sleep(1.0);//Waits for pineapple to roll off
	HV_straight(-500,-10);
	HV_turn(true,500,18);
	//HV_straight(500,8);
	set_servo_position(LA_TW_SRV, LA_TW_SRV_MAX);//Twists Left Arm past vertical upside down
	sleep(0.3);
	set_servo_position(LA_HD_SRV, LA_HD_SRV_MAX);//Opens Left Arm Hand completely
	sleep(0.4);
}

void NonPcode::HV_move_house()
{
	HV_RA_lower_hand();//Lowers Right Arm Hand
	set_servo_position(RA_TW_SRV, RA_TW_SRV_MIDDLE+40);//Twists Right Arm to the side of the robot
	sleep(.7);
	HV_RA_raise_hand();//Raise Right Arm Hand
	set_servo_position(RA_TW_SRV, 128);//Twists Right Arm Hand back to the front of the robot
	sleep(.7);
}

void NonPcode::HV_RA_lower_hand()
{
	set_servo_position(RA_HD_SRV, RA_HD_SRV_MIN);//Lowers Right Arm Hand
	sleep(0.7);
}

void NonPcode::HV_RA_raise_hand()
{
	set_servo_position(RA_HD_SRV, RA_HD_SRV_MAX);//Raise Right Arm Hand
	sleep(0.7);
}

#endif

//ChangeValue function(s) from '06 Botball - by Farz
void NonPcode::changeValue(char name[],int *val,int inc)
{
    unsigned char x,y;
    pprintf("\nChange %s\n",name);
    display_get_xy(&x,&y);
    pprintf("%s:%d   ",name,*val);
    while(!a_button())
      {
        if(up_button())
          {
            *val += inc;
            display_set_xy(x,y);
            pprintf("%s:%d   ",name,*val);
            while(b_button()&& up_button());
        }
        if(down_button())
          {
            *val -= inc;
            display_set_xy(x,y);
            pprintf("%s:%d   ",name,*val);
            while(b_button()&& down_button());
        }
    }
}

void NonPcode::DigitalPinPower(int pin, bool on)
{
	*m_dataDir |= (1<<((pin-8)*2+1));
	if(on)
		*m_data |= (1<<((pin-8)*2+1));
	else
		*m_data &= ~(1<<((pin-8)*2+1));
}

int NonPcode::start(ICRobot * robot)
{
	//WARNING!!! DO NOT REMOVE THE FOLLOWING LINE!
	if(!nonpcodeparent) nonpcodeparent=robot;

	UART_SET_BT();
	CBluetoothComm * dev_bt = new CBluetoothComm(DBQ_BT_BASE);
	dev_bt->WriteString("ATVER,ver1\r");
	char * buf = new char[0x200];
	int size = dev_bt->ReadResponse(buf, 0x200, 200, 1000);
	pprintf(buf);
	
	//char devices[][] = new char[2][10];
	//dev_bt->Discover(devices,2);
	//dev_bt->MConnect();
	
	
	/*if (!dev_bt->IsConnected()) {
		pprintf("timed out");
		return -1;
	}
	char * read = new char[20];
	dev_bt->Read(read, 20);
	*/


	return 0;
}

void NonPcode::Register (IInterrupt * pint, unsigned char vector)
{
	m_vectors[vector]=pint;
}

void NonPcode::RegisterDigital(void (NonPcode::*func)(int), unsigned char pin, bool risingEdge)
{
	pin*=2;

	m_pins[pin]=func;

    //Set lower pin to input
	*m_dataDir &= 0xffff & ~(1 << pin);
	*m_data &= 0xffff & ~(1 << pin);

	//Get interrupt on this pin
	*m_intMask |= (1 << pin);

	*m_intEdge &= ~(1 << pin); //set to zero
	if(risingEdge) *m_intEdge |= (1 << pin); //set to one if rising edge wanted
}

void NonPcode::UnRegister (unsigned char vector)
{
	m_vectors[vector]=0;
}

void NonPcode::startOtherNPC(ICRobot * robot)
{
	if(!nonpcodeparent) nonpcodeparent=robot;
	void (*startnpc)(ICRobot*) = (void (*)(ICRobot*))othernpc;
	(*startnpc)(nonpcodeparent);
}




#include "icrobotpbufcontext.h"
void NonPcode::pprintf(char *fmt,...)
{
	static bool firstPrint=true;
	if(firstPrint) {
		//g_printBuffer->Clear(g_printBuffer->BUF_IC);
	}

	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	g_printBuffer->PrintfIC("%s", buf);
	
	if(firstPrint) {
		//g_printBuffer->Clear(g_printBuffer->BUF_IC);
		//ICRobotPBufContext::Push(*g_printBuffer, *parent);
		g_printBuffer->ShowICPrintBuffer();
		firstPrint=false;
	}
}

NonPcode::~NonPcode()
{
	//SetViewOff();
}

NonPcode::NonPcode()
//: m_text()
:timer()
{
	//v_on = false;
	//timer = new CSimpTimer(167761408);
	//timer = CSimpTimer();
	v_mseconds = 0;
	interrupts = InterruptContSingleton.InstancePtr();
	m_data = (volatile unsigned short *)DG_BASE;
	m_dataDir = m_data + 1;
	m_intMask = m_data + 2;
	m_intEdge = m_data + 3;
	m_intStatus = m_data + 4;
}

long unsigned int NonPcode::mseconds()
{
	long unsigned int ret;
	timer.GetCount(&ret);
	v_mseconds = ret/1000;
	//pprintf("%d\n",ret);
	return v_mseconds;
}

/*
bool NonPcode::SetViewOn()
{
	//Change to Mode 3 and blank BGs
	BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_3);
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);
	BFSET(GBA_REG_DISPCNT, drawBG0, 0);
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);
	BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);

	Blank(RGB16(31,31,31));

	v_on=true;

	return v_on;
}

void NonPcode::Blank(unsigned short color)
{
	if(!v_on) return;
	while(BFGET(GBA_REG_DISPSTAT, isVBlank));

	int x,y;
	for(x = 0; x < GBA_SCREEN_WIDTH; x++)   
        for(y = 0; y < GBA_SCREEN_HEIGHT; y++)  
           VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
}

bool NonPcode::SetViewOff()
{
	//Back to normal... oh shit?
	//Change to Mode 0
	BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_0);
	//Turn on BG0,BG2
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);
	BFSET(GBA_REG_DISPCNT, drawBG0, 1);
	//Turn off BG3,BG1
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	//BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);
	v_on=false;
	return v_on;
}
*/


#endif
