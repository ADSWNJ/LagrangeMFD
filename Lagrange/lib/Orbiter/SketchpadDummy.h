#ifndef SKETCHPADDUMMY_H
#define SKETCHPADDUMMY_H

#include <OrbiterSDK.h>

namespace EnjoLib
{
class SketchpadDummy : public oapi::Sketchpad
{
    public:
        SketchpadDummy();
        virtual ~SketchpadDummy();

        oapi::Font *SetFont (oapi::Font *font) const { return NULL; }
        oapi::Pen *SetPen (oapi::Pen *pen) const { return NULL; }
        oapi::Brush *SetBrush (oapi::Brush *brush) const { return NULL; }
        void SetTextAlign (TAlign_horizontal tah=LEFT, TAlign_vertical tav=TOP) {}
        DWORD SetTextColor (DWORD col) { return 0; }
        DWORD SetBackgroundColor (DWORD col) { return 0; }
        void SetBackgroundMode (BkgMode mode) {}
        DWORD GetCharSize () { return 0; }
        DWORD GetTextWidth (const char *str, int len = 0) { return 0; }
        void SetOrigin (int x, int y) {}
        bool Text (int x, int y, const char *str, int len) { return false; }
        bool TextBox (int x1, int y1, int x2, int y2, const char *str, int len) { return false; }
        void Pixel (int x, int y, DWORD col) {}
        void MoveTo (int x, int y) {}
        void LineTo (int x, int y) {}
        void Line (int x0, int y0, int x1, int y1) {}
        void Rectangle (int x0, int y0, int x1, int y1) {}
        void Ellipse (int x0, int y0, int x1, int y1) {}
        void Polygon (const oapi::IVECTOR2 *pt, int npt) {}
        void Polyline (const oapi::IVECTOR2 *pt, int npt) {}
        void PolyPolygon (const oapi::IVECTOR2 *pt, const int *npt, const int nline) {}
        void PolyPolyline (const oapi::IVECTOR2 *pt, const int *npt, const int nline) {}
        HDC GetDC() { return NULL; }
    protected:
    private:
};
}
#endif // SKETCHPADDUMMY_H
