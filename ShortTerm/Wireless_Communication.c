#include <reg52.h>
#include <intrins.h>
#include "api.h"

#define uchar unsigned char

/***************************************************/
#define TX_ADR_WIDTH   5  // 5�ֽڿ�ȵķ���/���յ�ַ
#define TX_PLOAD_WIDTH 10  // ����ͨ����Ч���ݿ��

//����һ����̬���͵�ַ
uchar code TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01}; 
uchar RX_BUF[TX_PLOAD_WIDTH];			 //4 byte��ȵĽ��ջ���
uchar TX_BUF[TX_PLOAD_WIDTH];			 //4 byte��ȵķ��仺��
uchar flag;								 //��־λ
uchar DATA = 0x01;
uchar bdata sta;
uchar i, n = 0;					 	 //�����λѰַ�ı���sta
sbit RX_DR = sta^6;
sbit TX_DS = sta^5;
sbit MAX_RT = sta^4;
sbit CE = P1^0;
sbit CSN = P1^1;
sbit SCK = P1^2;
sbit MOSI = P1^3;
sbit MISO = P1^4;
sbit IRQ = P1^6;
sbit IRIN = P3^2;	//������� 
sbit BEEP = P0^4;   //������

uchar set1, set2, number;
uchar IRCOM[7];
bit flag_keybeep, senddata, flag_number;

/******************************************/
/*				��ʱ����
/******************************************/
void delay(uchar x)    //x*0.14MS
{
	uchar i;
	while(x--){
		for(i=0;i<13;i++) {}
	}
}

void delayms(uchar count)//����Ϊ������
{
	uchar i,j;
	for(i=0;i<count;i++)
		for(j=0;j<240;j++); 
} 

/***********************************/
/*			����������
/***********************************/
void beep()
{
	uchar i;
	for(i=0;i<100;i++){
		delay(4);
		BEEP = ~BEEP;             //BEEPȡ��
	} 
	BEEP = 1;                     //�رշ�����
}


/****************************/
/*			LCD 1602
/****************************/					
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
void display(uchar x,uchar i,uchar j)
{
	uchar datachar[2];
	datachar[0] = x%10 + '0';
	datachar[1] = '\0';
   	lcd_pos(6+i,j);
	prints(datachar);	
}

/**************************************************
����: init_io()

����: ��ʼ��IO
/**************************************************/
void init_io(void)
{
	CE  = 0;        // ����
	CSN = 1;        // SPI��ֹ
	SCK = 0;        // SPIʱ���õ�
	IRQ = 1;        // �жϸ�λ
}

/**************************************************
������delay_ms()

�������ӳ�x����
/**************************************************/
void delay_ms(uchar x)
{
    uchar i, j;
    i = 0;
    for(i=0; i<x; i++){
       j = 250;
       while(--j);
	   j = 250;
       while(--j);
    }
}

/**************************************************
������SPI_RW()

����������SPIЭ�飬дһ�ֽ����ݵ�nRF24L01��ͬʱ��nRF24L01
����һ�ֽ�
/**************************************************/
uchar SPI_RW(uchar byte)
{
	uchar i;
   	for(i=0; i<8; i++){         // ѭ��8��
   		MOSI = (byte & 0x80);   // byte���λ�����MOSI
   		byte <<= 1;             // ��һλ��λ�����λ
   		SCK = 1;                // ����SCK��nRF24L01��MOSI����1λ���ݣ�ͬʱ��MISO���1λ����
   		byte |= MISO;       	// ��MISO��byte���λ
   		SCK = 0;            	// SCK�õ�
   	}
    return(byte);           	// ���ض�����һ�ֽ�
}

/**************************************************
������SPI_RW_Reg()

������д����value��reg�Ĵ���
/**************************************************/
uchar SPI_RW_Reg(uchar reg, uchar value)
{
	uchar status;
  	CSN = 0;                   // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);      // ѡ��Ĵ�����ͬʱ����״̬��
  	SPI_RW(value);             // Ȼ��д���ݵ��üĴ���
  	CSN = 1;                   // CSN���ߣ��������ݴ���
  	return(status);            // ����״̬�Ĵ���
}

