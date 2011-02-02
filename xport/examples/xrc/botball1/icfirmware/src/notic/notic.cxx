#include "notic.h"
//#include ".h"

#ifdef USE_OWN_CLASS

#include <stdio.h>
#include <stdarg.h>

//Uncomment below if you wish to use the camera
//#define USE_PCODE_CAM

#ifdef USE_PCODE_CAM
#include "pcode-cam.h"
#endif



#define RIGHT 2
#define LEFT 0
#define FRONT 8
#define BACK 9

#define LM(x) motor(0,x)
#define RM(x) motor(2,x)

#define NEVER (~((unsigned long)0))

int main(ICRobot * rob)
{
static NotIC * npc;
//npc = new NotIC();
npc->start(rob);
}



int NotIC::start(ICRobot * robot)
{
	//WARNING!!!: DO NOT REMOVE THE FOLLOWING LINE!
	if(!nonpcodeparent) nonpcodeparent=robot;

	CSimpTimer timer;
	timer = CSimpTimer();

	int left_speed=100,right_speed=100;
	long unsigned int time=0,endwait=NEVER;

	RM(right_speed);
	LM(left_speed);
	
	while(b_button()==0)
	{
		pprintf("e-w = %ld\n",endwait-(time>>10));
		if(endwait!=NEVER) timer.GetCount(&time);
		if(digital(FRONT)==1)
		{
			RM(-right_speed);
			LM(-left_speed/4);
			endwait = 1000+(time>>10);
		}else if((digital(BACK)==1) || 
					(endwait!=NEVER && 
							((long signed int)(endwait - (time>>10)) <0)
					)
				)
		{
			RM(right_speed);
			LM(left_speed);
			endwait=NEVER;
		}
	}
	


/*
	pprintf("sizeof(char) == %d\n", sizeof(char));
	pprintf("sizeof(short) == %d\n", sizeof(short));
	pprintf("sizeof(int) == %d\n", sizeof(int));
	pprintf("sizeof(long) == %d\n", sizeof(long));
	pprintf("sizeof(long long) == %d\n", sizeof(long long));
*/
		
//test printf stuff:

	/*pprintf("Test!");


	CSimpTimer timer;
	long unsigned start;

	timer.GetCount(&start);
	parent = robot;
	//SetViewOn();
	motor(0,100);
	long unsigned begin,end;
	timer.GetCount(&begin); end = begin;
	while(end < begin + 1000){
		timer.GetCount(&end);
		//defer();
		pprintf("%d\n",end);
	}
	//motor(0,100);
	ao();
	enable_servos();
	set_servo_position(3,30);
	tone(500., 1.0);
	sleep(1.0);
	set_servo_position(3,200);
	sleep(1.0);
	set_servo_position(3,30);
	disable_servos();
	mrp(0,300,200);
	bmd(0);
	pprintf("Done?");
*/
	return 0;
}

void NotIC::startOtherNPC(ICRobot * robot)
{
	if(!nonpcodeparent) nonpcodeparent=robot;
	void (*startnpc)(ICRobot*) = (void (*)(ICRobot*))othernpc;
	(*startnpc)(robot);
}




#include "icrobotpbufcontext.h"
void NotIC::pprintf(char *fmt,...)
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

NotIC::~NotIC()
{
	//SetViewOff();
}

NotIC::NotIC()
//: m_text()
:timer()
{
	//v_on = false;
	//timer = new CSimpTimer(167761408);
	//timer = CSimpTimer();
	v_mseconds = 0;
}

long unsigned int NotIC::mseconds()
{
	long unsigned int ret;
	timer.GetCount(&ret);
	v_mseconds = ret/1000;
	//pprintf("%d\n",ret);
	return v_mseconds;
}

/*
bool NotIC::SetViewOn()
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

void NotIC::Blank(unsigned short color)
{
	if(!v_on) return;
	while(BFGET(GBA_REG_DISPSTAT, isVBlank));

	int x,y;
	for(x = 0; x < GBA_SCREEN_WIDTH; x++)   
        for(y = 0; y < GBA_SCREEN_HEIGHT; y++)  
           VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
}

bool NotIC::SetViewOff()
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
