#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "mcu_com.pb.h"
#include "zone_data.hh"

#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


struct lph_args {
  const struct zd_arg *a;
  int lph_idx;
};

static bool decode_lphArray(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  struct lph_args *lph_args = (struct lph_args*) *arg;
  const struct zd_arg *lph_arg = lph_args->a;
  if (lph_args->lph_idx == 0)
    memset(lph_arg->lph_arr, 0, sizeof lph_arg->lph_arr[0] * lph_arg->lph_arr_len);

  uint64_t value;
  if (!pb_decode_varint(stream, &value))
    return false;
  if (lph_args->lph_idx < lph_arg->lph_arr_len)
    lph_arg->lph_arr[lph_args->lph_idx++] = (int) value;
  return true;
}


static bool encode_lphArray(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
  const struct zd_arg *lph_arg = (const struct zd_arg*) *arg;

  for (int i = 0; i < lph_arg->lph_arr_len; ++i) {
  if (!pb_encode_tag_for_field(stream, field))
      return false;
    if (!pb_encode_varint(stream, lph_arg->lph_arr[i]))
      return false;
  }
  return true;
}

bool decode_zoneData(const uint8_t *src_buf, size_t src_buf_len, const struct zd_arg *dst_zd_arg) {
  ZoneData zd = ZoneData_init_zero;
  zd.lph.funcs.decode = decode_lphArray;
  struct lph_args arg = { .a = dst_zd_arg };
  zd.lph.arg = &arg;

  /* Create a stream that reads from the buffer. */
  pb_istream_t stream = pb_istream_from_buffer(src_buf, src_buf_len);

  /* Now we are ready to decode the message. */
  bool status = pb_decode(&stream, &ZoneData_msg, &zd);


  return status;
}


int encode_zoneData(uint8_t *dst_msg_uf, size_t  dst_msg_buf_len, const struct zd_arg *zd_arg) {
  ZoneData zd = ZoneData_init_default;
  zd.lph.funcs.encode = encode_lphArray;

  zd.lph.arg = (void *)zd_arg;
  zd.zone_count = zd_arg->lph_arr_len;
  pb_ostream_t stream = pb_ostream_from_buffer(dst_msg_uf, dst_msg_buf_len);
  bool status = pb_encode(&stream, &ZoneData_msg, &zd);
  return status ? stream.bytes_written : -1;
}
