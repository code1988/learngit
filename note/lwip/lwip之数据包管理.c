											���ݰ��ṹ
/*****************************************************************************************************************
struct pbuf
{
	struct pbuf *next;	// ָ����һ��pbuf�ṹ
	void *payload		// ����ָ�룬ָ���pbuf����¼����������
	u16_t tot_len;		// ��ǰpbuf�ͺ�������pbuf�а����������ܳ�
	u16_t len;			// ��ǰpbuf��Ԫ��¼�����ݳ���
	u8_t type;			// ��ǰpbuf��Ԫ������
	u8_t flags;			// ״̬λ��δ�õ�
	u16_t ref;			// ָ���pbuf��ָ����������pbuf�����õĴ���
}
next:һ�����ݰ���Ҫ���pbuf�ṹ������ȫ����	����Щpbuf�ṹ�������һ������ͨ��nextʵ��
payload:������������ǽ�����pbuf�ṹ���RAM�ռ䣬Ҳ������ROM�еĿռ䣬��type�ֶξ���
type:��ʾpbuf�����ͣ���������
		typedef enum{
			PBUF_RAM,	// pbuf�������������ݴ���ͬһ�����ڴ����
			PBUF_ROM,	// pbuf������������ROM��
			PBUF_REF,	// pbuf������������RAM�У���λ����pbuf�ṹ����λ���޹�
			PBUF_POOL	// pbuf�������������ݴ���ͬһ�ڴ����
		}pbuf_type;
ref:�·���pbufʱ�ͻ���1�����ݰ��ͷ�ʱ��ͨ����ֵ�жϵ�ǰpbuf�ڵ��Ƿ���Ա��ͷ�
*****************************************************************************************************************/

											���ݰ����롢�ͷ�
