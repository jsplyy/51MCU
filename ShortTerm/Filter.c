#include <reg52.h>
#include <math.h>
#include <intrins.h>
#define uchar unsigned char
#define lint long int
#define uint unsigned int

//---------	AD9851����IO����------------------------
sbit load = P3^4;      	 	//AD9851�źŸ���λFQ_UP
sbit clk = P3^6;      	 	//д������ʱ��WCLK	 
sbit dat = P3^7; 				//����λD7	
sbit reset = P3^5;
sbit clk2 = P0^0;
sbit A1 = P1^4;
sbit B1 = P1^3;
sbit C1 = P1^2;
sbit A2 = P1^7;
sbit B2 = P1^6;
sbit C2 = P1^5;
sbit key1 = P3^2;		//���� 
sbit key2 = P3^3;
sbit rs = P0^7;			//����/���� 
sbit rw = P0^6;			//��/д 
sbit e = P0^5;			//ʹ�� 

/*------------------------��AD�й����⹦�ܼĴ�������------------------------------*/
sfr  P1ASF = 0x9D;
sfr  ADC_CONTR = 0xBC;
sfr  ADC_RES = 0xBD;
sfr  ADC_RESL = 0xBE;
sfr  AUXR1 = 0xA2;

/*-------------------------------ADC�������ö���-------------------------------------*/
#define ADC_OFF ADC_COUNTER=0
#define	ADC_ON 		(1 << 7)	//AD�ϵ�
#define ADC_90T		(3 << 5)	//ADת���ٶ�ѡ��
#define ADC_180T	(2 << 5)
#define ADC_360T	(1 << 5)
#define ADC_540T	0
#define ADC_CLRFLAG	239			//ADת����ɱ�־λ�����0
#define ADC_START   (1 << 3)	//ADת��ʹ�ܣ��Զ���0
#define ADC_SELEC_CHL 0x06		//P1.0��P1.1����Ϊģ�⹦��ADʹ��
#define ADC_CH0		0			//ͨ��ѡ�� 0~7
#define ADC_CH1		0x01
#define ADC_CH2		0x02
#define ADC_CH3		0x03
#define ADC_CH4		0x04
#define ADC_CH5		0x05
#define ADC_CH6		0x06
#define ADC_CH7		0x07
#define ADC_MOD0	0			//ת���������ģʽ0: ADC_RES[7:0]+ADC_RES[1:0]

#define	ADC_MOD1	(1 << 2)	//ת���������ģʽ1: ADC_RES[1:0]+ADC_RES[7:0]

int adc_result1 = 0 , adc_result2 = 0;		//ADת�����������
uchar i, j, k, m = 0; 
bit flag = 1;

//////////////////////////////////////////////////////////////
///-----��STC12C5A60AD ADת��������ȷ����-----------------//
///*1.P1ASF^1 / P1ASF^2 = 1; 	ͨ��ʹ��					//
///*2.ADC_POWER=1;	  ADC�ϵ�								//
///*3.SPEED0=1, SPEED1=1;									//
///*4.CHS2=0,CHS1=0,CHS0=1;  ѡ��һ��ͨ�� 					//
///*5.ADC_START=1;											//
///*6.delay;	�ʵ���ʱ									//
///*7.ADRJ=1;												//
///*8.uint result={"000000",ADC_RES[1:0],ADC_RESL[7:0]};	//
//////////////////////////////////////////////////////////////

//------------------------ADCת����������------------------------//
void ADC(uint channel)
{		
		float result1 = 0 , result2 = 0;	//��ADת�����ת���ɱ�����ֵ

	 	if(channel==1)				//ѡ��ͨ��1
			ADC_CONTR |= ADC_CH1;
	
		if(channel==2)				//ѡ��ͨ��2
			ADC_CONTR |= ADC_CH2;
			 	
		ADC_CONTR |= ADC_START;		//��ʼת��
	
		while((ADC_CONTR&0x10)!=0x10);	//�ȴ�ת�����
		ADC_CONTR &= 0xEF;			//��������־λ

	    if(channel==1){
			adc_result1 = ADC_RES;		
			adc_result1	= (adc_result1<<8) | ADC_RESL;	//����ת�����
			result1=((float)adc_result1/1024)*5;  //ת���������5.00V �ĵ�ƽֵ
			ADC_CONTR &= 0xF8;			  //���ͨ��ѡ��		
		}
		if(channel==2){
		  	adc_result2 = ADC_RES;		
			adc_result2	= (adc_result2<<8) | ADC_RESL;	//����ת�����
			result2=((float)adc_result2/1024)*5;  //ת���������5.00V �ĵ�ƽֵ		
			ADC_CONTR &= 0xF8;			  //���ͨ��ѡ��	
		}
		 
}
 /*--------------------��ʱ����-------------------------*/
