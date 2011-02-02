void wait_for_light(int light_port_)
{
	int l_on_, l_off_, l_mid_, t;
	
	/* print statements may look funny but are spaced for LCD*/
	pprintf ("Cal. with sensoron port #%d\n", light_port_);
	sleep(2.);
	
	pprintf("Cal: press L when light on\n");
	while(!l_button());
	l_on_=analog(light_port_); /*sensor value when light is on*/
	beep();
	
	pprintf("Cal: light on   value is=%d\n", l_on_);
	sleep(1.);
	beep();
	
	pprintf("Cal: press R when light off\n");
	while(!r_button());
	l_off_=analog(light_port_); /*sensor value when light is off*/
	beep();
	
	pprintf("Cal: light off  value is=%d\n", l_off_);
	sleep(1.);
	beep();
	
	if((l_off_-l_on_)>=15){ /*bright = small values */
		l_mid_=(l_on_+l_off_)/2;
		pprintf("Good CalibrationDiff=%d Waiting\n",(l_off_-l_on_));
		while(analog(light_port_)>l_mid_) 
		tone(440.,0.1);
	}
	else{
		if(l_off_<128){
			pprintf("Bad Calibration Add Shielding!!\n");
			while(1) /*never terminate */
			for(t=100; t<10000; t+=200) tone((float)t,0.05);
		}
		else{
			pprintf("Bad Calibration Aim sensor!!\n");
			while(1) /*never terminate */
			for(t=10000; t>200; t-=200) tone((float)t,0.05);
		}
	}
}

void shut_down_in(int seconds)
{
	GBA_REG_TM2D = (unsigned short)(-0x4000);
	BFSET(GBA_REG_TM2CNT,clockSpan,3);
	//BFSET(GBA_REG_TM2CNT,intOnOverflow,1);
	//interrupts->Register(this,CInterruptContSingleton::TIMER2);
	//interrupts->Unmask(CInterruptContSingleton::TIMER2);

	GBA_REG_TM3D = (unsigned short)(-seconds);
	//BFSET(GBA_REG_TM2CNT,clockSpan,3);
	BFSET(GBA_REG_TM3CNT,incOnOverflow,1);
	BFSET(GBA_REG_TM3CNT,intOnOverflow,1);

	interrupts->Register(this,CInterruptContSingleton::TIMER3);
	interrupts->Unmask(CInterruptContSingleton::TIMER3);

	BFSET(GBA_REG_TM2CNT,enableTimer,1);
	BFSET(GBA_REG_TM3CNT,enableTimer,1);
}
