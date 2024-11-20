#ifndef __MPACK_H__
#define __MPACK_H__

// The following code is adapted from libmpack: https://github.com/libmpack/libmpack ///////////////

#ifndef MPACK_API
# define MPACK_API extern
#endif

#include <assert.h>
#include <limits.h>
#include <stddef.h>

#ifdef __GNUC__
# define FPURE __attribute__((const))
# define FNONULL __attribute__((nonnull))
# define FNONULL_ARG(x) __attribute__((nonnull x))
# define FUNUSED __attribute__((unused))
#else
# define FPURE
# define FNONULL
# define FNONULL_ARG(x)
# define FUNUSED
#endif

#if UINT_MAX == 0xffffffff
typedef int mpack_sint32_t;
typedef unsigned int mpack_uint32_t;
#elif ULONG_MAX == 0xffffffff
typedef long mpack_sint32_t;
typedef unsigned long mpack_uint32_t;
#else
# error "can't find unsigned 32-bit integer type"
#endif

typedef struct mpack_value_s {
  mpack_uint32_t lo, hi;
} mpack_value_t;


enum {
  MPACK_OK = 0,
  MPACK_EOF = 1,
  MPACK_ERROR = 2
};

#define MPACK_MAX_TOKEN_LEN 9  /* 64-bit ints/floats plus type code */

typedef enum {
  MPACK_TOKEN_NIL       = 1,
  MPACK_TOKEN_BOOLEAN   = 2,
  MPACK_TOKEN_UINT      = 3,
  MPACK_TOKEN_SINT      = 4,
  MPACK_TOKEN_FLOAT     = 5,
  MPACK_TOKEN_CHUNK     = 6,
  MPACK_TOKEN_ARRAY     = 7,
  MPACK_TOKEN_MAP       = 8,
  MPACK_TOKEN_BIN       = 9,
  MPACK_TOKEN_STR       = 10,
  MPACK_TOKEN_EXT       = 11
} mpack_token_type_t;

typedef struct mpack_token_s {
  mpack_token_type_t type;  /* Type of token */
  mpack_uint32_t length;    /* Byte length for str/bin/ext/chunk/float/int/uint.
                               Item count for array/map. */
  union {
    mpack_value_t value;    /* 32-bit parts of primitives (bool,int,float) */
    const char *chunk_ptr;  /* Chunk of data from str/bin/ext */
    int ext_type;           /* Type field for ext tokens */
  } data;
} mpack_token_t;

typedef struct mpack_tokbuf_s {
  char pending[MPACK_MAX_TOKEN_LEN];
  mpack_token_t pending_tok;
  size_t ppos, plen;
  mpack_uint32_t passthrough;
} mpack_tokbuf_t;

#define MPACK_TOKBUF_INITIAL_VALUE { { 0 }, { 0, 0, { { 0, 0 } } }, 0, 0, 0 }

MPACK_API void mpack_tokbuf_init(mpack_tokbuf_t *tb) FUNUSED FNONULL;
MPACK_API int mpack_read(mpack_tokbuf_t *tb, const char **b, size_t *bl,
    mpack_token_t *tok) FUNUSED FNONULL;
MPACK_API int mpack_write(mpack_tokbuf_t *tb, char **b, size_t *bl,
    const mpack_token_t *tok) FUNUSED FNONULL;


#if ULLONG_MAX == 0xffffffffffffffff
typedef long long mpack_sintmax_t;
typedef unsigned long long mpack_uintmax_t;
#elif UINT64_MAX == 0xffffffffffffffff
typedef int64_t mpack_sintmax_t;
typedef uint64_t mpack_uintmax_t;
#else
typedef mpack_sint32_t mpack_sintmax_t;
typedef mpack_uint32_t mpack_uintmax_t;
#endif

#ifndef bool
# define bool unsigned
#endif

