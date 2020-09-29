#include "app/config/proj_app_cfg.h"
#include <cli/cli.h>
#include <string.h>
#include "app/settings/config.h"
#include "misc/bcd.h"
#include "cli_imp.h"
#include "app/cli/cli_app.h"
#include "misc/int_types.h"


const char *Obj_tag="";
#define SET_OBJ_TAG(tag) Obj_tag=(tag)
#define OBJ_TAG (Obj_tag+0)
#define OBJ_TAG_TIMER "timer"
#define OBJ_TAG_CONFIG "config"
#define OBJ_TAG_MCU "mcu"
#define OBJ_TAG_SEND "send"


static void  cli_out_top_tag(void) {
    io_puts("tf: ");
}

static void  cli_out_reply_tag(void) {
  if (cli_msgid) {
    io_puts("cli_replyResult="), io_putd(cli_msgid), io_puts(": ");
  } else {
    io_puts("cli_replyResult: ");
  }
}

static void  cli_out_obj_tag(void) {
    io_puts(OBJ_TAG), io_puts(":");
}

static void  cli_out_start_reply(void) {
  cli_out_top_tag();
  cli_out_reply_tag();
  cli_out_obj_tag();
}


static void  cli_out_start_timer_reply(void) {
  SET_OBJ_TAG(OBJ_TAG_TIMER);
  cli_out_start_reply();
}


static void  cli_out_start_config_reply(void) {
  SET_OBJ_TAG(OBJ_TAG_CONFIG);
  cli_out_start_reply();
}

static void  cli_out_start_mcu_reply(void) {
  SET_OBJ_TAG(OBJ_TAG_MCU);
  cli_out_start_reply();
}


typedef void (*void_fun_ptr)(void);
const int OUT_MAX_LEN = 80;

static void  cli_out_entry(void_fun_ptr tag, const char *key, const char *val, int len) {
  static int length;

  if (!key || len == -1) {
    if (length) {
      io_puts(";\n");
      length = 0;
    }
  } else {
    if (key)
      len += strlen(key) + 1;
    if (val)
      len += strlen(val);

    if ((length + len) > OUT_MAX_LEN && length > 0) {
      io_puts(";\n");
      length = 0;
    }
    if (length == 0) {
      tag();
    }
    length += len;
    if (key) {
      io_putc(' '), io_puts(key), io_putc('=');
    }
    if (val) {
      io_puts(val);
    }
  }
}

void   cli_out_set_config(void) {
  SET_OBJ_TAG(OBJ_TAG_CONFIG);
}

void   cli_out_set_x(const char *obj_tag) {
  SET_OBJ_TAG(obj_tag);
}
void cli_out_close() {
  cli_out_x_reply_entry(0, 0, 0);
}
void  cli_out_x_reply_entry(const char *key, const char *val, int len) {
   if (!cli_isInteractive())
    return;
  cli_out_entry(cli_out_start_reply, key, val, len);
}

void  cli_out_timer_reply_entry(const char *key, const char *val, int len) {
   if (!cli_isInteractive())
    return;
  cli_out_entry(cli_out_start_timer_reply, key, val, len);
}

void  cli_out_config_reply_entry(const char *key, const char *val, int len) {
   if (!cli_isInteractive())
    return;
  cli_out_entry(cli_out_start_config_reply, key, val, len);
}

void  cli_out_mcu_reply_entry(const char *key, const char *val, int len) {
   if (!cli_isInteractive())
    return;
  cli_out_entry(cli_out_start_mcu_reply, key, val, len);
}
