#ifndef PILLOW_RESIZE_COPY
#define PILLOW_RESIZE_COPY

#include "ImPlatformSim.h"
#include "ImagingSim.h"


static Imaging
_copy(Imaging imOut, Imaging imIn)
{
    ImagingSectionCookie cookie;
    int y;

    if (!imIn) {
        return (Imaging) ImagingError_ValueError(NULL);
    }

    imOut = ImagingNew2Dirty(imIn->mode, imOut, imIn);
    if (!imOut) {
        return NULL;
    }

    // ImagingCopyPalette(imOut, imIn);

    ImagingSectionEnter(&cookie);
    if (imIn->block != NULL && imOut->block != NULL) {
        memcpy(imOut->block, imIn->block, imIn->ysize * imIn->linesize);
    } else {
        for (y = 0; y < imIn->ysize; y++) {
            memcpy(imOut->image[y], imIn->image[y], imIn->linesize);
        }
    }
    ImagingSectionLeave(&cookie);

    return imOut;
}

Imaging
ImagingCopy(Imaging imIn)
{
    return _copy(NULL, imIn);
}

Imaging
ImagingCopy2(Imaging imOut, Imaging imIn)
{
    return _copy(imOut, imIn);
}
#endif
