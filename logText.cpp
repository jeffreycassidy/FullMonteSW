#include "logger.hpp"

LoggerText::LoggerText(string fn) : logFile(fn.c_str())
{
    printHeader();
}

LoggerText::LoggerText(const char* fn) :
    logFile(fn)
{
    printHeader();
}

void LoggerText::printHeader()
{
    logFile << "# Complete packet life history" << endl;
    logFile << "# Event codes " << endl;
    logFile << "#   L   Launch new packet " << endl;
    logFile << "#   B   Boundary, same material " << endl;
    logFile << "#   T   Total internal reflection" << endl;
    logFile << "#   F   Fresnel reflection" << endl;
    logFile << "#   R   Transmitted with refraction" << endl;
    logFile << "#   A   Absorption" << endl;
    logFile << "#   E   Exit" << endl;
    logFile << "#" << endl;
    logFile << "# Column layout (space delimited)" << endl;
    logFile << "# Event Type | Position | Direction | Arriving packet weight | Weight deposited here" << endl;
    logFile << "# L (1.0,2.0,3.0) (0.5774,0.5774,-0.5774) 1.0 0.1" << endl;
    logFile << "#" << endl;
}

void LoggerText::eventLaunch(const Ray3 r,unsigned IDt,double w)
{
    logFile << "L " << r.first << ' ' << r.second << ' ' << IDt << ' ' << w << endl;
}

void LoggerText::eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw)
{
    logFile << "A " << p << ' ' << w0 << ' ' << dw << endl;
}

void LoggerText::eventBoundary(Point3 p,int IDf,int IDts,int IDte)
{
    logFile << "B " << p << ' ' << IDf << ' ' << IDts << ' ' << IDte << endl;
}

void LoggerText::eventReflectInternal(Point3 p,UVect3 d)
{
    logFile << "T " << p << ' ' << d << endl;
}

//  r       Exiting ray
//  IDf     Face ID exited from

void LoggerText::eventExit(Ray3 r,int IDf,double w)
{
    logFile << "E " << r.first << ' ' << r.second << ' ' << IDf << ' ' << w << endl;
}

void LoggerText::eventRefract(Point3 p,UVect3 d)
{
    logFile << "R " << p << ' ' << d << endl;
}

void LoggerText::eventReflectFresnel(Point3 p,UVect3 d)
{
    logFile << "F " << p << ' ' << d << endl;
}

void LoggerText::eventRouletteWin(double w0,double w)
{
    logFile << "W " << w0 << ' ' << w << endl;
}

void LoggerText::eventDie(double w)
{
    logFile << "D " << w << endl;
}

void LoggerText::eventScatter(UVect3 d0,UVect3 d,double g)
{
    logFile << "S " << d << ' ' << g << endl;
}
