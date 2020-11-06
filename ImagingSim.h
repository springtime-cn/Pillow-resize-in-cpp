/*
 * The Python Imaging Library
 * $Id$
 *
 * declarations for the imaging core library
 *
 * Copyright (c) 1997-2005 by Secret Labs AB
 * Copyright (c) 1995-2005 by Fredrik Lundh
 *
 * See the README file for information on usage and redistribution.
 */
#ifndef PILLOW_RESIZE_IMAGINGSIM
#define PILLOW_RESIZE_IMAGINGSIM


#include "ImPlatformSim.h"
#include "ImagingUtils.h"
#include <cstdlib>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


#ifndef M_PI
#define M_PI    3.1415926535897932384626433832795
#endif

typedef void* ImagingSectionCookie;


/* -------------------------------------------------------------------- */

/*
 * Image data organization:
 *
 * mode     bytes       byte order
 * -------------------------------
 * 1        1           1
 * L        1           L
 * P        1           P
 * I        4           I (32-bit integer, native byte order)
 * F        4           F (32-bit IEEE float, native byte order)
 * RGB      4           R, G, B, -
 * RGBA     4           R, G, B, A
 * CMYK     4           C, M, Y, K
 * YCbCr    4           Y, Cb, Cr, -
 * Lab      4           L, a, b, -
 *
 * experimental modes (incomplete):
 * LA       4           L, -, -, A
 * PA       4           P, -, -, A
 * I;16     2           I (16-bit integer, native byte order)
 *
 * "P" is an 8-bit palette mode, which should be mapped through the
 * palette member to get an output image.  Check palette->mode to
 * find the corresponding "real" mode.
 *
 * For information on how to access Imaging objects from your own C
 * extensions, see http://www.effbot.org/zone/pil-extending.htm
 */

/* Handles */

//typedef struct ImagingMemoryInstance* Imaging;

//typedef struct ImagingAccessInstance* ImagingAccess;
//typedef struct ImagingHistogramInstance* ImagingHistogram;
//typedef struct ImagingOutlineInstance* ImagingOutline;
//typedef struct ImagingPaletteInstance* ImagingPalette;

/* pixel types */
#define IMAGING_TYPE_UINT8 0
#define IMAGING_TYPE_INT32 1
#define IMAGING_TYPE_FLOAT32 2
#define IMAGING_TYPE_SPECIAL 3 /* check mode for details */

#define IMAGING_MODE_LENGTH 6+1 /* Band names ("1", "L", "P", "RGB", "RGBA", "CMYK", "YCbCr", "BGR;xy") */

struct ImagingMemoryBlock{
    char *ptr;
    int size;
};

struct ImagingPaletteInstance {

    /* Format */
    char mode[IMAGING_MODE_LENGTH];     /* Band names */

    /* Data */
    UINT8 palette[1024];/* Palette data (same format as image data) */

    INT16* cache;       /* Palette cache (used for predefined palettes) */
    int keep_cache;     /* This palette will be reused; keep cache */

};
typedef ImagingPaletteInstance* ImagingPalette;
#define ImagingPaletteCache(p, r, g, b)\
    p->cache[(r>>2) + (g>>2)*64 + (b>>2)*64*64]

struct ImagingMemoryInstance {
    ImagingMemoryInstance()
    :image8(nullptr), image32(nullptr), image(nullptr), block(nullptr),destroy(nullptr)
    {}

    /* Format */
    char mode[IMAGING_MODE_LENGTH];     /* Band names ("1", "L", "P", "RGB", "RGBA", "CMYK", "YCbCr", "BGR;xy") */
    int type;           /* Data type (IMAGING_TYPE_*) */
    int depth;          /* Depth (ignored in this version) */
    int bands;          /* Number of bands (1, 2, 3, or 4) */
    int xsize;          /* Image dimension. */
    int ysize;

    /* Colour palette (for "P" images only) */
    ImagingPalette palette;

    /* Data pointers */
    UINT8 **image8;     /* Set for 8-bit images (pixelsize=1). */
    INT32 **image32;    /* Set for 32-bit images (pixelsize=4). */

