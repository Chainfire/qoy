/*

QOY - The "Quite OK YCbCr420A" format for fast, lossless* image compression
                * colorspace conversion to/from RGBA is lossy, if used

Dominic Szablewski - https://phoboslab.org
Jorrit "Chainfire" Jongma

-- BASED ON

QOI - The "Quite OK Image" format for fast, lossless image compression

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2021 Dominic Szablewski
Copyright(c) 2021 Jorrit "Chainfire" Jongma

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

NOTICE: QOY follows QOI's license. If QOI is re-licensed, you may apply that
license to QOY as well.


-- About

QOY encodes and decodes images in a lossless format, as QOI does. But where
QOI encodes RGBA, QOY encodes YCbCr 4:2:0 A (YCbCrA 4:2:0:4).

QOY can work with both RGBA and YCbCrA pixel data. In case of RGBA data, it
is converted to and from the YCbCrA colorspace on-the-fly, which is a lossy
operation. QOY is only lossless when input and output are YCbCrA.

QOY performance with RGBA pixel data is similar to QOI, and is about 1.5x
(encoding) to 2.0x (decoding) faster when using YCbCrA pixel data.

QOY encoded size is about half of QOI's, while the differences in decoded
RGBA output are virtually indistinguishable.

QOY's aim is to remain small, simple, and fast, while offering "quite ok"
compression levels.



-- Synopsis

// Define `QOY_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define QOY_IMPLEMENTATION
#include "qoy.h"

// Encode and store an RGBA buffer to the file system. The qoy_desc describes
// the input pixel data.
qoy_write("image_new.qoy", rgba_pixels, &(qoy_desc){
    .width = 1920,
    .height = 1080,
    .channels = 4,
    .colorspace = QOY_COLORSPACE_SRGB
});

// Load and decode a QOY image from the file system into a 32bbp RGBA buffer.
// The qoy_desc struct will be filled with the width, height, number of channels
// and colorspace read from the file header.
qoy_desc desc;
void *rgba_pixels = qoy_read("image.qoy", &desc, 4);



-- Documentation

This library provides the following functions;

- qoy_read    -- read and decode a QOY file to RGBA
- qoy_write   -- encode RGBA and write a QOY file

- qoy_decode  -- decode a QOY image from memory to an RGBA or YCbCrA buffer
- qoy_encode  -- encode an RGBA or YCbCrA buffer into a QOY image in memory

- qoy_ycbcra_size     -- calculate size of YCbCrA buffer
- qoy_rgba_to_ycbcra  -- convert buffer from RGBA to YCbCrA colorspace
- qoy_ycbcra_to_rgba  -- convert buffer from YCbCrA to RGBA colorspace


See the function declaration below for the signature and more information.

If you don't want/need the qoy_read and qoy_write functions, you can define
QOY_NO_STDIO before including this library.

This library uses malloc() and free(). To supply your own malloc implementation
you can define QOY_MALLOC and QOY_FREE before including this library.


-- Buffer formats

All channels are 8-bits wide. Buffers encode top to bottom, left to right.

The RGB(A) format for each line encodes each channel for each pixel
individually, in R G B (A) order, 3 (no alpha) or 4 (alpha) bytes per pixel.

The YCbCr(A) 4:2:0(:4) format uses 2x2 blocks, of which Y (and A) are stored
for each pixel, while Cb and Cr are averaged and stored for the entire 2x2
block. Two lines are interleaved for Y (and A):

.-----------.       .-----------------------------------------------------.
|  0  |  2  |  -->  | Y0 | Y1 | Y2 | Y3 | Cb | Cr ( | A0 | A1 | A2 | A3 ) |
|-----+-----|       `-----------------------------------------------------`
|  1  |  3  |
`-----------`

This results in 6 (no alpha) or 10 (alpha) bytes per 4 pixels.


-- Colorspace conversion

Colorspace conversion between RGBA and YCbCrA is lossy (while the encoding
of the YCbCrA by QOY directly is not).

As YCbCrA requires both width and height to be even, in case of odd height
the last line is repeated, and in case of odd width the last pixel of each
line is repeated.

The colorspace conversion used by this library is based on the formulas
used by JPEG: https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion

The library uses an integer math approximation:

RGBA --> YCbCrA:

        ycbcra.y[0] = ((1254097 * rgba[0].r) + (2462056 * rgba[0].g) + (478151 * rgba[0].b)) >> 22
        ycbcra.y[1] = ((1254097 * rgba[1].r) + (2462056 * rgba[1].g) + (478151 * rgba[1].b)) >> 22
        ycbcra.y[2] = ((1254097 * rgba[2].r) + (2462056 * rgba[2].g) + (478151 * rgba[2].b)) >> 22
        ycbcra.y[3] = ((1254097 * rgba[3].r) + (2462056 * rgba[3].g) + (478151 * rgba[3].b)) >> 22
        r4 = rgba[0].r + rgba[1].r + rgba[2].r + rgba[3].r
        g4 = rgba[0].g + rgba[1].g + rgba[2].g + rgba[3].g
        b4 = rgba[0].b + rgba[1].b + rgba[2].b + rgba[3].b
        ycbcra.cb = clamp_8bit((134217728 - (44233 * r4) - (86839 * g4) + (b4 << 17) + (1 << 19)) >> 20)
        ycbcra.cr = clamp_8bit(134217728 + (r4 << 17) - (109757 * g4) - (21315 * b4) + (1 << 19)) >> 20)
        ycbcra.a[0] = rgba[0].a
        ycbcra.a[1] = rgba[1].a
        ycbcra.a[2] = rgba[2].a
        ycbcra.a[3] = rgba[3].a
        
YCbCrA --> RGBA:

        r_diff = (11760828 * (ycbcra.cr - 128)) >> 23
        g_diff = ((2886822 * (ycbcra.cb - 128)) + (5990607 * (ycbcra.cr - 128))) >> 23
        b_diff = (14864613 * (ycbcra.cb - 128)) >> 23

        rgba[0].r = clamp_8bit(ycbcra.y[0] + r_diff)
        rgba[0].g = clamp_8bit(ycbcra.y[0] - g_diff)
        rgba[0].b = clamp_8bit(ycbcra.y[0] + b_diff)
        rgba[0].a = ycbcra.a[0]

        rgba[1].r = clamp_8bit(ycbcra.y[1] + r_diff)
        rgba[1].g = clamp_8bit(ycbcra.y[1] - g_diff)
        rgba[1].b = clamp_8bit(ycbcra.y[1] + b_diff)
        rgba[1].a = ycbcra.a[1]

        rgba[2].r = clamp_8bit(ycbcra.y[2] + r_diff)
        rgba[2].g = clamp_8bit(ycbcra.y[2] - g_diff)
        rgba[2].b = clamp_8bit(ycbcra.y[2] + b_diff)
        rgba[2].a = ycbcra.a[2]

        rgba[3].r = clamp_8bit(ycbcra.y[3] + r_diff)
        rgba[3].g = clamp_8bit(ycbcra.y[3] - g_diff)
        rgba[3].b = clamp_8bit(ycbcra.y[3] + b_diff)
        rgba[3].a = ycbcra.a[3]

The color channels are assumed to not be premultiplied with the alpha channel
("un-premultiplied alpha").

If RGBA buffers are used for input or output, the library converts between
colorspaces on-the-fly (per two RGBA lines). You can of course use your own
conversions and pass/request only YCbCrA buffers from the library.

While QOY's alpha handling assumes 8-bit, if you convert the input alpha channel
to 4-bit or 2-bit before passing the buffers to QOY, they should be encoded
reasonably efficiently.


-- Data Format

A QOY file has a 14 byte header, followed by any number of data "chunks" and an
8-byte end marker.

struct qoy_header_t {
    char     magic[4];   // magic bytes "qoyf"
    uint32_t width;      // image width in pixels (BE)
    uint32_t height;     // image height in pixels (BE)
    uint8_t  channels;   // 3 = without alpha, 4 = with alpha
    uint8_t  colorspace; // 0 = sRGB with linear alpha, 1 = all channels linear (hint only)
};

Images are encoded from top to bottom, left to right. The decoder and encoder 
start with {y: [0, 0, 0, 0], cb: 0, cr: 0, a: [255, 255, 255, 255]} as the
previous block value. An image is complete when all blocks specified by
width * height have been covered - with width and height rounded up to
multiples of 2. The header stores the real width and height, not the rounded
version.

YCbCr and Alpha channels are encoded separately.

YCbCr channels are encoded as:
 - a run of the previous block
 - a difference to the previous block's YCbCr values
 - all full YCbCr values for the block

Alpha channel is encoded as:
 - nothing: absence of encoded A values repeats last
 - a difference to the previous block's A values
 - a single full A value for the entire block
 - all full A values for the entire block

Alpha values precede YCbCr values, this way, if there's no alpha tag present
before an YCbCr tag, the decoder knows to repeat the previous alpha value.
This also means a change of A interrupts and restarts a run of YCbCr repeats,
otherwise the decoder would be unable to discern to which pixel in the run
the new A value applies.

Differences in Y (and A) values apply to the last horizontal pixel, not the
last block: y[0] and y[1] compare to y[2] and y[3] of the last block,
while y[2] and y[3] of the current block compare to y[0] and y[1] of the
current block.

The exception is when A repeats: the a[2] value of the previous block is used
for a[0], a[1], a[2], and a[3] of the current block.

Both these last two points must also be taken into account when decoding runs!

Differences are encoded with bias: for example, a 5-bit difference runs from
-16 to +15, 32 values, bias +16, encoded as 0 (-16) to 31 (15).

Each chunk starts with a tag of variable length, followed by a number of data
bits. The bit length of chunks is divisible by 8 - i.e. all chunks are byte
aligned. All values encoded in these data bits have the most significant bit
on the left.

Tags are non-ambiguous and with the right masks can be checked in any order.

The byte stream's end is marked with 8 0xff bytes, which cannot naturally
occur in the encoder.


The possible chunks are:


.- QOY_OP_A18 ----------------------.
|     Byte[0]     |     Byte[1]     |
| 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
|-----------------+-----------------|
| 1 1 1 1 1 0 0 0 |        a        |
`-----------------------------------`
8-bit tag b11111000
8-bit a[0..3], literal, applied to all pixels


.- QOY_OP_A42 ----------------------------.
|     Byte[0]     |        Byte[1]        |
| 7 6 5 4 3 2 1 0 | 7 6   5 4   3 2   1 0 |
|-----------------+-----+-----+-----+-----|
| 1 1 1 1 1 0 0 1 |  a0 |  a1 |  a2 |  a3 |
`-----------------------------------------`
8-bit tag b11111001
2-bit a[0], difference with previous a[2], -2..1, bias+2
2-bit a[1], difference with previous a[3], -2..1, bias+2
2-bit a[2], difference with current  a[0], -2..1, bias+2
2-bit a[3], difference with current  a[1], -2..1, bias+2


.- QOY_OP_A44 --------------------------------------------.
|     Byte[0]     |      Byte[1]      |      Byte[2]      |
| 7 6 5 4 3 2 1 0 | 7 6 5 4   3 2 1 0 | 7 6 5 4   3 2 1 0 |
|-----------------+---------+---------+---------+---------|
| 1 1 1 1 1 0 1 0 |    a0   |    a1   |    a2   |    a3   |
`---------------------------------------------------------`
8-bit tag 11111010
4-bit a[0], difference with previous a[2], -8..7, bias+8
4-bit a[1], difference with previous a[3], -8..7, bias+8
4-bit a[2], difference with current  a[0], -8..7, bias+8
4-bit a[3], difference with current  a[1], -8..7, bias+8


.- QOY_OP_A48 ----------------------------------------------------------------------------.
|     Byte[0]     |     Byte[1]     |     Byte[2]     |     Byte[3]     |     Byte[4]     |
| 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
|-----------------+-----------------+-----------------+-----------------+-----------------|
| 1 1 1 1 1 0 1 1 |        a0       |        a1       |        a2       |        a3       |
`-----------------------------------------------------------------------------------------`
8-bit tag b11111011
8-bit a[0], literal
8-bit a[1], literal
8-bit a[2], literal
8-bit a[3], literal


.- QOY_OP_321 -----------------------------------.
|        Byte[0]        |         Byte[1]        |
| 7   6 5 4   3 2 1   0 | 7 6   5 4 3   2 1    0 |
|---+-------+-------+---+-----+-------+-----+-----
| 0 |   y0  |   y1  |    y2   |   y4  |  cb | cr |
`------------------------------------------------`
1-bit tag b0
3-bit y[0], difference with previous y[2], -4..3, bias+4
3-bit y[1], difference with previous y[3], -4..3, bias+4
3-bit y[2], difference with current  y[0], -4..3, bias+4
3-bit y[3], difference with current  y[1], -4..3, bias+4
2-bit cb  , difference with previous     , -2..1, bias+2
1-bit cr  , difference with previous     , -1..0, bias+1


.- QOY_OP_433 ----------------------------------------------------.
|        Byte[0]      |       Byte[1]       |      Byte[2]        |
| 7 6   5 4 3 2   1 0 | 7 6   5 4 3 2   1 0 | 7 6   5 4 3   2 1 0 |
|-----+---------+-----+-----+---------+-----+-----+-------+-------|
| 1 0 |    y0   |     y1    |    y3   |     y4    |   cb  |   cr  |
`-----------------------------------------------------------------`
2-bit tag b10
4-bit y[0], difference with previous y[2], -8..7, bias+8
4-bit y[1], difference with previous y[3], -8..7, bias+8
4-bit y[2], difference with current  y[0], -8..7, bias+8
4-bit y[3], difference with current  y[1], -8..7, bias+8
3-bit cb  , difference with previous     , -4..3, bias+4
3-bit cr  , difference with previous     , -4..3, bias+4


.- QOY_OP_554 --------------------------------------------------------------------.
|      Byte[0]      |       Byte[1]     |        Byte[2]      |      Byte[3]      |
| 7 6 5   4 3 2 1 0 | 7 6 5 4 3   2 1 0 | 7 6   5 4 3 2 1   0 | 7 6 5 4   3 2 1 0 |
|-------+-----------+-----------+-------+-----+-----------+---+---------+---------|
| 1 1 0 |     y0    |     y1    |      y3     |     y4    |      cb     |    cr   |
`---------------------------------------------------------------------------------`
3-bit tag b110
5-bit y[0], difference with previous y[2], -16..15, bias+16
5-bit y[1], difference with previous y[3], -16..15, bias+16
5-bit y[2], difference with current  y[0], -16..15, bias+16
5-bit y[3], difference with current  y[1], -16..15, bias+16
5-bit cb  , difference with previous     , -16..15, bias+16
4-bit cr  , difference with previous     ,  -8..7 , bias+8


.- QOY_OP_666 --------------------------------------------------------------------------------------.
|      Byte[0]      |      Byte[1]      |       Byte[2]     |      Byte[3]      |      Byte[4]      |
| 7 6 5 4   3 2 1 0 | 7 6   5 4 3 2 1 0 | 7 6 5 4 3 2   1 0 | 7 6 5 4   3 2 1 0 | 7 6   5 4 3 2 1 0 |
|---------+---------------+-------------+-------------+---------------+---------+-----+-------------|
| 1 1 1 0 |       y0      |      y1     |      y3     |       y4      |       cb      |      cr     |
`---------------------------------------------------------------------------------------------------`
4-bit tag b1110
6-bit y[0], difference with previous y[2], -32..31, bias+32
6-bit y[1], difference with previous y[3], -32..31, bias+32
6-bit y[2], difference with current  y[0], -32..31, bias+32
6-bit y[3], difference with current  y[1], -32..31, bias+32
6-bit cb  , difference with previous     , -32..31, bias+32
6-bit cr  , difference with previous     , -32..31, bias+32


.- QOY_OP_865 ----------------------------------------------------------------------------------------------------------.
|      Byte[0]      |      Byte[1]      |       Byte[2]     |      Byte[3]      |      Byte[4]      |      Byte[5]      |
| 7 6 5 4 3   2 1 0 | 7 6 5 4 3   2 1 0 | 7 6 5 4 3   2 1 0 | 7 6 5 4 3   2 1 0 | 7 6 5 4 3   2 1 0 | 7 6 5   4 3 2 1 0 |
|-----------+-------+-----------+-------+-----------+-------+-----------+-------+-----------+-------+-------+-----------|
| 1 1 1 1 0 |         y0        |         y1        |         y2        |         y3        |       cb      |     cr    |
`-----------------------------------------------------------------------------------------------------------------------`
5-bit tag b11110
8-bit y[0], difference with previous y[2], -128..127, bias+128
8-bit y[1], difference with previous y[3], -128..127, bias+128
8-bit y[2], difference with current  y[0], -128..127, bias+128
8-bit y[3], difference with current  y[1], -128..127, bias+128
6-bit cb  , difference with previous     ,  -32..31 , bias+32
5-bit cr  , difference with previous     ,  -16..15 , bias+16


.- QOY_OP_888 ----------------------------------------------------------------------------------------------------------------.
|     Byte[0]     |     Byte[1]     |      Byte[2]    |     Byte[3]     |     Byte[4]     |     Byte[5]     |     Byte[6]     |
| 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
|-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------|
| 1 1 1 1 1 1 1 0 |        y0       |        y1       |        y2       |        y3       |        cb       |        cr       |
`-----------------------------------------------------------------------------------------------------------------------------`
8-bit tag b11111110
8-bit y[0], literal
8-bit y[1], literal
8-bit y[2], literal
8-bit y[3], literal
8-bit cb  , literal
8-bit cr  , literal


.- QOY_OP_RUN_1 --.
|     Byte[0]     |
| 7 6 5 4 3 2 1 0 |
|-----------------|
| 1 1 1 1 1 1 0 0 |
`-----------------`
8-bit tag b11111100
current y[0] and y[2] become previous y[2]
current y[1] and y[3] become previous y[3]
Unless preceded by alpha, current a[0..3] become previous a[2]


.- QOY_OP_RUN_X ----------------------.
|     Byte[0]     |      Byte[1]      |
| 7 6 5 4 3 2 1 0 | 7   6 5 4 3 2 1 0 |
|-----------------+---+---------------|
| 1 1 1 1 1 1 0 1 | 0 |     count     |
`-------------------------------------`
8-bit tag b11111101
1-bit tag b0
7-bit count, 2..129, bias-2


.- QOY_OP_RUN_X [2]-------------------------------------.
|     Byte[0]     |      Byte[1]      |     Byte[2]     |
| 7 6 5 4 3 2 1 0 | 7   6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
|-----------------+---+---------------+-----------------|
| 1 1 1 1 1 1 0 1 | 1 |              count              |
`-------------------------------------------------------`
8-bit tag b11111101
1-bit tag b1
15-bit count, 130..32897, bias-130


.- QOY_OP_EOF ----.
|     Byte[0]     |
| 7 6 5 4 3 2 1 0 |
|-----------------|
| 1 1 1 1 1 1 1 1 |
`-----------------`
8-bit tag b11111111
End of file, decoder should never read this, logic should complete reading the
byte before as all blocks should be processed at that point. Reading this tag
is thus an error. The file is padded by 8 * QOY_OP_EOF, which makes it
searchable, as this sequence is guaranteed to not occur naturally in the
encoded pixel data.

*/


