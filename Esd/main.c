#include "SYSTEM.H"

#define ISP_READ                1 
#define ISP_PROGRAM             2
#define ISP_DEL                 3
#define WAIT_TIME 1;  /*���õȴ�ʱ��
						   40MHZ ���� 0
						   20MHZ ���� 1
						   10MHZ ���� 2
						   5MHZ ���� 3	 */
/*--------------------------------------------------------------*/
SYSTEMTIME CurrentTime;
SYSTEMTIME setTime;
unchar mode = 0;	//Mode Value 
unchar alarm;
unchar alarmset[2], check[2], alarmhchar[3], alarmmchar[3];	
unchar set1, set2, number;
unchar IRCOM[7];
unchar keyon, pos = 0;
bit key, flag_keybeep, flag_beep, flag_number, flag_lcd, alarmstop, mode_change = 0;

sbit K1 = P1^0;
sbit K2 = P1^1;
sbit K3 = P1^2;
sbit K4 = P1^3;
sbit BEEP = P1^5; //������
sbit RELAY = P1^4;         //�̵���������
sbit IRIN = P3^2;//����ӿ� 

void Keyscan(void);
void delayms(unchar);
void mode1(void);
void mode2(void);
void mode3(void);
void beepmode(void);
/******************************************************************/
/*                   ����                                   */
/******************************************************************/
void IR_init(){
	IRIN = 1;
	BEEP = 1;
	RELAY = 1;                 
	delayms(10);                 //��ʱ
	IE = 0x81;                 //�������ж��ж�,ʹ�� INT0 �ⲿ�ж�
	TCON = 0x01;               //������ʽΪ���帺���ش���

}

void IR_task(){
	if(mode == 1){
		if(flag_number){
			switch (pos){
				case 0: if(number == 2){
							if((setTime.Hour%10)<4) 
								setTime.Hour = 10*number + setTime.Hour%10;
						}
						else if(number<2)
								setTime.Hour = 10*number + setTime.Hour%10;
						break;
				case 1: if(setTime.Hour<20)
							setTime.Hour = number + 10*(setTime.Hour/10);
						else if(number<4)
							setTime.Hour = number + 20;
						break;
				case 2: if(number<6) 
							setTime.Minute = 10*number + setTime.Minute%10;
						break;
				case 3: setTime.Minute = number + 10*(setTime.Minute/10);
						break;
				default:break;
			}
			DS1302_SetTime(DS1302_HOUR  , setTime.Hour);	//����ʱ 
			DS1302_SetTime(DS1302_MINUTE, setTime.Minute);	  //���÷�
		}
	}
	else if(mode == 2){
		if(flag_number){
			switch (pos){
				case 0: if(number == 2){
							if((alarmset[0]%10)<4) 
								alarmset[0] = 10*number + alarmset[0]%10;
						}
						else if(number<2)
							alarmset[0] = 10*number + alarmset[0]%10;
						break;
				case 1: if(setTime.Hour<20)
							alarmset[0] = number + 10*(alarmset[0]/10);
						else if(number<4)
							alarmset[0] = number + 20;
						break;
				case 2: if(number<6) 
							alarmset[1] = 10*number + alarmset[1]%10;
						break;
				case 3: alarmset[1] = number + 10*(alarmset[1]/10);
						break;
				default:break;
			}
		}
	}
}
						
			

/**********************************************************/
void delay(unchar x){    //x*0.14MS
	unchar i;
	while(x--){
	for(i=0;i<13;i++) {}
	}
}
/**********************************************************/
void beep()
{
	unchar i;
	for(i=0;i<100;i++){
	delay(4);
	BEEP = ~BEEP;                 //BEEPȡ��
	} 
	BEEP = 1;                      //�رշ�����
}	  


//
void isp_disable(void)
{
	ISP_CONTR = 0x00;
	ISP_CMD = 0x00;
	ISP_TRIG = 0x00;
	EA = 1;
}


//-----------�߳�------------------------------------------
void isp_del(unint addr_h_l)	 
{
  
  ISP_ADDRH = (unchar)((addr_h_l >> 8) & 0X00FF);
  ISP_ADDRL = (unchar)(addr_h_l & 0X00FF);
  ISP_CONTR = WAIT_TIME;
  ISP_CONTR = ISP_CONTR|0X80;
  ISP_CMD   = ISP_DEL;
  EA = 0;
  ISP_TRIG  = 0x46;
  ISP_TRIG  = 0xB9;
  _nop_();
	isp_disable();

}

