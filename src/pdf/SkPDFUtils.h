/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFUtils_DEFINED
#define SkPDFUtils_DEFINED

#include "SkPaint.h"
#include "SkPath.h"
#include "SkStream.h"

class SkMatrix;
class SkPDFArray;
struct SkRect;

#if 0
#define PRINT_NOT_IMPL(str) fprintf(stderr, str)
#else
#define PRINT_NOT_IMPL(str)
#endif

#define NOT_IMPLEMENTED(condition, assert)                         \
    do {                                                           \
        if ((bool)(condition)) {                                   \
            PRINT_NOT_IMPL("NOT_IMPLEMENTED: " #condition "\n");   \
            SkDEBUGCODE(SkASSERT(!assert);)                        \
        }                                                          \
    } while (0)

namespace SkPDFUtils {

sk_sp<SkPDFArray> RectToArray(const SkRect& rect);
sk_sp<SkPDFArray> MatrixToArray(const SkMatrix& matrix);
void AppendTransform(const SkMatrix& matrix, SkWStream* content);

void MoveTo(SkScalar x, SkScalar y, SkWStream* content);
void AppendLine(SkScalar x, SkScalar y, SkWStream* content);
void AppendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                 SkScalar ctl2X, SkScalar ctl2Y,
                 SkScalar dstX, SkScalar dstY, SkWStream* content);
void AppendRectangle(const SkRect& rect, SkWStream* content);
void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                     bool doConsumeDegerates, SkWStream* content);
inline void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                     SkWStream* content) {
    SkPDFUtils::EmitPath(path, paintStyle, true, content);
}
void ClosePath(SkWStream* content);
void PaintPath(SkPaint::Style style, SkPath::FillType fill,
                      SkWStream* content);
void StrokePath(SkWStream* content);
void DrawFormXObject(int objectIndex, SkWStream* content);
void ApplyGraphicState(int objectIndex, SkWStream* content);
void ApplyPattern(int objectIndex, SkWStream* content);

// 3 = '-', '.', and '\0' characters.
// 9 = number of significant digits
// abs(FLT_MIN_10_EXP) = number of zeros in FLT_MIN
const size_t kMaximumFloatDecimalLength = 3 + 9 - FLT_MIN_10_EXP;
// FloatToDecimal is exposed for unit tests.
size_t FloatToDecimal(float value,
                      char output[kMaximumFloatDecimalLength]);
void AppendScalar(SkScalar value, SkWStream* stream);
void WriteString(SkWStream* wStream, const char* input, size_t len);

inline void WriteUInt16BE(SkDynamicMemoryWStream* wStream, uint16_t value) {
    static const char gHex[] = "0123456789ABCDEF";
    char result[4];
    result[0] = gHex[       value >> 12 ];
    result[1] = gHex[0xF & (value >> 8 )];
    result[2] = gHex[0xF & (value >> 4 )];
    result[3] = gHex[0xF & (value      )];
    wStream->write(result, 4);
}

}  // namespace SkPDFUtils

#endif