void delay_50us(uint x)			
{
    uchar i,j;
	for(i=0;i<x;i++)
		for(j=0;j<10;j++);
}

/*--------------------Һ������--------------------------*/
//æ�źż��
void testbusy(void)			 
{
    do{rs = 0;
	rw = 1;
	e = 1;
	P2 = 0xff;
	_nop_();}
	while((P2&0x80)==0x80);
	e = 0;
}

 //дָ��
void send_com(uchar com_data)	  
{
    testbusy();
	rs = 0;
	rw = 0;
	e = 0;
	P2 = com_data; 	
	delay_50us(5);
	e = 1;	
	delay_50us(5);
	e = 0;			  
}

//д����
void send_data(uchar tab_data)	  
{
	rs = 1;
	rw = 0;
	e = 0;
	P2 = tab_data;
	delay_50us(5);
	e = 1; 	
	delay_50us(5);
	e = 0;
}

 //��ʼ������	  
void init(void)			
{
    delay_50us(10);
	send_com(0x30);
	send_com(0x30);
	send_com(0x0c);
	send_com(0x01);
	send_com(0x06);
	delay_50us(1);
}

//���GDRAM
void clrgdram(void)		   
{
    uchar x,y;
	for(y=0;y<64;y++)		 //64��
	for(x=0;x<16;x++){		 //16��
	    send_com(0x34);
		send_com(y+0x80);	 //�е�ַ
		send_com(x+0x80);	 //�е�ַ
		send_com(0x30);
		send_data(0x00);
		send_data(0x00);
	}
}

uchar readbyte(void)
{
    uchar byreturnvalue;
	testbusy();
	P2 = 0xff;
	rs = 1;
	rw  =1;
	e = 0;
	e = 1;
	byreturnvalue = P2;
	e = 0;
	return byreturnvalue;
}

//���㺯��
void drawpoint(uchar x, uchar y, uchar color)	  
{
    uchar row, tier, tier_bit;
	uchar readH, readL;
	send_com(0x34);
	send_com(0x36);
	tier = x>>4;
	tier_bit = x&0x0f;
	if(y<32){
	    row = y;
	}
	else{
	    row = y-32;
		tier += 8;
	}
	send_com(row+0x80);
	send_com(tier+0x80);
	readbyte();
	readH = readbyte();
	readL = readbyte();
	send_com(row+0x80);
	send_com(tier+0x80);
	if(tier_bit<8){
	    switch(color){
		    case 0: readH&=(~(0x01<<(7-tier_bit)));
			break;
			case 1: readH|=(0x01<<(7-tier_bit));
			break;
			case 2: readH^=(0x01<<(7-tier_bit));
			break;
			default:
			break;
		}
		send_data(readH);
		send_data(readL);
	}
	else{
	    switch(color){
		    case 0: readL&=(~(0x01<<(15-tier_bit)));
			break;
			case 1: readL|=(0x01<<(15-tier_bit));
			break;
			case 2: readL^=(0x01<<(15-tier_bit));
			break;
			default:
			break;
		}
		send_data(readH);
		send_data(readL);
	}
	send_com(0x30);
}

//��ʾ�ַ��� 
void prints(uchar code*s)			
{
    while(*s>0){
        send_data(*s);
        s++;
        delay_50us(50);
    }
}

/*--------------------------------------------------------------*/
//��ʱ��������
void delayms(uchar count){		//�ӳٺ���������Ϊ������
	uchar i,j;
	for(i=0;i<count;i++)
		for(j=0;j<240;j++); 
} 

				
void delay(uint t)
{
	uint i;
	while(t--)
	{
		/* ����12Mʱ�ӣ�Լ��ʱ1ms */
		for (i=0;i<125;i++)
		{}
	}
}

