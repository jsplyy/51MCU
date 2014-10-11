#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int

sbit shi_key = P3^5;		//ʮλ���� 
sbit ge_key = P3^4;		//��λ����

sbit dir_key = P3^3;		//ת�����򰴼� 
sbit MA = P0^1;			//��������ź� A 
sbit MB = P0^2;			//��������ź� B 

long shi = 3, ge = 0, set_freq = 30;
uchar num = 0, gao_num = 30, di_num = 90;
bit	dir = 0;
long pulse_count = 0;   //�ⲿ������� 
uint t0_count = 0;	  //��ʱ�� t0 �жϼ���������1s��ʱ 
long freq_temp = 0, freq = 0;	  //Ƶ�ʼ����ݴ�ֵ	
int flag = 0;				//ת�ٱȽ������־λ
bit comp = 1;				//ת����ȷ��־λ

/*--------------------------------------------------------------*/
/*                     LCD�������� 								��/
/*--------------------------------------------------------------*/
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
void display(long x,int i)
{
	uchar datachar[8];
	datachar[6] = x%10+'0';
	datachar[5] = (x/10)%10 + '0';
	datachar[4] = (x/100)%10 + '0';
	datachar[3] = (x/1000)%10 + '0';
	datachar[2] = (x/10000)%10 + '0';
	datachar[1] = (x/100000)%10 + '0';
	datachar[0] = (x/1000000)%10 + '0';
	datachar[7] = '\0';
   	lcd_pos(0,i);
	prints(datachar);	
}

//-------------------------��ʱ����----------------------------// 
void delay(uchar i)
{	
	uchar j,k;
	for(j=i;j>0;j--)
		for(k=125;k>0;k--);
}

//-----------------------������⺯��--------------------------// 	 
void key()
{
	if(!shi_key){
		delay(5);		//����
		if(!shi_key){	
			comp = 1;
			shi = (shi+1)%7;		//�趨ת��ʮλ���Ϊ6
			set_freq = shi*10 + ge;
			while(!shi_key);
		}
	}	
	if(!ge_key){
		delay(5);		//����
		if(!ge_key){	
			comp = 1;
			ge = (ge+1)%10;		//�趨ת�ٸ�λ���Ϊ9
			set_freq = shi*10 + ge;
			while(!ge_key);
		}
	}	
	if(!dir_key){
		delay(5);		//����
		if(!dir_key){
			dir = ~dir;  //ת���־λ��ת 
			while(!dir_key);
		}
	}
}	 

//------------------------���Ƶ������---------------------------//
void motor()
{
	uchar i;
	if(dir){		//����ת�� 
 		MB = 0;		//MB�˺�Ϊ0����MA�˼��������ź� 
		for(i=0;i<di_num;i++){
			MA = 0;
			delay(2);
		}	
		for(i=0;i<gao_num;i++){
			MA = 1;
			delay(2);
		}
	}	
	else{	    	//����ת�� 	
 		MA = 0;		//MA�˺�Ϊ0����MB�˼��������ź� 
		for(i=0;i<di_num;i++){
			MB = 0;
			delay(2);
		}	
		for(i=0;i<gao_num;i++){														 
			MB = 1;
			delay(2);
		}
	}     
}

//--------------------��ʱ���ж�t0��ʼ������-------------------//
void t0_init()
{
	TMOD |= 0x01;		//��ʱ��0����ģʽΪ MODEL 1,timer 
	TH0 = 0xFC;
	TL0 = 0x18;
	ET0 = 1;			//��ʱ��0�ж�ʹ��	
	IP = 0x02;			//�趨��ʱ��0�ж����ȼ���� 
	TR0 = 1;			//��ʱ������ 
}

//---------------------�ⲿ�ж�int0��ʼ������-------------------//
void int0_init()
{
	EX0 = 1;			//�ⲿ�ж�ʹ��
	IT0 = 1;			//�ⲿ�ж�0���ñ��ش��� 
}

//------------------------�ⲿ�ж�0�жϺ���----------------------//
void int0_int() interrupt 0
{
	pulse_count++; 
} 

//----------------------�ڲ���ʱ��t0�жϺ���----------------------//
void t0_int() interrupt 1
{
	TH0 = 0xFC;
	TL0 = 0x18;
	EA = 0;				//��ʱ�ر����ж� 	
	t0_count++;
	if(t0_count==1000){
		t0_count = 0;				//��ʱ���жϼ�������
		freq=pulse_count;	
		pulse_count = 0;			//����������� 
		display(freq,1);
		display(set_freq,0);
		if((freq<=(set_freq+1))&&(freq>=(set_freq+1)))
			comp = 0;
		if(comp){
			flag = 0;
				if((freq>=(set_freq-20))&&(freq<(set_freq-10))){
					gao_num = gao_num + 10;
					di_num = di_num - 10;
				}
				else if((freq>=(set_freq-10))&&(freq<(set_freq-5))){
					gao_num = gao_num + 5;
					di_num = di_num - 5;
				}
				else if((freq>=(set_freq-5))&&(freq<(set_freq-1))){
					gao_num = gao_num + 1;
					di_num = di_num - 1;
				}
				else if((freq<=(set_freq+20))&&(freq>(set_freq+10))){
					gao_num = gao_num - 10;
					di_num = di_num + 10;
				}
				else if((freq<=(set_freq+10))&&(freq>(set_freq+5))){
					gao_num = gao_num - 5;
					di_num = di_num + 5;
				}
				else if((freq<=(set_freq+5))&&(freq>(set_freq+1))){
					gao_num = gao_num - 1;
					di_num = di_num + 1;
				}
		} 
	}
	EA = 1;	 					//�����ж� 
}

//------------------------������---------------------------// 
void main()
{	
	EA = 1;					//�����ж� 
	lcd_init();
	lcd_pos(13,0);
	prints("r/s");
	lcd_pos(13,1);
	prints("r/s");
	t0_init();
	int0_init();

	while(1){		
		MA = 0;
		MB = 0;
		key();
		motor();
	}
}
