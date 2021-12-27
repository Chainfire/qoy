# QOY - The "Quite OK YCbCr420A" format for fast, lossless* image compression
( * colorspace conversion to/from RGBA is lossy, if used )

Single-file MIT licensed library for C/C++

See [qoy.h](./qoy.h) for the documentation and format specification.

Based on Dominic Szablewski's [QOI](https://github.com/phoboslab/qoi),
originally forked at [71ff2ac961c1424116112844f82815935c95c9f6](https://github.com/phoboslab/qoi/tree/71ff2ac961c1424116112844f82815935c95c9f6).


## Why?

See [QOI](https://github.com/phoboslab/qoi) for its motivation.

I just wanted to see how it would work for YCbCr 4:2:0 for fun, so I started
playing with the code. Ended up changing most of the code, which is why this repo
isn't a direct fork. Still, you can clearly see it started out with QOI.


## What?

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


## Limitations

**File format is not yet final! QOY will probably be renamed to something
less confusing.**

As stated above, QOY is lossless for YCbCr data and lossy for RGB data.

The QOY file format allows for huge images with up to 18 exa-pixels. A streaming 
en-/decoder can handle these with minimal RAM requirements, assuming there is 
enough storage space.

This particular implementation of QOY however is limited to images with a 
maximum size of 600 million pixels. It will safely refuse to en-/decode anything
larger than that. This is not a streaming en-/decoder. It loads the whole image
file into RAM before doing any work and is not extensively optimized for 
performance (but it's still very fast).

Colorspace conversion is already implemented on a two-line base, so that should
plug right into streaming code. Colorspace conversion also seems like a good
candidate for SIMD optimization.


## Benchmarks vs QOI

```
## Benchmarking ../images/textures_pk/*.png -- 1 runs

## Total for ../images/textures_pk
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          0.3         0.4        131.15        107.62        76   57.4%
qoy-rgb:      0.2         0.3        179.46        135.25        45   33.7%
qoy-ycc:      0.1         0.2        356.73        187.13        45   33.7%

## Benchmarking ../images/screenshot_game/*.png -- 1 runs

## Total for ../images/screenshot_game
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          3.1         3.6        201.16        176.39       509   24.1%
qoy-rgb:      3.2         3.4        198.28        186.28       287   13.6%
qoy-ycc:      1.5         2.1        420.95        301.34       287   13.6%

## Benchmarking ../images/icon_512/*.png -- 1 runs

## Total for ../images/icon_512
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          0.7         0.7        391.21        376.49        87    8.5%
qoy-rgb:      1.3         1.2        207.61        225.75        64    6.3%
qoy-ycc:      0.6         0.6        471.96        443.12        64    6.3%

## Benchmarking ../images/photo_kodak/*.png -- 1 runs

## Total for ../images/photo_kodak
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          3.0         3.8        131.46        104.41       671   58.3%
qoy-rgb:      2.1         2.8        183.66        138.41       338   29.4%
qoy-ycc:      1.1         2.1        356.51        190.70       338   29.4%

## Benchmarking ../images/textures_photo/*.png -- 1 runs

## Total for ../images/textures_photo
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          8.0         9.4        131.16        112.12      1986   64.7%
qoy-rgb:      5.8         7.5        180.58        138.93      1105   36.0%
qoy-ycc:      3.0         5.4        349.12        192.85      1105   36.0%

## Benchmarking ../images/textures_pk01/*.png -- 1 runs

## Total for ../images/textures_pk01
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          0.8         0.9        169.48        151.34       164   32.4%
qoy-rgb:      0.7         0.9        184.10        152.45        95   18.9%
qoy-ycc:      0.4         0.6        371.01        229.91        95   18.9%

## Benchmarking ../images/icon_64/*.png -- 1 runs

## Total for ../images/icon_64
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          0.0         0.0        214.06        179.88         4   29.3%
qoy-rgb:      0.0         0.0        174.51        153.14         3   20.0%
qoy-ycc:      0.0         0.0        382.30        259.63         3   20.0%

## Benchmarking ../images/photo_wikipedia/*.png -- 1 runs

## Total for ../images/photo_wikipedia
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          8.7        10.8        124.76        100.50      1998   62.9%
qoy-rgb:      5.9         7.9        183.52        136.66       926   29.2%
qoy-ycc:      3.0         5.8        359.83        185.50       926   29.2%

## Benchmarking ../images/textures_plants/*.png -- 1 runs

## Total for ../images/textures_plants
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          4.1         4.9        256.86        218.67       922   22.2%
qoy-rgb:      5.7         6.1        188.04        173.92       464   11.2%
qoy-ycc:      2.5         3.8        418.94        282.28       464   11.2%

## Benchmarking ../images/pngimg/*.png -- 1 runs

## Total for ../images/pngimg
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          7.9         8.3        230.49        219.55      1386   19.6%
qoy-rgb:      9.8         9.7        185.02        186.89       651    9.2%
qoy-ycc:      4.5         5.7        401.25        317.93       652    9.2%

## Benchmarking ../images/photo_tecnick/*.png -- 1 runs

## Total for ../images/photo_tecnick
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:         11.1        14.3        129.97        101.04      2450   58.1%
qoy-rgb:      7.7        10.2        186.72        140.52      1116   26.5%
qoy-ycc:      3.9         7.4        368.78        193.82      1116   26.5%

## Benchmarking ../images/screenshot_web/*.png -- 1 runs

## Total for ../images/screenshot_web
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:         24.2        21.9        335.21        370.86      2704    8.5%
qoy-rgb:     41.5        36.2        195.79        224.61      1698    5.4%
qoy-ycc:     18.7        18.3        435.20        445.01      1697    5.3%

## Benchmarking ../images/textures_pk02/*.png -- 1 runs

## Total for ../images/textures_pk02
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          2.2         2.3        136.49        134.83       448   37.8%
qoy-rgb:      1.7         2.1        181.77        146.19       256   21.6%
qoy-ycc:      0.8         1.4        358.54        214.76       256   21.6%

# Grand total for ../images
        decode ms   encode ms   decode mpps   encode mpps   size kb    rate
qoi:          2.4         2.7        192.64        169.86       451   27.5%
qoy-rgb:      2.4         2.7        190.35        174.32       234   14.3%
qoy-ycc:      1.2         1.7        401.19        275.66       234   14.3%
```