    /* Internals */
    char **image;       /* Actual raster data. */
    char *block;        /* Set if data is allocated in a single block. */
    ImagingMemoryBlock *blocks;     /* Memory blocks for pixel storage */

    int pixelsize;      /* Size of a pixel, in bytes (1, 2 or 4) */
    int linesize;       /* Size of a line, in bytes (xsize * pixelsize) */

    /* Virtual methods */
    void (*destroy)(ImagingMemoryInstance*  im);
};

typedef ImagingMemoryInstance* Imaging;

#define IMAGING_PIXEL_1(im,x,y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_L(im,x,y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_LA(im,x,y) ((im)->image[(y)][(x)*4])
#define IMAGING_PIXEL_P(im,x,y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_PA(im,x,y) ((im)->image[(y)][(x)*4])
#define IMAGING_PIXEL_I(im,x,y) ((im)->image32[(y)][(x)])
#define IMAGING_PIXEL_F(im,x,y) (((FLOAT32*)(im)->image32[y])[x])
#define IMAGING_PIXEL_RGB(im,x,y) ((im)->image[(y)][(x)*4])
#define IMAGING_PIXEL_RGBA(im,x,y) ((im)->image[(y)][(x)*4])
#define IMAGING_PIXEL_CMYK(im,x,y) ((im)->image[(y)][(x)*4])
#define IMAGING_PIXEL_YCbCr(im,x,y) ((im)->image[(y)][(x)*4])

#define IMAGING_PIXEL_UINT8(im,x,y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_INT32(im,x,y) ((im)->image32[(y)][(x)])
#define IMAGING_PIXEL_FLOAT32(im,x,y) (((FLOAT32*)(im)->image32[y])[x])

struct ImagingAccessInstance {
  const char* mode;
  void* (*line)(Imaging im, int x, int y);
  void (*get_pixel)(Imaging im, int x, int y, void* pixel);
  void (*put_pixel)(Imaging im, int x, int y, const void* pixel);
};

struct ImagingHistogramInstance {

    /* Format */
    char mode[IMAGING_MODE_LENGTH];     /* Band names (of corresponding source image) */
    int bands;          /* Number of bands (1, 3, or 4) */

    /* Data */
    long *histogram;    /* Histogram (bands*256 longs) */

};

struct ImagingMemoryArenaStruct {
    int alignment;        /* Alignment in memory of each line of an image */
    int block_size;       /* Preferred block size, bytes */
    int blocks_max;       /* Maximum number of cached blocks */
    int blocks_cached;    /* Current number of blocks not associated with images */
    ImagingMemoryBlock *blocks_pool;
    int stats_new_count;           /* Number of new allocated images */
    int stats_allocated_blocks;    /* Number of allocated blocks */
    int stats_reused_blocks;       /* Number of blocks which were retrieved from a pool */
    int stats_reallocated_blocks;  /* Number of blocks which were actually reallocated after retrieving */
    int stats_freed_blocks;        /* Number of freed blocks */
};
typedef ImagingMemoryArenaStruct* ImagingMemoryArena;

void
ImagingMemoryClearCache(ImagingMemoryArena arena, int new_size);

/* standard transforms */
#define IMAGING_TRANSFORM_AFFINE 0
#define IMAGING_TRANSFORM_PERSPECTIVE 2
#define IMAGING_TRANSFORM_QUAD 3


/* standard filters */
#define IMAGING_TRANSFORM_NEAREST 0
#define IMAGING_TRANSFORM_BOX 4
#define IMAGING_TRANSFORM_BILINEAR 2
#define IMAGING_TRANSFORM_HAMMING 5
#define IMAGING_TRANSFORM_BICUBIC 3
#define IMAGING_TRANSFORM_LANCZOS 1

typedef int (*ImagingTransformMap)(double* X, double* Y,
                                   int x, int y, void* data);
typedef int (*ImagingTransformFilter)(void* out, Imaging im,
                                      double x, double y);
