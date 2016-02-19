											数据包结构
/*****************************************************************************************************************
struct pbuf
{
	struct pbuf *next;	// 指向下一个pbuf结构
	void *payload		// 数据指针，指向该pbuf所记录的数据区域
	u16_t tot_len;		// 当前pbuf和后续所有pbuf中包含的数据总长
	u16_t len;			// 当前pbuf单元记录的数据长度
	u8_t type;			// 当前pbuf单元的类型
	u8_t flags;			// 状态位，未用到
	u16_t ref;			// 指向该pbuf的指针数，即该pbuf被引用的次数
}
next:一个数据包需要多个pbuf结构才能完全描述	，这些pbuf结构就组成了一张链表，通过next实现
payload:数据区域可以是紧跟在pbuf结构后的RAM空间，也可以是ROM中的空间，由type字段决定
type:表示pbuf的类型，定义如下
		typedef enum{
			PBUF_RAM,	// pbuf与其描述的数据处于同一连续内存堆中
			PBUF_ROM,	// pbuf描述的数据在ROM中
			PBUF_REF,	// pbuf描述的数据在RAM中，但位置与pbuf结构所处位置无关
			PBUF_POOL	// pbuf与其描述的数据处于同一内存池中
		}pbuf_type;
ref:新分配pbuf时就会置1，数据包释放时，通过该值判断当前pbuf节点是否可以被释放
*****************************************************************************************************************/

											数据包申请、释放
/*****************************************************************************************************************
数据包申请函数有二个重要参数，一个是想申请的数据包pbuf类型，另一个是该数据包是在协议栈哪一层被申请的。
申请函数根据层次的不同，会在pbuf数据区前预留出相应的offset值，层次定义如下
																		typedef enum{
																			PBUF_TRANSTPORT,	// 传输层
																			PBUF_IP,			// 网络层
																			PBUF_LINK,			// 链路层
																			PBUF_RAW			// 原始层，不预留任何空间
																		}pbuf_layer
struct pbuf *pbuf_alloc(pbuf_layer layer,u16_t length,pbuf_type type)
{
	struct pbuf *p,*q,*r;
	u16_t offset;		// 预留首部空间的长度
	s32_t rem_len;		// 还要申请的数据长度
	
	// 根据层次的不同，计算预留长度
    offset = 0;		
    switch (layer) 
    {
    	case PBUF_TRANSPORT:
        // 传输层，预留出TCP首部长度
        offset += PBUF_TRANSPORT_HLEN;
        case PBUF_IP:
        // 网络层或以上各层，预留出IP首部长度
        offset += PBUF_IP_HLEN;
        case PBUF_LINK:
        // 链路层或以上各层，预留出链路层首部长度
        offset += PBUF_LINK_HLEN;
        break;
        case PBUF_RAW:
		// 如果是原始层，则不预留任何空间(常用于数据包接收)
        break;
        default:
        return NULL;
    }
    
    // 根据pbuf类型，来分配对应的内存空间
    switch (type)
    {
    	case PBUF_POOL:		// 这种类型的pbuf，意味着可能需要分配几个POOL
	        // 分配一个POOL作为pbuf链表头
	        p = (struct pbuf *)memp_malloc(MEMP_PBUF_POOL);
	        if (p == NULL) 
	       	{
	          	PBUF_POOL_IS_EMPTY();
	          	return NULL;
	        }
	
			// 分配成功之后，初始化pbuf各字段
	        p->type = type;
	        p->next = NULL;
	        p->payload = LWIP_MEM_ALIGN((void *)((u8_t *)p + (SIZEOF_STRUCT_PBUF + offset)));	//作为链表头的第一个pbuf，payload指向的数据起始区域，需要预留出首部空间
	        p->tot_len = length;
        	p->len = LWIP_MIN(length, PBUF_POOL_BUFSIZE_ALIGNED - LWIP_MEM_ALIGN_SIZE(offset));	//作为链表头的第一个pbuf，len的取值也需要考虑到首部空间
        	
        	// 检查已分配的POOL是否满足长度要求，不满足则继续分配
        	rem_len = length - p->len;
        	r = p;
        	while (rem_len > 0) 
        	{
        		// 分配一个新的POOL
	            q = (struct pbuf *)memp_malloc(MEMP_PBUF_POOL);
	            if (q == NULL) 
	            {
	                PBUF_POOL_IS_EMPTY();
	                pbuf_free(p);
	                return NULL;
	            }
	            
	            // 分配成功之后，初始化pbuf各字段,并将新的pbuf链接进链表
	            q->type = type;
	            q->flags = 0;
	            q->next = NULL;
	            r->next = q;
	            q->tot_len = (u16_t)rem_len;
	            q->len = LWIP_MIN((u16_t)rem_len, PBUF_POOL_BUFSIZE_ALIGNED);	// 除了作为链表头的pbuf，其余pbuf的数据区域都不需要预留首部，所以payload直接指向pbuf结构之后，len最大可取POOL尺寸
            	q->payload = (void *)((u8_t *)q + SIZEOF_STRUCT_PBUF);
            	q->ref = 1;

	            // 更新还要申请的长度
	            rem_len -= q->len;
	            
	            r = q;
        	}
        	break;
        case PBUF_RAM:      // 这种类型的pbuf直接在内存堆中申请
	        p = (struct pbuf*)mem_malloc(LWIP_MEM_ALIGN_SIZE(SIZEOF_STRUCT_PBUF + offset) + LWIP_MEM_ALIGN_SIZE(length));
	        if (p == NULL) 
	        {
	          return NULL;
	        }
	
	        // 分配成功之后，初始化pbuf各字段
	        p->payload = LWIP_MEM_ALIGN((void *)((u8_t *)p + SIZEOF_STRUCT_PBUF + offset));
	        p->len = p->tot_len = length;
	        p->next = NULL;
	        p->type = type;
	    	break;
	    case PBUF_ROM:      // 对于PBUF_ROM和PBUF_REF类型的pbuf，只分配pbuf结构
	    case PBUF_REF:
	    	p = (struct pbuf *)memp_malloc(MEMP_PBUF);
        	if (p == NULL)
        		return NULL;
        		
        	// 分配成功之后，初始化pbuf各字段,注意这两种类型的pbuf在初始化时payload字段首先置空
	        p->payload = NULL;
	        p->len = p->tot_len = length;
	        p->next = NULL;
	        p->type = type;
	        break;
	    default:
	    	return NULL;
    }
    
    // 到这里，pbuf申请成功，设置剩余字段，返回pbuf指针
  	p->ref = 1;
  	p->flags = 0;
  	
  	return p;
}	

数据包的释放前提是pbuf的ref字段为0，并且能被删除的pbuf必然是某个pbuf链表的首节点，一旦跳过首节点，直接释放某个中间节点，会导致严重错误，调用数据包释放函数时必须格外注意这点	
u8_t pbuf_free(struct pbuf *p)
{
	u16_t type;
	struct pbuf *q;
  	u8_t count;

  	if (p == NULL) 
  		return 0;
  		
  	count = 0;
  	while (p != NULL) 
  	{
  		u16_t ref;
  		
  		ref = --(p->ref);   // 该pbuf引用次数减1
  		// 判断该pbuf的引用次数是否为0，为0则删除该pbuf，不为0则不删除
        if (ref == 0) 
        {
        	q = p->next;
        	
        	// 判断要删除的pbuf类型，调用相应的内存管理函数删除
        	type = p->type;
        	if (type == PBUF_POOL) 
            {
            	memp_free(MEMP_PBUF_POOL, p);
            } 
            else if (type == PBUF_ROM || type == PBUF_REF) 
            {
              	memp_free(MEMP_PBUF, p);
            } 
            else 
            {
              	mem_free(p);
            }
            
            count++;	// 更新删除的pbuf数量
            
            p = q;
        }
        else
        	p = NULL;
  	}
  	
  	// 返回成功删除的pbuf个数
  	return count;
}
*****************************************************************************************************************/

									其他数据包操作函数
