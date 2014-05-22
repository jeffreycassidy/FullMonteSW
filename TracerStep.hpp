#include <array>

class TracerStep {

public:
    char event;
    float weight;
    std::array<float,3> pos;
    float t;


    static const char eventCodes[];
    enum EventType { Launch, Absorb, Scatter, Boundary, Refract, ReflectInternal, ReflectFresnel, Exit, Die };

    static bool IsDirectionChange(const TracerStep& s){
    	switch(s.event){
    	case Launch:
    	case Scatter:
    	case Refract:
    	case ReflectInternal:
    	case ReflectFresnel:
    	case Exit:
    	case Die:
    		return true;
    	}
    	return false;
    }

    // Geometry must be read when launching a new photon or traversing a boundary
    static bool IsGeometryRead(const TracerStep& s){
    	switch(s.event){
    	case Launch:
    	case Boundary:
    	case Refract:
    		return true;
    	}
    	return false;
    }

    static bool IsAccumulate(const TracerStep& s){
    	switch(s.event){
    	case Launch:
    	case Absorb:
    	case Exit:
    		return true;
    	}
    	return false;
    }

    // convert to array<float,3> by returning position
    operator std::array<float,3>() const { return pos; }
};
