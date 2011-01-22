#include <Producer/System>

#include <iostream>
#include <fstream>
#include <sstream>

namespace Producer {

unsigned int getNumberOfProcessors()
{
#ifdef __linux
    std::ifstream cpuinfo("/proc/cpuinfo");
    if( !cpuinfo )
        return 1; // There has to be at least 1

    std::string s;
    int numprocessors = 0;
    while( !cpuinfo.eof() )
    {
        /*
        char linebuff[1024];
        cpuinfo.getline(linebuff, sizeof(linebuff));
        std::stringstream ss(linebuff);
        */
        std::string s;
        cpuinfo >> s;
        if( s == "processor" )
            numprocessors++;
    }

    return numprocessors;

#else
    return 1;
#endif
}

}
