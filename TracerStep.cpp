#include <iostream>
#include "TracerStep.hpp"

const char TracerStep::eventCodes[] = { 'L','A','S','B','R','T','F','E','D','W' };

/** Display the single-character event code */
std::ostream& operator<<(std::ostream& os,TracerStep::EventType e)
{
    return os << TracerStep::eventCodes[e];
}
