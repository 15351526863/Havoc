#pragma once
#include "../datatypes/color.h"
#include "../datatypes/vector.h"

typedef unsigned long HScheme, HPanel, HTexture, HCursor, HFont;

enum EFontDrawType : int
{
    FONT_DRAW_DEFAULT = 0,
    FONT_DRAW_NONADDITIVE,
    FONT_DRAW_ADDITIVE,
    FONT_DRAW_TYPE_COUNT = 2
};
enum EFontFlags
{
    FONTFLAG_NONE,
    FONTFLAG_ITALIC = 0x001,
    FONTFLAG_UNDERLINE = 0x002,
    FONTFLAG_STRIKEOUT = 0x004,
    FONTFLAG_SYMBOL = 0x008,
    FONTFLAG_ANTIALIAS = 0x010,
    FONTFLAG_GAUSSIANBLUR = 0x020,
    FONTFLAG_ROTARY = 0x040,
    FONTFLAG_DROPSHADOW = 0x080,
    FONTFLAG_ADDITIVE = 0x100,
    FONTFLAG_OUTLINE = 0x200,
    FONTFLAG_CUSTOM = 0x400,
    FONTFLAG_BITMAP = 0x800
};

struct Vertex_t
{
    Vertex_t() = default;
    Vertex_t(const Vector2D& p, const Vector2D& c = Vector2D(0, 0))
    {
        vecPosition = p;
        vecCoordinate = c;
    }
    void Init(const Vector2D& p, const Vector2D& c = Vector2D(0, 0))
    {
        vecPosition = p;
        vecCoordinate = c;
    }
    Vector2D vecPosition{};
    Vector2D vecCoordinate{};
};

enum FontRenderFlag_t
{
    FONT_LEFT = 0,
    FONT_RIGHT = 1 << 1,
    FONT_CENTER = 1 << 2,
    FONT_OUTLINE = 1 << 3
};

class ISurface
{
public:
    void DrawSetColor(Color col) { 
        MEM::CallVFunc<void>(this, 14, col); 
    }

    void DrawSetColor(int r, int g, int b, int a) {
        MEM::CallVFunc<void>(this, 15, r, g, b, a); 
    }

    void DrawFilledRect(int x0, int y0, int x1, int y1) {
        MEM::CallVFunc<void>(this, 16, x0, y0, x1, y1); 
    }

    void DrawFilledRectFade(int x0, int y0, int x1, int y1, uint32_t a0, uint32_t a1, bool h) {
        MEM::CallVFunc<void>(this, 123, x0, y0, x1, y1, a0, a1, h); 
    }

    void DrawOutlinedRect(int x0, int y0, int x1, int y1) {
        MEM::CallVFunc<void>(this, 18, x0, y0, x1, y1); 
    }

    void DrawLine(int x0, int y0, int x1, int y1) { 
        MEM::CallVFunc<void>(this, 19, x0, y0, x1, y1);
    }

    void DrawPolyLine(int* x, int* y, int n) {
        MEM::CallVFunc<void>(this, 20, x, y, n);
    }

    void DrawSetTextFont(HFont f) {
        MEM::CallVFunc<void>(this, 23, f);
    }

    void DrawSetTextColor(Color c) { 
        MEM::CallVFunc<void>(this, 24, c);
    }

    void DrawSetTextColor(int r, int g, int b, int a) {
        MEM::CallVFunc<void>(this, 25, r, g, b, a); 
    }

    void DrawSetTextPos(int x, int y) {
        MEM::CallVFunc<void>(this, 26, x, y); 
    }

    void DrawPrintText(const wchar_t* t, int n, EFontDrawType d = FONT_DRAW_DEFAULT) {
        MEM::CallVFunc<void>(this, 28, t, n, d); 
    }

    void DrawSetTextureRGBA(int i, const unsigned char* r, int w, int h) { 
        MEM::CallVFunc<void>(this, 37, i, r, w, h); 
    }

    void DrawSetTexture(int i) { 
        MEM::CallVFunc<void>(this, 38, i);
    }

    int CreateNewTextureID(bool p = false) { 
        return MEM::CallVFunc<int>(this, 43, p); 
    }

    void UnLockCursor() { 
        MEM::CallVFunc<void>(this, 66);
    }

    void LockCursor() {
        MEM::CallVFunc<void>(this, 67);
    }

    HFont FontCreate() { 
        return MEM::CallVFunc<HFont>(this, 71);
    }

    bool SetFontGlyphSet(HFont f, const char* n, int t, int w, int b, int s, int fl, int rn = 0, int rx = 0) { 
        return MEM::CallVFunc<bool>(this, 72, f, n, t, w, b, s, fl, rn, rx); 
    }

    void GetTextSize(HFont f, const wchar_t* t, int& w, int& h) { 
        MEM::CallVFunc<void>(this, 79, f, t, std::ref(w), std::ref(h)); 
    }

    void PlaySoundSurface(const char* n) { 
        MEM::CallVFunc<void>(this, 82, n); 
    }

    void DrawOutlinedCircle(int x, int y, int r, int s) {
        MEM::CallVFunc<void>(this, 103, x, y, r, s); 
    }

    void DrawTexturedPolygon(int n, Vertex_t* v, bool c = true) {
        MEM::CallVFunc<void>(this, 106, n, v, c); 
    }

    void DrawTexturedPolyLine(
        const Vertex_t* p, int n) { MEM::CallVFunc<void>(this, 104, p, n); 
    }

    void SetClipRect(int x, int y, int w, int h)
    {
        *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x280) = true;
        MEM::CallVFunc<void>(this, 147, x, y, x + w + 1, y + h + 1);
        *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x280) = false;
    }

    void GetClipRect(int& x, int& y, int& x2, int& y2) {
        MEM::CallVFunc<void>(this, 146, x, y, x2, y2); 
    }
};