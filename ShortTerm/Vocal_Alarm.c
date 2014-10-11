#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int

sbit Data = P1^4;			//�������
sbit Cs = P1^5;			//Ƭѡ�ź�
sbit Clk = P1^6;			//WT588D��ʱ���ź�

sbit alarm_key = P3^5;		//��ʱ����
sbit vol_set_key = P3^4;		//�������ڰ������ӵ�����������Сֵ
sbit time_adj_key = P3^3;		//ʱ����ڰ���
sbit time_selc_key = P3^2;	//ʱ�����λѡ��

uchar vol = 0xE5;			//����ֵ����Χ[E0H,E7H]
uchar addr = 0x00; 				//�������ŵ�ַ��ֱ�������������Ÿõ�ַ�����ź�
static uint miao = 0, fen = 0, shi = 0;	//ʱ�����
uint t0_count = 0;			//��ʱ���жϼ���
uint flag = 0;				//ʱ�����λѡ���־λ
uint gao = 0, di = 0;			//ʱ�������ʱ����

/*--------------------------------------------------------------*/
/*                     LCD�������� 								��/
/*--------------------------------------------------------------*/

//------------------------LCD�ӿڶ���---------------------------//					
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
void display(uint x,uint y,uint t)
{
	uchar datachar[3];
	datachar[1] = t%10+'0';
	datachar[0] = (t/10)%10 + '0';
	datachar[2] = '\0';
   	lcd_pos(x,y);
	prints(datachar);	
}

//--------------------------��ʾ��ǰʱ��-----------------------//
void display_time()
{  	
	display(4,1,shi);
	display(7,1,fen);
	display(10,1,miao);
}

/*--------------------------------------------------------------*/
/*                     ʱ�ӵ��ں������� 						*/
/*--------------------------------------------------------------*/
//--------------------------����ں���--------------------------//
void miao_inc()
{
	miao++;
	if(miao>59){		//���λ
		miao = 0;
		fen++;
		if(fen>59){		//�ֽ�λ
			fen = 0;
			shi++;
			if(shi>23){	//ʱ����
				shi = 0;
			}
		}
	}
}

//--------------------------�ֵ��ں���--------------------------//
void fen_inc()
{
	
	fen++;
	if(fen>59){		//�ֽ�λ
		fen = 0;
		miao = 0;
		shi++;
		if(shi>23){	//ʱ����
			shi = 0;
		}
	}	
}

//--------------------------ʱ���ں���--------------------------//
void shi_inc()
{
	shi++;
	if(shi>23){	//ʱ����
		shi = 0;
		fen = 0;
		miao = 0;
	}
}


//----------------------��ʱk��50us�ӳ���---------------------//
void delay50us(uint k)		
{
	uint i,j;
	for(j=0;j<k;j++)
		for(i=0;i<25;i++);
}

//----------------------���߿����ӳ���----------------------//
void send_threelines(uchar addr) 
{ 
	uint i;
	Cs = 0; 
 	delay50us(100);    /* ѡ�� �� Ƭ �� ���ֵ� ƽ 5ms */ 

 	for(i=0;i<8;i++){ 
	 	Clk = 0; 
	 	if(addr & 1)	
			Data = 1; 
		else 	
			Data = 0; 
	 	addr>>=1; 
	 	delay50us(3);    /*���ʱ������ 150us */ 
	
		Clk = 1; 
	 	delay50us(3); 
	} 
	Cs = 1;			 
}

//----------------------���������Ӻ���-----------------------//
void vol_adj(uchar set_vol)
{
	send_threelines(set_vol);
}

//-------------------����������ǰʱ��-----------------------//
void alarm_now(uint shi_1,uint fen_1)
{
	uint gao, di;
	send_threelines(0X0C);		  //�Ȳ�����ʾ����
	delay50us(3000);

	di = shi_1%10;				  //�ж�"ʱ"�ĵ�λ 
	gao = shi_1/10;			  	  //�ж�"ʱ"�ĸ�λ
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
	display_time();				  //������������ͻ��ԭ����
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////	
	if(gao==0){		  			  	  
		send_threelines(di); 		   //��λΪ0ʱֻ���ŵ�λ
	}
	else{							  
		if(gao==1){					  
			if(di==0){
				send_threelines(0x0A); //��λΪ1�ҵ�λΪ0ʱֻ��"ʮ"
			}
			else{
				send_threelines(0x0A); //��λΪ1�ҵ�λ��Ϊ0ʱ��"ʮ"�͵�λ
				delay50us(2000);
				send_threelines(di);
			}
		}
		else if(di==0){				   //��λ��Ϊ0��Ϊ1���ҵ�λΪ0ʱ���Ÿ�λ��"ʮ"
			send_threelines(gao);	  
			delay50us(2000);
			send_threelines(0x0A);	   //"ʮ"
		}
		else{
			send_threelines(gao);	   //��λ��Ϊ0��Ϊ1���ҵ�λ��Ϊ0ʱ���Ÿ�λ��"ʮ"�͵�λ
			delay50us(2000);
			send_threelines(0x0A);	   //"ʮ"
			delay50us(2000);
			send_threelines(di);
		}
	}

	delay50us(2000);			  //��ʱ200ms
	send_threelines(0x0B);		  //"��"
	delay50us(2000);

	di = fen_1%10;					  //�ж�"��"�ĸ�λ
	gao = (fen_1/10)%10;			  //�ж�"��"�ĵ�λ
	if(gao==0){		  			  //��λΪ0ʱֻ���ŵ�λ
		send_threelines(di); 		
	}
	else{
		if(gao==1){
			if(di==0){
				send_threelines(0x0A);
			}
			else{
				send_threelines(0x0A);
				delay50us(2000);
				send_threelines(di);
			}
		}
		else if(di==0){
			send_threelines(gao);	  //��λ��Ϊ0�ҵ�λΪ0ʱ���Ÿ�λ��"ʮ"
			delay50us(2000);
			send_threelines(0x0A);	  //"ʮ"
		}
		else{
			send_threelines(gao);	  //��λ��Ϊ0�ҵ�λ��Ϊ0ʱ���Ÿ�λ��"ʮ"�͵�λ
			delay50us(2000);
			send_threelines(0x0A);	  //"ʮ"
			delay50us(2000);
			send_threelines(di);
		}
	}
	
	delay50us(2000);
	send_threelines(0x0D);		  //"��"
}

