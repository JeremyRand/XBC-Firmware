#include "npserial.h"
#ifndef NO_NPSERIAL
#include <malloc.h>
#include "CPrintBuffer.h"
#define printf g_printBuffer->PrintfIC

NPSerial::NPSerial()
{
	uartdev = new CUartDevice();
	space = (char *)malloc(128);
	if(space == 0) { printf("Not enough space???"); while(1);};
}

NPSerial::~NPSerial()
{
	free(space);
}

void NPSerial::WriteBit(char * data)
{
	static char one=0x0102;
	static char null=0x0101;
	if(*data==0) uartdev->WriteData(&null,2);
	else if(*data==1) uartdev->WriteData(&one,2);
	else uartdev->WriteData(data,1);
}

char * NPSerial::GetNewNPC()
{
	//4 bytes = 1 long = len
	//int iBytesRead = m_commDevice->Read((char*)&ucPacketSize, 1);
	printf("Connecting. ");
	uartdev->Connect();
	uartdev->SetBaudRate(DUC_BAUD_38K);
	printf("Waiting...\n");
	while(uartdev->QueryReadCount()<4){};
	//char lenc[4]={0,0,0,0};
	char * lenc = (char *)malloc(4);
	//printf("bytes read: %u\n",uartdev->Read(lenc,4));
	uartdev->Read(lenc,4);
	//printf("\nLen as char: %c\n",*lenc);
	long len = *(long*)lenc;
	printf("Len as long: %lu\n",len);
	printf("Len as hex: %lx\n",len);
	/*char * data="a";
	uartdev->Write(data,2);*/
	//malloc(&space,len)
	//space = (char*)malloc(len);
	//ReadData(&space,len)
	//while(uartdev->QueryReadCount()<(signed int)(len&(~0x03))){};
	//printf("waiting: %u",uartdev->QueryReadCount());
	printf("missing: %u",len - uartdev->QueryReadCount());
	int temp =0;
	short just;
	short percent=1;
	while(len-temp>0)
	{
		if(temp%100>=0 && temp%100<=10) printf(".");
		if(temp%(len/10)>=0 && temp%(len/10)<=10) printf("\n%d",percent++);
		//printf("left: %lu\t\t",uartdev->QueryReadCount());
		//printf("last got: %x--",space[temp]);
		//printf("%d",len-temp);
		while(uartdev->QueryReadCount()<4){}
			//|| uartdev->QueryReadCount()==(len-temp)){}
		//printf(".");
		char * added = space+temp;
		if(uartdev->QueryReadCount()>=4)
		{
			//just = uartdev->ReadData(added,1);
			//printf("%d\n",uartdev->QueryReadCount());
			just=0;
			for(int a=0;a<4;a++){
				just += uartdev->ReadData(added+a,1);
				//printf("%d,%d:%x\n",temp,a,*(added+a));
				//uartdev->WriteData(added+a,1);
				WriteBit(added+a);
			}
			//uartdev->WriteData(added,4);
			//printf("a");
			temp+=just;
		}
		else
		{
			printf("%d:::%d",len-temp,uartdev->QueryReadCount());
			temp += uartdev->ReadData(added,len-temp);
			//uartdev->WriteData(added,len-temp);
			for(int x=0;x<len-temp;x++)
				WriteBit(added+x);
		}
		
		//temp+=uartdev->Read(space+temp,len-temp);
		//printf(".");
	}
	uartdev->Disconnect();
	
	//printf("still missing: %lu",len - uartdev->QueryReadCount());
	//temp += uartdev->Read(space,uartdev->QueryReadCount());
	//int temp = uartdev->Read(space,len);
	space[len+1]=0;
	
	printf("Len is: %lX\nRead in: %d\n",len, temp);
	//printf("space = %s",space);
	return space;
}

#endif /* #ifndef NO_NPSERIAL */
