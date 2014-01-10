#include "io_timos.hpp"
#include "fluencemap.hpp"

using namespace std;

int main(int argc,char **argv)
{
    TetraMesh m(argv[1]);
    SurfaceFluenceMap surf_timos(m),surf_fm(m);
    VolumeFluenceMap  vol_timos(m), vol_fm(m);

    // read the TIM-OS output
    readTIMOSOutput(argv[2],mesh,surf_timos,vol_timos);

    // read the FullMonte output
    surf_fm = readSurfaceOutput(argv[3]);
    vol_fm  = readVolumeOutput(argv[4]);
}
