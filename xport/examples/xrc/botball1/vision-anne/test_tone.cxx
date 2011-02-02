#include "tone.h"
#include <textdisp.h>
#include <gba.h>

//////////////////////////////////////////////////////////
// Definitions to map gba.h defines to the docs in tone.h
#define SOUNDCNT_L GBA_REG_SGCNT0_L
#define SOUNDCNT_H GBA_REG_SGCNT0_H
#define SOUNDCNT_X GBA_REG_SGCNT1

CTextDisp td(TDM_LCD_AND_CPORT);

void test_sound(void)
{ 
  //Play a sound on channel 1

  //turn on sound circuit
  GBA_REG_SGCNT1 = 0x80;
  //full volume, enable sound 1 to left and right
  GBA_REG_SGCNT0_L=0x1177;
  // Overall output ratio - Full
  GBA_REG_SGCNT0_H = 2;

  GBA_REG_SG10_L=0x0056; //sweep shifts=6, increment, sweep time=39.1ms
  GBA_REG_SG10_H=0xf780; //duty=50%,envelope decrement
  GBA_REG_SG11=0x8400; //frequency=0x0400, loop mode
} 

volatile int foo;

void test_sleep()
{
  for(int i=0; i<100000; i++) {
    foo = i;
  }
}

int main(void)
{
  //test_sound();

  //turn on sound circuit
    //GBA_REG_SGCNT1 = 0x80;

  printf("-----------------------\n");
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));
  CTone::EnableMaster(true);
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));

  CTone tone;

  printf("Ch 0: 0x%x = 0x%x\n", &(GBA_REG_SG10_L), (int)(GBA_REG_SG10_L));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG10_H), (int)(GBA_REG_SG10_H));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG11), (int)(GBA_REG_SG11));

  tone.SetFreq(60);
  tone.SetEnable(true);
  
  printf("\n--------------------\n");
  printf("Freq 60\n\n");
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));
  printf("Ch 0: 0x%x = 0x%x\n", &(GBA_REG_SG10_L), (int)(GBA_REG_SG10_L));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG10_H), (int)(GBA_REG_SG10_H));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG11), (int)(GBA_REG_SG11));

  test_sleep();
  tone.SetFreq(120);
  
  printf("\n--------------------\n");
  printf("Freq 120\n\n");
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));
  printf("Ch 0: 0x%x = 0x%x\n", &(GBA_REG_SG10_L), (int)(GBA_REG_SG10_L));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG10_H), (int)(GBA_REG_SG10_H));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG11), (int)(GBA_REG_SG11));


  test_sleep();
  tone.SetFreq(240);

  printf("\n--------------------\n");
  printf("Freq 240\n\n");
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));
  printf("Ch 0: 0x%x = 0x%x\n", &(GBA_REG_SG10_L), (int)(GBA_REG_SG10_L));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG10_H), (int)(GBA_REG_SG10_H));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG11), (int)(GBA_REG_SG11));


  test_sleep();
  tone.SetEnable(false);

  printf("\n--------------------\n");
  printf("Disabled\n\n");
  printf("Master: 0x%x = 0x%x\n", &SOUNDCNT_L, (int)(SOUNDCNT_L));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_H, (int)(SOUNDCNT_H));
  printf("        0x%x = 0x%x\n", &SOUNDCNT_X, (int)(SOUNDCNT_X));
  printf("Ch 0: 0x%x = 0x%x\n", &(GBA_REG_SG10_L), (int)(GBA_REG_SG10_L));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG10_H), (int)(GBA_REG_SG10_H));
  printf("      0x%x = 0x%x\n", &(GBA_REG_SG11), (int)(GBA_REG_SG11));
}
