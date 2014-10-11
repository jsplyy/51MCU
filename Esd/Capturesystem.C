#include <reg52.h>
#include <I2C.H>
#include <intrins.h>
#define  PCF8591 0x90	//PCF8591 ��ַ
#define uint unsigned int
#define uchar unsigned char

extern GetTemp();							//���������ⲿ����
extern unsigned int  idata Temperatureint;		//���������ⲿ����
extern unsigned char  idata Temperaturefloat;	//�¶ȵ�����������С������

sbit K1 = P1^0;
uchar mode;

/*--------------------------------------------------------------*/
/*                     LCD�������� 								��/
/*--------------------------------------------------------------*/

//LCD�ӿڶ���					
sfr		 io	= 0x80;				//P0-0x80,P1-0x90,P2-0xA0,P3-0xB0;
sbit	 rs = P2^6;				//LCD����/����ѡ���(H/L)
sbit	 rw = P2^5;				//LCD��/дѡ���(H/L)
sbit	 ep = P2^7;				//LCDʹ�ܿ���
sbit     bz = io^7;				//LCDæ��־λ

//��������		
void lcd_busy(void);			//����LCDæµ״̬����
void lcd_wcmd(uchar cmd);		//д��ָ�LCD����
void lcd_wdat(uchar dat);		//д�����ݵ�LCD����
void lcd_pos (uchar x, bit y);	//LCD����ָ��λ�ó���
void lcd_init(void);			//LCD��ʼ���趨����

//����LCDæµ״̬
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

//д��ָ�LCD
void lcd_wcmd(uchar cmd)
{							
	lcd_busy();	//���æ
	rs = 0;		//ָ��
	rw = 0;		//д��
	ep = 1;	
	io = cmd;	//ָ��
	ep = 0;		//�½�����Ч	
}

//LCD����ָ��λ�ó���
void lcd_pos(uchar x, bit y)
{						
	if(y)lcd_wcmd(x|0xc0);	//y=1,�ڶ�����ʾ;y=0,��һ����ʾ		0<=x<16
	else lcd_wcmd(x|0x80);	//����ָ��=80+��ַ��(00H~27H,40H~67H)
}

//д�����ݺ���
void lcd_wdat(unsigned char Data)
{
	lcd_busy();  //���æ
	rs = 1;		 //����
	rw = 0;		 //д��
	ep = 1;
	io = Data;	 //����
	ep = 0;		 //�½�����Ч
}

//��ʾ�ַ���
void prints(uchar *string)
{
	while(*string) {lcd_wdat(*string);string++;}
}

//LCD��ʼ���趨	
void lcd_init()
{						
	lcd_wcmd(0x38);		//����LCDΪ16X2��ʾ,5X7����,��λ���ݽӿ�
	lcd_wcmd(0x06);		//LCD��ʾ����ƶ�����(����ַָ���1,������ʾ���ƶ�)
	lcd_wcmd(0x0c);		//LCD����ʾ���������(��겻��˸,����ʾ"_")
	lcd_wcmd(0x01);		//���LCD����ʾ����
}


		
/*----------------------------------------------------------------*/
/*               				ADC����               			  */
/*----------------------------------------------------------------*/

//�����ֽ�
bit ISendByte(uchar sla, uchar c)
{
	Start_I2c();              //��������
	SendByte(sla);            //����������ַ
	if(ack==0) return 0;
	SendByte(c);              //��������
	if(ack==0) return 0;
	Stop_I2c();               //��������
	return 1;
}

//���ֽ�
uchar IRcvByte(uchar sla)
{  
	uchar c;
	Start_I2c();          //��������
	SendByte(sla+1);      //����������ַ
	if(ack==0) return 0;
	c=RcvByte();          //��ȡ����0	
	Ack_I2c(1);           //���ͷǾʹ�λ
	Stop_I2c();           //��������
	return c;
}  

/*----------------------------------------------------------------*/
/*               				������ʾ              			  */
/*----------------------------------------------------------------*/

//�¶�
void displaytemp(uint x, uchar y)
{
	uchar tempchar[5];
	tempchar[1] = x%10+'0';
	tempchar[0] = (x/10)%10 + '0';
	tempchar[2] = '.';
	tempchar[3] = y + '0';
	tempchar[4] = '\0';
   	lcd_pos(0,1);
	prints("Temp:   ");
	lcd_pos(9,1);
	prints(tempchar);
	lcd_pos(13,1);
	prints("    ");	
}

//��ѹ
void displayvoltage(uint x)
{
	uchar volchar[5];
	volchar[3] = x%10+'0';
	volchar[2] = (x/10)%10 + '0';
	volchar[1] = '.';
	volchar[0] = (x/100)%10 + '0';
	volchar[4] = '\0';
   	lcd_pos(0,1);
	prints("Voltage:");
	lcd_pos(9,1);
	prints(volchar);
	lcd_pos(13,1);
	prints("    ");	
}

//����
void displaylight(uint x)
{
	uchar lightchar[2];
	if (((x/100)%10) > 4)
		lightchar[0] = 'A';
	else if (((x/100)%10) > 3)
		lightchar[0] = 'B';
	else if (((x/100)%10) > 2)
		lightchar[0] = 'C';
	else if (((x/100)%10) > 1)
		lightchar[0] = 'D';
	else 
		lightchar[0] = 'E';
	lightchar[1] = '\0';
   	lcd_pos(0,1);
	prints("Intense:");
	lcd_pos(9,1);
	prints(lightchar);
	lcd_pos(10,1);
	prints("       ");	
}

/*----------------------------------------------------------------*/
/*               			ģʽ�Ӻ���               			  */
/*----------------------------------------------------------------*/

//ģʽ1����ȡ�¶�
void mode1()
{
	GetTemp();
	displaytemp(Temperatureint,Temperaturefloat);
}

//ģʽ2����ȡ��ѹ
void mode2()
{
	uint vol;
	ISendByte(PCF8591,0x41);
	vol = IRcvByte(PCF8591)*2;		//ģ��ת����1
	displayvoltage(vol);
}

//ģʽ3����ȡ����ǿ��
void mode3()
{
	uint light;
	ISendByte(PCF8591,0x43);
	light = IRcvByte(PCF8591)*2;	//ģ��ת����3
 	displaylight(light);
}

/*----------------------------------------------------------------*/
/*               			����������               			  */
/*----------------------------------------------------------------*/

//��ʱ��������
void delayms(uchar count){		//�ӳٺ���������Ϊ������
	uchar i,j;
	for(i=0;i<count;i++)
		for(j=0;j<240;j++); 
} 

//������
int main()
{
	//��ʱ����ʼ��������DS18B20	
	TMOD|= 0x11;
    TH1 = 0xD8;
    TL1 = 0xF0;	
	IE = 0x8A;	
    TR1  = 1; 
	lcd_init();
	lcd_pos(0,0); 
	prints("Mode:");
	while(1){
		if(!K1){
			delayms(5);
			if(!K1){
				mode = (mode + 1) % 3;
				while(!K1);
				delayms(5);
				while(!K1);
			}
		}
		switch(mode){		//����ģʽ�л�
			case 0: lcd_pos(6,0);prints("Temp   ");mode1();break;
			case 1: lcd_pos(6,0);prints("Voltage");mode2();break;
			case 2: lcd_pos(6,0);prints("Light  ");mode3();break;
			default:break;
		}
	} 
}