/**************************************************
������SPI_Read()

��������reg�Ĵ�����һ�ֽ�
/**************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	SPI_RW(reg);                // ѡ��Ĵ���
  	reg_val = SPI_RW(0);        // Ȼ��ӸüĴ���������
  	CSN = 1;                    // CSN���ߣ��������ݴ���
  	return(reg_val);            // ���ؼĴ�������
}

/**************************************************
������SPI_Read_Buf()

��������reg�Ĵ�������bytes���ֽڣ�ͨ��������ȡ����ͨ��
���ݻ����/���͵�ַ
/**************************************************/
uchar SPI_Read_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);       // ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0; i<bytes; i++)
    	pBuf[i] = SPI_RW(0);    // ����ֽڴ�nRF24L01����
  	CSN = 1;                    // CSN���ߣ��������ݴ���
  	return(status);             // ����״̬�Ĵ���
}

/**************************************************
������SPI_Write_Buf()

��������pBuf�����е�����д�뵽nRF24L01��ͨ������д�뷢
��ͨ�����ݻ����/���͵�ַ
/**************************************************/
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);       // ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0; i<bytes; i++)
    	SPI_RW(pBuf[i]);        // ����ֽ�д��nRF24L01
  	CSN = 1;                    // CSN���ߣ��������ݴ���
  	return(status);             // ����״̬�Ĵ���
}

/**************************************************
������RX_Mode()

�����������������nRF24L01Ϊ����ģʽ���ȴ����շ����豸�����ݰ�
/**************************************************/
void RX_Mode(void)
{
	CE = 0;
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // �����豸����ͨ��0ʹ�úͷ����豸��ͬ�ķ��͵�ַ
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG + RF_CH, 40);                 // ѡ����Ƶͨ��0x40
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);  // ����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ��
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);            // ���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);              // CRCʹ�ܣ�16λCRCУ�飬�ϵ磬����ģʽ
  	CE = 1;                                            // ����CE���������豸
}

/**************************************************
������TX_Mode()

�����������������nRF24L01Ϊ����ģʽ����CE=1��������10us����
130us���������䣬���ݷ��ͽ����󣬷���ģ���Զ�ת�����ģʽ�ȴ�Ӧ���źš�
/**************************************************/
void TX_Mode(uchar * BUF)
{
	CE = 0;
  	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // д�뷢�͵�ַ
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ
  	SPI_Write_Buf(WR_TX_PLOAD, BUF, TX_PLOAD_WIDTH);                  // д���ݰ���TX FIFO
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);       // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);   // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x0a);  // �Զ��ط���ʱ�ȴ�250us+86us���Զ��ط�10��
  	SPI_RW_Reg(WRITE_REG + RF_CH, 40);         // ѡ����Ƶͨ��0x40
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);    // ���ݴ�����1Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      // CRCʹ�ܣ�16λCRCУ�飬�ϵ�
	CE = 1;
}

/**************************************************
������Check_ACK()

�������������豸���޽��յ����ݰ����趨û���յ�Ӧ����
���Ƿ��ط�
/**************************************************/
uchar Check_ACK(bit clear)
{
	while(IRQ);
	sta = SPI_RW(NOP);                    // ����״̬�Ĵ���
	if(MAX_RT)
		if(clear)                         // �Ƿ����TX FIFO��û������ڸ�λMAX_RT�жϱ�־���ط�
			SPI_RW(FLUSH_TX);
	SPI_RW_Reg(WRITE_REG + STATUS, sta);  // ���TX_DS��MAX_RT�жϱ�־
	IRQ = 1;
	if(TX_DS)
		return(0x00);
	else
		return(0xff);
}

/**************************************************
������CheckInput()

������������룬������Ӧ
/**************************************************/
void CheckInput()
{		
	if(flag_number){				//����������    
		TX_BUF[i] = number;         // �����͵�����
		display(TX_BUF[i],i,0);		//��ʾ
		flag_number = 0;
		i++;
	}
	if(senddata){					//��ȷ�Ϸ���
		lcd_pos(6,0);
		prints("          ");	    //�����ʾ
		TX_BUF[i] = 0xff;           // �����͵�����
		TX_Mode(TX_BUF);			// ��nRF24L01����Ϊ����ģʽ����������
		Check_ACK(1);               // �ȴ�������ϣ����TX FIFO
		delay_ms(250);
		delay_ms(250);
		RX_Mode();	        		// ����Ϊ����ģʽ
		i = 0;
		senddata = 0;
	}
}