/* -----------------------------------------------------------------------------
Header - Public functions */

#ifndef QOY_H
#define QOY_H

#ifdef __cplusplus
extern "C" {
#endif

/* A pointer to a qoy_desc struct has to be supplied to all of qoy's functions.
It describes either the input format (for qoy_write and qoy_encode), or is
filled with the description read from the file header (for qoy_read and
qoy_decode).

The colorspace in this qoy_desc is an enum where
    0 = sRGB, i.e. gamma scaled RGB channels and a linear alpha channel
    1 = all channels are linear
You may use the constants QOY_COLORSPACE_SRGB or QOY_COLORSPACE_LINEAR. The
colorspace is purely informative. It will be saved to the file header, but
does not affect en-/decoding in any way. */

#define QOY_COLORSPACE_SRGB   0
#define QOY_COLORSPACE_LINEAR 1

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char colorspace;
} qoy_desc;

/* qoy_encode and qoy_decode can work with RGB(A) and YCbCr 4:2:0 (A) buffers.
Internally they always use YCbCr 4:2:0 (A) buffers, with conversion from/to
RGB(A) on-the-fly per every two lines of RGB(A). You need to tell the function
the format of the buffer using these defines. */

#define QOY_FORMAT_RGBA 0
#define QOY_FORMAT_YCBCR420A 1

