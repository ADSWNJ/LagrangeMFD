#ifndef ENGINE_H
#define ENGINE_H

struct Engine
{
    Engine()
    {
        F = isp = ThrAngle = 0;
    }
    double F, isp, ThrAngle;
};

#endif // ENGINE_H
