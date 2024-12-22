#ifndef SRE_LOG_H
#define SRE_LOG_H

#include <sre/logging/errors.h>
#include <stdio.h>

enum
{
    SRE_LOGGING_VERBOSITY_0,    // no logging
    SRE_LOGGING_VERBOSITY_1,    // logs errors only
    SRE_LOGGING_VERBOSITY_2,    // logs everything
};

#endif //SRE_LOG_H