#ifndef QOY_NO_STDIO

/* Encode raw RGB or RGBA pixels into a QOY image and write it to the file
system. The qoy_desc struct must be filled with the image width, height,
number of channels (3 = RGB, 4 = RGBA) and the colorspace. 

The function returns 0 on failure (invalid parameters, or fopen or malloc 
failed) or the number of bytes written on success. */

int qoy_write(const char *filename, const void *data, const qoy_desc *desc);


/* Read and decode a QOY image from the file system. If channels is 0, the
number of channels from the file header is used. If channels is 3 or 4 the
output format will be forced into this number of channels.

The function either returns NULL on failure (invalid data, or malloc or fopen
failed) or a pointer to the decoded pixels. On success, the qoy_desc struct
will be filled with the description from the file header.

The returned pixel data should be free()d after use. */

void *qoy_read(const char *filename, qoy_desc *desc, int channels);

#endif /* QOY_NO_STDIO */


/* Encode raw RGB(A) or YCbCr 4:2:0 (A) pixels into a QOY image in memory.

The function either returns NULL on failure (invalid parameters or malloc 
failed) or a pointer to the encoded data on success. On success the out_len 
is set to the size in bytes of the encoded data.

in_channels specifies the number of channels in *data, which need not match
desc->channels; must be 0 (use desc->channels), 3 (no alpha) or 4 (alpha).

in_format specifies the buffer format of *data; QOY_FORMAT_RGBA to convert
and encode RGBA data, or QOY_FORMAT_YCBCR420A to encode YCbCrA.

The returned qoy data should be free()d after use. */

void *qoy_encode(const void *data, const qoy_desc *desc, int *out_len, int in_channels, int in_format);


/* Decode a QOY image from memory.

The function either returns NULL on failure (invalid parameters or malloc 
failed) or a pointer to the decoded pixels. On success, the qoy_desc struct
is filled with the description from the file header.

out_channels specifies the number of channels in the returned buffer, which
need not match desc->channels; must be 0 (use desc->channels), 3 (no alpha)
or 4 (alpha).

out_format specifies the buffer format of the returned data; QOY_FORMAT_RGBA
to decode and convert to RGBA, or QOY_FORMAT_YCBCR420A to decode YCbCrA.

The returned pixel data should be free()d after use. */