MPACK_API mpack_token_t mpack_pack_nil(void) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_boolean(unsigned v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_uint(mpack_uintmax_t v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_sint(mpack_sintmax_t v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_float_compat(double v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_float_fast(double v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_number(double v) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_chunk(const char *p, mpack_uint32_t l)
  FUNUSED FPURE FNONULL;
MPACK_API mpack_token_t mpack_pack_str(mpack_uint32_t l) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_bin(mpack_uint32_t l) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_ext(int type, mpack_uint32_t l)
  FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_array(mpack_uint32_t l) FUNUSED FPURE;
MPACK_API mpack_token_t mpack_pack_map(mpack_uint32_t l) FUNUSED FPURE;
MPACK_API bool mpack_unpack_boolean(mpack_token_t t) FUNUSED FPURE;
MPACK_API mpack_uintmax_t mpack_unpack_uint(mpack_token_t t) FUNUSED FPURE;
MPACK_API mpack_sintmax_t mpack_unpack_sint(mpack_token_t t) FUNUSED FPURE;
MPACK_API double mpack_unpack_float_fast(mpack_token_t t) FUNUSED FPURE;
MPACK_API double mpack_unpack_float_compat(mpack_token_t t) FUNUSED FPURE;
MPACK_API double mpack_unpack_number(mpack_token_t t) FUNUSED FPURE;

/* The mpack_{pack,unpack}_float_fast functions should work in 99% of the
 * platforms. When compiling for a platform where floats don't use ieee754 as
 * the internal format, pass
 * -Dmpack_{pack,unpack}_float=mpack_{pack,unpack}_float_compat to the
 *  compiler.*/
#ifndef mpack_pack_float
# define mpack_pack_float mpack_pack_float_fast
#endif
#ifndef mpack_unpack_float
# define mpack_unpack_float mpack_unpack_float_fast
#endif

#include <string.h>


#define UNUSED(p) (void)p;
#define ADVANCE(buf, buflen) ((*buflen)--, (unsigned char)*((*buf)++))
#define TLEN(val, range_start) ((mpack_uint32_t)(1 << (val - range_start)))
#ifndef MIN
# define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

static int mpack_rtoken(const char **buf, size_t *buflen,
    mpack_token_t *tok);
static int mpack_rpending(const char **b, size_t *nl, mpack_tokbuf_t *tb);
static int mpack_rvalue(mpack_token_type_t t, mpack_uint32_t l,
    const char **b, size_t *bl, mpack_token_t *tok);
static int mpack_rblob(mpack_token_type_t t, mpack_uint32_t l,
    const char **b, size_t *bl, mpack_token_t *tok);
static int mpack_wtoken(const mpack_token_t *tok, char **b, size_t *bl);
static int mpack_wpending(char **b, size_t *bl, mpack_tokbuf_t *tb);
static int mpack_wpint(char **b, size_t *bl, mpack_value_t v);
static int mpack_wnint(char **b, size_t *bl, mpack_value_t v);
static int mpack_wfloat(char **b, size_t *bl, const mpack_token_t *v);
static int mpack_wstr(char **buf, size_t *buflen, mpack_uint32_t len);
static int mpack_wbin(char **buf, size_t *buflen, mpack_uint32_t len);
static int mpack_wext(char **buf, size_t *buflen, int type,
    mpack_uint32_t len);
static int mpack_warray(char **buf, size_t *buflen, mpack_uint32_t len);
static int mpack_wmap(char **buf, size_t *buflen, mpack_uint32_t len);
static int mpack_w1(char **b, size_t *bl, mpack_uint32_t v);
static int mpack_w2(char **b, size_t *bl, mpack_uint32_t v);
static int mpack_w4(char **b, size_t *bl, mpack_uint32_t v);
static mpack_value_t mpack_byte(unsigned char b);
static int mpack_value(mpack_token_type_t t, mpack_uint32_t l,
    mpack_value_t v, mpack_token_t *tok);
static int mpack_blob(mpack_token_type_t t, mpack_uint32_t l, int et,
    mpack_token_t *tok);

MPACK_API void mpack_tokbuf_init(mpack_tokbuf_t *tokbuf)
{
  tokbuf->ppos = 0;
  tokbuf->plen = 0;
  tokbuf->passthrough = 0;
}

MPACK_API int mpack_read(mpack_tokbuf_t *tokbuf, const char **buf,
    size_t *buflen, mpack_token_t *tok)
{
  int status;
  size_t initial_ppos, ptrlen, advanced;
  const char *ptr, *ptr_save;
  assert(*buf && *buflen);

  if (tokbuf->passthrough) {
    /* pass data from str/bin/ext directly as a MPACK_TOKEN_CHUNK, adjusting
     * *buf and *buflen */
    tok->type = MPACK_TOKEN_CHUNK;
    tok->data.chunk_ptr = *buf;
    tok->length = MIN((mpack_uint32_t)*buflen, tokbuf->passthrough);
    tokbuf->passthrough -= tok->length;
    *buf += tok->length;
    *buflen -= tok->length;
    goto done;
  }

  initial_ppos = tokbuf->ppos;

  if (tokbuf->plen) {
    if (!mpack_rpending(buf, buflen, tokbuf)) {
      return MPACK_EOF;
    }
    ptr = tokbuf->pending;
    ptrlen = tokbuf->ppos;
  } else {
    ptr = *buf;
    ptrlen = *buflen;
  }

  ptr_save = ptr;

  if ((status = mpack_rtoken(&ptr, &ptrlen, tok))) {
    if (status != MPACK_EOF) return MPACK_ERROR;
    /* need more data */
    assert(!tokbuf->plen);
    /* read the remainder of *buf to tokbuf->pending so it can be parsed
     * later with more data. only required when tokbuf->plen == 0 or else
     * it would have been done already. */
    tokbuf->plen = tok->length + 1;
    assert(tokbuf->plen <= sizeof(tokbuf->pending));
    tokbuf->ppos = 0;
    status = mpack_rpending(buf, buflen, tokbuf);
    assert(!status);
    return MPACK_EOF;
  }

  advanced = (size_t)(ptr - ptr_save) - initial_ppos;
  tokbuf->plen = tokbuf->ppos = 0;
  *buflen -= advanced;
  *buf += advanced;

  if (tok->type > MPACK_TOKEN_MAP) {
    tokbuf->passthrough = tok->length;
  }

done:
  return MPACK_OK;
}

MPACK_API int mpack_write(mpack_tokbuf_t *tokbuf, char **buf, size_t *buflen, const mpack_token_t *t)
{
  int status;
  char *ptr;
  size_t ptrlen;
  mpack_token_t tok = tokbuf->plen ? tokbuf->pending_tok : *t;
  assert(*buf && *buflen);

  if (tok.type == MPACK_TOKEN_CHUNK) {
    size_t written, pending, count;
    if (!tokbuf->plen) tokbuf->ppos = 0;
    written = tokbuf->ppos;
    pending = tok.length - written;
    count = MIN(pending, *buflen);
    memcpy(*buf, tok.data.chunk_ptr + written, count);
    *buf += count;
    *buflen -= count;
    tokbuf->ppos += count;
    tokbuf->plen = count == pending ? 0 : tok.length;
    if (count == pending) {
      return MPACK_OK;
    } else {
      tokbuf->pending_tok = tok;
      return MPACK_EOF;
    }
  }

  if (tokbuf->plen) return mpack_wpending(buf, buflen, tokbuf);

  if (*buflen < MPACK_MAX_TOKEN_LEN) {
    ptr = tokbuf->pending;
    ptrlen = sizeof(tokbuf->pending);
  } else {
    ptr = *buf;
    ptrlen = *buflen;
  }

  if ((status = mpack_wtoken(&tok, &ptr, &ptrlen))) return status;

  if (*buflen < MPACK_MAX_TOKEN_LEN) {
    size_t toklen = sizeof(tokbuf->pending) - ptrlen;
    size_t write_cnt = MIN(toklen, *buflen);
    memcpy(*buf, tokbuf->pending, write_cnt);
    *buf += write_cnt;
    *buflen -= write_cnt;
    if (write_cnt < toklen) {
      assert(!*buflen);
      tokbuf->plen = toklen;
      tokbuf->ppos = write_cnt;
      tokbuf->pending_tok = tok;
      return MPACK_EOF;
    }
  } else {
    *buflen -= (size_t)(ptr - *buf);
    *buf = ptr;
  }

  return MPACK_OK;
}

static int mpack_rtoken(const char **buf, size_t *buflen,
    mpack_token_t *tok)
{
  unsigned char t = ADVANCE(buf, buflen);
  if (t < 0x80) {
    /* positive fixint */
    return mpack_value(MPACK_TOKEN_UINT, 1, mpack_byte(t), tok);
  } else if (t < 0x90) {
    /* fixmap */
    return mpack_blob(MPACK_TOKEN_MAP, t & 0xf, 0, tok);
  } else if (t < 0xa0) {
    /* fixarray */
    return mpack_blob(MPACK_TOKEN_ARRAY, t & 0xf, 0, tok);
  } else if (t < 0xc0) {
    /* fixstr */
    return mpack_blob(MPACK_TOKEN_STR, t & 0x1f, 0, tok);
  } else if (t < 0xe0) {
    switch (t) {
      case 0xc0:  /* nil */
        return mpack_value(MPACK_TOKEN_NIL, 0, mpack_byte(0), tok);
      case 0xc2:  /* false */
        return mpack_value(MPACK_TOKEN_BOOLEAN, 1, mpack_byte(0), tok);
      case 0xc3:  /* true */
        return mpack_value(MPACK_TOKEN_BOOLEAN, 1, mpack_byte(1), tok);
      case 0xc4:  /* bin 8 */
      case 0xc5:  /* bin 16 */
      case 0xc6:  /* bin 32 */
        return mpack_rblob(MPACK_TOKEN_BIN, TLEN(t, 0xc4), buf, buflen, tok);
      case 0xc7:  /* ext 8 */
      case 0xc8:  /* ext 16 */
      case 0xc9:  /* ext 32 */
        return mpack_rblob(MPACK_TOKEN_EXT, TLEN(t, 0xc7), buf, buflen, tok);
      case 0xca:  /* float 32 */
      case 0xcb:  /* float 64 */
        return mpack_rvalue(MPACK_TOKEN_FLOAT, TLEN(t, 0xc8), buf, buflen, tok);
      case 0xcc:  /* uint 8 */
      case 0xcd:  /* uint 16 */
      case 0xce:  /* uint 32 */
      case 0xcf:  /* uint 64 */
        return mpack_rvalue(MPACK_TOKEN_UINT, TLEN(t, 0xcc), buf, buflen, tok);
      case 0xd0:  /* int 8 */
      case 0xd1:  /* int 16 */
      case 0xd2:  /* int 32 */
      case 0xd3:  /* int 64 */
        return mpack_rvalue(MPACK_TOKEN_SINT, TLEN(t, 0xd0), buf, buflen, tok);
      case 0xd4:  /* fixext 1 */
      case 0xd5:  /* fixext 2 */
      case 0xd6:  /* fixext 4 */
      case 0xd7:  /* fixext 8 */
      case 0xd8:  /* fixext 16 */
        if (*buflen == 0) {
          /* require only one extra byte for the type code */
          tok->length = 1;
          return MPACK_EOF;
        }
        tok->length = TLEN(t, 0xd4);
        tok->type = MPACK_TOKEN_EXT;
        tok->data.ext_type = ADVANCE(buf, buflen);
        return MPACK_OK;
      case 0xd9:  /* str 8 */
      case 0xda:  /* str 16 */
      case 0xdb:  /* str 32 */
        return mpack_rblob(MPACK_TOKEN_STR, TLEN(t, 0xd9), buf, buflen, tok);
      case 0xdc:  /* array 16 */
      case 0xdd:  /* array 32 */
        return mpack_rblob(MPACK_TOKEN_ARRAY, TLEN(t, 0xdb), buf, buflen, tok);
      case 0xde:  /* map 16 */
      case 0xdf:  /* map 32 */
        return mpack_rblob(MPACK_TOKEN_MAP, TLEN(t, 0xdd), buf, buflen, tok);
      default:
        return MPACK_ERROR;
    }
  } else {
    /* negative fixint */
    return mpack_value(MPACK_TOKEN_SINT, 1, mpack_byte(t), tok);
  }
}

static int mpack_rpending(const char **buf, size_t *buflen,
    mpack_tokbuf_t *state)
{
  size_t count;
  assert(state->ppos < state->plen);
  count = MIN(state->plen - state->ppos, *buflen);
  memcpy(state->pending + state->ppos, *buf, count);
  state->ppos += count;
  if (state->ppos < state->plen) {
    /* consume buffer since no token will be parsed yet. */
    *buf += *buflen;
    *buflen = 0;
    return 0;
  }
  return 1;
}

static int mpack_rvalue(mpack_token_type_t type, mpack_uint32_t remaining,
    const char **buf, size_t *buflen, mpack_token_t *tok)
{
  if (*buflen < remaining) {
    tok->length = remaining;
    return MPACK_EOF;
  }

  mpack_value(type, remaining, mpack_byte(0), tok);

  while (remaining) {
    mpack_uint32_t byte = ADVANCE(buf, buflen), byte_idx, byte_shift;
    byte_idx = (mpack_uint32_t)--remaining;
    byte_shift = (byte_idx % 4) * 8;
    tok->data.value.lo |= byte << byte_shift;
    if (remaining == 4) {
      /* unpacked the first half of a 8-byte value, shift what was parsed to the
       * "hi" field and reset "lo" for the trailing 4 bytes. */
      tok->data.value.hi = tok->data.value.lo;
      tok->data.value.lo = 0;
    }
  }

  if (type == MPACK_TOKEN_SINT) {
    mpack_uint32_t hi = tok->data.value.hi;
    mpack_uint32_t lo = tok->data.value.lo;
    mpack_uint32_t msb = (tok->length == 8 && hi >> 31) ||
                         (tok->length == 4 && lo >> 31) ||
                         (tok->length == 2 && lo >> 15) ||
                         (tok->length == 1 && lo >> 7);
    if (!msb) {
      tok->type = MPACK_TOKEN_UINT;
    }
  }

  return MPACK_OK;
}

static int mpack_rblob(mpack_token_type_t type, mpack_uint32_t tlen,
    const char **buf, size_t *buflen, mpack_token_t *tok)
{
  mpack_token_t l;
  mpack_uint32_t required = tlen + (type == MPACK_TOKEN_EXT ? 1 : 0);

  if (*buflen < required) {
    tok->length = required;
    return MPACK_EOF;
  }

  l.data.value.lo = 0;
  mpack_rvalue(MPACK_TOKEN_UINT, tlen, buf, buflen, &l);
  tok->type = type;
  tok->length = l.data.value.lo;

  if (type == MPACK_TOKEN_EXT) {
    tok->data.ext_type = ADVANCE(buf, buflen);
  }

  return MPACK_OK;
}

static int mpack_wtoken(const mpack_token_t *tok, char **buf,
    size_t *buflen)
{
  switch (tok->type) {
    case MPACK_TOKEN_NIL:
      return mpack_w1(buf, buflen, 0xc0);
    case MPACK_TOKEN_BOOLEAN:
      return mpack_w1(buf, buflen, tok->data.value.lo ? 0xc3 : 0xc2);
    case MPACK_TOKEN_UINT:
      return mpack_wpint(buf, buflen, tok->data.value);
    case MPACK_TOKEN_SINT:
      return mpack_wnint(buf, buflen, tok->data.value);
    case MPACK_TOKEN_FLOAT:
      return mpack_wfloat(buf, buflen, tok);
    case MPACK_TOKEN_BIN:
      return mpack_wbin(buf, buflen, tok->length);
    case MPACK_TOKEN_STR:
      return mpack_wstr(buf, buflen, tok->length);
    case MPACK_TOKEN_EXT:
      return mpack_wext(buf, buflen, tok->data.ext_type, tok->length);
    case MPACK_TOKEN_ARRAY:
      return mpack_warray(buf, buflen, tok->length);
    case MPACK_TOKEN_MAP:
      return mpack_wmap(buf, buflen, tok->length);
    default:
      return MPACK_ERROR;
  }
}

static int mpack_wpending(char **buf, size_t *buflen, mpack_tokbuf_t *state)
{
  size_t count;
  assert(state->ppos < state->plen);
  count = MIN(state->plen - state->ppos, *buflen);
  memcpy(*buf, state->pending + state->ppos, count);
  state->ppos += count;
  *buf += count;
  *buflen -= count;
  if (state->ppos == state->plen) {
    state->plen = 0;
    return MPACK_OK;
  }
  return MPACK_EOF;
}

static int mpack_wpint(char **buf, size_t *buflen, mpack_value_t val)
{
  mpack_uint32_t hi = val.hi;
  mpack_uint32_t lo = val.lo;

  if (hi) {
    /* uint 64 */
    return mpack_w1(buf, buflen, 0xcf) ||
           mpack_w4(buf, buflen, hi)   ||
           mpack_w4(buf, buflen, lo);
  } else if (lo > 0xffff) {
    /* uint 32 */
    return mpack_w1(buf, buflen, 0xce) ||
           mpack_w4(buf, buflen, lo);
  } else if (lo > 0xff) {
    /* uint 16 */
    return mpack_w1(buf, buflen, 0xcd) ||
           mpack_w2(buf, buflen, lo);
  } else if (lo > 0x7f) {
    /* uint 8 */
    return mpack_w1(buf, buflen, 0xcc) ||
           mpack_w1(buf, buflen, lo);
  } else {
    return mpack_w1(buf, buflen, lo);
  }
}

static int mpack_wnint(char **buf, size_t *buflen, mpack_value_t val)
{
  mpack_uint32_t hi = val.hi;
  mpack_uint32_t lo = val.lo;

  if (lo <= 0x80000000) {
    /* int 64 */
    return mpack_w1(buf, buflen, 0xd3) ||
           mpack_w4(buf, buflen, hi)   ||
           mpack_w4(buf, buflen, lo);
  } else if (lo <= 0xffff7fff) {
    /* int 32 */
    return mpack_w1(buf, buflen, 0xd2) ||
           mpack_w4(buf, buflen, lo);
  } else if (lo <= 0xffffff7f) {
    /* int 16 */
    return mpack_w1(buf, buflen, 0xd1) ||
           mpack_w2(buf, buflen, lo);
  } else if (lo <= 0xffffffe0) {
    /* int 8 */
    return mpack_w1(buf, buflen, 0xd0) ||
           mpack_w1(buf, buflen, lo);
  } else {
    /* negative fixint */
    return mpack_w1(buf, buflen, (mpack_uint32_t)(0x100 + lo));
  }
}

static int mpack_wfloat(char **buf, size_t *buflen,
    const mpack_token_t *tok)
{
  if (tok->length == 4) {
    return mpack_w1(buf, buflen, 0xca) ||
           mpack_w4(buf, buflen, tok->data.value.lo);
  } else if (tok->length == 8) {
    return mpack_w1(buf, buflen, 0xcb) ||
           mpack_w4(buf, buflen, tok->data.value.hi) ||
           mpack_w4(buf, buflen, tok->data.value.lo);
  } else {
    return MPACK_ERROR;
  }
}

static int mpack_wstr(char **buf, size_t *buflen, mpack_uint32_t len)
{
  if (len < 0x20) {
    return mpack_w1(buf, buflen, 0xa0 | len);
  } else if (len < 0x100) {
    return mpack_w1(buf, buflen, 0xd9) ||
           mpack_w1(buf, buflen, len);
  } else if (len < 0x10000) {
    return mpack_w1(buf, buflen, 0xda) ||
           mpack_w2(buf, buflen, len);
  } else {
    return mpack_w1(buf, buflen, 0xdb) ||
           mpack_w4(buf, buflen, len);
  }
}

static int mpack_wbin(char **buf, size_t *buflen, mpack_uint32_t len)
{
  if (len < 0x100) {
    return mpack_w1(buf, buflen, 0xc4) ||
           mpack_w1(buf, buflen, len);
  } else if (len < 0x10000) {
    return mpack_w1(buf, buflen, 0xc5) ||
           mpack_w2(buf, buflen, len);
  } else {
    return mpack_w1(buf, buflen, 0xc6) ||
           mpack_w4(buf, buflen, len);
  }
}

static int mpack_wext(char **buf, size_t *buflen, int type,
    mpack_uint32_t len)
{
  mpack_uint32_t t;
  assert(type >= 0 && type < 0x80);
  t = (mpack_uint32_t)type;
  switch (len) {
    case 1: mpack_w1(buf, buflen, 0xd4); return mpack_w1(buf, buflen, t);
    case 2: mpack_w1(buf, buflen, 0xd5); return mpack_w1(buf, buflen, t);
    case 4: mpack_w1(buf, buflen, 0xd6); return mpack_w1(buf, buflen, t);
    case 8: mpack_w1(buf, buflen, 0xd7); return mpack_w1(buf, buflen, t);
    case 16: mpack_w1(buf, buflen, 0xd8); return mpack_w1(buf, buflen, t);
    default:
      if (len < 0x100) {
        return mpack_w1(buf, buflen, 0xc7) ||
               mpack_w1(buf, buflen, len)  ||
               mpack_w1(buf, buflen, t);
      } else if (len < 0x10000) {
        return mpack_w1(buf, buflen, 0xc8) ||
               mpack_w2(buf, buflen, len)  ||
               mpack_w1(buf, buflen, t);
      } else {
        return mpack_w1(buf, buflen, 0xc9) ||
               mpack_w4(buf, buflen, len)  ||
               mpack_w1(buf, buflen, t);
      }
  }
}

static int mpack_warray(char **buf, size_t *buflen, mpack_uint32_t len)
{
  if (len < 0x10) {
    return mpack_w1(buf, buflen, 0x90 | len);
  } else if (len < 0x10000) {
    return mpack_w1(buf, buflen, 0xdc) ||
           mpack_w2(buf, buflen, len);
  } else {
    return mpack_w1(buf, buflen, 0xdd) ||
           mpack_w4(buf, buflen, len);
  }
}

static int mpack_wmap(char **buf, size_t *buflen, mpack_uint32_t len)
{
  if (len < 0x10) {
    return mpack_w1(buf, buflen, 0x80 | len);
  } else if (len < 0x10000) {
    return mpack_w1(buf, buflen, 0xde) ||
           mpack_w2(buf, buflen, len);
  } else {
    return mpack_w1(buf, buflen, 0xdf) ||
           mpack_w4(buf, buflen, len);
  }
}

static int mpack_w1(char **b, size_t *bl, mpack_uint32_t v)
{
  (*bl)--;
  *(*b)++ = (char)(v & 0xff);
  return MPACK_OK;
}

static int mpack_w2(char **b, size_t *bl, mpack_uint32_t v)
{
  *bl -= 2;
  *(*b)++ = (char)((v >> 8) & 0xff);
  *(*b)++ = (char)(v & 0xff);
  return MPACK_OK;
}

static int mpack_w4(char **b, size_t *bl, mpack_uint32_t v)
{
  *bl -= 4;
  *(*b)++ = (char)((v >> 24) & 0xff);
  *(*b)++ = (char)((v >> 16) & 0xff);
  *(*b)++ = (char)((v >> 8) & 0xff);
  *(*b)++ = (char)(v & 0xff);
  return MPACK_OK;
}

static int mpack_value(mpack_token_type_t type, mpack_uint32_t length,
    mpack_value_t value, mpack_token_t *tok)
{
  tok->type = type;
  tok->length = length;
  tok->data.value = value;
  return MPACK_OK;
}

static int mpack_blob(mpack_token_type_t type, mpack_uint32_t length,
    int ext_type, mpack_token_t *tok)
{
  tok->type = type;
  tok->length = length;
  tok->data.ext_type = ext_type;
  return MPACK_OK;
}

static mpack_value_t mpack_byte(unsigned char byte)
{
  mpack_value_t rv;
  rv.lo = byte;
  rv.hi = 0;
  return rv;
}

static int mpack_fits_single(double v);
static mpack_value_t mpack_pack_ieee754(double v, unsigned m, unsigned e);
static int mpack_is_be(void) FPURE;
static double mpack_fmod_pow2_32(double a);


#define POW2(n) \
  ((double)(1 << (n / 2)) * (double)(1 << (n / 2)) * (double)(1 << (n % 2)))

#define MPACK_SWAP_VALUE(val)                                  \
  do {                                                         \
    mpack_uint32_t lo = val.lo;                                \
    val.lo = val.hi;                                           \
    val.hi = lo;                                               \
  } while (0)

MPACK_API mpack_token_t mpack_pack_nil(void)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_NIL;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_boolean(unsigned v)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_BOOLEAN;
  rv.data.value.lo = v ? 1 : 0;
  rv.data.value.hi = 0;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_uint(mpack_uintmax_t v)
{
  mpack_token_t rv;
  rv.data.value.lo = v & 0xffffffff;
  rv.data.value.hi = (mpack_uint32_t)((v >> 31) >> 1);
  rv.type = MPACK_TOKEN_UINT;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_sint(mpack_sintmax_t v)
{
  if (v < 0) {
    mpack_token_t rv;
    mpack_uintmax_t tc = -((mpack_uintmax_t)(v + 1)) + 1;
    tc = ~tc + 1;
    rv = mpack_pack_uint(tc);
    rv.type = MPACK_TOKEN_SINT;
    return rv;
  }

  return mpack_pack_uint((mpack_uintmax_t)v);
}

MPACK_API mpack_token_t mpack_pack_int32(int v){
  mpack_token_t rv;
  rv.data.value.lo = v;
  rv.data.value.hi = 0;
  if (v >= 0){
    rv.type = MPACK_TOKEN_UINT;
  } else {
    rv.type = MPACK_TOKEN_SINT;
  }
  return rv;
}

MPACK_API mpack_token_t mpack_pack_float_compat(double v)
{
  /* ieee754 single-precision limits to determine if "v" can be fully
   * represented in 4 bytes */
  mpack_token_t rv;

  if (mpack_fits_single(v)) {
    rv.length = 4;
    rv.data.value = mpack_pack_ieee754(v, 23, 8);
  } else {
    rv.length = 8;
    rv.data.value = mpack_pack_ieee754(v, 52, 11);
  }

  rv.type = MPACK_TOKEN_FLOAT;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_float_fast(double v)
{
  /* ieee754 single-precision limits to determine if "v" can be fully
   * represented in 4 bytes */
  mpack_token_t rv;

  if (mpack_fits_single(v)) {
    union {
      float f;
      mpack_uint32_t m;
    } conv;
    conv.f = (float)v;
    rv.length = 4;
    rv.data.value.lo = conv.m;
    rv.data.value.hi = 0;
  } else {
    union {
      double d;
      mpack_value_t m;
    } conv;
    conv.d = v;
    rv.length = 8;
    rv.data.value = conv.m;
    if (mpack_is_be()) {
      MPACK_SWAP_VALUE(rv.data.value);
    }
  }

  rv.type = MPACK_TOKEN_FLOAT;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_number(double v)
{
  mpack_token_t tok;
  double vabs;
  vabs = v < 0 ? -v : v;
  assert(v <= 9007199254740991. && v >= -9007199254740991.);
  tok.data.value.hi = (mpack_uint32_t)(vabs / POW2(32));
  tok.data.value.lo = (mpack_uint32_t)mpack_fmod_pow2_32(vabs);

  if (v < 0) {
    /* Compute the two's complement */
    tok.type = MPACK_TOKEN_SINT;
    tok.data.value.hi = ~tok.data.value.hi;
    tok.data.value.lo = ~tok.data.value.lo + 1;
    if (!tok.data.value.lo) tok.data.value.hi++;
    if (tok.data.value.lo == 0 && tok.data.value.hi == 0) tok.length = 1;
    else if (tok.data.value.lo < 0x80000000) tok.length = 8;
    else if (tok.data.value.lo < 0xffff7fff) tok.length = 4;
    else if (tok.data.value.lo < 0xffffff7f) tok.length = 2;
    else tok.length = 1;
  } else {
    tok.type = MPACK_TOKEN_UINT;
    if (tok.data.value.hi) tok.length = 8;
    else if (tok.data.value.lo > 0xffff) tok.length = 4;
    else if (tok.data.value.lo > 0xff) tok.length = 2;
    else tok.length = 1;
  }

  if (mpack_unpack_number(tok) != v) {
    return mpack_pack_float(v);
  }

  return tok;
}

MPACK_API mpack_token_t mpack_pack_chunk(const char *p, mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_CHUNK;
  rv.data.chunk_ptr = p;
  rv.length = l;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_str(mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_STR;
  rv.length = l;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_bin(mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_BIN;
  rv.length = l;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_ext(int t, mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_EXT;
  rv.length = l;
  rv.data.ext_type = t;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_array(mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_ARRAY;
  rv.length = l;
  return rv;
}

MPACK_API mpack_token_t mpack_pack_map(mpack_uint32_t l)
{
  mpack_token_t rv;
  rv.type = MPACK_TOKEN_MAP;
  rv.length = l;
  return rv;
}

MPACK_API bool mpack_unpack_boolean(mpack_token_t t)
{
  return t.data.value.lo || t.data.value.hi;
}

MPACK_API mpack_uintmax_t mpack_unpack_uint(mpack_token_t t)
{
  return (((mpack_uintmax_t)t.data.value.hi << 31) << 1) | t.data.value.lo;
}

MPACK_API int mpack_unpack_uint32(mpack_token_t t)
{
  return (int) t.data.value.lo;
}

/* unpack signed integer without relying on two's complement as internal
 * representation */
MPACK_API mpack_sintmax_t mpack_unpack_sint(mpack_token_t t)
{
  mpack_uint32_t hi = t.data.value.hi;
  mpack_uint32_t lo = t.data.value.lo;
  mpack_uintmax_t rv = lo;
  // This assert fails because length is not initialized
  // The problem has probably gone undetected because asserts disappear with -O
  /* assert(t.length <= sizeof(mpack_sintmax_t)); */

  if (t.length == 8) {
    rv |= (((mpack_uintmax_t)hi) << 31) << 1;
  }

  /* reverse the two's complement so that lo/hi contain the absolute value.
   * note that we have to mask ~rv so that it reflects the two's complement
   * of the appropriate byte length */
  rv = (~rv & (((mpack_uintmax_t)1 << ((t.length * 8) - 1)) - 1)) + 1;
  /* negate and return the absolute value, making sure mpack_sintmax_t can
   * represent the positive cast. */
  return -((mpack_sintmax_t)(rv - 1)) - 1;
}

MPACK_API int mpack_unpack_sint32(mpack_token_t t)
{
  int rv = (int)t.data.value.lo;
  rv = ~rv + 1;
  return -(rv - 1) - 1;
}


MPACK_API double mpack_unpack_float_compat(mpack_token_t t)
{
  mpack_uint32_t sign;
  mpack_sint32_t exponent, bias;
  unsigned mantbits;
  unsigned expbits;
  double mant;

  if (t.data.value.lo == 0 && t.data.value.hi == 0)
    /* nothing to do */
    return 0;

  if (t.length == 4) mantbits = 23, expbits = 8;
  else mantbits = 52, expbits = 11;
  bias = (1 << (expbits - 1)) - 1;

  /* restore sign/exponent/mantissa */
  if (mantbits == 52) {
    sign = t.data.value.hi >> 31;
    exponent = (t.data.value.hi >> 20) & ((1 << 11) - 1);
    mant = (t.data.value.hi & ((1 << 20) - 1)) * POW2(32);
    mant += t.data.value.lo;
  } else {
    sign = t.data.value.lo >> 31;
    exponent = (t.data.value.lo >> 23) & ((1 << 8) - 1);
    mant = t.data.value.lo & ((1 << 23) - 1);
  }

  mant /= POW2(mantbits);
  if (exponent) mant += 1.0; /* restore leading 1 */
  else exponent = 1; /* subnormal */
  exponent -= bias;

  /* restore original value */
  while (exponent > 0) mant *= 2.0, exponent--;
  while (exponent < 0) mant /= 2.0, exponent++;
  return mant * (sign ? -1 : 1);
}

MPACK_API double mpack_unpack_float_fast(mpack_token_t t)
{
  if (t.length == 4) {
    union {
      float f;
      mpack_uint32_t m;
    } conv;
    conv.m = t.data.value.lo;
    return conv.f;
  } else {
    union {
      double d;
      mpack_value_t m;
    } conv;
    conv.m = t.data.value;

    if (mpack_is_be()) {
      MPACK_SWAP_VALUE(conv.m);
    }

    return conv.d;
  }
}

MPACK_API double mpack_unpack_number(mpack_token_t t)
{
  double rv;
  mpack_uint32_t hi, lo;
  if (t.type == MPACK_TOKEN_FLOAT) return mpack_unpack_float(t);
  assert(t.type == MPACK_TOKEN_UINT || t.type == MPACK_TOKEN_SINT);
  hi = t.data.value.hi;
  lo = t.data.value.lo;
  if (t.type == MPACK_TOKEN_SINT) {
    /* same idea as mpack_unpack_sint, except here we shouldn't rely on
     * mpack_uintmax_t having 64-bits, operating on the 32-bit words separately.
     */
    if (!hi) {
      assert(t.length <= 4);
      hi = 0;
      lo = (~lo & (((mpack_uint32_t)1 << ((t.length * 8) - 1)) - 1));
    } else {
      hi = ~hi;
      lo = ~lo;
    }
    lo++;
    if (!lo) hi++;
  }
  rv = (double)lo + POW2(32) * hi;
  return t.type == MPACK_TOKEN_SINT ? -rv : rv;
}

static int mpack_fits_single(double v)
{
  return (float)v == v;
}

static mpack_value_t mpack_pack_ieee754(double v, unsigned mantbits,
    unsigned expbits)
{
  mpack_value_t rv = {0, 0};
  mpack_sint32_t exponent, bias = (1 << (expbits - 1)) - 1;
  mpack_uint32_t sign;
  double mant;

  if (v == 0) {
    rv.lo = 0;
    rv.hi = 0;
    goto end;
  }

  if (v < 0) sign = 1, mant = -v;
  else sign = 0, mant = v;

  exponent = 0;
  while (mant >= 2.0) mant /= 2.0, exponent++;
  while (mant < 1.0 && exponent > -(bias - 1)) mant *= 2.0, exponent--;

  if (mant < 1.0) exponent = -bias; /* subnormal value */
  else mant = mant - 1.0; /* remove leading 1 */
  exponent += bias;
  mant *= POW2(mantbits);

  if (mantbits == 52) {
    rv.hi = (mpack_uint32_t)(mant / POW2(32));
    rv.lo = (mpack_uint32_t)(mant - rv.hi * POW2(32));
    rv.hi |= ((mpack_uint32_t)exponent << 20) | (sign << 31);
  } else if (mantbits == 23) {
    rv.hi = 0;
    rv.lo = (mpack_uint32_t)mant;
    rv.lo |= ((mpack_uint32_t)exponent << 23) | (sign << 31);
  }

end:
  return rv;
}

static int mpack_is_be(void)
{
  union {
    mpack_uint32_t i;
    char c[sizeof(mpack_uint32_t)];
  } test;

  test.i = 1;
  return test.c[0] == 0;
}

/* this simplified version of `fmod` that returns the remainder of double
 * division by 0xffffffff, which is enough for our purposes */
static double mpack_fmod_pow2_32(double a)
{
  return a - ((double)(mpack_uint32_t)(a / POW2(32)) * POW2(32));
}


#endif // end __MPACK_H__



#ifndef __MLCMPACK_HEADER_ONLY_H__
#define __MLCMPACK_HEADER_ONLY_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Forward declarations
struct Schema;

typedef enum {
  MORLOC_NIL,
  MORLOC_BOOL,
  MORLOC_SINT8,
  MORLOC_SINT16,
  MORLOC_SINT32,
  MORLOC_SINT64,
  MORLOC_UINT8,
  MORLOC_UINT16,
  MORLOC_UINT32,
  MORLOC_UINT64,
  MORLOC_FLOAT32,
  MORLOC_FLOAT64,
  MORLOC_STRING,
  MORLOC_ARRAY,
  MORLOC_TUPLE,
  MORLOC_MAP
} morloc_serial_type;

#define SCHEMA_NIL    'z'
#define SCHEMA_BOOL   'b'
#define SCHEMA_SINT   'i'
#define SCHEMA_UINT   'u'
#define SCHEMA_FLOAT  'f'
#define SCHEMA_STRING 's'
#define SCHEMA_ARRAY  'a'
#define SCHEMA_TUPLE  't'
#define SCHEMA_MAP    'm'

#define BUFFER_SIZE 4096

// Schema definition
//  * Primitives have no parameters
//  * Arrays have one
//  * Tuples and records have one or more
typedef struct Schema {
    morloc_serial_type type;
    size_t size; // number of parameters
    size_t width; // bytes in the object when stored in an array
    size_t* offsets;
    struct Schema** parameters;
    char** keys; // field names, used only for records
} Schema;

typedef struct Array {
  size_t size;
  void* data;
} Array;

// Prototypes

Schema* parse_schema(const char** schema_ptr);

// Main pack function for creating morloc-encoded MessagePack data
int pack(const void* mlc, const char* schema_str, char** mpkptr, size_t* mpk_size);
int pack_with_schema(const void* mlc, const Schema* schema, char** mpkptr, size_t* mpk_size);

int unpack(const char* mpk, size_t mpk_size, const char* schema_str, void** mlcptr);
int unpack_with_schema(const char* mpk, size_t mpk_size, const Schema* schema, void** mlcptr);


// Helper function to create a schema with parameters
Schema* create_schema_with_params(morloc_serial_type type, size_t width, size_t size, Schema** params, char** keys) {
    Schema* schema = (Schema*)malloc(sizeof(Schema));
    if (!schema) return NULL;

    schema->type = type;
    schema->size = size;
    schema->width = width;
    schema->offsets = NULL;
    schema->parameters = params;
    schema->keys = keys;

    // for tuples and maps, generate the element offsets
    if(params){
      schema->offsets = (size_t*)calloc(size, sizeof(size_t));
      schema->offsets[0] = 0;
      for(size_t i = 1; i < size; i++){
        schema->offsets[i] += params[i-1]->width;
      }
    }

    return schema;
}

Schema* nil_schema() {
    return create_schema_with_params(MORLOC_NIL, 1, 0, NULL, NULL);
}

Schema* bool_schema() {
    return create_schema_with_params(MORLOC_BOOL, 1, 0, NULL, NULL);
}

Schema* uint_schema(size_t width) {
    switch(width){
      case 1:
        return create_schema_with_params(MORLOC_UINT8, width, 0, NULL, NULL);
      case 2:
        return create_schema_with_params(MORLOC_UINT16, width, 0, NULL, NULL);
      case 4:
        return create_schema_with_params(MORLOC_UINT32, width, 0, NULL, NULL);
      case 8:
        return create_schema_with_params(MORLOC_UINT64, width, 0, NULL, NULL);
      default:
        fprintf(stderr, "Integers may only have widths of 1, 2, 4, or 8 bytes; found %uz", width);
        return NULL;
    }
}

Schema* sint_schema(size_t width) {
    switch(width){
      case 1:
        return create_schema_with_params(MORLOC_SINT8, width, 0, NULL, NULL);
      case 2:
        return create_schema_with_params(MORLOC_SINT16, width, 0, NULL, NULL);
      case 4:
        return create_schema_with_params(MORLOC_SINT32, width, 0, NULL, NULL);
      case 8:
        return create_schema_with_params(MORLOC_SINT64, width, 0, NULL, NULL);
      default:
        fprintf(stderr, "Integers may only have widths of 1, 2, 4, or 8 bytes; found %uz", width);
        return NULL;
    }
}

Schema* float_schema(size_t width) {
    switch(width){
      case 4:
        return create_schema_with_params(MORLOC_FLOAT32, width, 0, NULL, NULL);
      case 8:
        return create_schema_with_params(MORLOC_FLOAT64, width, 0, NULL, NULL);
      default:
        fprintf(stderr, "Floats may only have widths of 4 or 8 bytes, found %uz\n", width);
        return NULL;
    }
}

Schema* string_schema() {
    return create_schema_with_params(MORLOC_STRING, sizeof(Array), 0, NULL, NULL);
}

Schema* tuple_schema(Schema** params, size_t size) {
    size_t width = 0;
    for(size_t i = 0; i < size; i++){
      width += params[i]->width;
    }
    return create_schema_with_params(MORLOC_TUPLE, width, size, params, NULL);
}


Schema* array_schema(Schema* array_type) {
    Schema** params = (Schema**)malloc(sizeof(Schema*));
    if (!params) return NULL;

    params[0] = array_type;

    return create_schema_with_params(MORLOC_ARRAY, sizeof(Array), 1, params, NULL);
}

Schema* map_schema(size_t size, char** keys, Schema** params) {
    size_t width = 0;
    for(size_t i = 0; i < size; i++){
      width += params[i]->width;
    }
    return create_schema_with_params(MORLOC_MAP, width, size, params, keys);
}


void* nil_data(){
  int8_t* nil = (int8_t*)malloc(sizeof(int8_t));
  return nil;
}

void* bool_data(bool x){
  bool* xptr = (bool*)malloc(sizeof(bool));
  *xptr = x;
  return xptr;
}

void* uint8_data(uint8_t x){
  uint8_t* xptr = (uint8_t*)malloc(sizeof(uint8_t));
  *xptr = x;
  return xptr;
}

void* uint16_data(uint16_t x){
  uint16_t* xptr = (uint16_t*)malloc(sizeof(uint16_t));
  *xptr = x;
  return xptr;
}

void* uint32_data(uint32_t x){
  uint32_t* xptr = (uint32_t*)malloc(sizeof(uint32_t));
  *xptr = x;
  return xptr;
}

void* uint64_data(uint64_t x){
  uint64_t* xptr = (uint64_t*)malloc(sizeof(uint64_t));
  *xptr = x;
  return xptr;
}

void* sint8_data(int8_t x){
  int8_t* xptr = (int8_t*)malloc(sizeof(int8_t));
  *xptr = x;
  return xptr;
}

void* sint16_data(int16_t x){
  int16_t* xptr = (int16_t*)malloc(sizeof(int16_t));
  *xptr = x;
  return xptr;
}

void* sint32_data(int32_t x){
  int32_t* xptr = (int32_t*)malloc(sizeof(int32_t));
  *xptr = x;
  return xptr;
}

void* sint64_data(int64_t x){
  int64_t* xptr = (int64_t*)malloc(sizeof(int64_t));
  *xptr = x;
  return xptr;
}

void* float32_data(float x){
  float* xptr = (float*)malloc(sizeof(float));
  *xptr = x;
  return xptr;
}

void* float64_data(double x){
  double* xptr = (double*)malloc(sizeof(double));
  *xptr = x;
  return xptr;
}

void* string_data(const char* x, size_t size){
  Array* data = (Array*)malloc(sizeof(Array));
  data->size = size;
  // TODO - only copy if the string data is not already in shared memory
  data->data = (char*)malloc(size * sizeof(char));
  memcpy(data->data, x, size);
  return data;
}

void* tuple_data_(Schema** params, size_t size, size_t width){
  return (void*)malloc(width);
}

Array* array_data(void* data, size_t width, size_t size){
  Array* array;
  if(data){
      array = (Array*)data;
  } else {
      array = (Array*)malloc(sizeof(Array));
  }
  array->size = size;
  array->data = (void*)malloc(size * width);
  return array;
}


// packing ####

// Try to add `added_size` bytes of space to a buffer, if there is not enough
// space, increase the buffer size.
void upsize(
  char** data,            // data that will be resized
  char** data_ptr,        // pointer that will be updated to preserve offset
  size_t* remaining_size, // remaining data size
  size_t added_size       // the number of bytes that need to be added
){
    // check if any action is needed
    if (added_size <= *remaining_size) {
        return;
    }

    size_t used_size = *data_ptr - *data;
    size_t buffer_size = used_size + *remaining_size;

    // find an appropriate size for the new data
    while (added_size > *remaining_size) {
        if (buffer_size > SIZE_MAX / 2) {
            buffer_size += BUFFER_SIZE;
        } else {
            buffer_size *= 2;
        }
        *remaining_size = buffer_size - used_size;
    }

    // allocate memory for the new data
    *data = (char*)realloc(*data, buffer_size);

    // point old pointer to the same offset in the new data
    *data_ptr = *data + used_size;
}


// write data to a packet, if the buffer is too small, increase its size
void write_to_packet(
  const void* src,                // source data
  char** packet,            // destination
  char** packet_ptr,        // location in the destination that will be written to
  size_t* packet_remaining, // remaining data size
  size_t size               // the number of bytes to write

){
    upsize(packet, packet_ptr, packet_remaining, size);
    memcpy(*packet_ptr, src, size);
    *packet_ptr += size;
    *packet_remaining -= size;
}


// write a token to a packet, increase buffer size as needed
int dynamic_mpack_write(
  mpack_tokbuf_t* tokbuf,
  char** packet,
  char** packet_ptr,
  size_t* packet_remaining,
  mpack_token_t* token,
  size_t extra_size
) {
    int result = 0;
    if(*packet_remaining <= 0){
        upsize(packet, packet_ptr, packet_remaining, 1);
    }
    result = mpack_write(tokbuf, packet_ptr, packet_remaining, token);
    if (result == MPACK_EOF || *packet_remaining == 0) {
        upsize(packet, packet_ptr, packet_remaining, token->length + extra_size);
        if (result == MPACK_EOF) {
            mpack_write(tokbuf, packet_ptr, packet_remaining, token);
        }
    }
    return result;
}

size_t parse_schema_size(const char** schema_ptr){
  char c = **schema_ptr;
  size_t size =
    // characters 0-9 are integers 0-9
    (c >= 0x30 && c <= 0x39) * (c - 0x30) +
    // characters a-z are integers 10-35
    (c >= 0x61 && c <= 0x7a) * (c - 0x61 + 10) +
    // characters A-Z are integers 36-61
    (c >= 0x41 && c <= 0x5a) * (c - 0x41 + 36) +
    // '+' is 62
    (c == '+') * 62 +
    // '/' is 63
    (c == '/') * 63;
  (*schema_ptr)++;
  return size;
}

char* parse_schema_key(const char** schema_ptr){
  size_t key_size = parse_schema_size(schema_ptr);
  char* key = (char*)calloc(key_size+1, sizeof(char));
  memcpy(key, *schema_ptr, key_size);
  *schema_ptr += key_size;
  return key;
}

Schema* parse_schema(const char** schema_ptr){
  Schema* schema = 0;
  Schema** params;
  char c = **schema_ptr;
  (*schema_ptr)++;
  size_t size;
  char** keys; 

  switch(c){
    case SCHEMA_ARRAY:
      return array_schema(parse_schema(schema_ptr));
    case SCHEMA_TUPLE:
      size = parse_schema_size(schema_ptr);
      params = (Schema**)calloc(size, sizeof(Schema*));
      for(size_t i = 0; i < size; i++){
        params[i] = parse_schema(schema_ptr);
      }
      return tuple_schema(params, size);
    case SCHEMA_MAP:
      size = parse_schema_size(schema_ptr);
      keys = (char**)calloc(size, sizeof(char*));
      params = (Schema**)calloc(size, sizeof(Schema*));
      for(size_t i = 0; i < size; i++){
        keys[i] = parse_schema_key(schema_ptr);
        params[i] = parse_schema(schema_ptr);
      }
      return map_schema(size, keys, params);
    case SCHEMA_NIL:
      return nil_schema();
    case SCHEMA_BOOL:
      return bool_schema();
    case SCHEMA_SINT:
      size = parse_schema_size(schema_ptr);
      return sint_schema(size);
    case SCHEMA_UINT:
      size = parse_schema_size(schema_ptr);
      return uint_schema(size);
    case SCHEMA_FLOAT:
      size = parse_schema_size(schema_ptr);
      return float_schema(size);
    case SCHEMA_STRING:
      return string_schema();
    default:
      fprintf(stderr, "Unrecognized schema type '%c'\n", c);
      return NULL;
  }

  return NULL;
}

//  The main function for writing MessagePack
int pack_data(
  const void* mlc,           // input data structure
  const Schema* schema,      // input data schema
  char** packet,             // a pointer to the messagepack data
  char** packet_ptr,         // the current position in the buffer
  size_t* packet_remaining,  // bytes from current position to the packet end
  mpack_tokbuf_t* tokbuf
) {
    mpack_token_t token;
    int result;

    switch (schema->type) {
        case MORLOC_NIL:
            token = mpack_pack_nil();
            break;
        case MORLOC_BOOL:
            token = mpack_pack_boolean(*(int8_t*)mlc != 0);
            break;
        case MORLOC_UINT8:
        case MORLOC_UINT16:
        case MORLOC_UINT32:
        case MORLOC_SINT8:
        case MORLOC_SINT16:
        case MORLOC_SINT32:
            token = mpack_pack_int32(*(int32_t*)mlc);
            break;
        case MORLOC_UINT64:
        case MORLOC_SINT64:
            // TODO: fix the bug in 64 bit handling in the mpack library
            perror("Not yet supported");
            return 1;
            break;
        case MORLOC_FLOAT32:
        case MORLOC_FLOAT64:
            token = mpack_pack_float(*(double*)mlc);
            break;
        case MORLOC_STRING:
            token = mpack_pack_str(((Array*)mlc)->size);
            break;
        case MORLOC_ARRAY:
            token = mpack_pack_array(schema->parameters[0]->width);
            break;
        case MORLOC_TUPLE:
            token = mpack_pack_array(schema->size);
            break;
        case MORLOC_MAP:
            token = mpack_pack_map(schema->size);
            break;
        default:
            fprintf(stderr, "Unexpected morloc type\n");
            return 1;
    }

    dynamic_mpack_write(tokbuf, packet, packet_ptr, packet_remaining, &token, 0);

    size_t array_length;
    size_t array_width;
    size_t element_idx;
    Schema* array_schema;
    Array* array;

    switch(schema->type){
      case MORLOC_STRING:
        array = (Array*)mlc;
        write_to_packet(array->data, packet, packet_ptr, packet_remaining, array->size);
        break;
      case MORLOC_ARRAY:
        array_length = ((Array*)mlc)->size;
        array_schema = schema->parameters[0];
        array_width = array_schema->width;
        for (size_t i = 0; i < array_length; i++) {
            pack_data(
              (char*)mlc + i * array_width,
              array_schema,
              packet,
              packet_ptr,
              packet_remaining,
              tokbuf
            );
        }
        break;
      case MORLOC_TUPLE:
        // tuples are unboxed data
        // each element in the tuple is represented by element->width bytes
        element_idx = 0;
        for (size_t i = 0; i < schema->size; i++) {
            pack_data(
              (char*)mlc + element_idx,
              schema->parameters[i],
              packet,
              packet_ptr,
              packet_remaining,
              tokbuf
            );
            // move the index to the location after the current element
            element_idx += schema->parameters[i]->width;
        }
        break;
      case MORLOC_MAP:
        // map memory representation is equivalent to tuples
        // the keys exist only in the schema
        element_idx = 0;
        for (size_t i = 0; i < schema->size; i++) {

            char* key = schema->keys[i];
            size_t key_len = strlen(key);

            // write key string token
            token = mpack_pack_str(key_len);
            dynamic_mpack_write(tokbuf, packet, packet_ptr, packet_remaining, &token, 8);

            // write string bytes
            write_to_packet((void*)key, packet, packet_ptr, packet_remaining, key_len);

            // write value
            pack_data((char*)mlc + element_idx, schema->parameters[i], packet, packet_ptr, packet_remaining, tokbuf);

            // move the index to the location after the current element
            element_idx += schema->parameters[i]->width;
        }
        break;
    }

    return 0;
}



int pack_with_schema(const void* mlc, const Schema* schema, char** packet, size_t* packet_size) {
    *packet_size = 0;

    *packet = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (*packet == NULL) return 1;
    size_t packet_remaining = BUFFER_SIZE;
    char* packet_ptr = *packet;

    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);

    int pack_result = pack_data(mlc, schema, packet, &packet_ptr, &packet_remaining, &tokbuf);

    // mutate packet_size (will be used outside)
    *packet_size = packet_ptr - *packet;

    // Trim the output buffer to the exact size needed
    if (packet_remaining > 0) {
        *packet = (char*)realloc(*packet, *packet_size);
    }

    return pack_result;
}

int pack(const void* mlc, const char* schema_str, char** mpk, size_t* mpk_size) {
    Schema* schema = parse_schema(&schema_str);
    return pack_with_schema(mlc, schema, mpk, mpk_size);
}



// terminal parsers
int parse_bool(        void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_nil(         void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_bytes(       void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_int(    int, void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_float(  int, void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);

// nested parsers
int parse_array( void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_map(   void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_tuple( void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);
int parse_obj(   void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token);

int parse_nil(void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    int8_t result = 0;
    memcpy(mlc, &result, sizeof(int8_t));
    return 0;
}

int parse_bool(void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    bool result = mpack_unpack_boolean(*token);
    memcpy(mlc, &result, sizeof(bool));
    return 0;
}

int parse_int(int schema_type, void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    int64_t result;
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    switch(token->type){
      case MPACK_TOKEN_UINT:
        switch(schema_type){
          case MORLOC_UINT8:
            result = (uint8_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_UINT16:
            result = (uint16_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_UINT32:
            result = (uint32_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_UINT64:
            result = (uint64_t)mpack_unpack_uint32(*token);
            break;
          default:
            break;
        }
        break;
      case MPACK_TOKEN_SINT:
        switch(schema_type){
          case MORLOC_SINT8:
            result = (int8_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_SINT16:
            result = (int16_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_SINT32:
            result = (int32_t)mpack_unpack_uint32(*token);
            break;
          case MORLOC_SINT64:
            result = (int64_t)mpack_unpack_uint32(*token);
            break;
          default:
            break;
        }
        break;
      default:
        fprintf(stderr, "Bad token %d\n", token->type);
        return 1;
    }
    memcpy(mlc, &result, sizeof(int64_t));
    return 0;
}

int parse_float(int schema_type, void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    if(schema_type == MORLOC_FLOAT32){
      float result = (float)mpack_unpack_float(*token);
      memcpy(mlc, &result, sizeof(float));
    } else {
      double result = (double)mpack_unpack_float(*token);
      memcpy(mlc, &result, sizeof(double));
    }

    return 0;
}


char* parse_key(mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    size_t size = token->length;
    char* key = (char*)calloc(token->length + 1, sizeof(char));

    size_t str_idx = 0;

    while(size - str_idx > 0){
        mpack_read(tokbuf, buf_ptr, buf_remaining, token);
        memcpy(
          key + str_idx,
          token->data.chunk_ptr,
          token->length * sizeof(char)
        );
        str_idx += token->length;
    }

    return key;
}

int parse_bytes(void* mlc, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    Array* result = (Array*) mlc;

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    result->size = token->length;
    result->data = (char*)malloc(result->size * sizeof(char));

    size_t str_idx = 0;
    while((result->size - str_idx) > 0){
        mpack_read(tokbuf, buf_ptr, buf_remaining, token);
        memcpy(
          (char*)result->data + str_idx,
          token->data.chunk_ptr,
          token->length * sizeof(char)
        );
        str_idx += token->length;
    }
    return 0;
}

int parse_array(void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    int exitcode = 0;
    Array* result = (Array*) mlc;

    size_t element_size = schema->width;
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    result->size = token->length;
    result->data = (void*)calloc(result->size, element_size);

    for(size_t i = 0; i < result->size; i++){
        exitcode = parse_obj((char*)result->data + i * element_size, schema, tokbuf, buf_ptr, buf_remaining, token);
        if(exitcode != 0){
          return exitcode;
        }
    }
    return 0;
}

int parse_tuple(void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    size_t offset = 0;
    int exitcode = 0;

    mpack_read(tokbuf, buf_ptr, buf_remaining, token);

    for(size_t i = 0; i < schema->size; i++){
        exitcode = parse_obj((char*)mlc + offset, schema->parameters[i], tokbuf, buf_ptr, buf_remaining, token);
        if(exitcode != 0){
          return exitcode;
        }
        offset += schema->parameters[i]->width;
    }

    return 0;
}

int parse_map(void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    int exitcode = 0;
    size_t offset = 0;
    mpack_read(tokbuf, buf_ptr, buf_remaining, token);
    char* key;

    for(size_t i = 0; i < schema->size; i++){
        key = parse_key(tokbuf, buf_ptr, buf_remaining, token);;
        exitcode = parse_obj((char*)mlc + offset, schema->parameters[i], tokbuf, buf_ptr, buf_remaining, token);
        if(exitcode != 0){
          return exitcode;
        }
        offset += schema->parameters[i]->width;
    }

    return 0;
}

int parse_obj(void* mlc, const Schema* schema, mpack_tokbuf_t* tokbuf, const char** buf_ptr, size_t* buf_remaining, mpack_token_t* token){
    switch(schema->type){
      case MORLOC_NIL:
        return parse_nil(mlc, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_BOOL:
        return parse_bool(mlc, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_SINT8:
      case MORLOC_SINT16:
      case MORLOC_SINT32:
      case MORLOC_SINT64:
      case MORLOC_UINT8:
      case MORLOC_UINT16:
      case MORLOC_UINT32:
      case MORLOC_UINT64:
        return parse_int(schema->type, mlc, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_FLOAT32:
      case MORLOC_FLOAT64:
        return parse_float(schema->type, mlc, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_STRING:
        return parse_bytes(mlc, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_ARRAY:
        return parse_array(mlc, schema->parameters[0], tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_MAP:
        return parse_map(mlc, schema, tokbuf, buf_ptr, buf_remaining, token);
      case MORLOC_TUPLE:
        return parse_tuple(mlc, schema, tokbuf, buf_ptr, buf_remaining, token);
      default:
        return 1;
    }
}

int unpack_with_schema(const char* mgk, size_t mgk_size, const Schema* schema, void** mlcptr) {
    // Use the existing unpack_with_schema function, but adapt it to the new prototype
    const char* buf = mgk;
    size_t buf_remaining = mgk_size;

    mpack_tokbuf_t tokbuf;
    mpack_tokbuf_init(&tokbuf);
    mpack_token_t token;

    void* mlc = (void*)malloc(schema->width);

    int exitcode = parse_obj(mlc, schema, &tokbuf, &mgk, &buf_remaining, &token);

    *mlcptr = mlc;

    return exitcode;
}

int unpack(const char* mpk, size_t mpk_size, const char* schema_str, void** mlcptr) {
    const Schema* schema = parse_schema(&schema_str);
    return unpack_with_schema(mpk, mpk_size, schema, mlcptr);
}


#endif // ending __MLCMPACK_HEADER_ONLY_H__
