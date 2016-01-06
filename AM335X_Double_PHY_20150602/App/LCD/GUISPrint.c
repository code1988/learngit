/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: GUIsprint.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 131029
**��  ��  ��  ��: V0.1
**��          ��: ����grlib�ӿں���ʵ�ַ���Һ����ʾ��һЩ��������Ҫʵ�ֹ���Ϊprint������
                  ����Ӣ���뺺��ͬʱ��ʾ����ӡ��ͬ��ʽ�����ݵȡ����к�����������Χ��sContext
                  ԭ����ֱ������BSP����������ǵ�������sContext��ʽ���Լ��ֿ�Ƚ϶࣬������
                  ��Ӧ�ñ���
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
/* Private define-----------------------------------------------------------------------------*/
#define	BSPGUI_HFONT_MAX		((BSPGUI_XSIZE/BSPGUI_HFONT_X)*(BSPGUI_YSIZE/(BSPGUI_HFONT_Y+BSPGUI_HFONT_Y_A)))	// ������ʾ�İ������
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes-------------------------
---------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : BSP_GUIDispChars
* Description	: ����ʼλ�ÿ�ʼ��ʾ����ַ���
* Input		    : x0:X����
		          y0:y����
	              len:���ݳ���
                  *pData:����ָ��
* Output	    : TRUE/FALSE
* Note(s)	    : ���ﴫ�ݵ�������ASCII���,�����ֿ�����ָ��
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U BSP_GUIDispChars(INT16U x0,INT16U y0,INT16U len,INT8U *pData)
{
    
    while(len--)
    {
        if(*pData<0x80) //��ǣ�������grlib����20���� 
        {
            GrContextFontSet(&sContext, &g_sFontCm20); 
            GrStringDraw(&sContext, (const char *)pData,1, x0, y0+2,0);
            x0+=12;
            pData++;//�������ƫ��һ���ֽ�
        }
        else
        {
            GrContextFontSet(&sContext, &g_sFontCH24);
            GrStringDraw(&sContext,(const char *)pData,1,x0,y0,0);  
            x0+=24;
            pData+=2;//��������ƫ��2���ֽ�
            if(len)
                len--;
        }          
    }  
    return len;
}
/***********************************************************************************************
* Function	    : BSP_GUISprint
* Description	: ��ʾһ������(�е�����sprinf����,�������)
* Input		    : x0:x��ʼ����(0 ~ BSPGUI_XSIZE-1)
                     y0:y��ʼ����(0 ~ BSPGUI_YSIZE-1)
                      *p:���Ʋ���,����������ʾ��ʽ��,�е����Ʊ�׼�⺯��printf()�Ĳ����÷�
                            *p="u.vwx",�����¼��������
                                    ��һ����'u'�ַ��Ǳ�����ʾ��λ��,1~BSPGUI_SPRINT_DISP_NUM,������λ('12')Ҳ����һλ('6')
                                    �ڶ�����'.v'�ַ��Ǳ���С����λ��,���û�������������
                                    ��������'w'�ַ�����ʾ���Ϳ���
                                            'd':������ʮ����(���Ĳ���ʾ����,������ʾ'-'),�����ʾλ��10
                                            'u':�޷���ʮ����,�����ʾλ��10
                                            'p':��С����Ĵ�д��ʮ��������(��ʵ��BCD����ʾ),������ִ���9���ַ�,��ȫ����ʾ'-',С���㱣��
                                            'x':Сд��ʮ������
                                            'X':��д��ʮ������
                                            'f':������(û��)
                                    ���Ĳ���'x'�ַ�����һЩ���⹦��
                                            'Z':��ʾ��λ��0
                                            'L':ȥ����λ��0,������������
                                            ��:��λ��0����ʾ,����λ�ñ���
                                    ע:С����ռһ��λ��
                      *pData:����,����(INT8U *),����ʮ������,������ת����INT32,�ٴ���,���������ʾλ��Ϊ10
                      ����:
                            1 �޷���ʮ����
                                    INT8U data = 0x12;
                                    INT32U temp;
                                    temp = (INT32U)data;
                                    BSP_GUISprint(x0,y0,"2u",(INT8U *)&temp);
                                    ��ʾ:"18";����:2
                            2 �з���ʮ����
                                    INT8U data = -1;
                                    INT32S temp;
                                    temp = (INT32S)data;
                                    BSP_GUISprint(x0,y0,"3d",(INT8U *)&temp);
                                    ��ʾ:" -1";����:3
                            3 ʮ������
                                    INT16U data = 0x0123;
                                    BSP_GUISprint(x0,y0,"4x",(INT8U *)&data);
                                    ��ʾ:" 123";����:4
                                    BSP_GUISprint(x0,y0,"4xL",(INT8U *)&data);
                                    ��ʾ:"123";����:3
                                    BSP_GUISprint(x0,y0,"4xZ",(INT8U *)&data);
                                    ��ʾ:"0123";����:4
                            4 ��С�����ʮ������
                                    INT16U data = 0x0123;
                                    BSP_GUISprint(x0,y0,"3.1p",(INT8U *)&data);
                                    ��ʾ:" 12.3";����:5
                                    BSP_GUISprint(x0,y0,"1.3p",(INT8U *)&data);
                                    ��ʾ:"0.003";����:5
                                    BSP_GUISprint(x0,y0,"5.7p",(INT8U *)&data);
                                    ��ʾ:"0.0000123";����:9
                                    BSP_GUISprint(x0,y0,"2.4p",(INT8U *)&data);
                                    ��ʾ:"0.0023";����:6
* Output	    : ʵ����ʾ�ĳ���
* Note(s)	    : 
* Contributor	: 081205  wangyao	
***********************************************************************************************/
INT8U BSP_GUISprint(INT16U x0,INT16U y0,const tFont *pFont,INT8U *p,INT8U *pData)
{
    INT8U i,temp;
    INT8U dispBuff[BSPGUI_SPRINT_DISP_NUM];
    INT8U dispBitNum;							// ��ʾλ��
    INT8U dispFloat;							// С����λ��
    INT8U dispType;								// ��ʾ����
    INT8U dispPlace;							// λ�ÿ���
    INT8U dispError=0;							// 1:���������,������'p'��ʾ�г��ִ���9������
    INT8U placeObject=0;						// ��ʾλ��,�����з������ķ�����ʾ��
    INT32U data=0;
    
    // �õ�������ʾ����
    dispBitNum = 0;
    i = p[1]-'0';
    if(i<=9)
    {
        dispBitNum = (p[0] - '0')*10 + i;
        i = 2;
    }
    else
    {
        dispBitNum += p[0] - '0';
        i = 1;
    }
    if(dispBitNum > BSPGUI_SPRINT_DISP_NUM)
        dispBitNum = BSPGUI_SPRINT_DISP_NUM;
    if(dispBitNum == 0)
        dispBitNum = 1;
    // �õ���������С��λ��
    dispFloat = 0;
    if(p[i] == '.')
    {
        i++;
        dispFloat = p[i++] - '0';
        if((dispFloat > 9)||(dispFloat >= (BSPGUI_SPRINT_DISP_NUM-1)))
            return 0;
    }
    // С����ҲҪռһ��λ��
    if(dispFloat)
    {
        if(dispBitNum >= BSPGUI_SPRINT_DISP_NUM)	// С��ռһλ
            dispBitNum = BSPGUI_SPRINT_DISP_NUM - 1;
    }
    // �õ�������ʾ����
    dispType = p[i++];
    // �õ�λ�ÿ��Ʋ���(ȥ����λ0,���Ƶ�)
    dispPlace = p[i++];
    // ��֯���ݵ�����
    switch(dispType)
    {
        case 'd':								// �з���ʮ������
        {
            i = 4;
            if(dispBitNum > 10)					// ���ô���10λ,��Ϊ32λ����������ʾ10λʮ����ֵ
                dispBitNum=10;
            if(dispBitNum < BSPGUI_SPRINT_DISP_NUM)	// ��һ������λ��
                dispBitNum++;
            placeObject = 1;					// ��һλ���Ƿ���
            // ��������
            data = pData[3];
            data = data << 8;
            data += pData[2];
            data = data << 8;
            data += pData[1];
            data = data << 8;
            data += pData[0];
            if(pData[i-1] & 0x80)				// �Ƿ��Ǹ���
            {
                dispBuff[0] = '-';				// ��ʾ'-'
                data = ((-data) + 1);			// ���ݱ������
            }
            else								// ����
            {
                dispBuff[0] = ' ';				// ����ʾ'+'
            }
            // ����û��break��
        }
        case 'u':								// �޷���ʮ������
        {
			if(placeObject == 0)
			{
				// ��������
				data = pData[3];
				data = data << 8;
				data += pData[2];
				data = data << 8;
				data += pData[1];
				data = data << 8;
				data += pData[0];
				if(dispBitNum > 10)				// ���ô���10λ,��Ϊ32λ����������ʾ10λʮ����ֵ
					dispBitNum=10;
			}
			// �����ݷ���buff,�������λ����buff[0],��������,��ת����ASCII��
			for(i=dispBitNum;i>placeObject;i--)
			{
                dispBuff[i-1] = data % 10;
                dispBuff[i-1] = dispBuff[i-1] + '0';
                data = data / 10;					
			}
			break;
        }
        case 'p':	// ��С�����Сд��ʮ��������(��ʵ��BCD����ʾ)
                // ����û��break��
        case 'x':	// Сд��ʮ��������
                // ����û��break��
        case 'X':  // ��д��ʮ��������
        {
            INT8U hex='A';						// ���������Сд,��������Ĵ������ͳһ
            if(dispType == 'x')
                    hex = 'a';						// Сд
            if(dispBitNum & 0x01)				// �����ʾλ��Ϊ����,���ȵ����������λ����
            {
                // ��4λ
                temp = pData[dispBitNum>>1]&0x0f;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD�벻�ܳ��ִ���9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[0] = temp;
            }
            // �����ݷ���buff,�������λ����buff[0],��������,��ת����ASCII��
            for(i=(dispBitNum>>1);i>0;i--)		// ���ֽڴ���(2��λ)
            {
                // ��4λ
                temp = pData[i-1]>>4;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD�벻�ܳ��ִ���9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[dispBitNum-(i<<1)] = temp;
                // ��4λ
                temp = pData[i-1]&0x0f;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD�벻�ܳ��ִ���9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[dispBitNum-(i<<1)+1] = temp;
            }
            break;
        }
        case 'f':								// ������
        {
            break;
        }
        default:
            return 0;
    }
	// ���С����
	if(dispFloat)
	{
		if(dispFloat < dispBitNum)		// ��ʾλ������С�����������
		{
			// С��������ݶ�����һλ
			for(i=0;i<dispFloat;i++)
				dispBuff[dispBitNum-i] = dispBuff[dispBitNum-i-1];
			// ���С����
			dispBuff[dispBitNum-dispFloat] = '.';
			dispBitNum++;
		}
		else							// С��λ�����ڵ�����ʾλ�������
		{
			// С��������ݶ�����һλ
			for(i=0;i<dispBitNum;i++)
				dispBuff[dispFloat-i+1] = dispBuff[dispBitNum-i-1];
			// ��λ��'0'
			for(i=0;i<dispFloat-dispBitNum+2;i++)
				dispBuff[i] = '0';
			// ���С����
			dispBuff[1]='.';
			dispBitNum = dispFloat+2;
		}
	}
	// λ�ô���(��λ��0�Ƿ���ʾ,�Ƿ���Ҫ����)
	if(dispPlace == 'L')						// ����
	{
		// �����һ��λ����'-',��ô�ӵڶ���λ�ÿ�ʼ����
		if(dispBuff[0]=='-')
			placeObject = 1;
		else
			placeObject = 0;
		i=placeObject;
		temp = 0;
		// ������Ҫ�ƶ���λ��
		while(i<dispBitNum-1)
		{
			if((dispBuff[i]=='0')||(dispBuff[i]==' '))
				temp++;
			else if(dispBuff[i]=='.')			// ��λ����С����
			{
				// С����ǰһλд��'0'
				if(i&&temp)
				{
					temp--;
					dispBuff[i-1]='0';
					break;
				}
				else							// ���ܳ��������λ
					return 0;
			}
			else
				break;
			i++;
		}
		// �ƶ�
		if(temp)
		{
			for(i=placeObject;i<dispBitNum-temp;i++)
				dispBuff[i] = dispBuff[i+temp];
		}
		dispBitNum = dispBitNum - temp;
	}
	else if(dispPlace != 'Z')				// ȥ����λ����
	{
		// �����һ��λ����'-',��ô�ӵڶ���λ�ÿ�ʼ����
		if(dispBuff[0]=='-')
			placeObject = 1;
		else
			placeObject = 0;
		i=placeObject;
		// ���' '
		while(i<dispBitNum-1)
		{
			if(dispBuff[i]=='0')
				dispBuff[i] = ' ';
			else if(dispBuff[i]=='.')			// ��λ����С����
			{
				// С����ǰһλд��'0'
				if(i)
				{
					dispBuff[i-1]='0';
					break;
				}
				else							// ���ܳ��������λ
					return 0;
			}
			else
				break;
			i++;
		}
	}
	// ������
	if(dispError)
	{
		for(i=0;i<dispBitNum;i++)
		{
			// ���в���С������������'-'
			if(dispBuff[i] != '.')
				dispBuff[i] = '-';
		}
	}
	// ��ʾ
	GrContextFontSet(&sContext, pFont); //��ӡ20�����Сg_sFontCm20g_sFontCmss48b
    GrStringDraw(&sContext, (const char *)dispBuff,dispBitNum, x0, y0,0);
	return dispBitNum;
}
/***********************************************************************************************
* Function		: GUIPrintf
* Description	: ��ʾ�ַ�������,�÷�����ͬ��׼printf����,ֻ��ǰ�����x,y����
* Input			: x0:x��ʼ����(0 ~ BSPGUI_XSIZE-1)
				  y0:y��ʼ����(0 ~ BSPGUI_YSIZE-1)
				  *pStr,...:�÷�ͬ��׼printf����
* Output		: ʵ����ʾ�����ݳ���,0:����
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U GUIPrintf(INT16U x0,INT16U y0,const char *pStr,...)
{
	INT16U len;
	char Buff[40];
	va_list arg_ptr;
	va_start(arg_ptr,pStr);
	len = vsprintf(Buff,pStr,arg_ptr);
	va_end(arg_ptr);
	//assert_param(len<BSPGUI_HFONT_MAX);// �������,����Ĵ���Ƚ�����,���������
	return BSP_GUIDispChars(x0,y0,len,(INT8U *)Buff);
}
/***********************************************************************************************
* Function		: BSP_GUIFill
* Description	: LCD�������
* Input			: x0:x��ʼ����
				  y0:y��ʼ����
				  x1:x��������
				  y1:y��������
				  color:��Ҫ��������
* Output		: 
* Note(s)		: ����grlib��tRectangle�����ú�����
* Contributor	: 140221	wangyao
***********************************************************************************************/
void BSP_GUIFill(INT16U x0,INT16U y0,INT16U x1,INT16U y1,INT32U color)
{
    tRectangle sRect; 
    
    // Fill the top 24 rows of the screen with blue to create the banner.
    sRect.sXMin = x0;
    sRect.sYMin = y0;
    sRect.sXMax = x1;
    sRect.sYMax = y1;
    GrContextForegroundSet(&sContext, color);//ClrLightSteelBlue);//
    GrRectFill(&sContext, &sRect);
}
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/