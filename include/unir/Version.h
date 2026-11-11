#pragma once

// unir version.

#include <string>

#define UNIR_VERSION_MAJOR 1
#define UNIR_VERSION_MINOR 2
#define UNIR_VERSION_PATCH 4
#define UNIR_VERSION_STRING "1.2.4"

#define UNIR_VERSION_AT(ma, mi, pa) ((ma) * 10000 + (mi) * 100 + (pa))
#define UNIR_VERSION UNIR_VERSION_AT(UNIR_VERSION_MAJOR, UNIR_VERSION_MINOR, UNIR_VERSION_PATCH)

namespace unir {

/// unir version, "1.2.4"; development builds and builds for a test license
/// server end with -TEST ("1.2.4-TEST") and are never shipped to customers.
std::string version();

/// Versions of the engines inside the build: "dmr 1.1.1, ssccr 0.1.0, qrr 0.1.0".
std::string engineVersions();

} // namespace unir
