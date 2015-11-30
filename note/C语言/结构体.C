//**********************************************************************************
struct 是一种数据类型，结构变量在使用前，同其他变量一样，要先对其进行定义。
定义结构变量的一般格式：
	struct 结构名
	{
		类型 变量名；
		类型 变量名；
		....	
	}结构变量;
	
例：
	struct string
	{
		char name[8];
		int age;
		char sex[2];
		cahr depart[20];
		float wage1,wage2,wage3,wage4,wage5;		
	}person;
这个例子定义了一个结构名为string的结构变量person.

如果省略结构变量名person,则变成对结构的说明,用已经说明过的结构名也可以定义结构变量.
例：
	struct string
	{
		char name[8];
		int age;
		char sex[2];
		cahr depart[20];
		float wage1,wage2,wage3,wage4,wage5;	
	};
	struct string person;
这种方法就可以用来定义多个具有相同形式的结构变量.
	struct string code,jialin,...;
	
结构名可省略.
结构变量名不是指向该结构的地址，这点与数组名含义不同！！！因此若需要访问结构中第一个成员的首地址应该是： &结构变量名 
另一种方法是定义结构体指针 *结构变量名，这样若需要访问结构中第一个成员的首地址应该是： 结构变量名

结构变量对结构成员的表示方法：
	结构变量.成员名
结构指针对结构成员的表示方法：
	结构指针->成员名
	
结构数组用法；
例:
	struct							or         struct string				
	{										   {
		char name[8];					           char name[8];	
		int age;                                   int age;
		char sex[2];							   char sex[2];				
		char addr[20];							   char addr[20];
	}student[40];							   };
											   struct string student[40];
结构数组成员的访问是以数组元素为结构变量的,其形式为：
	结构数组元素.成员名
例：
	student[2].name			//访问整个name数组
	student[2].name[0]		//访问name数组存放的第一个元素，即name首字母
	student[27].age
	
结构指针用法；
例：
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	/*结构体声明*/
	struct string
	{
		char name[8];
		int age;
		char sex[2];
		char depart[20];
	};
	
	void main(void)
	{
		struct string *student;										//结构指针变量student定义	
		student = (struct string*)malloc(sizeof(struct string));	//结构指针使用前要进行初始化，即分配整个结构长度的字节空间，然后将其地址作为结构指针返回，此处用到了强制转换
	
		strcpy(student->name, "code");								//字符串赋值函数
		student->age  = 26;
		printf("%s\n",student->name);
		printf("%d\n",student->age);
	}							   	   
//------------------------------------------------------------------------------------------------------											 
嵌套结构：
例(普通嵌套)：
	struct addr							//addr结构声明
	{
		int city;			
	};
	
	struct string 
	{
        char name[8]; 
        int age; 
        struct addr address; 			//嵌套结构
    }student; 
    
    student.address.city = 1000;		//嵌套结构赋值

例2(结构体指针嵌套)：
	#include "stdio.h"
	#include "malloc.h"
	
	typedef struct{
	int c;
	char d;
	}s2;
	
	typedef struct {
	int a;
	char b;
	struct s2 *p;
	}s1;
	
	s1 *structer;
	
	
	int main()
	{
		structer = malloc(sizeof(s1));
		structer->p = malloc(sizeof(struct s2));
		
		structer->p->c = 2;
		//structer->a = 3;
		printf("the value is %d\n",structer->p->c);
		return 0;
	}
	结论：结构体指针使用前必须初始化指向的内存区域。
		  结构体指针可以嵌套使用，但必须注意，嵌套的结构体指针也必须初始化指向的内存区域，且必须按照由外而内的顺序初始化结构体指针，如上例中结构体指针structer和structer->p的初始化顺序不可		  颠倒
		      
	
/------------------------------------------------------------------------------------------------------    
位结构：								//MCU中定义register时常用手法
用于需要按位访问一个字节时.
位结构定义一般形式：
	struct 位结构名
	{
		类型 变量名:整型常数;
		类型 变量名:整型常数;
		....
	}位结构变量;
其中，类型必须是 signed int || unsigned int	。
切记，位结构所占空间为4字节对齐，空间大小跟成员数量无关，所有成员的位数之和有关！！！
例：
	#include "stdio.h"
	struct{
			int a1:8;
			int a2:8;
			char s1[2];
			char s2[2];
		}sa;
	int main()
	{
		printf("%d\n",sizeof(sa));		//这里绝不是6，而是8
		while(1);
	} 
	
