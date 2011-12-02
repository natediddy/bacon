/*
 * This file is part of bacon.
 *
 * bacon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bacon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bacon.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =======================================================
 *
 * MD5 implementation borrowed and adapted from:
 *
 *      http://userpages.umbc.edu/~mabzug1/cs/md5/md5.html
 *
 * (Specifically the C++ translation)
 * Below is the original copyright from the source files.
 *
 *               -- Nathan Forbes
 *
 * =======================================================
 *
 * based on:
 *
 * MD5.H - header file for MD5C.C
 * MDDRIVER.C - test driver for MD2, MD4 and MD5
 *
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */


#include <cstdio>

#include "htmldoc.h"
#include "htmlparser.h"
#include "md5.h"
#include "util.h"

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

BACON_NAMESPACE_BEGIN

using std::string;

namespace {

inline unsigned int rotateLeft(unsigned int x, unsigned int n)
{
    return (x << n) | (x >> (32 - n));
}

inline unsigned int F(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & y) | (~x & z);
}

inline unsigned int G(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & z) | (y & ~z);
}

inline unsigned int H(unsigned int x, unsigned int y, unsigned int z)
{
    return x ^ y ^ z;
}

inline unsigned int I(unsigned int x, unsigned int y, unsigned int z)
{
    return y ^ (x | ~z);
}

inline void FF(unsigned int &a,
               unsigned int b,
               unsigned int c,
               unsigned int d,
               unsigned int x,
               unsigned int s,
               unsigned int ac)
{
    a += F(b, c, d) + x + ac;
    a = rotateLeft(a, s) + b;
}

inline void GG(unsigned int &a,
               unsigned int b,
               unsigned int c,
               unsigned int d,
               unsigned int x,
               unsigned int s,
               unsigned int ac)
{
    a += G(b, c, d) + x + ac;
    a = rotateLeft(a, s) + b;
}

inline void HH(unsigned int &a,
               unsigned int b,
               unsigned int c,
               unsigned int d,
               unsigned int x,
               unsigned int s,
               unsigned int ac)
{
    a += H(b, c, d) + x + ac;
    a = rotateLeft(a, s) + b;
}

inline void II(unsigned int &a,
               unsigned int b,
               unsigned int c,
               unsigned int d,
               unsigned int x,
               unsigned int s,
               unsigned int ac)
{
    a += I(b, c, d) + x + ac;
    a = rotateLeft(a, s) + b;
}

void md5_memcpy(unsigned char *output,
                unsigned char *input,
                unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
        output[i] = input[i];
}

void md5_memset(unsigned char *output,
                unsigned char value,
                unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
        output[i] = value;
}

class Md5Impl {
public:
    Md5Impl(FILE *stream);
    string toString() const;
private:
    void encode(unsigned char *output,
                unsigned int *input,
                unsigned int len);
    void decode(unsigned int *output,
                unsigned char *input,
                unsigned int len);
    void transform(unsigned char block[64]);
    void update(unsigned char *buffer, unsigned int len);
    void update();
    void finalize();
private:
    FILE *mStream;
    unsigned int mState[4];
    unsigned int mCount[2];
    unsigned char mBuffer[64];
    unsigned char mDigest[16];
    bool mFinalized;
};

Md5Impl::Md5Impl(FILE *stream)
{
    mStream = stream;
    mFinalized = false;

    mCount[0] = 0;
    mCount[1] = 0;
    mState[0] = 0x67452301;
    mState[1] = 0xefcdab89;
    mState[2] = 0x98badcfe;
    mState[3] = 0x10325476;

    update();
    finalize();
}

string Md5Impl::toString() const
{
    if (!mFinalized)
        return string("");

    char buf[33];

    for (int i = 0; i < 16; i++)
        sprintf(buf + i * 2, "%02x", mDigest[i]);

    buf[32] = '\0';
    return string(buf);
}

