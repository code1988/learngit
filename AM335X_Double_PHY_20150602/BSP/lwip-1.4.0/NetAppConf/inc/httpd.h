#ifndef _HTTPD_H
#define _HTTPD_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "netconf.h" 
#define HTTP_DOC_TYPE     "<!DOCTYPE HTML PUBLIC \"-//W3C/DTD HTML 4.0 Transitional//EN\">\r\n"   
/* header type detected
 * available to callback via call to httpread_reply_code_get()
 */
typedef enum {
  HTTPREAD_HDR_TYPE_UNKNOWN = 0,      /* none of the following */
  HTTPREAD_HDR_TYPE_REPLY = 1,        /* hdr begins w/ HTTP/ */
  HTTPREAD_HDR_TYPE_GET = 2,          /* hdr begins with GET<sp> */
  HTTPREAD_HDR_TYPE_HEAD = 3,         /* hdr begins with HEAD<sp> */
  HTTPREAD_HDR_TYPE_POST = 4,         /* hdr begins with POST<sp> */
  HTTPREAD_HDR_TYPE_PUT = 5,          /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_DELETE = 6,       /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_TRACE = 7,        /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_CONNECT = 8,      /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_NOTIFY = 9,       /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_M_SEARCH = 10,    /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_M_POST = 11,      /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_SUBSCRIBE = 12,   /* hdr begins with ... */
  HTTPREAD_HDR_TYPE_UNSUBSCRIBE = 13, /* hdr begins with ... */

  HTTPREAD_N_HDR_TYPES    /* keep last */
}HTTP_HDR_TYPE;
typedef enum {
  HTTP_OK = 200,
  HTTP_BAD_REQUEST = 400,
  UPNP_INVALID_ACTION = 401,
  UPNP_INVALID_ARGS = 402,
  HTTP_NOT_FOUND = 404,
  HTTP_PRECONDITION_FAILED = 412,
  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_UNIMPLEMENTED = 501,
  UPNP_ACTION_FAILED = 501,
  UPNP_ARG_VALUE_INVALID = 600,
  UPNP_ARG_VALUE_OUT_OF_RANGE = 601,
  UPNP_OUT_OF_MEMORY = 603
}HTTP_REPLY_CODE;


#define VAR_OK              0
#define VAR_FAIL            -1
#define VAR_BAD             -2
#define VAR_NOTFOUND        -3
#define VAR_FLAG_ERROR      1
#define VAR_FLAG_BR         2
#define VAR_FLAG_COLOR      4

