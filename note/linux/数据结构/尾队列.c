1. 类似于链表的 struct list_head，尾队列也抽象出一个模块用来组建完整的队列元素，定义如下：
    #define TAILQ_ENTRY(elem) \
        struct {                \
            struct elem *tqe_next;\
            struct elem **tqe_prev;\
        }
    tqe_next 是指向下一个元素的指针,tqe_prev 是指向前一个元素的tqe_next的地址的指针
    所以 *(tqe_prev) 就是指向本元素的指针

    完整的尾队列元素定义为（假设数据部分是一个int）：
        struct int_node {
            int num;
            TAILQ_ENTRY(int_node) entry;
        }
        
2. 类似于链表，尾队列将队列头抽象为一个单独的数据结构，不存在数据部分，定义如下：
    #define TAILQ_HEAD(head,elem) \
        struct name {               \
            struct elem *tqh_first;\
            struct elem **tqh_last;\
        }
    tqh_first 是指向队列中第一个元素的指针，tqh_last 是指向最后一个元素的tqe_next的地址的指针
    所以 *(tqh_last) 固定为NULL

    队列头使用前需要初始化：
        #define TAILQ_INIT(head) do{    \
            (head)->tqh_first = NULL; \
            (head)->tqh_last = &(head)->tqh_first; \
        }while(0)

