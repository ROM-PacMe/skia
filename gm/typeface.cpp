/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "SkSurfaceProps.h"
#include "SkTypeface.h"
#include "SkTypes.h"

static void getGlyphPositions(const SkPaint& paint, const uint16_t glyphs[],
                             int count, SkScalar x, SkScalar y, SkPoint pos[]) {
    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());

    SkAutoSTMalloc<128, SkScalar> widthStorage(count);
    SkScalar* widths = widthStorage.get();
    paint.getTextWidths(glyphs, count * sizeof(uint16_t), widths);

    for (int i = 0; i < count; ++i) {
        pos[i].set(x, y);
        x += widths[i];
    }
}

static void applyKerning(SkPoint pos[], const int32_t adjustments[], int count,
                         const SkPaint& paint) {
    SkScalar scale = paint.getTextSize() / paint.getTypeface()->getUnitsPerEm();

    SkScalar globalAdj = 0;
    for (int i = 0; i < count - 1; ++i) {
        globalAdj += adjustments[i] * scale;
        pos[i + 1].fX += globalAdj;
    }
}

static void drawKernText(SkCanvas* canvas, const void* text, size_t len,
                         SkScalar x, SkScalar y, const SkPaint& paint) {
    SkTypeface* face = paint.getTypeface();
    if (!face) {
        canvas->drawText(text, len, x, y, paint);
        return;
    }

    SkAutoSTMalloc<128, uint16_t> glyphStorage(len);
    uint16_t* glyphs = glyphStorage.get();
    int glyphCount = paint.textToGlyphs(text, len, glyphs);
    if (glyphCount < 1) {
        return;
    }

    SkAutoSTMalloc<128, int32_t> adjustmentStorage(glyphCount - 1);
    int32_t* adjustments = adjustmentStorage.get();
    if (!face->getKerningPairAdjustments(glyphs, glyphCount, adjustments)) {
        canvas->drawText(text, len, x, y, paint);
        return;
    }

    SkPaint glyphPaint(paint);
    glyphPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkAutoSTMalloc<128, SkPoint> posStorage(glyphCount);
    SkPoint* pos = posStorage.get();
    getGlyphPositions(glyphPaint, glyphs, glyphCount, x, y, pos);

    applyKerning(pos, adjustments, glyphCount, glyphPaint);
    canvas->drawPosText(glyphs, glyphCount * sizeof(uint16_t), pos, glyphPaint);
}

static const struct {
    const char* fName;
    SkTypeface::Style   fStyle;
} gFaceStyles[] = {
    { "sans-serif", SkTypeface::kNormal },
    { "sans-serif", SkTypeface::kBold },
    { "sans-serif", SkTypeface::kItalic },
    { "sans-serif", SkTypeface::kBoldItalic },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal },
    { "monospace", SkTypeface::kBold },
    { "monospace", SkTypeface::kItalic },
    { "monospace", SkTypeface::kBoldItalic },
};

static const int gFaceStylesCount = SK_ARRAY_COUNT(gFaceStyles);

class TypefaceStylesGM : public skiagm::GM {
    sk_sp<SkTypeface> fFaces[gFaceStylesCount];
    bool fApplyKerning;

public:
    TypefaceStylesGM(bool applyKerning)
        : fApplyKerning(applyKerning) {
        memset(fFaces, 0, sizeof(fFaces));
    }

protected:
    void onOnceBeforeDraw() override {
        for (int i = 0; i < gFaceStylesCount; i++) {
            fFaces[i] = SkTypeface::MakeFromName(
                    sk_tool_utils::platform_font_name(
                        gFaceStyles[i].fName), SkFontStyle::FromOldStyle(gFaceStyles[i].fStyle));
        }
    }

    SkString onShortName() override {
        SkString name("typefacestyles");
        if (fApplyKerning) {
            name.append("_kerning");
        }
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(30));

        const char* text = fApplyKerning ? "Type AWAY" : "Hamburgefons";
        const size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar dy = paint.getFontMetrics(nullptr);
        SkScalar y = dy;

