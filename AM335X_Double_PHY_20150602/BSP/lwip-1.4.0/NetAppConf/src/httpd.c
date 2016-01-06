/***************************************************************************** 
* Http.c - HTTP common utilities.
*
*
* portions Copyright (c) 2006 by Michael Vysotsky.
*
******************************************************************************/
#include "sysconfig.h"
#include "bsp.h"
#include "netconf.h"
#include "httpd.h"
#include "string.h"
#include "ctype.h"
#include "netconf.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/






/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
const char * http_response = 
    "HTTP/1.1 200 OK\r\n"
    "MIME-Version: 1.0\r\n"
    "Server: unspecified, UPnP/1.0, unspecified\r\n";

const char * http_preamble = 
"Content-type: text/html\r\n\r\n"
HTTP_DOC_TYPE;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static int  word_eq(char *s1, char *s2);

const char * HttpdResponseHdr(void){ 
  return http_response;
}
const char * HttpdPreamble(void){ 
  return http_preamble;
}


HTTP_HDR_TYPE  HttpdReadHdrAnalyze(char ** buff, char ** uri, size_t *uri_len)
{
  HTTP_HDR_TYPE hdr_type = HTTPREAD_HDR_TYPE_UNKNOWN;
  char * hbp = *buff;
  * uri = NULL;
  /* First line is special */
  if (!isgraph(*hbp))
    return HTTPREAD_HDR_TYPE_UNKNOWN;
  if (strncmp(hbp, "HTTP/", strlen("HTTP/")) == 0) {
    hdr_type = HTTPREAD_HDR_TYPE_REPLY;
    hbp += 5;
    while (isgraph(*hbp))
      hbp++;
    while (*hbp == ' ' || *hbp == '\t')
      hbp++;
    if (!isdigit(*hbp))
     return HTTPREAD_HDR_TYPE_UNKNOWN;
    if((HTTP_REPLY_CODE)atol(hbp) != HTTP_OK)
      return HTTPREAD_HDR_TYPE_UNKNOWN;    
  } else if (word_eq(hbp, "GET"))
    hdr_type = HTTPREAD_HDR_TYPE_GET;
  else if (word_eq(hbp, "HEAD"))
    hdr_type = HTTPREAD_HDR_TYPE_HEAD;
  else if (word_eq(hbp, "POST"))
    hdr_type = HTTPREAD_HDR_TYPE_POST;
  else if (word_eq(hbp, "PUT"))
    hdr_type = HTTPREAD_HDR_TYPE_PUT;
  else if (word_eq(hbp, "DELETE"))
    hdr_type = HTTPREAD_HDR_TYPE_DELETE;
  else if (word_eq(hbp, "TRACE"))
    hdr_type = HTTPREAD_HDR_TYPE_TRACE;
  else if (word_eq(hbp, "CONNECT"))
    hdr_type = HTTPREAD_HDR_TYPE_CONNECT;
  else if (word_eq(hbp, "NOTIFY"))
    hdr_type = HTTPREAD_HDR_TYPE_NOTIFY;
  else if (word_eq(hbp, "M-SEARCH"))
    hdr_type = HTTPREAD_HDR_TYPE_M_SEARCH;
  else if (word_eq(hbp, "M-POST"))
    hdr_type = HTTPREAD_HDR_TYPE_M_POST;
  else if (word_eq(hbp, "SUBSCRIBE"))
    hdr_type = HTTPREAD_HDR_TYPE_SUBSCRIBE;
  else if (word_eq(hbp, "UNSUBSCRIBE"))
    hdr_type = HTTPREAD_HDR_TYPE_UNSUBSCRIBE;
  else {
  }
  /* skip type */
  while (isgraph(*hbp))
    hbp++;
  while (*hbp == ' ' || *hbp == '\t')
    hbp++;
    /* parse uri.
     * Find length, allocate memory for translated
     * copy, then translate by changing %<hex><hex>
     * into represented value.
     */
   hbp = strchr(hbp,'/');
   if(!hbp)  return HTTPREAD_HDR_TYPE_UNKNOWN;
   hbp++;

  *uri = hbp;
  char *ptr_uri = strchr(hbp,' ');
  if(ptr_uri==NULL){
    *uri = NULL;
    *uri_len = 0;
    return HTTPREAD_HDR_TYPE_UNKNOWN;
  }
  *uri_len =ptr_uri - hbp;
  *buff = hbp + 1; 
  return hdr_type;
}
/* Check words for equality, where words consist of graphical characters
 * delimited by whitespace
 * Returns nonzero if "equal" doing case insensitive comparison.
 */
