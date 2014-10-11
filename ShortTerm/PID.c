#include <>reg52.h>	  	// ʹ��ԭ����reg52.hͷ�ļ������⹦�ܼĴ����Լ���������
#include <string.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char

/*------------------------��AD�й����⹦�ܼĴ�������-------------------------*/
sfr  P1ASF = 0x9D;
sfr  ADC_CONTR = 0xBC;
sfr  ADC_RES = 0xBD;
sfr  ADC_RESL = 0xBE;
sfr  AUXR1 = 0xA2;

/*----------------------------ADC�������ö���--------------------------------*/
#define ADC_OFF ADC_COUNTER=0
#define	ADC_ON 		(1 << 7)	//AD�ϵ�
#define ADC_90T		(3 << 5)	//ADת���ٶ�ѡ��
#define ADC_180T	(2 << 5)
#define ADC_360T	(1 << 5)
#define ADC_540T	0
#define ADC_CLRFLAG	239			//ADת����ɱ�־λ�����0
#define ADC_START   (1 << 3)	//ADת��ʹ�ܣ��Զ���0
#define ADC_SELEC_CHL 0x06		//P1.1��P1.2����Ϊģ�⹦��ADʹ��
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

/*-------------------------------DAC2904�������ö���-------------------------------------*/
sbit PD = P0^0;		//����ģʽ
sbit GSET = P0^1;	//��������ģʽ��������Ϊ��·DACͬ�����ڻ�ֱ����
sbit WRT1 = P0^2;	//DAC1дʹ���ź�
sbit CLK1 = P0^3;	//DAC1ʱ��

sbit Dac13 = P3^5;	//DAC����ӿ�
sbit Dac12 = P3^4;
sbit Dac11 = P3^3;
sbit Dac10 = P3^2;
sbit Dac9 = P3^1;
sbit Dac8 = P3^0;
sbit Dac7 = P2^7;
sbit Dac6 = P2^6;
sbit Dac5 = P2^5;
sbit Dac4 = P2^4;
sbit Dac3 = P2^3;
sbit Dac2 = P2^2;
sbit Dac1 = P2^1;
sbit Dac0 = P2^0;

int adc_result1 = 0 , adc_result2 = 0;		//ADת�����������
int dac_out=0;								//DAC�����DAC2904Ϊ14 bit

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

//---------------------------PID�ṹ�嶨��--------------------------//
typedef	struct	PID
{
	int	Setpoint;		  	//�趨Ŀ��ֵ
	float	P;	      			//��������
	float	I;					//���ֳ���
	float	D;					//΢�ֳ���
	float	LastError;			//eror[-1]
	float	PrevError;			//error[-2]
} PID;

PID sPID;				//����һ����̬PID����

//--------------------------LCD�ӿڶ���------------------------------//					
sfr		 io	= 0xA0;				//P0-0x80,P1-0x90,P2-0xA0,P3-0xB0;
sbit	 rs = P0^7;				//LCD����/����ѡ���(H/L)
sbit	 rw = P0^6;				//LCD��/дѡ���(H/L)
sbit	 ep = P0^5;				//LCDʹ�ܿ���
sbit     bz = io^7;				//LCDæ��־λ

//------------------------����LCDæµ״̬-----------------------//
void lcd_busy(void)
{	
	do{
		ep = 0;
		rs = 0;		//ָ��
		rw = 1;		//����
		io = 0xff;
		ep = 1;
		_nop_();	//�ߵ�ƽ����	1us	
	}while(bz);		//bz=1��ʾæ,bz=0��ʾ����
	ep = 0;		
}

//--------------------------д��ָ�LCD----------------------------//
void lcd_wcmd(uchar cmd)
{							
	lcd_busy();	//���æ
	rs = 0;		//ָ��
	rw = 0;		//д��
	ep = 1;	
	io = cmd;	//ָ��
	ep = 0;		//�½�����Ч	
}

//-------------------------LCD����ָ��λ�ó���-----------------------//
void lcd_pos(uchar x, bit y)
{						
	if(y)lcd_wcmd(x|0xc0);	//y=1,�ڶ�����ʾ;y=0,��һ����ʾ		0<=x<16
	else lcd_wcmd(x|0x80);	//����ָ��=80+��ַ��(00H~27H,40H~67H)
}

//----------------------------д�����ݺ���------------------------//
void lcd_wdat(unsigned char Data)
{
	lcd_busy();  //���æ
	rs = 1;		 //����
	rw = 0;		 //д��
	ep = 1;
	io = Data;	 //����
	ep = 0;		 //�½�����Ч
}

//---------------------------��ʾ�ַ���---------------------------//
void prints(uchar *string)
{
	while(*string) {lcd_wdat(*string);string++;}
}

//--------------------------LCD��ʼ���趨-------------------------//
void lcd_init()
{						
	lcd_wcmd(0x38);		//����LCDΪ16X2��ʾ,5X7����,��λ���ݽӿ�
	lcd_wcmd(0x06);		//LCD��ʾ����ƶ�����(����ַָ���1,������ʾ���ƶ�)
	lcd_wcmd(0x0c);		//LCD����ʾ���������(��겻��˸,����ʾ"_")
	lcd_wcmd(0x01);		//���LCD����ʾ����
}

//--------------------------����ת�ַ�����-----------------------//
void display(float x,uint i)
{
	uchar datachar_int[3];		 //��������
	uchar datachar_res[3];		 //С������
	datachar_int[1] = (int)x%10+'0';
	datachar_int[0] = ((int)x/10)%10 + '0';
	datachar_int[2] = '\0';
		
	datachar_res[0] = ((int)(x*10))%10+'0';
	datachar_res[1] = ((int)(x*100))%10 + '0';
	datachar_res[2] = '\0';
   	lcd_pos(5,i);
	prints(datachar_int);		 //��ʾ����
	lcd_pos(7,i);
	prints(".");
	lcd_pos(8,i);
	prints(datachar_res);		 //��ʾС��
}