void *qoy_decode(const void *data, int size, qoy_desc *desc, int out_channels, int out_format);


/* Calculate size of YCbCrA buffer, channels must be 3 (no alpha) or 4 (alpha) */

int qoy_ycbcra_size(int width, int height, int channels);


/* Convert buffer from RGBA to YCbCrA colorspace.

Both channels_in and channels_out must 3 (no alpha) or 4 (alpha), but need not
be the same. If alpha is included in the output but was not present in the
input, alpha values are set to 255.

The output buffer must be pre-allocated, use
qoy_ycbcra_size(width, height, channels_out) to determine the required size.

Returns the number of bytes written. */

int qoy_rgba_to_ycbcra(const void* rgba_in, int width, int height, int channels_in, int channels_out, void *ycbcr420a_out);


/* Convert buffer from YCbCrA to RGBA colorspace.

Both channels_in and channels_out must 3 (no alpha) or 4 (alpha), but need not
be the same. If alpha is included in the output but was not present in the
input, alpha values are set to 255.

The output buffer must be pre-allocated, use width*height*channels_out to
determine the required size.

Returns the number of bytes written. */

int qoy_ycbcra_to_rgba(const void* ycbcr420a_in, int width, int height, int channels_in, int channels_out, void *rgba_out);


#ifdef __cplusplus
}
#endif
#endif /* QOY_H */


/* -----------------------------------------------------------------------------
Implementation */

#ifdef QOY_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

#ifndef QOY_MALLOC
    #define QOY_MALLOC(sz) malloc(sz)
    #define QOY_FREE(p)    free(p)
#endif

#define QOY_OP_321_MASK 0x80 /* 1.......                                                                          */
#define QOY_OP_321      0x00 /* 0yyyYYYy yyYYYbbr                                     y:4*3 cb:2 cr:1 --> 2 bytes */
#define QOY_OP_433_MASK 0xc0 /* 11......                                                                          */
#define QOY_OP_433      0x80 /* 10yyyyYY YYyyyyYY YYbbbrrr                            y:4*4 cb:3 cr:3 --> 3 bytes */
#define QOY_OP_554_MASK 0xe0 /* 111.....                                                                          */
#define QOY_OP_554      0xc0 /* 110yyyyy YYYYYyyy yyYYYYYb bbbbrrrr                   y:4*5 cb:5 cr:4 --> 4 bytes */
#define QOY_OP_666_MASK 0xf0 /* 1111....                                                                          */
#define QOY_OP_666      0xe0 /* 1110yyyy yyYYYYYY yyyyyyYY YYYYbbbb bbrrrrrr          y:4*6 cb:6 cr:6 --> 5 bytes */
#define QOY_OP_865_MASK 0xf8 /* 11111...                                                                          */
#define QOY_OP_865      0xf0 /* 11110yyy yyyyyYYY YYYYYyyy yyyyyYYY YYYYYbbb bbbrrrrr y:4*8 cb:6 cr:5 --> 6 bytes */

#define QOY_OP_A_MASK   0xfc /* 111111..                                                                          */
#define QOY_OP_A_ANY    0xf8 /* 11111000                                                                          */
#define QOY_OP_A18      0xf8 /* 11111000 aaaaaaaa                                     a:1*8           --> 2 bytes */
#define QOY_OP_A42      0xf9 /* 11111001 aaAAaaAA                                     a:4*2           --> 2 bytes */
#define QOY_OP_A44      0xfa /* 11111010 aaaaAAAA aaaaAAAA                            a:4*4           --> 3 bytes */
#define QOY_OP_A48      0xfb /* 11111011 aaaaaaaa AAAAAAAA aaaaaaaa AAAAAAAA          a:4*8           --> 5 bytes */

#define QOY_OP_RUN_MASK 0xff /* 11111111                                                                          */
#define QOY_OP_RUN_1    0xfc /* 11111100                                              [1]             --> 1 byte  */
#define QOY_OP_RUN_X    0xfd /* 11111101 0ccccccc                                     [2..129]        --> 2 bytes */
                             /* 11111101 1ccccccc cccccccc                            [130..32897]    --> 3 bytes */

#define QOY_OP_888_MASK 0xff /* 11111111                                                                          */
#define QOY_OP_888      0xfe /* 11111110 y8 y8 y8 y8 b8 r8                            y:4*8 cb:8 cr:8 --> 7 bytes */

#define QOY_OP_EOF_MASK 0xff /* 11111111                                                                          */
#define QOY_OP_EOF      0xff /* 11111111*8 cannot be produced by the encoder, *6 is the max using QOY_OP_888      */

#define QOY_MAGIC \
    (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
     ((unsigned int)'y') <<  8 | ((unsigned int)'f'))
#define QOY_HEADER_SIZE 14

/* 2GB is the max file size that this implementation can safely handle. We guard
against anything larger than that, assuming the worst case with 3 bytes per
pixel (7 YCbCr + 5 A per 4 pixels), rounded down to a nice clean value. 600 million
pixels ought to be enough for anybody. */
#define QOY_PIXELS_MAX ((unsigned int)600000000)

#pragma pack(push, 1)
typedef struct __attribute__((__packed__)) {
    unsigned char r, g, b, a;
} qoy_rgba_t;
typedef struct __attribute__((__packed__)) {
    unsigned char y[4], cb, cr, a[4];
} qoy_ycbcr420a_t;
typedef struct __attribute__((__packed__)) {
    signed char y[4], cb, cr, a[4];
} qoy_ycbcr420a_diff_t;
#pragma pack(pop)

static const unsigned char qoy_padding[8] = { QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF, QOY_OP_EOF };

void qoy_write_32(unsigned char *bytes, int *p, unsigned int v) {
    bytes[(*p)++] = (0xff000000 & v) >> 24;
    bytes[(*p)++] = (0x00ff0000 & v) >> 16;
    bytes[(*p)++] = (0x0000ff00 & v) >> 8;
    bytes[(*p)++] = (0x000000ff & v);
}

unsigned int qoy_read_32(const unsigned char *bytes, int *p) {
    unsigned int a = bytes[(*p)++];
    unsigned int b = bytes[(*p)++];
    unsigned int c = bytes[(*p)++];
    unsigned int d = bytes[(*p)++];
    return a << 24 | b << 16 | c << 8 | d;
}

int qoy_ycbcra_size(int width, int height, int channels) {
    return ((height + 1) >> 1) * ((width + 1) & ~0x01) * (channels == 4 ? 5 : 3);
}

static inline unsigned char qoy_8bit_clamp(int i) {
    if (i < 0) return 0;
    if (i > 255) return 255;
    return i;
}