static int word_eq(char *s1, char *s2)
{
  int c1;
  int c2;
  int end1 = 0;
  int end2 = 0;
  for (;;) {
    c1 = *s1++;
    c2 = *s2++;
    if (isalpha(c1) && isupper(c1))
      c1 = tolower(c1);
    if (isalpha(c2) && isupper(c2))
      c2 = tolower(c2);
    end1 = !isgraph(c1);
    end2 = !isgraph(c2);
    if (end1 || end2 || c1 != c2)
      break;
  }
  return end1 && end2;  /* reached end of both words? */
}
/* Get Parameters string */

char * HttpdGetStringValue(char *data, int data_len, int max_value_len, const char * parameter, bool string)
{
  if(data == NULL)
    return NULL;
  int i,val_len=0;
  char * buff = NULL;
  bool found=FALSE;
  for (i=0;i<data_len;i++){
    if(strncmp ((data+i),parameter, strlen(parameter))==0){
      char *value = (char*)(data+i) + strlen(parameter); 
      for(i=0; i< max_value_len; ++i){
        if(((*value == '"')&&string)||((*value == '=')&&!string)){
          ++value;
          break;
        }
        ++value;
      }
      if((strncmp(parameter,value,strlen(parameter))==0)&&!string){
        found = TRUE;   
        val_len = strlen(parameter);
      }else
        for(i=0; i< max_value_len; ++i){
          if(((*(value+i) == '"')&&string)||
             (strncmp((value+i),"\r\n",sizeof("\r\n"))==0&&!string)||
             ((*(value+i) == '&')&&!string)||(((data+data_len)==(value+i))&&!string)){
            found = TRUE;   
            val_len = i;
            break; 
          }
        }
      if(found){
        buff = mem_malloc(val_len+1);
        if(!buff)
          return NULL;
        memset(buff,0,(val_len+1));
        if(val_len)
          strncpy(buff,value,val_len);
        break;
      }
    } 
  }
  return buff;
}

static bool CheckIpParameters(char * charIp, struct ip_addr *ip)
{
  if(charIp == NULL)
    return FALSE;
  DWORD i3,i2,i1,i0;
  if(sscanf(charIp,"%d.%d.%d.%d", &i3, &i2, &i1, &i0) != 4){
    return FALSE;
  }else if((i3|i2|i1|i0)>255){
    return FALSE;                
  }else{
    if(ip)
      ip->addr = htonl((DWORD)((i3<<24)|(i2<<16)|(i1<<8)|i0));
  }
  return TRUE;
}