        if (fApplyKerning) {
            paint.setSubpixelText(true);
        } else {
            paint.setLinearText(true);
        }
        for (int i = 0; i < gFaceStylesCount; i++) {
            paint.setTypeface(fFaces[i]);
            canvas->drawText(text, textLen, x, y, paint);
            if (fApplyKerning) {
                drawKernText(canvas, text, textLen, x + 240, y, paint);
            }
            y += dy;
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

static void rotate_about(SkCanvas* canvas,
                         SkScalar degrees,
                         SkScalar px, SkScalar py) {
    canvas->translate(px, py);
    canvas->rotate(degrees);
    canvas->translate(-px, -py);
}

class TypefaceRenderingGM : public skiagm::GM {
    sk_sp<SkTypeface> fFace;

public:
    TypefaceRenderingGM() { }

protected:
    void onOnceBeforeDraw() override {
        fFace = MakeResourceAsTypeface("/fonts/hintgasp.ttf");
    }

    SkString onShortName() override {
        SkString name("typefacerendering");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        struct AliasType {
            bool antiAlias;
            bool subpixelAntitalias;
            bool inLayer;
        } constexpr aliasTypes[] {
            { false, false, false },  // aliased
            { true,  false, false },  // anti-aliased
            { true,  true , false },  // subpixel anti-aliased
            { true,  false, true  },  // anti-aliased in layer (flat pixel geometry)
            { true,  true , true  },  // subpixel anti-aliased in layer (flat pixel geometry)
        };

        // The hintgasp.ttf is designed for the following sizes to be different.
        // GASP_DOGRAY                                      0x0002   0<=ppem<=10
        // GASP_SYMMETRIC_SMOOTHING                         0x0008   0<=ppem<=10
        // GASP_GRIDFIT                                     0x0001  11<=ppem<=12
        // GASP_SYMMETRIC_GRIDFIT                           0x0004  11<=ppem<=12
        // GASP_DOGRAY|GASP_GRIDFIT                         0x0003  13<=ppem<=14
        // GASP_SYMMETRIC_SMOOTHING|GASP_SYMMETRIC_GRIDFIT  0x000C  13<=ppem<=14
        // (neither)                                        0x0000  15<=ppem
        constexpr SkScalar textSizes[] = { 10, 12, 14, 16 };

        constexpr SkPaint::Hinting hintingTypes[] = { SkPaint::kNo_Hinting,
                                                      SkPaint::kSlight_Hinting,
                                                      SkPaint::kNormal_Hinting,
                                                      SkPaint::kFull_Hinting };

        struct SubpixelType {
            bool requested;
            SkVector offset;
        } constexpr subpixelTypes[] = {
            { false, { 0.00, 0.00 } },
            { true , { 0.00, 0.00 } },
            { true , { 0.25, 0.00 } },
            { true , { 0.25, 0.25 } },
        };

        constexpr bool rotateABitTypes[] = { false, true };

        SkPaint paint;
        SkScalar x = 0;
        SkScalar xMax = x;
        SkScalar xBase = 0;
        SkScalar y = 0;  // The baseline of the previous output
        for (const SubpixelType subpixel : subpixelTypes) {
            y = 0;
            paint.setSubpixelText(subpixel.requested);

            for (const AliasType& alias : aliasTypes) {
                paint.setAntiAlias(alias.antiAlias);
                paint.setLCDRenderText(alias.subpixelAntitalias);
                SkAutoCanvasRestore acr(canvas, false);
                if (alias.inLayer) {
                    canvas->saveLayer(nullptr, &paint);
                }

                for (const SkScalar& textSize : textSizes) {
                    x = xBase + 5;
                    paint.setTextSize(textSize);

                    SkScalar dy = SkScalarCeilToScalar(paint.getFontMetrics(nullptr));
                    y += dy;
                    for (const SkPaint::Hinting& hinting : hintingTypes) {
                        paint.setHinting(hinting);

                        for (const bool& rotateABit : rotateABitTypes) {
                            SkAutoCanvasRestore acr(canvas, true);
                            if (rotateABit) {
                                rotate_about(canvas, 2, x + subpixel.offset.x(),
                                                        y + subpixel.offset.y());
                            }
                            canvas->drawText("A", 1, x + subpixel.offset.x(),
                                                     y + subpixel.offset.y(), paint);

                            SkScalar dx = SkScalarCeilToScalar(paint.measureText("A", 1)) + 5;
                            x += dx;
                            xMax = SkTMax(x, xMax);
                        }
                    }
                }
                y += 10;
            }
            xBase = xMax;
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new TypefaceStylesGM(false); )
DEF_GM( return new TypefaceStylesGM(true); )
DEF_GM( return new TypefaceRenderingGM(); )