//---------д����------------------------------------------------------------------------------//
void isp_write(unint write_add,unchar write_data)	
{
  
  unchar isp_data_addr_h,isp_data_addr_l;
  isp_data_addr_h = (unchar)((write_add >> 8) & 0X00FF);
  isp_data_addr_l = (unchar)(write_add & 0X00FF);

  ISP_DATA  = write_data; 
  ISP_ADDRH = isp_data_addr_h;
  ISP_ADDRL = isp_data_addr_l;
  ISP_CONTR = WAIT_TIME;
  ISP_CONTR = ISP_CONTR|0X80;
  ISP_CMD   = ISP_PROGRAM;
  EA = 0;
  ISP_TRIG  = 0x46;
  ISP_TRIG  = 0xB9;
  _nop_();
  isp_disable();
  
}

//---------------������------------------------------------------------------------------------//
unchar isp_read(unint isp_data_addr)
{

  ISP_ADDRH = (unchar)((isp_data_addr >> 8) & 0X00FF);
  ISP_ADDRL = (unchar)(isp_data_addr & 0X00FF);
  ISP_CONTR = WAIT_TIME;
  ISP_CONTR = ISP_CONTR|0X80;
  ISP_CMD   = ISP_READ;
  EA = 0;
  ISP_TRIG  = 0x46;
  ISP_TRIG  = 0xB9;
  _nop_();
  isp_disable();
  return (unchar)ISP_DATA;
}

//Main Function
void main(void)
{
	unint i;
	lcd_init();
	DS1302_Initial();
	IR_init();
	lcd_pos(0,0);	   //���� Mode:��ʾλ��
	prints("Mode:");  //��ʾ Mode:�ַ�
	lcd_pos(0,1);	   //���� Time:��ʾλ��
   	prints("Time:"); 
	lcd_pos(8,0);
	prints("Alarm:");
	  
    
	while(1){
		Keyscan();	   //�������
		lcd_pos(15,0);
		lcd_wdat((~flag_beep) | 0x30);
	
		for(i=0;i<2;i++)
			check[i] = isp_read(0x2000|(i<<9));
		 
		DS1302_GetTime(&CurrentTime);
		if((check[0] == CurrentTime.Hour) && (check[1] == CurrentTime.Minute))
			{if(!alarmstop)
				beepmode();}
		else 
			alarmstop = 0;
		
			 
		switch(mode){
			case 0: lcd_pos(6,0);
					lcd_wdat(mode | 0x31);					
					mode1();break;
			case 1: lcd_pos(6,0);
					lcd_wdat((mode+0x31));
					mode2();break;
			case 2: lcd_pos(6,0);
					lcd_wdat(mode | 0x31);
					mode3();break;
			default:break;
		}
	}	
}


/*--------------------------------------------------------------*/
//����ɨ��
void Keyscan(void){	
    if(!K1){
		delayms(5);
		if(!K1){
			mode = (mode + 1) % 3;
			keyon = 1;
			while(!K1);
			delayms(5);
			while(!K1);
		}
	}
	else if(!K2){
		delayms(5);
		if(!K2){
			keyon = 2;
			key = ~key;
			while(!K2);
			delayms(5);
			while(!K2);
		}
	}
	else if(!K3){
		delayms(5);
		if(!K3){
			keyon = 3;
			flag_beep = ~flag_beep;
			while(!K3);
			delayms(5);
			while(!K3);
		}
	}
	else if(!K4){
		delayms(5);
		if(!K4){
			keyon = 4;
			flag_beep = ~flag_beep;
			while(!K4);
			delayms(5);
			while(!K4);
		}
	}
	else keyon = 0;
	if(keyon){
		if(flag_keybeep)
			beep();
	}
}

/*--------------------------------------------------------------*/
//��ʱ��������
void delayms(unchar count){		//�ӳٺ���������Ϊ������
	unchar i,j;
	for(i=0;i<count;i++)
		for(j=0;j<240;j++); 
} 
/*--------------------------------------------------------------*/
void mode1(){
	keyon = 0;	 
	DS1302_GetTime(&CurrentTime);  //���DS1302ʱ������
	TimeToStr(&CurrentTime);	   //ʱ��ת�����ַ�
	lcd_pos(6,1);  //������ʾλ��
	prints(CurrentTime.TimeString);
}

