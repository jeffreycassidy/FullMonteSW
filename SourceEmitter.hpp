#include "SourceDescription.hpp"
#include "Source.hpp"

// SourceEmitter has the actual guts to emit photons; SourceDescription just tells you what it is
// avoids the need to include Random classes, AVX math, etc.