//------------------------���㱨ʱ--------------------------//
void alarm_int()
{
	switch(shi){
		case 0:	 	 send_threelines(0x0E);	break;
		case 1:	 	 send_threelines(0x0F);	break;
		case 2:		 send_threelines(0x10);	break;
		case 3:		 send_threelines(0x11);	break;
		case 4:	 	 send_threelines(0x12);	break;
		case 5:	  	 send_threelines(0x13);	break;
		case 6:	     send_threelines(0x14);	break;
		case 7:	 	 send_threelines(0x15);	break;
		case 8:	 	 send_threelines(0x16);	break;
		case 9:	 	 send_threelines(0x17);	break;
		case 10:	 send_threelines(0x18);	break;
		case 11:	 send_threelines(0x19);	break;
		case 12:	 send_threelines(0x1A);	break;
		case 13:	 send_threelines(0x1B);	break;
		case 14:	 send_threelines(0x1C);	break;
		case 15:	 send_threelines(0x1D);	break;
		case 16:	 send_threelines(0x1E);	break;
		case 17:	 send_threelines(0x1F);	break;
		case 18:	 send_threelines(0x20);	break;
		case 19:	 send_threelines(0x21);	break;
		case 20:	 send_threelines(0x22);	break;
		case 21:	 send_threelines(0x23);	break;
		case 22:	 send_threelines(0x24);	break;
		case 23:	 send_threelines(0x25);	break;
		default:	 break;
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

//----------------------�ڲ���ʱ��t0�жϺ���----------------------//
void t0_int() interrupt 1
{
	TH0 = 0xFC;
	TL0 = 0x18;
	
	EA = 0;						//��ʱ�ر����ж� 	
	t0_count++;
	if(t0_count==1000){
		t0_count = 0;
		miao_inc();				//��+1
		display_time();			//��ʾ��ǰʱ��
		if((miao==0)&&(fen==0))	//������ʱ��ʱ
			alarm_int();
	}
	EA = 1;						//�����ж�
}		

//-------------------------�������--------------------------//
void key()
{
	if(!vol_set_key){
		delay50us(4);		//����
		if(!vol_set_key){
			vol = vol+1;
			if(vol>0xE7)	
				vol = 0xE2;	 //����ֵ���ΪE7H	
			while(!vol_set_key);
		    vol_adj(vol);	//�������������Ӻ���
		}
	}
	if(!alarm_key){
		delay50us(4);		//����
		if(!alarm_key){
			while(!alarm_key);
		    alarm_now(shi,fen);		//��ǰʱ����������
		}
	}	 
	if(!time_selc_key){
		delay50us(4);
		if(!time_selc_key){
			flag = (flag+1)%4;
			display(14,1,flag+1);
			while(!time_selc_key);			
		}
	}
	if(!time_adj_key){
		delay50us(4);
		if(!time_adj_key){
			switch(flag){
				case 0:	gao = (fen/10)%10;
						di = (di+1)%10;  
						fen = gao*10 + di;
						break;
				case 1:	di = fen%10;
						gao = (gao+1)%6;
						fen = gao*10 + di;
						break;
				case 2:	gao = (shi/10)%10;
						di = (di+1)%10; 
						shi = gao*10 + di;
						if(shi>23) 
							shi = 20;
						break;
				case 3:	di = shi%10;
						gao = (gao+1)%3;
						shi = gao*10 + di;
						if(shi>23) 
							shi = di;
						break;
				default: break;
			}
		}
		while(!time_adj_key);
	}
	
}	 

//-------------------------������------------------------------//
void main()
{	
	EA = 1;				   //�����ж�
	vol_adj(0xE3);		   //Ԥ������
	lcd_init();
	lcd_pos(2,0);
	prints("Present Time");
	lcd_pos(6,1);
	prints(":");
	lcd_pos(9,1);
	prints(":"); 	
	t0_init();
	display_time();			//��ʾ��ǰʱ��
	vol_adj(0xE5);			//�趨��ʼ����
							
	while(1){
		key(); 	
	}
}