例：
	/*******Pic16f1827_PROT_B_register_define******/
	struct 
	{
        volatile unsigned LATB0               : 1;
        volatile unsigned LATB1               : 1;
        volatile unsigned LATB2               : 1;
        volatile unsigned LATB3               : 1;
        volatile unsigned LATB4               : 1;
        volatile unsigned LATB5               : 1;
        volatile unsigned LATB6               : 1;
        volatile unsigned LATB7               : 1;
    }LATAB; 
    
    LATAB.LATAB3 = 0;			//位结构赋值
    
//-----------------------------------------------------------------------------------------------------
typedef的结构体指针用法：
例：
	typedef struct a
	{
		struct b *c;	
	}*d;
	d e;
	
	struct b
	{
		char f;	
	}
	e->c=0xf0049000;
当typedef+结构体时，这里的 *d 不再是结构体指针变量，而是代表一个结构体指针类型.
//---------------------------------------------------------------------------------------------------------
位结构应用模板(KL25_ADC模块)：
	#define ADC0_SE3A_PE22					(0x66C60U)
	//通用外围模块内存区域定义
	typedef struct
	{
		uint32_t m_ModuleIndex:	3;
		uint32_t m_PortIndex:	3;
		uint32_t m_MuxIndex:	3;
		uint32_t m_PinBaseIndex:5;
		uint32_t m_PinCntIndex:	3;
		uint32_t m_ChlIndex:	5;
		uint32_t m_SpecDefine1:	2;
	}PeripheralMapTypeDef;
	
	void ADC_Init(t)
	{
		uint8_t i;
		ADC_Type *ADCx = ADC0;
		PeripheralMapTypeDef *pADC_Map = (PeripheralMapTypeDef*)&(0x66C60U);	
		//存放数据 0x66C60U 的内存地址强制转换成 PeripheralMapTypeDef* 类型的位结构指针变量
		//再把内存地址赋给相同类型的位结构指针变量 pADC_Map模块，这样就把内存地址中存放的数据 0x66C60U 按位映射到了pADC_Map模块对应的结构体成员中
		//m_ModuleIndex:	0x00000000
		//m_PortIndex:		0x00000004
		//m_MuxIndex:		0x00000001
		//m_PinBaseIndex:	0x00000016
		//m_PinCntIndex:	0x00000001
		//m_ChlIndex:		0x00000003
		//m_SpecDefine1:	0x00000000	
		
		SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK; 							//Open Clock gate 	
		ADCx->CFG1 &= ~(ADC_CFG1_MODE_MASK); 						//ADC_Precision
		ADCx->CFG1 |= ADC_CFG1_MODE(ADC_InitStruct->ADC_Precision);
		ADCx->CFG1 &= ~ADC_CFG1_ADICLK_MASK;						//ADC Input Clock  = BusClock
		ADCx->CFG1 |=  ADC_CFG1_ADICLK(0); 
		ADCx->CFG1 &= ~ADC_CFG1_ADLSMP_MASK;						//No long sample time
		ADCx->CFG1 &= ~ADC_CFG1_ADIV_MASK;							//longest sample clock div
		ADCx->CFG1 |= ADC_CFG1_ADIV(3); 
		if(pADC_Map->m_PinCntIndex > 1)								//Single or Differential
		{
			ADCx->SC1[pADC_Map->m_SpecDefine1] |= ADC_SC1_DIFF_MASK; 
		}
		else
		{
			ADCx->SC1[pADC_Map->m_SpecDefine1] &= ~ADC_SC1_DIFF_MASK; 
		}		
		i = ADC_Cal(ADCx);											//Calieration
		//trigger source
		(ADC_TRIGGER_HW == ADC_InitStruct->ADC_TriggerSelect)?(ADCx->SC2 |= ADC_SC2_ADTRG_MASK):(ADCx->SC2 &= ~ADC_SC2_ADTRG_MASK);
		//config PinMux
		for(i=0;i<pADC_Map->m_PinCntIndex;i++)
		{
			PinMuxConfig(pADC_Map->m_PortIndex,pADC_Map->m_PinBaseIndex+i,pADC_Map->m_MuxIndex);
		}
	}

	