static inline int qoy_rgba_to_ycbcra_two_lines(const void* rgba_in, int width, int lines, int channels_in, int channels_out, void *ycbcr420a_out) {
    if (channels_in != 4) channels_in = 3;
    if (channels_out != 4) channels_out = 3;
    unsigned char *line1 = (unsigned char *)rgba_in;
    unsigned char *line2 = lines == 2 ? line1 + width * channels_in : line1;
    unsigned char *out = ycbcr420a_out;
    int size_out = (channels_out == 4) ? 10 : 6;
    int written = 0;
    for (int i = 0; i < width; i += 2, line1 += channels_in * 2, line2 += channels_in * 2, out += size_out) {
        qoy_rgba_t *p1 = (qoy_rgba_t *)line1;
        qoy_rgba_t *p2 = (qoy_rgba_t *)line2;
        qoy_rgba_t *p3 = (qoy_rgba_t *)(((width & 0x01) == 1 && i == width - 1) ? line1 : line1 + channels_in);
        qoy_rgba_t *p4 = (qoy_rgba_t *)(((width & 0x01) == 1 && i == width - 1) ? line2 : line2 + channels_in);
        qoy_ycbcr420a_t *pout = (qoy_ycbcr420a_t *)out;
        pout->y[0] = ((1254097 * p1->r) + (2462056 * p1->g) + (478151 * p1->b)) >> 22;
        pout->y[1] = ((1254097 * p2->r) + (2462056 * p2->g) + (478151 * p2->b)) >> 22;
        pout->y[2] = ((1254097 * p3->r) + (2462056 * p3->g) + (478151 * p3->b)) >> 22;
        pout->y[3] = ((1254097 * p4->r) + (2462056 * p4->g) + (478151 * p4->b)) >> 22;
        unsigned int r4 = p1->r + p2->r + p3->r + p4->r;
        unsigned int g4 = p1->g + p2->g + p3->g + p4->g;
        unsigned int b4 = p1->b + p2->b + p3->b + p4->b;
        pout->cb = qoy_8bit_clamp((134217728 - (44233 * r4) - (86839 * g4) + (b4 << 17) + (1 << 19)) >> 20);
        pout->cr = qoy_8bit_clamp((134217728 + (r4 << 17) - (109757 * g4) - (21315 * b4) + (1 << 19)) >> 20);
        if (channels_out == 4) {
            if (channels_in == 4) {
                pout->a[0] = p1->a;
                pout->a[1] = p2->a;
                pout->a[2] = p3->a;
                pout->a[3] = p4->a;
            } else {
                pout->a[0] = 0xff;
                pout->a[1] = 0xff;
                pout->a[2] = 0xff;
                pout->a[3] = 0xff;
            }
        }
        written += size_out;
    }
    return written;
}

int qoy_rgba_to_ycbcra(const void* rgba_in, int width, int height, int channels_in, int channels_out, void *ycbcr420a_out) {
    unsigned char *pin = (unsigned char *)rgba_in;
    unsigned char *pout = (unsigned char *)ycbcr420a_out;
    int size_out = (channels_out == 4) ? 10 : 6;
    int written = 0;
    for (int y = 0; y < height; y += 2, pin += width * channels_in * 2, pout += size_out * (width >> 1)) {
        written += qoy_rgba_to_ycbcra_two_lines(
            pin,
            width,
            (height & 0x01) != 0 && y == height - 1 ? 1 : 2,
            channels_in,
            channels_out,
            pout
        );
    }
    return written;
}

static inline int qoy_ycbcra_to_rgba_two_lines(const void* ycbcr420a_in, int width, int lines, int channels_in, int channels_out, void *rgba_out) {
    unsigned char *line1 = (unsigned char *)rgba_out;
    unsigned char *line2 = lines == 2 ? line1 + width * channels_out : line1;
    unsigned char *in = (unsigned char *)ycbcr420a_in;
    int size_in = (channels_in == 4) ? 10 : 6;
    int written = 0;
    int r_diff, g_diff, b_diff;
    int a[4];
    for (int i = 0; i < width; i += 2, line1 += channels_out * 2, line2 += channels_out * 2, in += size_in) {
        qoy_ycbcr420a_t *pin = (qoy_ycbcr420a_t *)in;
        qoy_rgba_t *p1 = (qoy_rgba_t *)line1;
        qoy_rgba_t *p2 = (qoy_rgba_t *)line2;
        qoy_rgba_t *p3 = (qoy_rgba_t *)(((width & 0x01) == 1 && i == width - 1) ? line1 : line1 + channels_out);
        qoy_rgba_t *p4 = (qoy_rgba_t *)(((width & 0x01) == 1 && i == width - 1) ? line2 : line2 + channels_out);

        r_diff = (11760828 * (pin->cr - 128)) >> 23;
        g_diff = ((2886822 * (pin->cb - 128)) + (5990607 * (pin->cr - 128))) >> 23;
        b_diff = (14864613 * (pin->cb - 128)) >> 23;

        p1->r = qoy_8bit_clamp(pin->y[0] + r_diff);
        p1->g = qoy_8bit_clamp(pin->y[0] - g_diff);
        p1->b = qoy_8bit_clamp(pin->y[0] + b_diff);

        p2->r = qoy_8bit_clamp(pin->y[1] + r_diff);
        p2->g = qoy_8bit_clamp(pin->y[1] - g_diff);
        p2->b = qoy_8bit_clamp(pin->y[1] + b_diff);

        p3->r = qoy_8bit_clamp(pin->y[2] + r_diff);
        p3->g = qoy_8bit_clamp(pin->y[2] - g_diff);
        p3->b = qoy_8bit_clamp(pin->y[2] + b_diff);

        p4->r = qoy_8bit_clamp(pin->y[3] + r_diff);
        p4->g = qoy_8bit_clamp(pin->y[3] - g_diff);
        p4->b = qoy_8bit_clamp(pin->y[3] + b_diff);

        if (channels_out == 4) {
            if (channels_in == 4) {
                p1->a = pin->a[0];
                p2->a = pin->a[1];
                p3->a = pin->a[2];
                p4->a = pin->a[3];
            } else {
                p1->a = 255;
                p2->a = 255;
                p3->a = 255;
                p4->a = 255;
            }
        }

        written += channels_out * 4;
    }
    return written;
}

int qoy_ycbcra_to_rgba(const void* ycbcr420a_in, int width, int height, int channels_in, int channels_out, void *rgba_out) {
    unsigned char *pout = (unsigned char *)rgba_out;
    unsigned char *pin = (unsigned char *)ycbcr420a_in;
    int size_in = (channels_in == 4) ? 10 : 6;
    int written = 0;
    for (int y = 0; y < height; y += 2, pout += width * channels_out * 2, pin += size_in * (width >> 1)) {
        written += qoy_ycbcra_to_rgba_two_lines(
            pin,
            width,
            (height & 0x01) != 0 && y == height - 1 ? 1 : 2,
            channels_in,
            channels_out,
            pout
        );
    }
    return written;
}