//-----------------------------��ʱ����----------------------------//
void delay(uint x)	   
{
	uint i,j;
	for(i=0;i<x;i++);
		for(j=0;j<10000;j++);
}

//------------------------ADCת����������------------------------//
void ADC(uint channel)
{		
	float result1 = 0, result2 = 0;	//��ADת�����ת���ɱ�����ֵ

	if(channel==1){				//ѡ��ͨ��1
		ADC_CONTR |= ADC_CH1;
	}
	if(channel==2){				//ѡ��ͨ��2
		ADC_CONTR |= ADC_CH2;
	}
		
	ADC_CONTR |= ADC_START;		//��ʼת��

	while((ADC_CONTR&0x10)!=0x10);	//�ȴ�ת�����
	ADC_CONTR &= 0xEF;			//��������־λ

	if(channel==1){
		adc_result1 = ADC_RES;		
		adc_result1	= (adc_result1<<8) | ADC_RESL;	//����ת�����
		result1 = ((float)adc_result1/1024)*5;  //ת���������5.00V �ĵ�ƽֵ
		display(result1,0);		      //��ʾ
		ADC_CONTR &= 0xF8;			  //���ͨ��ѡ��	
	}
	if(channel==2){
		adc_result2 = ADC_RES;		
		adc_result2	= (adc_result2<<8) | ADC_RESL;	//����ת�����
		result2 = ((float)adc_result2/1024)*5;  //ת���������5.00V �ĵ�ƽֵ
		display(result2,1);		      //��ʾ			
		ADC_CONTR &= 0xF8;			  //���ͨ��ѡ��
	}	 
}

//----------------------------DACת����������----------------------------//
void DAC()
{	
	int temp,i;
	temp = dac_out;
	for(i=0;i<14;i++){					//DAC���
		switch(i){
		   case 0: if(temp%2) Dac0=1; else Dac0=0; break;
		   case 1: if(temp%2) Dac1=1; else Dac1=0; break;
		   case 2: if(temp%2) Dac2=1; else Dac2=0; break;
		   case 3: if(temp%2) Dac3=1; else Dac3=0; break;
		   case 4: if(temp%2) Dac4=1; else Dac4=0; break;
		   case 5: if(temp%2) Dac5=1; else Dac5=0; break;
		   case 6: if(temp%2) Dac6=1; else Dac6=0; break;
		   case 7: if(temp%2) Dac7=1; else Dac7=0; break;
		   case 8: if(temp%2) Dac8=1; else Dac8=0; break;
		   case 9: if(temp%2) Dac9=1; else Dac9=0; break;
		   case 10: if(temp%2) Dac10=1; else Dac10=0; break;
		   case 11: if(temp%2) Dac11=1; else Dac11=0; break;
		   case 12: if(temp%2) Dac12=1; else Dac12=0; break;
		   case 13: if(temp%2) Dac13=1; else Dac13=0; break;
		   default:break;
		}
		temp /= 2;
	}				
//	delay(500);					//��ʱ
	WRT1 = 1;					//DACдʹ��
	CLK1 = 1;					//DACʱ�Ӹߵ�ƽ	 	
//	delay(1000);					//��ʱ
	WRT1 = 0; 					//DACдʹ������
	CLK1 = 0;					//DACʱ�ӵ͵�ƽ
//	delay(500);					//��ʱ	
}

//----------------------------------PID��ʼ��-------------------------------//
void IncPIDInit()
{
	sPID.Setpoint = 0;		  
	sPID.P = 0.3;	      		
	sPID.I = 0.5;				
	sPID.D = 0.5;					
	sPID.LastError = 0;		
	sPID.PrevError = 0;				
}			
	
//--------------------------------����ʽPID�㷨-----------------------------//
int IncPID(int Nextpoint)
{
	int	iError = 0, iIncpid = 0;				//��ǰ���������	
    iError = sPID.Setpoint - Nextpoint;	 	//��ǰ���
    iIncpid = sPID.P*iError - sPID.I*sPID.LastError + sPID.D*sPID.PrevError; //��������
    sPID.PrevError = sPID.LastError;			//PrevError����
    sPID.LastError = iError;					//LastError����
    return(iIncpid);		  					//��������  	
}
  	
//------------------------------������------------------------------//
void main()
{	
	int Inc = 0;						//����DAC���������
	P1ASF = ADC_SELEC_CHL;				//ѡ��P1.1��P1.2��ΪADC�����,ͨ��1��ӦAD1��ͨ��2��ӦAD2
	ADC_CONTR &= 0;						//�ȶ�ADC_CONTR��ʼ��
	ADC_CONTR = ADC_ON | ADC_90T ;		//ADC�ϵ粢���ٶ���Ϊ90��ʱ������ T					
	AUXR1 |= ADC_MOD1;				    //ת������洢ģʽΪADC_RES[1:0]+ADC_RESL[7:0]
	
	lcd_init();
	lcd_pos(1,0);
	prints("AD1:");
	lcd_pos(1,1);
	prints("AD2:");
	lcd_pos(12,0);
	prints("V");
	lcd_pos(12,1);
	prints("V");	
	  
	IncPIDInit();
	GSET = 0;	  //˫�����������ģʽ
	PD = 0;		  //DAC2904�رս���ģʽ

	while(1){
		ADC(1);
		ADC(2);
		sPID.Setpoint = adc_result1;
		Inc = IncPID(adc_result2);
		dac_out = dac_out + Inc;			//DAC�����ʷֵ��������
		DAC();							//DAC���
	}
}