//
//
/////* Errcodes */
////#define IMAGING_CODEC_END        1
////#define IMAGING_CODEC_OVERRUN   -1
////#define IMAGING_CODEC_BROKEN    -2
////#define IMAGING_CODEC_UNKNOWN   -3
////#define IMAGING_CODEC_CONFIG    -8
////#define IMAGING_CODEC_MEMORY    -9
//
//
////#include "ImagingUtils.h"
//extern UINT8 *clip8_lookups;
//
// Storage.h
extern ImagingMemoryArenaStruct ImagingDefaultArena;
Imaging
ImagingNewPrologueSubtype(const char *mode, int xsize, int ysize, int size);
Imaging
ImagingNewPrologue(const char *mode, int xsize, int ysize);
void
ImagingDelete(Imaging im);
int
ImagingMemorySetBlocksMax(ImagingMemoryArena arena, int blocks_max);
void
ImagingMemoryClearCache(ImagingMemoryArena arena, int new_size);
ImagingMemoryBlock
memory_get_block(ImagingMemoryArena arena, int requested_size, int dirty);
void
memory_return_block(ImagingMemoryArena arena, ImagingMemoryBlock block);
static void
ImagingDestroyArray(Imaging im);
Imaging
ImagingAllocateArray(Imaging im, int dirty, int block_size);
static void
ImagingDestroyBlock(Imaging im);
Imaging
ImagingAllocateBlock(Imaging im);
Imaging
ImagingNewInternal(const char* mode, int xsize, int ysize, int dirty);
Imaging
ImagingNew(const char* mode, int xsize, int ysize);
Imaging
ImagingNewDirty(const char* mode, int xsize, int ysize);
Imaging
ImagingNewBlock(const char* mode, int xsize, int ysize);
Imaging
ImagingNew2Dirty(const char* mode, Imaging imOut, Imaging imIn);
void
ImagingCopyPalette(Imaging destination, Imaging source);
/**********************************************************
 * openCV Mat to Imaging
 * now support mode are: "L", "RGB", "RGBA", "CMYK", "YCbCr", "LAB", "HSV"
 * *******************************************************/
Imaging cvMatToImaging(const cv::Mat &imgMat, const char* mode="RGB");
/**********************************************************
 * Imaging to openCV Mat
 * now support mode are: "L", "RGB", "RGBA", "CMYK", "YCbCr", "LAB", "HSV"
 * *******************************************************/
int ImagingTocvMat(Imaging img, cv::Mat &imgMat);

/**********************************************************
 * Imaging to openCV Mat
 * now support mode are: "L", "RGB", "RGBA", "CMYK", "YCbCr", "LAB", "HSV"
 * *******************************************************/
int pillowResize(cv::Mat &imgIn, cv::Mat &imgOut, const char* mode, int width, int height, int filter=IMAGING_TRANSFORM_NEAREST);
// Palette
ImagingPalette
ImagingPaletteNew(const char* mode);
ImagingPalette
ImagingPaletteNewBrowser(void);
ImagingPalette
ImagingPaletteDuplicate(ImagingPalette palette);
void
ImagingPaletteDelete(ImagingPalette palette);
void
ImagingPaletteCacheUpdate(ImagingPalette palette, int r, int g, int b);
int
ImagingPaletteCachePrepare(ImagingPalette palette);
void
ImagingPaletteCacheDelete(ImagingPalette palette);
// Copy
static Imaging
_copy(Imaging imOut, Imaging imIn);
Imaging
ImagingCopy(Imaging imIn);
Imaging
ImagingCopy2(Imaging imOut, Imaging imIn);
// Except
void *ImagingError_ModeError(void);
void *ImagingError_ValueError(const char *message);
void *
ImagingError_OSError(void);
void *
ImagingError_MemoryError(void);
void *
ImagingError_ModeError(void);
void *
ImagingError_Mismatch(void);
void *
ImagingError_ValueError(const char *message);
void
ImagingError_Clear(void);
typedef void* ImagingSectionCookie;
void
ImagingSectionEnter(ImagingSectionCookie* cookie);
void
ImagingSectionLeave(ImagingSectionCookie* cookie);
// Resample.cpp
Imaging ImagingResample(Imaging imIn, int xsize, int ysize, int filter, float box[4]);
// Geometry.cpp
Imaging
ImagingTransform(Imaging imOut, Imaging imIn, int method,
                 int x0, int y0, int x1, int y1,
                 double a[8], int filterid, int fill);
#endif
