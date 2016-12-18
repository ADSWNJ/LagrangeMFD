#ifndef COLORS_H
#define COLORS_H

namespace EnjoLib
{

class Colors
{
public:
    typedef struct
    {
        double r,g,b;
    } COLOUR;
    COLOUR GreyToRGB(double v,double vmin,double vmax);
};

}
#endif // COLORS_H