void *qoy_encode(const void *data, const qoy_desc *desc, int *out_len, int in_channels, int in_format) {
    int internal_width = (desc->width + 1) & ~0x01;
    int internal_height = (desc->height + 1) & ~0x01;

    if (in_channels == 0) in_channels = desc->channels;
    if (
        data == NULL || out_len == NULL || desc == NULL ||
        desc->width == 0 || desc->height == 0 ||
        desc->channels < 3 || desc->channels > 4 ||
        in_channels < 3 || in_channels > 4 ||
        desc->colorspace > 1 ||
        internal_height >= QOY_PIXELS_MAX / internal_width ||
        in_format > 1
    ) {
        return NULL;
    }

    int max_size = QOY_HEADER_SIZE + ((internal_width * internal_height + 3) >> 2) * (desc->channels == 4 ? 12 : 7) + (int)sizeof(qoy_padding);

    unsigned char *bytes = (unsigned char *)QOY_MALLOC(max_size);
    if (!bytes) {
        return NULL;
    }

    int p = 0;
    qoy_write_32(bytes, &p, QOY_MAGIC);
    qoy_write_32(bytes, &p, desc->width);
    qoy_write_32(bytes, &p, desc->height);
    bytes[p++] = desc->channels;
    bytes[p++] = desc->colorspace;

    const unsigned char *pixels = (const unsigned char *)data;

    qoy_ycbcr420a_t px, px_prev = {0};
    qoy_ycbcr420a_diff_t px_diff;
    px_prev.a[0] = 255;
    px_prev.a[1] = 255;
    px_prev.a[2] = 255;
    px_prev.a[3] = 255;

    int run = 0;
    int size_ycbcra = (in_channels == 4) ? 10 : 6;
    unsigned char *buffer = (in_format != QOY_FORMAT_YCBCR420A) ? QOY_MALLOC(qoy_ycbcra_size(desc->width, desc->height, desc->channels)) : (unsigned char*)pixels;
    for (int y = 0; y < internal_height; y += 2) {
        if (in_format != QOY_FORMAT_YCBCR420A) {
            qoy_rgba_to_ycbcra_two_lines(
                pixels + y * desc->width * desc->channels,
                desc->width,
                desc->height != internal_height && y == desc->height - 1 ? 1 : 2,
                in_channels,
                desc->channels,
                buffer
            );
        }
        unsigned char* px_base = buffer;
        for (int x = 0; x < internal_width >> 1; x++, px_base += size_ycbcra) {
            qoy_ycbcr420a_t *px = (qoy_ycbcr420a_t *)px_base;
            px_diff.y[0] = px->y[0] - px_prev.y[2];
            px_diff.y[1] = px->y[1] - px_prev.y[3];
            px_diff.y[2] = px->y[2] - px->y[0];
            px_diff.y[3] = px->y[3] - px->y[1];
            px_diff.cb   = px->cb   - px_prev.cb;
            px_diff.cr   = px->cr   - px_prev.cr;

            int alpha_written = 0;
            if (desc->channels == 4) {
                alpha_written = 1;
                if (px->a[0] == px->a[1] && px->a[0] == px->a[2] && px->a[0] == px->a[3]) {
                    if (px->a[0] != px_prev.a[2]) {
                        bytes[p++] = QOY_OP_A18;
                        bytes[p++] = px->a[0];
                    } else {
                        alpha_written = 0;
                    }
                } else {
                    px_diff.a[0] = px->a[0] - px_prev.a[2];
                    px_diff.a[1] = px->a[1] - px_prev.a[3];
                    px_diff.a[2] = px->a[2] - px->a[0];
                    px_diff.a[3] = px->a[3] - px->a[1];

                    int a_bits;

                    signed char a_min = px_diff.a[0], a_max = px_diff.a[0];
                    if (px_diff.a[1] < a_min) a_min = px_diff.a[1];
                    if (px_diff.a[1] > a_max) a_max = px_diff.a[1];
                    if (px_diff.a[2] < a_min) a_min = px_diff.a[2];
                    if (px_diff.a[2] > a_max) a_max = px_diff.a[2];
                    if (px_diff.a[3] < a_min) a_min = px_diff.a[3];
                    if (px_diff.a[3] > a_max) a_max = px_diff.a[3];

                  //if (a_min      >=  -1 && a_max      <  1) a_bits  = 1; else  UNUSED
                    if (a_min      >=  -2 && a_max      <  2) a_bits  = 2; else
                  //if (a_min      >=  -4 && a_max      <  4) a_bits  = 3; else  UNUSED
                    if (a_min      >=  -8 && a_max      <  8) a_bits  = 4; else
                  //if (a_min      >= -16 && a_max      < 16) a_bits  = 5; else  UNUSED
                  //if (a_min      >= -32 && a_max      < 32) a_bits  = 6; else  UNUSED
                  //if (a_min      >= -64 && a_max      < 64) a_bits  = 7; else  UNUSED
                                                              a_bits  = 8;

                    if        (a_bits <= 2) {
                        bytes[p++] = QOY_OP_A42;
                        bytes[p++] = (px_diff.a[0] + 2) << 6 | (px_diff.a[1] + 2) << 4 | (px_diff.a[2] + 2) << 2 | (px_diff.a[3] + 2);
                    } else if (a_bits <= 4) {
                        bytes[p++] = QOY_OP_A44;
                        bytes[p++] = (px_diff.a[0] + 8) << 4 | (px_diff.a[1] + 8);
                        bytes[p++] = (px_diff.a[2] + 8) << 4 | (px_diff.a[3] + 8);
                    } else {
                        bytes[p++] = QOY_OP_A48;
                        bytes[p++] = px->a[0];
                        bytes[p++] = px->a[1];
                        bytes[p++] = px->a[2];
                        bytes[p++] = px->a[3];
                    }
                }
            }

            if (px_diff.y[0] == 0 && px_diff.y[1] == 0 && px_diff.y[2] == 0 && px_diff.y[3] == 0 && px_diff.y[4] == 0 && px_diff.cb == 0 && px_diff.cr == 0) {
                run++;
                if (alpha_written || run == 32770) run = 1;
                if (run == 1) {
                    bytes[p++] = QOY_OP_RUN_1;
                } else if (run == 2) {
                    bytes[p-1] = QOY_OP_RUN_X;
                    bytes[p++] = run - 2;
                } else if (run < 130) {
                    bytes[p-1] = run - 2;
                } else {
                    if (run == 130) p++;
                    bytes[p-2] = 0x80 | (run - 130) >> 8;
                    bytes[p-1] = (run - 130) & 0xFF;
                }
            } else {
                run = 0;
                int y_bits, cr_bits, cb_bits;

                signed char y_min = px_diff.y[0], y_max = px_diff.y[0];
                if (px_diff.y[1] < y_min) y_min = px_diff.y[1];
                if (px_diff.y[1] > y_max) y_max = px_diff.y[1];
                if (px_diff.y[2] < y_min) y_min = px_diff.y[2];
                if (px_diff.y[2] > y_max) y_max = px_diff.y[2];
                if (px_diff.y[3] < y_min) y_min = px_diff.y[3];
                if (px_diff.y[3] > y_max) y_max = px_diff.y[3];

              //if (y_min      >=  -1 && y_max      <  1) y_bits  = 1; else  UNUSED
              //if (y_min      >=  -2 && y_max      <  2) y_bits  = 2; else  UNUSED
                if (y_min      >=  -4 && y_max      <  4) y_bits  = 3; else
                if (y_min      >=  -8 && y_max      <  8) y_bits  = 4; else
                if (y_min      >= -16 && y_max      < 16) y_bits  = 5; else
                if (y_min      >= -32 && y_max      < 32) y_bits  = 6; else
              //if (y_min      >= -64 && y_max      < 64) y_bits  = 7; else  UNUSED
                                                          y_bits  = 8;

              //if (px_diff.cb >=  -1 && px_diff.cb <  1) cb_bits = 1; else  UNUSED
                if (px_diff.cb >=  -2 && px_diff.cb <  2) cb_bits = 2; else
                if (px_diff.cb >=  -4 && px_diff.cb <  4) cb_bits = 3; else
              //if (px_diff.cb >=  -8 && px_diff.cb <  8) cb_bits = 4; else  UNUSED
                if (px_diff.cb >= -16 && px_diff.cb < 16) cb_bits = 5; else
                if (px_diff.cb >= -32 && px_diff.cb < 32) cb_bits = 6; else
              //if (px_diff.cb >= -64 && px_diff.cb < 64) cb_bits = 7; else  UNUSED
                                                          cb_bits = 8;

                if (px_diff.cr >=  -1 && px_diff.cr <  1) cr_bits = 1; else
              //if (px_diff.cr >=  -2 && px_diff.cr <  2) cr_bits = 2; else  UNUSED
                if (px_diff.cr >=  -4 && px_diff.cr <  4) cr_bits = 3; else
                if (px_diff.cr >=  -8 && px_diff.cr <  8) cr_bits = 4; else
                if (px_diff.cr >= -16 && px_diff.cr < 16) cr_bits = 5; else
                if (px_diff.cr >= -32 && px_diff.cr < 32) cr_bits = 6; else
              //if (px_diff.cr >= -64 && px_diff.cr < 64) cr_bits = 7; else  UNUSED
                                                          cr_bits = 8;

                if      (y_bits <= 3 && cb_bits <= 2 && cr_bits <= 1) {
                    bytes[p++] = QOY_OP_321 | (px_diff.y[0] + 4) << 4 | (px_diff.y[1] + 4) << 1 | (px_diff.y[2] + 4) >> 2;
                    bytes[p++] = (px_diff.y[2] + 4) << 6 | (px_diff.y[3] + 4) << 3 | (px_diff.cb + 2) << 1 | (px_diff.cr + 1);
                } else if (y_bits <= 4 && cb_bits <= 3 && cr_bits <= 3) {
                    bytes[p++] = QOY_OP_433 | (px_diff.y[0] + 8) << 2 | (px_diff.y[1] + 8) >> 2;
                    bytes[p++] = (px_diff.y[1] + 8) << 6 | (px_diff.y[2] + 8) << 2 | (px_diff.y[3] + 8) >> 2;
                    bytes[p++] = (px_diff.y[3] + 8) << 6 | (px_diff.cb + 4) << 3 | (px_diff.cr + 4);
                } else if (y_bits <= 5 && cb_bits <= 5 && cr_bits <= 4) {
                    bytes[p++] = QOY_OP_554 | (px_diff.y[0] + 16);
                    bytes[p++] = (px_diff.y[1] + 16) << 3 | (px_diff.y[2] + 16) >> 2;
                    bytes[p++] = (px_diff.y[2] + 16) << 6 | (px_diff.y[3] + 16) << 1 | (px_diff.cb + 16) >> 4;
                    bytes[p++] = (px_diff.cb + 16) << 4 | (px_diff.cr + 8);
                } else if (y_bits <= 6 && cb_bits <= 6 && cr_bits <= 6) {
                    bytes[p++] = QOY_OP_666 | (px_diff.y[0] + 32) >> 2;
                    bytes[p++] = (px_diff.y[0] + 32) << 6 | (px_diff.y[1] + 32);
                    bytes[p++] = (px_diff.y[2] + 32) << 2 | (px_diff.y[3] + 32) >> 4;
                    bytes[p++] = (px_diff.y[3] + 32) << 4 | (px_diff.cb + 32) >> 2;
                    bytes[p++] = (px_diff.cb + 32) << 6 | (px_diff.cr + 32);
                } else if (y_bits <= 8 && cb_bits <= 6 && cr_bits <= 5) {
                    bytes[p++] = QOY_OP_865 | (px_diff.y[0] + 128) >> 5;
                    bytes[p++] = (px_diff.y[0] + 128) << 3 | (px_diff.y[1] + 128) >> 5;
                    bytes[p++] = (px_diff.y[1] + 128) << 3 | (px_diff.y[2] + 128) >> 5;
                    bytes[p++] = (px_diff.y[2] + 128) << 3 | (px_diff.y[3] + 128) >> 5;
                    bytes[p++] = (px_diff.y[3] + 128) << 3 | (px_diff.cb + 32) >> 3;
                    bytes[p++] = (px_diff.cb + 32) << 5 | (px_diff.cr + 16);
                } else {
                    bytes[p++] = QOY_OP_888;
                    bytes[p++] = px->y[0];
                    bytes[p++] = px->y[1];
                    bytes[p++] = px->y[2];
                    bytes[p++] = px->y[3];
                    bytes[p++] = px->cb;
                    bytes[p++] = px->cr;
                }
            }

            px_prev = *px;
        }
        if (in_format == QOY_FORMAT_YCBCR420A) buffer += size_ycbcra * (internal_width >> 1);
    }
    if (in_format != QOY_FORMAT_YCBCR420A) QOY_FREE(buffer);

    for (int i = 0; i < (int)sizeof(qoy_padding); i++) {
        bytes[p++] = qoy_padding[i];
    }

    *out_len = p;
    return bytes;
}

