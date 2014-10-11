#include <reg52.h>
#include <intrins.h>
#define uint  unsigned int
#define uchar unsigned char
			
sfr  io = 0xA0;		//P0-0x80,P1-0x90,P2-0xA0,P3-0xB0;
sbit rs = P0^7;		//LCD����/����ѡ���(H/L)
sbit rw = P0^6;		//LCD��/дѡ���(H/L)
sbit ep = P0^5;		//LCDʹ�ܿ���
sbit bz = io^7;		//LCDæ��־λ
sbit trig = P3^3;	//���������ź�����
sbit echo = P3^2;	//�����ź����
sbit beep = P0^4;   //������
uchar display[] = {"000 cm"};//������ʾ
long t, distance, sum, i;

/*------------�Ӻ�������---------------*/
//��ʱ
void delay(uint ms)
{
	uint t;
	while(ms--)
		for(t=0;t<120;t++);
}	

void NOP(void)
{
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();_nop_();_nop_();
	_nop_();_nop_();
}

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

//д�����ݺ���
void lcd_wdat(uchar Data)
{
	lcd_busy();  //���æ
	rs = 1;		 //����
	rw = 0;		 //д��
	ep = 1;
	io = Data;	 //����
	ep = 0;		 //�½�����Ч
}

//LCD����ָ��λ�ó���
void lcd_pos(uchar x, bit y)
{						
	if(y)lcd_wcmd(x|0xc0);	//y=1,�ڶ�����ʾ;y=0,��һ����ʾ		0<=x<16
	else lcd_wcmd(x|0x80);	//����ָ��=80+��ַ��(00H~27H,40H~67H)
}

//��ʾ�ַ���
void prints(uchar *string)
{
	while(*string) {lcd_wdat(*string);string++;}
}

//LCD��ʼ��
void lcd_init()
{						
	lcd_wcmd(0x38);		//����LCDΪ16X2��ʾ,5X7����,��λ���ݽӿ�
	lcd_wcmd(0x06);		//LCD��ʾ����ƶ�����(����ַָ���1,������ʾ���ƶ�)
	lcd_wcmd(0x0c);		//LCD����ʾ���������(��겻��˸,����ʾ"_")
	lcd_wcmd(0x01);		//���LCD����ʾ����
}

//������ģ���ʼ��
void HC_Init()
{
	trig = 1;					//��������
	NOP();
	trig = 0;
	distance = 0.19*t;			//�������
}

//����ת�ַ�
void data2char(long dat)
{
	display[0] = dat/1000 + '0';
	display[1] = dat/100%10 + '0';
	display[2] = dat/10%10 + '0';
}

//�������� 
void alarm(void)
{
	for(i=0;i<255;i++){
		beep = ~beep;
		delay(1);
	}
}
		
/*------------������-----------*/
void main()
{
	t = 1000;
	lcd_init();
	delay(5);
	lcd_pos(0,0);
	prints("Distance:");
	TMOD = 0x19;
	EA = 1;			//�����ж�
	TR0 = 1;		//������ʱ��
	EX0 = 1;		//���ⲿ�ж�
	IT0 = 1;		//����Ϊ�½����жϷ�ʽ

	while(1){
		sum=0;
		for(i=0;i<1000;i++){
		HC_Init();
		sum = sum + distance;
		}
		distance = sum/1000;		//����1000��ȡƽ�� 
		if((distance<50) && (distance>0))	//��С��5cm���� 
			alarm();
		else 
			beep = 1;
		data2char(distance);
		lcd_pos(0,1);
		prints(display);
	}
}

//�жϺ���
void int0() interrupt 0
{
	t = TH0*256 + TL0;	//������Ϊ������� 
	TH0 = 0;
	TL0 = 0;
}
