#ifndef cli_imp_h_
#define cli_imp_h_
#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMD_BUF_SIZE 128
extern char cmd_buf[CMD_BUF_SIZE];

// implementation interface for  cli*.c files
extern const char pin_state_args[];


typedef struct {
  char *key;
  char *val;
} clpar;
#define MAX_PAR 20
extern clpar par[MAX_PAR];

extern const char help_parmCmd[] ;
extern const char help_parmTimer[] ;
extern const char help_parmConfig[] ;
extern const char help_parmMcu[] ;
extern const char help_parmPair[] ;
extern const char help_parmHelp[] ;
extern const char help_None[] ;

extern uint16_t msgid;


#define NODEFAULT() if (val==0) return reply_failure()


/* cli.c */
void print_enr(void);
void msg_print(const char *msg, const char *tag);
void reply_print(const char *tag);
void reply_message(const char *tag, const char *msg);
void reply_id_message(uint16_t id, const char *tag, const char *msg);
void cli_out_timer_reply_entry(const char *key, const char *val, int len);
void cli_out_config_reply_entry(const char *key, const char *val, int len);
void cli_out_mcu_reply_entry(const char *key, const char *val, int len);
int process_parmCmd(clpar p[], int len);
int process_parmConfig(clpar p[], int len);
int process_parmMcu(clpar p[], int len);
int process_parmTimer(clpar p[], int len);
int process_parmHelp(clpar p[], int len);
int process_parmPair(clpar p[], int len);
/* cli.c */
void warning_unknown_option(const char *key);
void cli_msg_ready(void);
char *get_commandline(void);
int asc2bool(const char *s);
int parse_commandline(char *cl);
void reply_success(void);
int reply_failure(void);
bool config_receiver(const char *val);
bool config_transmitter(const char *val);
bool reply(bool success);
bool timerString2bcd(const char *src, uint8_t *dst, uint16_t size_dst);
int process_parm(clpar p[], int len);
void cli_process_cmdline(char *line);
void cli_loop(void);

void cli_print_json(const char *json); //FIXME

void cli_process_json(char *json);

#ifdef __cplusplus
}
#endif
#endif
