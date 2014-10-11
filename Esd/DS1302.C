# include "SYSTEM.H"

/*--------------------------------------------------------------*/
//���ż�λ�Ķ���
sbit DS1302_CLK = P3^6;   //ʱ������
sbit DS1302_IO  = P3^4;   //��������
sbit DS1302_RST = P3^5;   //��λ����
sbit ACC0 = ACC^0;
sbit ACC7 = ACC^7;

/*--------------------------------------------------------------*/
//��������
void DS1302_InputByte(unchar Data);           //д��һ���ֽ�(�ڲ�����)
unchar DS1302_OutputByte(void);               //��ȡһ���ֽ�(�ڲ�����)
void Write1302(unchar Address, unchar Data);  //��ָ����ַд��ָ��������
unchar Read1302(unchar Address);              //��ȡDS1302ָ����ַ�е�����
void DS1302_SetProtect(bit Flag);             //DS1302�Ƿ�д�뱣��
void DS1302_SetTime(unchar Address, unchar Value);//����ʱ�亯��
void DS1302_GetTime(SYSTEMTIME *Time);        //����ʱ��
void TimeToStr(SYSTEMTIME *Time);             //ʱ��ת�����ַ�
void DS1302_Initial (void);                   //DS1302��ʼ��

/*--------------------------------------------------------------*/
//д��һ���ֽ�(�ڲ�����)
void DS1302_InputByte(unchar Data){
	unchar i;
	ACC = Data;
	for(i=8; i>0; i--){	
		DS1302_CLK = 0;		
		DS1302_IO = ACC0;	//�ɵ�λ����λ		
		DS1302_CLK = 1;		//������д������
		ACC >>= 1;
	}
}

/*--------------------------------------------------------------*/
//��ȡһ���ֽ�(�ڲ�����)
unchar DS1302_OutputByte(void){
	unchar i;
	for(i=8; i>0; i--){			
		DS1302_CLK = 0;      //�½��ض���DS1302������
		ACC >>= 1;			 //�����ɵ͵���λ,ֻ����7����Чλ!!!
		ACC7 = DS1302_IO;	 //��һ������							 			
		DS1302_CLK = 1;		 						
	}						 
	return (ACC);
}

/*--------------------------------------------------------------*/
//��ָ����ַд��ָ��������
void Write1302(unchar Address, unchar Data){
	DS1302_RST = 0;
	DS1302_CLK = 0;
	DS1302_RST = 1; 
	DS1302_InputByte(Address);
	DS1302_InputByte(Data);
	DS1302_CLK = 1;
	DS1302_RST = 0;
}

/*--------------------------------------------------------------*/
//��ȡ1302ָ����ַ�е�����
unchar Read1302(unchar Address){
	unchar Data;
	DS1302_RST = 0;
	DS1302_CLK = 0;
	DS1302_RST = 1;				   //RST����,�������ݴ���
	DS1302_InputByte(Address|0x01);//��ȡָ����ַ����ָ��
	Data = DS1302_OutputByte();
	DS1302_CLK = 1;
	DS1302_RST = 0;
	return (Data);
}

/*--------------------------------------------------------------*/
//�Ƿ�д�뱣��
void DS1302_SetProtect(bit Flag){
	if(Flag)
		Write1302(0x8E,0x80);  //0x8e�����ֽڵ�ַ,bit7=WP WP=1 ��ֹ����д��DS1302
	else
		Write1302(0x8E,0x00);  //WP=0 ��������д��DS1302
}

/*--------------------------------------------------------------*/
//����ʱ�亯��
void DS1302_SetTime(unchar Address, unchar Value){
	DS1302_SetProtect(0);
	Write1302(Address,((Value/10)<<4|(Value%10)));
	DS1302_SetProtect(1);	
}

/*--------------------------------------------------------------*/
//����ʱ��
void DS1302_GetTime(SYSTEMTIME *Time){
	unchar ReadValue;
	ReadValue = Read1302(DS1302_SECOND);
	Time->Second = ((ReadValue&0x70)>>4)*10 + (ReadValue&0x0f);
	ReadValue = Read1302(DS1302_MINUTE);
	Time->Minute = ((ReadValue&0x70)>>4)*10 + (ReadValue&0x0f);
	ReadValue = Read1302(DS1302_HOUR);
	Time->Hour   = ((ReadValue&0x70)>>4)*10 + (ReadValue&0x0f);
}

/*--------------------------------------------------------------*/
//ʱ��ת�����ַ�
void TimeToStr(SYSTEMTIME *Time){
	Time->TimeString[0] = Time->Hour/10 + '0';
	Time->TimeString[1] = Time->Hour%10 + '0';
	Time->TimeString[2] = ':';
	Time->TimeString[3] = Time->Minute/10 + '0';
	Time->TimeString[4] = Time->Minute%10 + '0';
	Time->TimeString[5] = ':';
	Time->TimeString[6] = Time->Second/10 + '0';
	Time->TimeString[7] = Time->Second%10 + '0';
	Time->TimeString[8] = '\0';
}

/*--------------------------------------------------------------*/
//��ʼ��DS1302
void DS1302_Initial (void){
	unchar Second=Read1302(DS1302_SECOND);
	if(Second&0x80)//bit7=CH CH=0 ������������,CH=1����ֹͣ����
	DS1302_SetTime(DS1302_SECOND,0);
}