/*************************************************/
/*                   �����ʼ��                               
/*************************************************/
void IR_init(){
	IRIN = 1;
	BEEP = 1;
                 
	delayms(10);                 //��ʱ
	IE = 0x81;                 //�������ж��ж�,ʹ�� INT0 �ⲿ�ж�
	TCON = 0x01;               //������ʽΪ���帺���ش���
}

/**************************************************/
/*					������
/**************************************************/
void main(void)
{
	uchar j;
	lcd_init();
	IR_init();
	init_io();		              // ��ʼ��IO
	RX_Mode();		              // ����Ϊ����ģʽ
	lcd_pos(0,0);
	prints("Send:");
	lcd_pos(0,1);
	prints("Recv:");
	while(1){		
		CheckInput();             // ����ɨ��
		sta = SPI_Read(STATUS);	  // ��״̬�Ĵ���
	    if(RX_DR){				  // �ж��Ƿ���ܵ�����
			SPI_Read_Buf(RD_RX_PLOAD, RX_BUF, TX_PLOAD_WIDTH);  // ��RX FIFO��������
			flag = 1;
		}
		SPI_RW_Reg(WRITE_REG + STATUS, sta);  // ���RX_DS�жϱ�־
		if(flag){		           // �������
			j = 0;
			while(RX_BUF[j]!=0xff){
				display(RX_BUF[j],j,1);
				j++;
			}
			lcd_pos(6+j,1);
			prints("      ");
			flag = 0;		       // ���־
			delay_ms(250);
			delay_ms(250);
		}
	}
}

/****************************************/
/*			�ⲿ�ж�0��������
/****************************************/
void IR_IN() interrupt 0
{
	uchar j, k, N = 0;
	EX0 = 0;   
	delay(15);
	if(IRIN==1){
		EX0 = 1;
		return;
	} 
                           //ȷ��IR�źų���
    while(!IRIN)
		delay(1);            //��IR��Ϊ�ߵ�ƽ������9ms��ǰ���͵�ƽ�źš�

	for(j=0;j<4;j++){         //�ռ���������
		for (k=0;k<8;k++){        //ÿ��������8λ
			while(IRIN) delay(1);       //�� IR ��Ϊ�͵�ƽ������4.5ms��ǰ���ߵ�ƽ�źš�
 			while(!IRIN) delay(1);         //�� IR ��Ϊ�ߵ�ƽ
			while(IRIN){           //����IR�ߵ�ƽʱ��
				delay(1);
				N++;           
				if(N>=30){
					EX0 = 1;
	 				return;
				}                  //0.14ms���������Զ��뿪��
			}                        //�ߵ�ƽ�������                
			IRCOM[j] = IRCOM[j] >> 1;                  //�������λ����0��
			if (N>=8) IRCOM[j] = IRCOM[j] | 0x80;  //�������λ����1��
			N = 0;
		}//end for k
	}//end for j
	if (IRCOM[2]!=~IRCOM[3]){
		EX0 = 1;
		return;
	}

	IRCOM[5] = IRCOM[2] & 0x0F;     //ȡ����ĵ���λ
	IRCOM[6] = IRCOM[2] >> 4;       //����4�Σ�����λ��Ϊ����λ
 
   	set1 = IRCOM[6];
	set2 = IRCOM[5];
	flag_number = 0;
	if(set1==0){
		if(set2==12) {number = 1;flag_number = 1;}
		else if(set2==8) {number = 4;flag_number = 1;}
	}
	else if(set1==1){
		switch (set2){
			case 6: number = 0;flag_number = 1;break;
			case 12: number = 5;flag_number = 1;break;
			case 8: number = 2;flag_number = 1;break;
			default:break;
		}
	}
	else if(set1==4){
		switch (set2){
			case 2: number = 7;flag_number = 1;;break;
			case 10: number = 9;flag_number = 1;break;
			case 4: senddata = 1;break;
			case 7: flag_keybeep = ~flag_keybeep;break;
			default: break;
		}
	}		
    else if(set1==5){
		if(set2==2) {number = 8;flag_number = 1;}
		else if(set2==10) {number = 6;flag_number = 1;}
		else if(set2==14) {number = 3;flag_number = 1;}
	}
	if(flag_keybeep)
		beep();
	EX0 = 1; 
} 
 
