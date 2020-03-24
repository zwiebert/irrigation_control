#include "status_output.h"

#ifndef SO_MSG_T
#define SO_MSG_T so_msg_t
#endif

extern uint8_t so_target;
extern bool cli_isJson;

#define so_cco (!cli_isJson)
#define so_jco cli_isJson

#ifndef USE_JSON
#undef so_jco
#define so_jco false
#endif



void so_out_x_reply_entry_f(SO_MSG_T key,float val,int n);
void so_out_x_reply_entry_lx(SO_MSG_T key,int val);
void so_out_x_reply_entry_l(SO_MSG_T key,int val);
void so_out_x_reply_entry_d(SO_MSG_T key,int val);
void so_out_x_reply_entry_sl(const char *key,int val);
void so_out_x_reply_entry_sd(const char *key,int val);
void so_out_x_reply_entry_ss(const char *key,const char *val);
void so_out_x_reply_entry_s(SO_MSG_T key,const char *val);
void so_out_x_reply_entry(SO_MSG_T key,const char *val);
void so_out_x_close();
void so_out_x_open(const char *name);
SO_MSG_T so_parse_config_key(const char *k);
