#include <SPOGCKKS/tool/version.h>

std::string GET_SPOGCKKS_VERSION() {     
    std::ostringstream oss; 
    oss << SPOGCKKS_VERSION_MAJOR << "." << SPOGCKKS_VERSION_MINOR << "." << SPOGCKKS_VERSION_PATCH; 
    if(SPOGCKKS_VERSION_TWEAK != 0)
	    oss << " - " << SPOGCKKS_VERSION_TWEAK;
    return oss.str();
}