typedef struct _variant_var{
  const char * name;
  const char * hdr_name;
  union{
    void  * void_ptr_var;    
    void  (*func_var)(void);
    bool  bool_var;
    BYTE  byte_var;
    WORD  short_var;
    int   int_var;
    DWORD ulong_var;
    char  char_var;
    char  * string_var;
    char  * filename_var;
    struct ip_addr ip_var;
  }value;
  enum{
    BOOL_VAR,
    BYTE_VAR,
    SHORT_VAR,
    INT_VAR,
    ULONG_VAR,
    CHAR_VAR,
    STRING_VAR,
    FUNC_VAR,
    VOID_PTR_VAR,
    FILE_NAME_VAR,
    IP_VAR,
    TAG_VAR,
    NULL_VAR
  }type;
  INT8U   max_var_len;
  INT8U   flags;
  INT8U   size;
  enum{
    VAR_PALE_RED = 0x0FDDDCE,
    VAR_PALE_ORANGE = 0x0FFD990,
    VAR_PALE_YELLOW = 0x0FFF799,
    VAR_PALE_AVOCADO = 0x0C4DF9B,
    VAR_PALE_GREEN = 0x0B3FFB2,
    VAR_PALE_CYAN = 0x7FCBAE,
    VAR_PALE_BLUE = 0x081D3EB,
    VAR_PALE_CERULEAN = 0x0ADCDEC,
    VAR_PALE_PURPLE = 0x0D3B6D7,
    VAR_PALE_MAGENTA = 0x0FCDEE0,
    VAR_DULL_RED = 0x0CC6666,
    VAR_DULL_ORANGE = 0x0FF9900,
    VAR_DULL_YELLOW = 0x0D3BD2A,
    VAR_DULL_AVOCADO = 0x08FB73E,
    VAR_DULL_GREEN = 0x06DB286,
    VAR_DULL_CYAN = 0x04F9BAA,
    VAR_DULL_BLUE = 0x06D9AB4,
    VAR_DULL_CERULEAN = 0x06F85C2,
    VAR_DULL_PURPLE = 0x0897AB9,
    VAR_DULL_MAGENTA = 0x0B7798E,  
    VAR_RED = 0x0ED1C24,
    VAR_ORANGE = 0x0FF6600,
    VAR_YELLOW = 0x0FFFF00,
    VAR_AVOCADO = 0x09ACA3C,
    VAR_GREEN = 0x000FF00,
    VAR_CYAN = 0x000FFFF,
    VAR_BLUE = 0x00000FF,
    VAR_CERULEAN = 0x04F57A6,
    VAR_PURPLE = 0x0800080,
    VAR_MAGENTA = 0x0FF00FF,
    VAR_WHILE = 0x0FFFFFF,
    VAR_VERY_PALE_GRAY = 0x0F2F2F2,
    VAR_PALE_GRAY = 0x0E5E5E5,
    VAR_VERY_LIGHT_GREY = 0x0D9D9D9,
    VAR_LIGHT_GREY = 0x0CCCCCC,
    VAR_GRAY = 0x0B3B3B3,
    VAR_MEDIUM_GRAY = 0x0999999,
    VAR_DARK_GREY = 0x0808080,
    VAR_VERY_DARK_GREY = 0x0666666,
    VAR_BLACK = 0
  }color;   
}VARIANT;

#ifdef HTTPD_DEBUG
#define HTTPD_ASSERT_VARIANT(var, char_var) do { if(HttpdFindVariant(var,char_var)==NULL) \
  printf("Assertion variant \"%s\" failed at line %d in %s\n", char_var, __LINE__, __FILE__); } while(0)
#define HTTPD_ASSERT_VARIANTS(vars) do{ VARIANT *var; for(var = vars; var->type != NULL_VAR; ++var){\
  if((var->type == STRING_VAR || var->type == FUNC_VAR || var->type == VOID_PTR_VAR || var->type == FILE_NAME_VAR) &&\
    var->void_ptr_var == NULL) \
      printf("Assertion variant \"%s \" failed at line %d in %s\n", var->name, __LINE__, __FILE__); } while(0)
#else
#define HTTPD_ASSERT_VARIANT(var, char_var)
#define HTTPD_ASSERT_VARIANTS(vars)
#endif

HTTP_HDR_TYPE   HttpdReadHdrAnalyze(char ** buff, char ** uri,size_t *uri_len);
const char *    HttpdResponseHdr(void);
const char *    HttpdPreamble(void);
void            HttpdInit(WORD port, err_t (* recv)(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err));
int             HttpdInputType(char * buff, const char * tag, const char *type, const char *name, 
                     char * value, const char* style, const char * post , 
                     const char * header, int size, bool br, int in_len);
char *          HttpdGetStringValue(char *data, int data_len, int max_value_len, 
                          const char * parameter, bool string);
int             HttpdInputType(char * buff, const char * tag, const char *type, const char *name, 
                     char * value, const char* style, const char * post , 
                     const char * header, int size, bool br, int in_len);
int             HttpdInputBoolType(char * buff, const char * tag, const char *name, 
                     const char* style, bool num, 
                     const char * header, int size, bool br, int in_len);
int             HttpdInputIntType(char * buff, const char * tag, const char *type, const char *name, 
                     DWORD num, const char* style, const char * post , 
                     const char * header, int size, bool br, int in_len);
VARIANT *       HttpdFindVariant(VARIANT *var, const char *buff);
int             HttpdGetVariant(char *data, int data_len, VARIANT *var);
int             HttpdSetVariant(char * buf, INT16U buff_max, VARIANT *var);

#ifdef __cplusplus
}
#endif
#endif