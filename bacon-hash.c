/*
 * Copyright (C) 2013 Nathan Forbes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * The methods of generating an MD5 message digest below were derived from
 * the RSA Data Security, Inc. MD5 Message-Digest Algorithm.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-hash.h"
#include "bacon-util.h"

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac)           \
  do                                       \
  {                                        \
    (a) += F ((b), (c), (d)) + (x) + (ac); \
    (a) = ROTATE_LEFT ((a), (s));          \
    (a) += (b);                            \
  } while (false)

#define GG(a, b, c, d, x, s, ac)           \
  do                                       \
  {                                        \
    (a) += G ((b), (c), (d)) + (x) + (ac); \
    (a) = ROTATE_LEFT ((a), (s));          \
    (a) += (b);                            \
  } while (false)

#define HH(a, b, c, d, x, s, ac)           \
  do                                       \
  {                                        \
    (a) += H ((b), (c), (d)) + (x) + (ac); \
    (a) = ROTATE_LEFT ((a), (s));          \
    (a) += (b);                            \
  } while (false)

#define II(a, b, c, d, x, s, ac)           \
  do                                       \
  {                                        \
    (a) += I ((b), (c), (d)) + (x) + (ac); \
    (a) = ROTATE_LEFT ((a), (s));          \
    (a) += (b);                            \
  } while (false)

typedef struct
{
  unsigned int state[4];
  unsigned int count[2];
  unsigned char buffer[64];
} BaconMd5Ctx;

static unsigned char padding[64] =
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void
bacon_hash_encode (unsigned char *o, unsigned int *i, unsigned int n)
{
  unsigned int x;
  unsigned int y;

  for (x = 0, y = 0; y < n; x++, y += 4)
  {
    o[y] = (unsigned char) (i[x] & 0xff);
    o[y + 1] = (unsigned char) ((i[x] >> 8) & 0xff);
    o[y + 2] = (unsigned char) ((i[x] >> 16) & 0xff);
    o[y + 3] = (unsigned char) ((i[x] >> 24) & 0xff);
  }
}

static void
bacon_hash_decode (unsigned int *o, unsigned char *i, unsigned int n)
{
  unsigned int x;
  unsigned int y;

  for (x = 0, y = 0; y < n; x++, y += 4)
    o[x] = ((unsigned int) i[y]) |
           (((unsigned int) i[y + 1]) << 8) |
           (((unsigned int) i[y + 2]) << 16) |
           (((unsigned int) i[y + 3]) << 24);
}

static void
bacon_hash_transform (unsigned int state[4], unsigned char block[64])
{
  unsigned int a;
  unsigned int b;
  unsigned int c;
  unsigned int d;
  unsigned int x[16];

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];

  bacon_hash_decode (x, block, 64);

  FF (a, b, c, d, x[0], S11, 0xd76aa478);
  FF (d, a, b, c, x[1], S12, 0xe8c7b756);
  FF (c, d, a, b, x[2], S13, 0x242070db);
  FF (b, c, d, a, x[3], S14, 0xc1bdceee);
  FF (a, b, c, d, x[4], S11, 0xf57c0faf);
  FF (d, a, b, c, x[5], S12, 0x4787c62a);
  FF (c, d, a, b, x[6], S13, 0xa8304613);
  FF (b, c, d, a, x[7], S14, 0xfd469501);
  FF (a, b, c, d, x[8], S11, 0x698098d8);
  FF (d, a, b, c, x[9], S12, 0x8b44f7af);
  FF (c, d, a, b, x[10], S13, 0xffff5bb1);
  FF (b, c, d, a, x[11], S14, 0x895cd7be);
  FF (a, b, c, d, x[12], S11, 0x6b901122);
  FF (d, a, b, c, x[13], S12, 0xfd987193);
  FF (c, d, a, b, x[14], S13, 0xa679438e);
  FF (b, c, d, a, x[15], S14, 0x49b40821);

  GG (a, b, c, d, x[1], S21, 0xf61e2562);
  GG (d, a, b, c, x[6], S22, 0xc040b340);
  GG (c, d, a, b, x[11], S23, 0x265e5a51);
  GG (b, c, d, a, x[0], S24, 0xe9b6c7aa);
  GG (a, b, c, d, x[5], S21, 0xd62f105d);
  GG (d, a, b, c, x[10], S22,  0x2441453);
  GG (c, d, a, b, x[15], S23, 0xd8a1e681);
  GG (b, c, d, a, x[4], S24, 0xe7d3fbc8);
  GG (a, b, c, d, x[9], S21, 0x21e1cde6);
  GG (d, a, b, c, x[14], S22, 0xc33707d6);
  GG (c, d, a, b, x[3], S23, 0xf4d50d87);
  GG (b, c, d, a, x[8], S24, 0x455a14ed);
  GG (a, b, c, d, x[13], S21, 0xa9e3e905);
  GG (d, a, b, c, x[2], S22, 0xfcefa3f8);
  GG (c, d, a, b, x[7], S23, 0x676f02d9);
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);

  HH (a, b, c, d, x[5], S31, 0xfffa3942);
  HH (d, a, b, c, x[8], S32, 0x8771f681);
  HH (c, d, a, b, x[11], S33, 0x6d9d6122);
  HH (b, c, d, a, x[14], S34, 0xfde5380c);
  HH (a, b, c, d, x[1], S31, 0xa4beea44);
  HH (d, a, b, c, x[4], S32, 0x4bdecfa9);
  HH (c, d, a, b, x[7], S33, 0xf6bb4b60);
  HH (b, c, d, a, x[10], S34, 0xbebfbc70);
  HH (a, b, c, d, x[13], S31, 0x289b7ec6);
  HH (d, a, b, c, x[0], S32, 0xeaa127fa);
  HH (c, d, a, b, x[3], S33, 0xd4ef3085);
  HH (b, c, d, a, x[6], S34,  0x4881d05);
  HH (a, b, c, d, x[9], S31, 0xd9d4d039);
  HH (d, a, b, c, x[12], S32, 0xe6db99e5);
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8);
  HH (b, c, d, a, x[2], S34, 0xc4ac5665);

  II (a, b, c, d, x[0], S41, 0xf4292244);
  II (d, a, b, c, x[7], S42, 0x432aff97);
  II (c, d, a, b, x[14], S43, 0xab9423a7);
  II (b, c, d, a, x[5], S44, 0xfc93a039);
  II (a, b, c, d, x[12], S41, 0x655b59c3);
  II (d, a, b, c, x[3], S42, 0x8f0ccc92);
  II (c, d, a, b, x[10], S43, 0xffeff47d);
  II (b, c, d, a, x[1], S44, 0x85845dd1);
  II (a, b, c, d, x[8], S41, 0x6fa87e4f);
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0);
  II (c, d, a, b, x[6], S43, 0xa3014314);
  II (b, c, d, a, x[13], S44, 0x4e0811a1);
  II (a, b, c, d, x[4], S41, 0xf7537e82);
  II (d, a, b, c, x[11], S42, 0xbd3af235);
  II (c, d, a, b, x[2], S43, 0x2ad7d2bb);
  II (b, c, d, a, x[9], S44, 0xeb86d391);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  memset ((unsigned char *) x, 0, sizeof (x));
}

static void
bacon_hash_init (BaconMd5Ctx *ctx)
{
  ctx->count[0] = 0;
  ctx->count[1] = 0;

  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;
}

static void
bacon_hash_update (BaconMd5Ctx *ctx, unsigned char *input, unsigned int n)
{
  unsigned int i;
  unsigned int index;
  unsigned int n_part;

  index = ((ctx->count[0] >> 3) & 0x3F);
  if ((ctx->count[0] += (n << 3)) < (n << 3))
    ctx->count[1]++;

  ctx->count[1] += (n >> 29);
  n_part = 64 - index;

  if (n >= n_part)
  {
    memcpy (&ctx->buffer[index], input, n_part);
    bacon_hash_transform (ctx->state, ctx->buffer);
    for (i = n_part; i + 63 < n; i += 64)
      bacon_hash_transform (ctx->state, &input[i]);
    index = 0;
  }
  else
    i = 0;
  memcpy (&ctx->buffer[index], &input[i], n - i);
}

static void
bacon_hash_update_from_file (BaconMd5Ctx *ctx, FILE *fp)
{
  unsigned char buffer[1024];
  unsigned int n;

  while (true)
  {
    n = fread (buffer, 1, 1024, fp);
    if (!n)
      break;
    bacon_hash_update (ctx, buffer, n);
  }
}

static void
bacon_hash_final (BaconMd5Ctx *ctx,
                  unsigned char digest[BACON_HASH_DIGEST_SIZE])
{
  unsigned char bits[8];
  unsigned int index;
  unsigned int n_pad;

  bacon_hash_encode (bits, ctx->count, 8);
  index = ((ctx->count[0] >> 3) & 0x3f);
  n_pad = (index < 56) ? (56 - index) : (120 - index);

  bacon_hash_update (ctx, padding, n_pad);
  bacon_hash_update (ctx, bits, 8);

  bacon_hash_encode (digest, ctx->state, BACON_HASH_DIGEST_SIZE);
  memset ((unsigned char *) ctx, 0, sizeof (*ctx));
}

static void
bacon_hash_from_digest (char hash[BACON_HASH_SIZE],
                        unsigned char digest[BACON_HASH_DIGEST_SIZE])
{
  unsigned int i;

  for (i = 0; i < BACON_HASH_DIGEST_SIZE; i++)
    sprintf (hash + i * 2, "%02x", digest[i]);
  hash[BACON_HASH_SIZE - 1] = '\0';
}

void
bacon_hash_from_file (BaconHash *hash, const char *path)
{
  unsigned char digest[BACON_HASH_DIGEST_SIZE];
  FILE *fp;
  BaconMd5Ctx ctx;

  fp = bacon_env_fopen (path, "rb");
  memset (&ctx, 0, sizeof (BaconMd5Ctx));
  bacon_hash_init (&ctx);
  bacon_hash_update_from_file (&ctx, fp);
  bacon_hash_final (&ctx, digest);
  bacon_hash_from_digest (hash->hash, digest);
  bacon_env_fclose (fp);
}

bool
bacon_hash_match (const BaconHash *hash1, const BaconHash *hash2)
{
  return bacon_streq (hash1->hash, hash2->hash);
}