/*****************************************************************************************************************		
1. 在pbuf链表尾部释放一定空间，从而将数据包截断为某个长度值
void pbuf_realloc(struct pbuf *p, u16_t new_len)
{
	struct pbuf *q;
    u16_t rem_len; 
    s32_t grow;
	
	// 确保截断后的数据总长小于原数据总长
    if (new_len >= p->tot_len) 
        return;
	
	grow = new_len - p->tot_len;	// 需要截掉的长度，必然是个负值
	
	// 遍历pbuf链表，找到最后一个有效的pbuf节点
	rem_len = new_len;
	q = p;
	while (rem_len > q->len) 
	{
		rem_len -= q->len;
		q->tot_len += (u16_t)grow;
		q = q->next;
	}
	
	// 截断最后一个有效pbuf节点中的无效数据，注意！最后一个有效pbuf节点，只有是PBUF_RAM类型时，才需要释放无效数据的内存空间
	if ((q->type == PBUF_RAM) && (rem_len != q->len)) 
	{
		q = (struct pbuf *)mem_trim(q, (u16_t)((u8_t *)q->payload - (u8_t *)q) + rem_len);
	}
	
	// 更新最后一个有效pbuf节点的字段
	q->len = rem_len;
    q->tot_len = q->len;
    
    // 释放pbuf链表上多余的作废pbuf节点，其实就是针对PBUF_POOL类型
    if (q->next != NULL) 
    {
        pbuf_free(q->next);
    }
    q->next = NULL;
}

2. 调整pbuf中的payload指针，通常用于操作协议首部空间
u8_t pbuf_header(struct pbuf *p, s16_t header_size_increment)
{
	u16_t type;
  	void *payload;
  	u16_t increment_magnitude;
  	
  	// 入参合法性检测
  	if ((header_size_increment == 0) || (p == NULL))
    	return 0;
	
	// 获取指针偏移值，非负的，注意入参header_size_increment为正表示指针前移，为负表示指针后移
	if (header_size_increment < 0)
    	increment_magnitude = -header_size_increment;
    else
    	increment_magnitude = header_size_increment;
    
    // 根据pbuf类型不同，采用相应的偏移策略	
    type = p->type;
    payload = p->payload;
    if (type == PBUF_RAM || type == PBUF_POOL) 		// 跟数据区域连续的pbuf类型，允许前后偏移
    {
    	// 设置新的payload指针
    	p->payload = (u8_t *)p->payload - header_size_increment;
    	
    	// 检测新的payload指针是否越界
    	if ((u8_t *)p->payload < (u8_t *)p + SIZEOF_STRUCT_PBUF) 
    	{
    		p->payload = payload;
    		return 1;
    	}
    }
    else if (type == PBUF_REF || type == PBUF_ROM) 	// 跟数据区域分离的pbuf类型，只允许往后偏移
    {
    	if ((header_size_increment < 0) && (increment_magnitude <= p->len)) 
          	p->payload = (u8_t *)p->payload - header_size_increment;
        else 
        	return 1;
    }
    else
    	return 1;
    
    // 更新pbuf字段	
    p->len += header_size_increment;
    p->tot_len += header_size_increment;
    
    return 0;
}
							
*****************************************************************************************************************/

									