void ad9850_reset() //��ʼ�� 
{ 
	clk = 0; 
	load = 0; 
	reset = 0;

	reset = 1;
	reset = 0;

	clk = 1; 
	clk = 0; 

	load = 1; 
	load = 0; 
} 

					
void write_dds(unsigned long dds)
{
    uchar temp = 0x80;
	uchar i;
	load = 0;
	clk = 0;
	for(i=0;i<40;i++){
		clk = 0;
		delay(1);
		if(dds&0x00000001){
			dat = 1;
		}
		else dat = 0;
		delay(1);
		clk = 1;
		dds = dds>>1;
	}
	load = 1;
	clk = 0;
	delay(1);
	load = 0;
}

 void write_freq(unsigned long frequency)
{
	frequency = (unsigned long)(34.35973837*frequency);    //ʹ��125M����,frequence������Ҫ�����Ƶ��
	write_dds(frequency);
}

void main()
{	
	uchar i, colour = 1;
	uchar temp[90];
	unsigned long freq = 0; 
	init();
	clrgdram();
	send_com(0x01);
	delay_50us(10); 
	ad9850_reset();
	P1ASF = ADC_SELEC_CHL;				//ѡ��P1.1��P1.2��ΪADC�����,ͨ��1��ӦAD1��ͨ��2��ӦAD2
	ADC_CONTR &= 0;						//�ȶ�ADC_CONTR��ʼ��
	ADC_CONTR = ADC_ON | ADC_90T ;		//ADC�ϵ粢���ٶ���Ϊ90��ʱ������ T					
	AUXR1 |= ADC_MOD1;				    //ת������洢ģʽΪADC_RES[1:0]+ADC_RESL[7:0]
	send_com(0x80);				
    prints("Amp:");
	send_com(0x82);
    prints("00");
	send_com(0x83);
	prints("dB");
	A1 = 0;B1 = 0;C1 = 0;
	A2 = 0;B2 = 0;C2 = 0;
	EA = 1;         //�������ж�
	TMOD = 0x02;
	TH0 = 0xFF;
	TL0 = 0xFF;
	ET0 = 1;
	TR0 = 1;	 
	EX0 = 1;
	IT0 = 1;		//����Ϊ�½����жϷ�ʽ
	EX1 = 1;
	IT1 = 1;		//����Ϊ�½����жϷ�ʽ
	while(1){	
		if(flag){
			clrgdram();	 
			for(i=0;i<120;i++)	  			//������
				drawpoint(i,63,colour);
			for(i=0;i<54;i++)
				drawpoint(0,64-i,colour);
			for(i=0;i<90;i++){				//ɨƵ����ȡ��Чֵ
				write_freq(freq);
				freq = (freq + 25000)%2250000;
				if(k<5){						
					ADC(1);
					temp[i] = adc_result1;}
			}		
			for(i=4;i<94;i++)				//����
				drawpoint(i,64-temp[i-4],colour);
			flag = 0;	
		}
	}
}

void int0() interrupt 0
{
	delayms(5);
	if(!key1){
		k= (k+1)%7;
		flag = 1;
		while(!key1);
		delayms(5);
		while(!key1);
	}
	switch(k){
		case 0:
			A1=0;B1=0;C1=0;
			A2=0;B2=0;C2=0;break;
		case 1:
			A1=1;B1=0;C1=0;	
			A2=0;B2=0;C2=0;break;
		case 2:
			A1=0;B1=1;C1=0;
			A2=0;B2=0;C2=0;break;
		case 3:
			A1=1;B1=1;C1=0;
			A2=0;B2=0;C2=0;break;
		case 4:
			A1=0;B1=0;C1=1;
			A2=0;B2=0;C2=0;break;
		case 5:
			A1=0;B1=1;C1=0;
			A2=1;B2=1;C2=0;break;
		case 6:
			A1=0;B1=1;C1=0;
			A2=0;B2=0;C2=1;break;
	}
	send_com(0x82);
	send_data(k+'0');
   	send_data('0');
}

void timer0() interrupt 1
{
	clk2 = ~clk2;
	for(i=0;i<j;i++);
}
 
void int1() interrupt 2
{
	delayms(5);
	if(!key2){
		m = (m+1)%9;
		flag = 1;
		while(!key2);
		delayms(5);
		while(!key2);
	}
	switch(m){
		case 0:j=44;break;
		case 1:j=20;break;
		case 2:j=12;break;
		case 3:j=8;break;
		case 4:j=6;break;
		case 5:j=4;break;
		case 6:j=3;break;
		case 7:j=2;break;
		case 8:j=1;break;
		default:break;
	}			
} 