void mode2(){
	unchar i = 0;
	keyon = 0;
	DS1302_GetTime(&setTime);  //���DS1302ʱ������
	Keyscan();	 
	while(!keyon){
		if(flag_number|mode_change){
			IR_task();
			flag_number = 0;
			mode_change = 0;
			break;
		}
		delayms(50);
		i++;
		if(i==255){
			mode = 0;
			return;
		}	
		Keyscan();
	} 
	if(keyon == 3){ 
		if(key) 
			DS1302_SetTime(DS1302_HOUR  , (setTime.Hour + 1)%24);	//����ʱ
		else 
			DS1302_SetTime(DS1302_MINUTE, (setTime.Minute + 1)%60);	  //���÷�
	}
	if(keyon == 4){
		if(key)
			DS1302_SetTime(DS1302_HOUR  , ((unchar)(setTime.Hour - 1))%29);	//����ʱ
		else
			DS1302_SetTime(DS1302_MINUTE, ((unchar)(setTime.Minute - 1))%98);	  //���÷�
	}

	DS1302_GetTime(&CurrentTime);  //���DS1302ʱ������
	TimeToStr(&CurrentTime);	   //ʱ��ת�����ַ�  
	lcd_pos(6,1);  //������ʾλ��
	prints(CurrentTime.TimeString);	
}
 
void mode3(){
	unchar i = 0;
	unint j;
	keyon = 0;

	for(j=0;j<2;j++)
		check[j] = isp_read(0x2000|(j<<9));    
	alarmhchar[0] = check[0]/10 + '0';
	alarmhchar[1] = check[0]%10 + '0';
	alarmhchar[2] = '\0'; 
	alarmmchar[0] = check[1]/10 + '0';
	alarmmchar[1] = check[1]%10 + '0';
	alarmmchar[2] = '\0'; 
	lcd_pos(6,1); 
	prints(&alarmhchar);
	lcd_pos(8,1);
	prints(":");
    lcd_pos(9,1);
	prints(&alarmmchar);
	lcd_pos(11,1);
	prints("    ");
	
	Keyscan();
	while(!keyon){
		if(flag_number|mode_change){
			IR_task();
			flag_number = 0;
			mode_change = 0;
			break;
		}
		delayms(50);
		i++;
		if(i==255){
			mode = 0;
			return;
		}	
		Keyscan();
	}			 
	
	if(keyon == 3){
		if(key) 
			{alarm = (check[0]+1)%24;alarmset[0] = alarm;}//����ʱ
		else 
			{alarm = (check[1]+1)%60;alarmset[1] = alarm;}//���÷�
	}
	if(keyon == 4){
		if(key)
			{alarm = ((unchar)(check[0]-1))%29;alarmset[0] = alarm;}	//����ʱ
		else
			{alarm = ((unchar)(check[1]-1))%98;alarmset[1] = alarm;}	  //���÷�
	}

	for(j=0;j<2;j++){
		isp_del(0x2000|(j<<9));
		isp_write(0x2000|(j<<9), alarmset[j]);
	}   
}

void beepmode(){
	unint i;
	keyon = 0;
	for(i=0;i<10000;i++){
		Keyscan();
		if(flag_beep || keyon){
			alarmstop = 1;
			BEEP = 1;
			mode = 0;
			return;
		}
		delay(4);
		BEEP = ~BEEP;
		i++;
	DS1302_GetTime(&CurrentTime);  //���DS1302ʱ������
	TimeToStr(&CurrentTime);	   //ʱ��ת�����ַ�
	lcd_pos(6,1);  //������ʾλ��
	prints(CurrentTime.TimeString);
	}
}		
	
void IR_IN() interrupt 0{
	unchar j, k, N = 0;
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
	mode_change = 0;
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
			case 0:	pos = ((unchar)(pos - 1))%4;break;
			case 3: pos = (pos + 1)%4;break;
			case 2: number = 7;flag_number = 1;;break;
			case 10: number = 9;flag_number = 1;break;
			case 5: flag_lcd = ~flag_lcd;
					flag_lcd ? lcd_off() : lcd_on();
					break;
			case 6: mode = (mode + 1) % 3;mode_change = 1;break;
			case 4: flag_beep = ~flag_beep;break;
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