int HttpdGetVariant(char *data, int data_len, VARIANT *var)
{
  char *value = NULL;
  int ret = VAR_OK;
  DWORD param=0; 
  if(var->type == TAG_VAR ||  var->type == FILE_NAME_VAR)
    return 0;
  var->flags &= ~VAR_FLAG_ERROR;
  if(var->value.void_ptr_var && var->type == BOOL_VAR)
    var->value.bool_var = FALSE;
  if((value = HttpdGetStringValue((char *)data, data_len, var->max_var_len,var->name,FALSE))){
    if(var->value.void_ptr_var) 
      switch(var->type){
      case BOOL_VAR:  var->value.bool_var = TRUE; break;
      case BYTE_VAR:
        if((param = atol(value)) > 255){
          var->flags |= VAR_FLAG_ERROR;
          ret = VAR_BAD;
        }else
          var->value.byte_var = param; 
        break;
      case SHORT_VAR:
        if((param = atol(value)) > 65535){
          var->flags |= VAR_FLAG_ERROR;
          ret = VAR_BAD;
        }else
          var->value.short_var = param; 
        break;      
      case INT_VAR:   var->value.int_var = atol(value); break;
      case ULONG_VAR: var->value.ulong_var = atol(value); break;
      case CHAR_VAR:  var->value.char_var = *value; break;
      case STRING_VAR: strcpy(var->value.string_var,value); break;
      case IP_VAR:
        if(!CheckIpParameters(value, &var->value.ip_var)){
          var->flags |= VAR_FLAG_ERROR;
          ret = VAR_BAD;
        }break;
      case FUNC_VAR: if(var->value.func_var) var->value.func_var(); break;  
      default:
        ret = VAR_FAIL;
        var->flags |= VAR_FLAG_ERROR;
      }
    else{
      ret = VAR_FAIL;
      var->flags |= VAR_FLAG_ERROR;
    }
    mem_free(value);
    return ret;
  }
  return VAR_NOTFOUND;
}



VARIANT * HttpdFindVariant(VARIANT *var, const char *buff)
{
  for(; var->type != NULL_VAR; ++var)
    if(strncmp(buff, var->name, strlen(var->name))==0)
      return var;
  return NULL;
}