/*****************************************************************************************************************
���ݰ����뺯���ж�����Ҫ������һ��������������ݰ�pbuf���ͣ���һ���Ǹ����ݰ�����Э��ջ��һ�㱻����ġ�
���뺯�����ݲ�εĲ�ͬ������pbuf������ǰԤ������Ӧ��offsetֵ����ζ�������
																		typedef enum{
																			PBUF_TRANSTPORT,	// �����
																			PBUF_IP,			// �����
																			PBUF_LINK,			// ��·��
																			PBUF_RAW			// ԭʼ�㣬��Ԥ���κοռ�
																		}pbuf_layer
struct pbuf *pbuf_alloc(pbuf_layer layer,u16_t length,pbuf_type type)
{
	struct pbuf *p,*q,*r;
	u16_t offset;		// Ԥ���ײ��ռ�ĳ���
	s32_t rem_len;		// ��Ҫ��������ݳ���
	
	// ���ݲ�εĲ�ͬ������Ԥ������
    offset = 0;		
    switch (layer) 
    {
    	case PBUF_TRANSPORT:
        // ����㣬Ԥ����TCP�ײ�����
        offset += PBUF_TRANSPORT_HLEN;
        case PBUF_IP:
        // ���������ϸ��㣬Ԥ����IP�ײ�����
        offset += PBUF_IP_HLEN;
        case PBUF_LINK:
        // ��·������ϸ��㣬Ԥ������·���ײ�����
        offset += PBUF_LINK_HLEN;
        break;
        case PBUF_RAW:
		// �����ԭʼ�㣬��Ԥ���κοռ�(���������ݰ�����)
        break;
        default:
        return NULL;
    }
    
    // ����pbuf���ͣ��������Ӧ���ڴ�ռ�
    switch (type)
    {
    	case PBUF_POOL:		// �������͵�pbuf����ζ�ſ�����Ҫ���伸��POOL
	        // ����һ��POOL��Ϊpbuf����ͷ
	        p = (struct pbuf *)memp_malloc(MEMP_PBUF_POOL);
	        if (p == NULL) 
	       	{
	          	PBUF_POOL_IS_EMPTY();
	          	return NULL;
	        }
	
			// ����ɹ�֮�󣬳�ʼ��pbuf���ֶ�
	        p->type = type;
	        p->next = NULL;
	        p->payload = LWIP_MEM_ALIGN((void *)((u8_t *)p + (SIZEOF_STRUCT_PBUF + offset)));	//��Ϊ����ͷ�ĵ�һ��pbuf��payloadָ���������ʼ������ҪԤ�����ײ��ռ�
	        p->tot_len = length;
        	p->len = LWIP_MIN(length, PBUF_POOL_BUFSIZE_ALIGNED - LWIP_MEM_ALIGN_SIZE(offset));	//��Ϊ����ͷ�ĵ�һ��pbuf��len��ȡֵҲ��Ҫ���ǵ��ײ��ռ�
        	
        	// ����ѷ����POOL�Ƿ����㳤��Ҫ�󣬲��������������
        	rem_len = length - p->len;
        	r = p;
        	while (rem_len > 0) 
        	{
        		// ����һ���µ�POOL
	            q = (struct pbuf *)memp_malloc(MEMP_PBUF_POOL);
	            if (q == NULL) 
	            {
	                PBUF_POOL_IS_EMPTY();
	                pbuf_free(p);
	                return NULL;
	            }
	            
	            // ����ɹ�֮�󣬳�ʼ��pbuf���ֶ�,�����µ�pbuf���ӽ�����
	            q->type = type;
	            q->flags = 0;
	            q->next = NULL;
	            r->next = q;
	            q->tot_len = (u16_t)rem_len;
	            q->len = LWIP_MIN((u16_t)rem_len, PBUF_POOL_BUFSIZE_ALIGNED);	// ������Ϊ����ͷ��pbuf������pbuf���������򶼲���ҪԤ���ײ�������payloadֱ��ָ��pbuf�ṹ֮��len����ȡPOOL�ߴ�
            	q->payload = (void *)((u8_t *)q + SIZEOF_STRUCT_PBUF);
            	q->ref = 1;

	            // ���»�Ҫ����ĳ���
	            rem_len -= q->len;
	            
	            r = q;
        	}
        	break;
        case PBUF_RAM:      // �������͵�pbufֱ�����ڴ��������
	        p = (struct pbuf*)mem_malloc(LWIP_MEM_ALIGN_SIZE(SIZEOF_STRUCT_PBUF + offset) + LWIP_MEM_ALIGN_SIZE(length));
	        if (p == NULL) 
	        {
	          return NULL;
	        }
	
	        // ����ɹ�֮�󣬳�ʼ��pbuf���ֶ�
	        p->payload = LWIP_MEM_ALIGN((void *)((u8_t *)p + SIZEOF_STRUCT_PBUF + offset));
	        p->len = p->tot_len = length;
	        p->next = NULL;
	        p->type = type;
	    	break;
	    case PBUF_ROM:      // ����PBUF_ROM��PBUF_REF���͵�pbuf��ֻ����pbuf�ṹ
	    case PBUF_REF:
	    	p = (struct pbuf *)memp_malloc(MEMP_PBUF);
        	if (p == NULL)
        		return NULL;
        		
        	// ����ɹ�֮�󣬳�ʼ��pbuf���ֶ�,ע�����������͵�pbuf�ڳ�ʼ��ʱpayload�ֶ������ÿ�
	        p->payload = NULL;
	        p->len = p->tot_len = length;
	        p->next = NULL;
	        p->type = type;
	        break;
	    default:
	    	return NULL;
    }
    
    // �����pbuf����ɹ�������ʣ���ֶΣ�����pbufָ��
  	p->ref = 1;
  	p->flags = 0;
  	
  	return p;
}	

���ݰ����ͷ�ǰ����pbuf��ref�ֶ�Ϊ0�������ܱ�ɾ����pbuf��Ȼ��ĳ��pbuf������׽ڵ㣬һ�������׽ڵ㣬ֱ���ͷ�ĳ���м�ڵ㣬�ᵼ�����ش��󣬵������ݰ��ͷź���ʱ�������ע�����	
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
  		
  		ref = --(p->ref);   // ��pbuf���ô�����1
  		// �жϸ�pbuf�����ô����Ƿ�Ϊ0��Ϊ0��ɾ����pbuf����Ϊ0��ɾ��
        if (ref == 0) 
        {
        	q = p->next;
        	
        	// �ж�Ҫɾ����pbuf���ͣ�������Ӧ���ڴ������ɾ��
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
            
            count++;	// ����ɾ����pbuf����
            
            p = q;
        }
        else
        	p = NULL;
  	}
  	
  	// ���سɹ�ɾ����pbuf����
  	return count;
}
*****************************************************************************************************************/

									�������ݰ���������
