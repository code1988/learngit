CRC校验的C语言实现方法

以CRC-16作为多项式（CRC-16对应的16进制数为0x18005）.
任意长度数据流CRC校验码生成过程如下：
将数据流分成若干个8bit字符，用一个16bit的寄存器用来存放CRC校验值，且设定初值为0x0000；
将数据流的第一个8bit与16bit的CRC寄存器的高字节相异或，并将结果存入CRC寄存器高字节；
CRC寄存器左移一位，右边最低一位补上0，同时检查移出的那位，若那位为0，则继续按上述过程左移；若那位为1，则将CRC寄存器中的值与多项式（CRC-16）相异或，结果存入CRC寄存器；
继续左移并重复上述处理方法，直到8bit数据处理完毕，则此时CRC寄存器中的值就是第一个8bit数据对应的CRC校验码；
然后将此时CRC寄存器中的值作为初值，重复上述步骤来处理下一个8bit数据流，直到将所有的8bit数据流都处理完毕，此时CRC寄存器中的值就是整个数据流对应的CRC校验值。

注意点：
1.	CRC-16对应的16进制数0x18005我们只截取的了0x8005四位放入程式代码，最高的第17位其实是跟CRC寄存器每次左移后移出的那位做了异或处理，只不过是幕后行为，程式里不显。
2.	a^b^b=a^b^b^b^b=a^b1^…b2n ，异或用法规律

代码示例：

Unsigned short CRC_dsp(unsigned short reg,unsigned char data_crc)	//reg为CRC寄存器，data_crc为将要处理 的8bit数据流
{
	Unsigned short msb;					//msb最高位用于存放CRC寄存器左移的一位
	Unsigned short data;
	Unsigned short gx = 0x8005, i = 0;	//i为左移次数，gx为CRC-16对应的多项式
	
	
	data = (unsigned short )data_crc;	//强制转换数据类型，8bit变16bit 
	data <<= 8;							//数据流左移8位
	reg <<= data;						//将数据流跟CRC校验值相异或，CRC校验值初值可设0x0000
	
	do
	{
		msb = reg&0x8000;				//取出CRC寄存器最高位
		reg <<= 1;						//CRC寄存器左移一位
		if(msb==0x8000)					//判断移出的那位是0 or1
		{
			reg ^=gx;					//若是1，将CRC寄存器中的值与多项式（CRC-16）相异或，结果存入CRC寄存器
		}
		i++;							//控制左移循环8次
	}
	while(i<8);
	return (reg);
}
