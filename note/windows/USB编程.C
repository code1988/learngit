USB有5种标准描述符：设备描述符、配置描述符、字符描述符、接口描述符、端点描述符	//这些描述符存储在USB设备中，用于描述一个USB设备的全部属性
																			 //USB主机通过一系列命令来要求设备发送这些信息
USB设备描述符之间的关系：
					分层关系：设备描述符->配置描述符1->接口描述符1->端点描述符1	//USB主机获取描述符时，必须按照这个顺序获取
											...			...			...
										配置描述符n	接口描述符n	 端点描述符n

设备描述符
typedef struct _USB_DEVICE_DESCRIPTOR {
	UCHAR  bLength;						//设备描述符结构体的大小
	UCHAR  bDescriptorType;				//描述符类型编号，必须是USB_DEVICE_DESCRIPTOR_TYPE（0x01）
	USHORT bcdUSB;						//USB版本号
	UCHAR  bDeviceClass;				//USB分配的设备类代码
	UCHAR  bDeviceSubClass;				//USB分配的子类
	UCHAR  bDeviceProtocol;				//USB分配的设备协议代码
	UCHAR  bMaxPacketSize0;				//端点0的最大包大小，该值必须是8/16/32/64
	USHORT idVendor;					//厂商编号
	USHORT idProduct;					//产品编号
	USHORT bcdDevice;					//设备出厂编号
	UCHAR  iManufacturer;				//描述厂商字符串的索引
	UCHAR  iProduct;					//描述产品字符串的索引
	UCHAR  iSerialNumber;				//描述设备序列号字符串的索引
	UCHAR  bNumConfigurations;			//可能的配置数量
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

配置描述符
typedef struct _USB_CONFIGURATION_DESCRIPTOR {
  UCHAR  bLength;						//配置描述符结构体的大小
  UCHAR  bDescriptorType;				//描述符类型编号，必须是USB_CONFIGURATION_DESCRIPTOR_TYPE(0x02)
  USHORT wTotalLength;					//返回的配置信息（包含接口、端点、类、设备序列号等）的总长
  UCHAR  bNumInterfaces;				//该配置下支持的接口数量
  UCHAR  bConfigurationValue;			//选择配置时需要的参数
  UCHAR  iConfiguration;				//描述该配置的字符串索引值
  UCHAR  bmAttributes;					//供电模式的选择
  UCHAR  MaxPower;						//设备从总线提取的最大电流，该选项只有当bmAttributes=7时有效
} USB_CONFIGURATION_DESCRIPTOR, *PUSB_CONFIGURATION_DESCRIPTOR;