/*****************************************************************************************************************		
1. ��pbuf����β���ͷ�һ���ռ䣬�Ӷ������ݰ��ض�Ϊĳ������ֵ
void pbuf_realloc(struct pbuf *p, u16_t new_len)
{
	struct pbuf *q;
    u16_t rem_len; 
    s32_t grow;
	
	// ȷ���ضϺ�������ܳ�С��ԭ�����ܳ�
    if (new_len >= p->tot_len) 
        return;
	
	grow = new_len - p->tot_len;	// ��Ҫ�ص��ĳ��ȣ���Ȼ�Ǹ���ֵ
	
	// ����pbuf�����ҵ����һ����Ч��pbuf�ڵ�
	rem_len = new_len;
	q = p;
	while (rem_len > q->len) 
	{
		rem_len -= q->len;
		q->tot_len += (u16_t)grow;
		q = q->next;
	}
	
	// �ض����һ����Чpbuf�ڵ��е���Ч���ݣ�ע�⣡���һ����Чpbuf�ڵ㣬ֻ����PBUF_RAM����ʱ������Ҫ�ͷ���Ч���ݵ��ڴ�ռ�
	if ((q->type == PBUF_RAM) && (rem_len != q->len)) 
	{
		q = (struct pbuf *)mem_trim(q, (u16_t)((u8_t *)q->payload - (u8_t *)q) + rem_len);
	}
	
	// �������һ����Чpbuf�ڵ���ֶ�
	q->len = rem_len;
    q->tot_len = q->len;
    
    // �ͷ�pbuf�����϶��������pbuf�ڵ㣬��ʵ�������PBUF_POOL����
    if (q->next != NULL) 
    {
        pbuf_free(q->next);
    }
    q->next = NULL;
}

2. ����pbuf�е�payloadָ�룬ͨ�����ڲ���Э���ײ��ռ�
u8_t pbuf_header(struct pbuf *p, s16_t header_size_increment)
{
	u16_t type;
  	void *payload;
  	u16_t increment_magnitude;
  	
  	// ��κϷ��Լ��
  	if ((header_size_increment == 0) || (p == NULL))
    	return 0;
	
	// ��ȡָ��ƫ��ֵ���Ǹ��ģ�ע�����header_size_incrementΪ����ʾָ��ǰ�ƣ�Ϊ����ʾָ�����
	if (header_size_increment < 0)
    	increment_magnitude = -header_size_increment;
    else
    	increment_magnitude = header_size_increment;
    
    // ����pbuf���Ͳ�ͬ��������Ӧ��ƫ�Ʋ���	
    type = p->type;
    payload = p->payload;
    if (type == PBUF_RAM || type == PBUF_POOL) 		// ����������������pbuf���ͣ�����ǰ��ƫ��
    {
    	// �����µ�payloadָ��
    	p->payload = (u8_t *)p->payload - header_size_increment;
    	
    	// ����µ�payloadָ���Ƿ�Խ��
    	if ((u8_t *)p->payload < (u8_t *)p + SIZEOF_STRUCT_PBUF) 
    	{
    		p->payload = payload;
    		return 1;
    	}
    }
    else if (type == PBUF_REF || type == PBUF_ROM) 	// ��������������pbuf���ͣ�ֻ��������ƫ��
    {
    	if ((header_size_increment < 0) && (increment_magnitude <= p->len)) 
          	p->payload = (u8_t *)p->payload - header_size_increment;
        else 
        	return 1;
    }
    else
    	return 1;
    
    // ����pbuf�ֶ�	
    p->len += header_size_increment;
    p->tot_len += header_size_increment;
    
    return 0;
}
							
*****************************************************************************************************************/

									