void Md5Impl::encode(unsigned char *output,
                     unsigned int *input,
                     unsigned int len)
{
    for (unsigned int i = 0, j = 0; j < len; i++, j += 4) {
        output[j]   = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

void Md5Impl::decode(unsigned int *output,
                     unsigned char *input,
                     unsigned int len)
{
    for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((unsigned int)input[j]) |
            (((unsigned int)input[j+1]) << 8) |
            (((unsigned int)input[j+2]) << 16) |
            (((unsigned int)input[j+3]) << 24);
}

void Md5Impl::transform(unsigned char block[64])
{
    unsigned int a = mState[0];
    unsigned int b = mState[1];
    unsigned int c = mState[2];
    unsigned int d = mState[3];
    unsigned int x[16];

    decode(x, block, 64);

    FF(a, b, c, d, x[ 0], S11, 0xd76aa478);
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756);
    FF(c, d, a, b, x[ 2], S13, 0x242070db);
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee);
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf);
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a);
    FF(c, d, a, b, x[ 6], S13, 0xa8304613);
    FF(b, c, d, a, x[ 7], S14, 0xfd469501);
    FF(a, b, c, d, x[ 8], S11, 0x698098d8);
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af);
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);
    FF(b, c, d, a, x[11], S14, 0x895cd7be);
    FF(a, b, c, d, x[12], S11, 0x6b901122);
    FF(d, a, b, c, x[13], S12, 0xfd987193);
    FF(c, d, a, b, x[14], S13, 0xa679438e);
    FF(b, c, d, a, x[15], S14, 0x49b40821);

    GG(a, b, c, d, x[ 1], S21, 0xf61e2562);
    GG(d, a, b, c, x[ 6], S22, 0xc040b340);
    GG(c, d, a, b, x[11], S23, 0x265e5a51);
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d);
    GG(d, a, b, c, x[10], S22,  0x2441453);
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6);
    GG(d, a, b, c, x[14], S22, 0xc33707d6);
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87);
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed);
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8);
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9);
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

    HH(a, b, c, d, x[ 5], S31, 0xfffa3942);
    HH(d, a, b, c, x[ 8], S32, 0x8771f681);
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);
    HH(b, c, d, a, x[14], S34, 0xfde5380c);
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44);
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9);
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60);
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa);
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085);
    HH(b, c, d, a, x[ 6], S34,  0x4881d05);
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039);
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665);

    II(a, b, c, d, x[ 0], S41, 0xf4292244);
    II(d, a, b, c, x[ 7], S42, 0x432aff97);
    II(c, d, a, b, x[14], S43, 0xab9423a7);
    II(b, c, d, a, x[ 5], S44, 0xfc93a039);
    II(a, b, c, d, x[12], S41, 0x655b59c3);
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92);
    II(c, d, a, b, x[10], S43, 0xffeff47d);
    II(b, c, d, a, x[ 1], S44, 0x85845dd1);
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f);
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
    II(c, d, a, b, x[ 6], S43, 0xa3014314);
    II(b, c, d, a, x[13], S44, 0x4e0811a1);
    II(a, b, c, d, x[ 4], S41, 0xf7537e82);
    II(d, a, b, c, x[11], S42, 0xbd3af235);
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
    II(b, c, d, a, x[ 9], S44, 0xeb86d391);

    mState[0] += a;
    mState[1] += b;
    mState[2] += c;
    mState[3] += d;

    md5_memset((unsigned char *)x, 0, sizeof(x));
}

void Md5Impl::update(unsigned char *buffer, unsigned int len)
{
    if (mFinalized)
        return;

    unsigned int inputIndex;
    unsigned int bufferIndex;
    unsigned int bufferSpace;

    bufferIndex = (unsigned int)((mCount[0] >> 3) & 0x3f);
    if ((mCount[0] += ((unsigned int)len << 3)) <
            ((unsigned int)len << 3))
        mCount[1]++;

    mCount[1] += ((unsigned int)len >> 29);
    bufferSpace = 64 - bufferIndex;

    if (len >= bufferSpace) {
        md5_memcpy(mBuffer + bufferIndex, buffer, bufferSpace);
        transform(mBuffer);
        for (inputIndex = bufferSpace; inputIndex + 63 < len;
                inputIndex += 64)
            transform(buffer + inputIndex);
        bufferIndex = 0;
    } else {
        inputIndex = 0;
    }

    md5_memcpy(mBuffer + bufferIndex,
            buffer + inputIndex, len - inputIndex);
}

void Md5Impl::update()
{
    unsigned char buffer[1024];
    int len;

    while ((len = fread(buffer, 1, 1024, mStream)))
        update(buffer, len);
}

void Md5Impl::finalize()
{
    static unsigned char padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    if (mFinalized)
        return;

    unsigned char bits[8];
    unsigned int index;
    unsigned int padLen;

    encode(bits, mCount, 8);
    index = (unsigned int)((mCount[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);

    update(padding, padLen);
    update(bits, 8);
    encode(mDigest, mState, 16);

    md5_memset(mBuffer, 0, sizeof(*mBuffer));
    mFinalized = true;
}

} /* namespace */

Md5::Md5(const string &path,
         const string &deviceId,
         const string &deviceType)
    : File(path)
    , mLocalHash("")
    , mRemoteHash("")
{
    if (open("rb")) {
        Md5Impl impl(File::mStream);
        mLocalHash = impl.toString();
    } else {
        BACON_LOGW("could not read `%s' to generate MD5 hash!",
                name().c_str());
    }

    HtmlDoc doc(deviceId, deviceType);

    if (doc.fetch()) {
        HtmlParser parser(doc.content());
        mRemoteHash = parser.checksumStringForFile(baseName());
    }
}

bool Md5::verify()
{
    return mLocalHash == mRemoteHash;
}

BACON_NAMESPACE_END
