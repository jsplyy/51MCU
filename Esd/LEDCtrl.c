#include <reg51.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
			
//�����λ
sbit K1 = P1^0;
sbit K2 = P1^1;
//����ȫ�ֱ���	
uchar i, state, LED, low, high, keyon;
bit flag;

/*-----------------------------------------------*/
/*				�Ӻ�������						 */
/*-----------------------------------------------*/

//����5ms�ղ�����ʱ���������ڰ���ȥ����
void delay5ms()
{
	uchar i,j;
	for(i=0;i<5;i++)
		for(j=0;j<240;j++); 
}
 
//���徫ȷ��ʱ��������ʱʱ��Ϊ 10*x ms
void delay(uint x)
{	
	uint time;
	//��ʼ����������i�������ʱ������־
	i = 0;
	flag = 0;
	//���㶨ʱ����ֵ
	time = 65536 - 100*x;
	low = time%16 + 16*((time/16)%16);
	high = (time/256)%16 + 16*((time/4096)%16);
	TMOD = 0x01;	//����T0��ʱģʽ1
	TL0 = low; 		//����T0��8λ��ֵ
	TH0 = high;		//����T0��8λ��ֵ
	ET0 = 1;		//��ʱ��T0�ж�����
	TR0 = 1;		//��ʱ��T0����	
}

//���尴����⺯��������״̬����Ϊ����keyonֵ
//0Ϊ�ް��£�1Ϊ����K1��2Ϊ����K2(����ȥ�����ж�)
void Keyscan()
{
	if(!K1){
		delay5ms();
		if(!K1){
			keyon = 1;
			while(!K1);
			delay5ms();
			while(!K1);
		}
	}
	else if(!K2){
		delay5ms();
		if(!K2){
			keyon = 2;
			while(!K2);
			delay5ms();
			while(!K2);
		}
	}
	else keyon = 0;
}

//ģʽ1�������ƣ����ƣ����1��	
void mode1()
{
	LED = 0xfc;	//0xfc = 1111 1100
	P0 = LED;
	flag = 1;		      
	Keyscan();
	while(keyon != 1){
		Keyscan();
		if(!flag)continue;
		P0 = LED;
		delay(100);
		LED = _cror_(LED,1);		
	}
}

//ģʽ2����һ�ƣ����ƣ����0.5��	
void mode2()
{
	LED = 0xfe;	//0xfe = 1111 1110
	P0 = LED;
	flag = 1;
	Keyscan();
 	while(keyon != 1){
		Keyscan();
		if(!flag)continue; 
    	P0 = LED;
		delay(50);
		LED = _crol_(LED,1);		
	}
}

//ģʽ3�������ƣ����ƣ��ֶ�	
void mode3()
{
	LED = 0xfc;
	P0 = LED;	  
 	while(1){
		Keyscan();
		if(keyon == 1)
			return;
		else if(keyon == 2){
			LED = _cror_(LED,1);
			P0 = LED;
		}
	}				
}

//ģʽ4����һ�ƣ����ƣ��ֶ�	
void mode4()
{
	LED = 0xfe;
	P0 = LED;	  
 	while(1){
		Keyscan();
		if(keyon == 1)
			return;
		else if(keyon == 2){
			LED = _crol_(LED,1);
			P0 = LED;
		}
	}
}	

/*-----------------------------------------------*/
/*				����������						 */
/*-----------------------------------------------*/

int main()
{
	i = 0;
	EA = 1;	//�����ж�
	state = 3;
	while(1){	
		state = (state + 1) % 4;	//ģʽѭ���л�    	
		switch(state){				//����ģʽ
			case 0: mode1(); break;
			case 1: mode2(); break;
			case 2: mode3(); break;
			case 3: mode4(); break;
			default: break;
		}
	}
}

/*-----------------------------------------------*/
/*				�жϺ���						 */
/*-----------------------------------------------*/

//��ʱ��T0����ж�
void timer0_int(void) interrupt 1
{
	TL0 = low;
	TH0 = high;
	//����ʱ�������ñ�־λflag���������i
	if(++i == 100){
		i = 0;
		flag = 1;	
	}
}