int HttpdSetVariant(char * buf, INT16U buff_max, VARIANT *var)
{
  int length = -1;
  char * style = var->flags&VAR_FLAG_ERROR ?  "color:Red":"color:Black";
  if((var->flags&VAR_FLAG_ERROR)==0 && var->flags&VAR_FLAG_COLOR){
    char style_buf[32];
    style = style_buf;
    sprintf(style_buf,"color:#%06X",var->color);
  }  
  switch(var->type){    
  case BYTE_VAR:
    length= HttpdInputIntType(buf,NULL,"text",var->name,var->value.byte_var, 
                      style,NULL,var->hdr_name,var->size,
                      ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);
    break;
  case SHORT_VAR:
    length= HttpdInputIntType(buf,NULL,"text",var->name,var->value.short_var, 
                      style,NULL,var->hdr_name,var->size,
                      ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);
    break;    
  case INT_VAR:
    length= HttpdInputIntType(buf,NULL,"text",var->name,var->value.int_var, 
                      style,NULL,var->hdr_name,var->size,
                      ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);
    break;    
  case ULONG_VAR:
    length= HttpdInputIntType(buf,NULL,"text",var->name,var->value.ulong_var, 
                      style,NULL,var->hdr_name,var->size,
                      ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);    
    break;
  case CHAR_VAR:
    {
      char char_buf[] = {var->value.char_var,0};
      length= HttpdInputType(buf,NULL,"text", var->name, char_buf,style,NULL,
                     var->hdr_name,var->size,
                     ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);  
                          
    }    
    break;
  case TAG_VAR:
    if((strlen(buf)+strlen(var->name))>buff_max){
      length = -1;
    }else{
      strcat(buf,var->name);
      length =strlen(buf);
    }     
    break;
  case STRING_VAR:
    length= HttpdInputType(buf,NULL,"text", var->name, var->value.string_var,style,NULL,
                   var->hdr_name,var->size,
                   ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);     
    break;
  case BOOL_VAR:
    length= HttpdInputBoolType(buf,NULL,var->name,style, var->value.bool_var,
                       var->hdr_name,var->size,
                       ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);     
    break;
  case FUNC_VAR:
    length = HttpdInputType(buf,NULL,"submit",var->name,(char*)var->name, 
                      style,NULL,var->hdr_name,var->size,
                     ((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);
    break;
  case FILE_NAME_VAR:
    if((strlen(buf) + strlen("<a href=\"") + strlen(var->name) + strlen("\" style=\"")
       +strlen(style) + strlen("\">") +strlen(var->hdr_name) + strlen("</a>")+5)>buff_max)
      length =-1;
    else{
      strcat(buf,"<a href=\"");
      strcat(buf,var->value.filename_var);
      strcat(buf,"\" style=\"");
      strcat(buf,style);
      strcat(buf,"\">");
      strcat(buf,var->hdr_name);
      strcat(buf,"</a>");
      if(var->flags&VAR_FLAG_BR)
        strcat(buf,"<br/>");
      length = strlen(buf);
    }
    break;
  case IP_VAR:
    length = HttpdInputType(buf,NULL,"text",var->name,
                            htoa((struct in_addr*)&var->value.ip_var),style,NULL,var->hdr_name,
                            var->size,((var->flags&VAR_FLAG_BR)?  TRUE:FALSE),buff_max);     
    break;
  }
  return length;
}

int HttpdInputIntType(char * buff, const char * tag, const char *type, const char *name, 
                     DWORD num, const char* style, const char * post , 
                     const char * header, int size, bool br, int in_len)
{
  char*value = mem_malloc(12);
  if(value == NULL)
  return -1;
  memset(value,0,12);
  sprintf(value, "%ld",num);
  int ret = HttpdInputType(buff,tag,type,name,value,style,post,header,size,br,in_len);
  mem_free(value);
  return ret;
}
int HttpdInputBoolType(char * buff, const char * tag, const char *name, 
                     const char* style, bool num, 
                     const char * header, int size, bool br, int in_len)
{
  char * post = num ? "checked":NULL;
  return  HttpdInputType(buff,tag,"checkbox",name,(char*)name,style,post,header,size,br,in_len);
}
/* Build input metod string */
int HttpdInputType(char * buff, const char * tag, const char *type, const char *name, 
                     char * value, const char* style, const char * post , 
                     const char * header, int size, bool br, int in_len)
{
  int len = strlen("<input type=\"") + strlen(type);
  if(name);
    len += strlen("\" name=\"") + strlen(name);
  if(tag)
    len += 2*(strlen(tag) + 3);
  if(value)
    len +=  strlen("\" value=\"") + strlen(value);
  if(style)
    len +=  strlen("\" style=\"") + strlen(style);
  if(header)
    len += strlen(header);
  if(size)
    len += strlen("\" size=\"") + 6;
  if(br)
    len +=strlen("<br>");
  len += 6;
  if((strlen(buff) + len) > in_len)
    return -1;
  int pos = strlen(buff);
  if(tag){
    sprintf(&buff[pos],"<%s>",tag);  
  }
  pos = strlen(buff);
  sprintf(&buff[pos],"<input type=\"%s\" ",type);
  pos = strlen(buff);
  if(name){
    sprintf(&buff[pos],"name=\"%s\" ",name);
    pos = strlen(buff);
  }
  if(value){
    sprintf(&buff[pos],"value=\"%s\" ",value);
    pos = strlen(buff);
  }
  if(post){
    strcat(buff,post);
    pos = strlen(buff);
  }  
  if(style){
    sprintf(&buff[pos],"style=\"%s\" ",style);
    pos = strlen(buff);    
  }
  if(size){
    sprintf(&buff[pos],"size=\"%d\" ",size);
    pos = strlen(buff);    
  }

  strcat(buff," /> ");
  pos = strlen(buff);
  if(header){
    sprintf(&buff[pos],"%s",header);
    pos = strlen(buff);    
  }
  if(br){
    strcat(buff,"<br>");
    pos = strlen(buff);    
  }
  if(tag){
    sprintf(&buff[pos],"</%s>",tag);
  }
  strcat(buff,"\r\n");
  return strlen(buff);
}








/**
  * @brief  intialize HTTP webserver  
  * @param  port - http port
  * @param  recv - calback function
  * @retval none
  */
void HttpdInit(WORD port, err_t (* recv)(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err))
{
  struct tcp_pcb *pcb;
  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, port);
  pcb = tcp_listen(pcb);
  tcp_recv(pcb, recv);

}