void *qoy_decode(const void *data, int size, qoy_desc *desc, int out_channels, int out_format) {
    if (
        data == NULL || desc == NULL ||
        size < QOY_HEADER_SIZE + (int)sizeof(qoy_padding)
    ) {
        return NULL;
    }

    const unsigned char *bytes = (const unsigned char *)data;

    int p = 0;
    unsigned int header_magic = qoy_read_32(bytes, &p);
    desc->width = qoy_read_32(bytes, &p);
    desc->height = qoy_read_32(bytes, &p);
    desc->channels = bytes[p++];
    desc->colorspace = bytes[p++];
    if (out_channels == 0) out_channels = desc->channels;

    int internal_width = (desc->width + 1) & ~0x01;
    int internal_height = (desc->height + 1) & ~0x01;

    if (
        desc->width == 0 || desc->height == 0 ||
        desc->channels < 3 || desc->channels > 4 ||
        out_channels < 3 || out_channels > 4 ||
        desc->colorspace > 1 ||
        header_magic != QOY_MAGIC ||
        internal_height >= QOY_PIXELS_MAX / internal_width ||
        out_format > 1
    ) {
        return NULL;
    }

    unsigned char *pixels = (unsigned char *)QOY_MALLOC(out_format == QOY_FORMAT_YCBCR420A ? qoy_ycbcra_size(desc->width, desc->height, out_channels) : desc->width * desc->height * out_channels);
    if (!pixels) {
        return NULL;
    }

    qoy_ycbcr420a_t px = {0};
    px.a[0] = 255;
    px.a[1] = 255;
    px.a[2] = 255;
    px.a[3] = 255;

    int run = 0;
    int size_ycbcra = (out_channels == 4) ? 10 : 6;
    int chunks_len = size - (int)sizeof(qoy_padding);
    unsigned char *buffer = (out_format != QOY_FORMAT_YCBCR420A) ? QOY_MALLOC(qoy_ycbcra_size(desc->width, desc->height, out_channels)) : (unsigned char *)pixels;
    for (int y = 0; y < internal_height; y += 2) {
        unsigned char *px_write = buffer;
        for (int x = 0; x < internal_width >> 1; x++, px_write += size_ycbcra) {
            if (run > 0) {
                px.y[0] = px.y[2];
                px.y[1] = px.y[3];
                if (desc->channels == 4) {
                    px.a[0] = px.a[2];
                    px.a[1] = px.a[2];
                    px.a[3] = px.a[2];
                }
                run--;
            } else {
                if (p >= chunks_len) {
                    QOY_FREE(pixels);
                    return NULL;
                }

                unsigned char b1 = bytes[p++];
                if (desc->channels == 4 && run == 0) {
                    if ((b1 & QOY_OP_A_MASK) == QOY_OP_A_ANY) {
                        if        (b1 == QOY_OP_A18) {
                            px.a[0] = bytes[p++];
                            px.a[1] = px.a[0];
                            px.a[2] = px.a[0];
                            px.a[3] = px.a[0];
                        } else if (b1 == QOY_OP_A42) {
                            unsigned char b2 = bytes[p++];
                            px.a[0] = px.a[2] + ((b2 >> 6) & 0x03) - 2;
                            px.a[1] = px.a[3] + ((b2 >> 4) & 0x03) - 2;
                            px.a[2] = px.a[0] + ((b2 >> 2) & 0x03) - 2;
                            px.a[3] = px.a[1] +  (b2 & 0x03) - 2;
                        } else if (b1 == QOY_OP_A44) {
                            unsigned char b2 = bytes[p++];
                            unsigned char b3 = bytes[p++];
                            px.a[0] = px.a[2] + ((b2 >> 4) & 0x0F) - 8;
                            px.a[1] = px.a[3] +  (b2 & 0x0F) - 8;
                            px.a[2] = px.a[0] + ((b3 >> 4) & 0x0F) - 8;
                            px.a[3] = px.a[1] +  (b3 & 0x0F) - 8;
                        } else if (b1 == QOY_OP_A48) {
                            px.a[0] = bytes[p++];
                            px.a[1] = bytes[p++];
                            px.a[2] = bytes[p++];
                            px.a[3] = bytes[p++];
                        }
                        b1 = bytes[p++];
                    } else {
                        px.a[0] = px.a[2];
                        px.a[1] = px.a[2];
                        px.a[3] = px.a[2];
                    }
                }

                if        ((b1 & QOY_OP_EOF_MASK) == QOY_OP_EOF) {
                    QOY_FREE(pixels);
                    return NULL;
                } else if ((b1 & QOY_OP_RUN_MASK) == QOY_OP_RUN_1) {
                    px.y[0] = px.y[2];
                    px.y[1] = px.y[3];
                } else if ((b1 & QOY_OP_RUN_MASK) == QOY_OP_RUN_X) {
                    px.y[0] = px.y[2];
                    px.y[1] = px.y[3];
                    unsigned char b2 = bytes[p++];
                    if (b2 < 128) {
                        run = b2 + 2 - 1;
                    } else {
                        unsigned char b3 = bytes[p++];
                        run = ((b2 & 0x7F) << 8 | b3) + 130 - 1;
                    }
                } else if ((b1 & QOY_OP_888_MASK) == QOY_OP_888) {
                    px.y[0] = bytes[p++];
                    px.y[1] = bytes[p++];
                    px.y[2] = bytes[p++];
                    px.y[3] = bytes[p++];
                    px.cb   = bytes[p++];
                    px.cr   = bytes[p++];
                } else if ((b1 & QOY_OP_321_MASK) == QOY_OP_321) {
                    unsigned char b2 = bytes[p++];
                    px.y[0] = px.y[2] + ((b1 >> 4) & 0x07) - 4;
                    px.y[1] = px.y[3] + ((b1 >> 1) & 0x07) - 4;
                    px.y[2] = px.y[0] + ((b1 & 0x01) << 2) + ((b2 >> 6) & 0x03) - 4;
                    px.y[3] = px.y[1] + ((b2 >> 3) & 0x07) - 4;
                    px.cb   = px.cb   + ((b2 >> 1) & 0x03) - 2;
                    px.cr   = px.cr   +  (b2 & 0x01) - 1;
                } else if ((b1 & QOY_OP_433_MASK) == QOY_OP_433) {
                    unsigned char b2 = bytes[p++];
                    unsigned char b3 = bytes[p++];
                    px.y[0] = px.y[2] + ((b1 >> 2) & 0x0F) - 8;
                    px.y[1] = px.y[3] + ((b1 & 0x03) << 2) + ((b2 >> 6) & 0x03) - 8;
                    px.y[2] = px.y[0] + ((b2 >> 2) & 0x0F) - 8;
                    px.y[3] = px.y[1] + ((b2 & 0x03) << 2) + ((b3 >> 6) & 0x03) - 8;
                    px.cb   = px.cb   + ((b3 >> 3) & 0x07) - 4;
                    px.cr   = px.cr   +  (b3 & 0x07) - 4;
                } else if ((b1 & QOY_OP_554_MASK) == QOY_OP_554) {
                    unsigned char b2 = bytes[p++];
                    unsigned char b3 = bytes[p++];
                    unsigned char b4 = bytes[p++];
                    px.y[0] = px.y[2] +  (b1 & 0x1F) - 16;
                    px.y[1] = px.y[3] + ((b2 >> 3) & 0x1F) - 16;
                    px.y[2] = px.y[0] + ((b2 & 0x07) << 2) + ((b3 >> 6) & 0x03) - 16;
                    px.y[3] = px.y[1] + ((b3 >> 1) & 0x1F) - 16;
                    px.cb   = px.cb   + ((b3 & 0x01) << 4) + ((b4 >> 4) & 0x0F) - 16;
                    px.cr   = px.cr   +  (b4 & 0x0F) - 8;
                } else if ((b1 & QOY_OP_666_MASK) == QOY_OP_666) {
                    unsigned char b2 = bytes[p++];
                    unsigned char b3 = bytes[p++];
                    unsigned char b4 = bytes[p++];
                    unsigned char b5 = bytes[p++];
                    px.y[0] = px.y[2] + ((b1 & 0x0F) << 2) + ((b2 >> 6) & 0x03) - 32;
                    px.y[1] = px.y[3] +  (b2 & 0x3F) - 32;
                    px.y[2] = px.y[0] + ((b3 >> 2) & 0x3F) - 32;
                    px.y[3] = px.y[1] + ((b3 & 0x03) << 4) + ((b4 >> 4) & 0x0F) - 32;
                    px.cb   = px.cb   + ((b4 & 0x0F) << 2) + ((b5 >> 6) & 0x03) - 32;
                    px.cr   = px.cr   +  (b5 & 0x3F) - 32;
                } else if ((b1 & QOY_OP_865_MASK) == QOY_OP_865) {
                    unsigned char b2 = bytes[p++];
                    unsigned char b3 = bytes[p++];
                    unsigned char b4 = bytes[p++];
                    unsigned char b5 = bytes[p++];
                    unsigned char b6 = bytes[p++];
                    px.y[0] = px.y[2] + ((b1 & 0x07) << 5) + ((b2 >> 3) & 0x1F) - 128;
                    px.y[1] = px.y[3] + ((b2 & 0x07) << 5) + ((b3 >> 3) & 0x1F) - 128;
                    px.y[2] = px.y[0] + ((b3 & 0x07) << 5) + ((b4 >> 3) & 0x1F) - 128;
                    px.y[3] = px.y[1] + ((b4 & 0x07) << 5) + ((b5 >> 3) & 0x1F) - 128;
                    px.cb   = px.cb   + ((b5 & 0x07) << 3) + ((b6 >> 5) & 0x07) - 32;
                    px.cr   = px.cr   +  (b6 & 0x1F) - 16;
                }
            }

            memcpy(px_write, &px, size_ycbcra);
        }
        if (out_format != QOY_FORMAT_YCBCR420A) {
            qoy_ycbcra_to_rgba_two_lines(
                buffer,
                desc->width,
                desc->height != internal_height && y == desc->height - 1 ? 1 : 2,
                desc->channels,
                out_channels,
                pixels + y * desc->width * out_channels
            );
        } else {
            buffer += size_ycbcra * (internal_width >> 1);
        }
    }
    if (out_format != QOY_FORMAT_YCBCR420A) QOY_FREE(buffer);

    return pixels;
}

#ifndef QOY_NO_STDIO
#include <stdio.h>

int qoy_write(const char *filename, const void *data, const qoy_desc *desc) {
    FILE *f = fopen(filename, "wb");
    int size;
    void *encoded;

    if (!f) {
        return 0;
    }

    encoded = qoy_encode(data, desc, &size, desc->channels, QOY_FORMAT_RGBA);
    if (!encoded) {
        fclose(f);
        return 0;
    }

    fwrite(encoded, 1, size, f);
    fclose(f);

    QOY_FREE(encoded);
    return size;
}

void *qoy_read(const char *filename, qoy_desc *desc, int channels) {
    FILE *f = fopen(filename, "rb");
    int size, bytes_read;
    void *pixels, *data;

    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);

    data = QOY_MALLOC(size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    bytes_read = fread(data, 1, size, f);
    fclose(f);

    pixels = qoy_decode(data, bytes_read, desc, channels, QOY_FORMAT_RGBA);
    QOY_FREE(data);
    return pixels;
}

#endif /* QOY_NO_STDIO */
#endif /* QOY_IMPLEMENTATION */
