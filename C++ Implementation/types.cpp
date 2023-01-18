#include "types.h"

#include <stdint.h>
#include <iostream>


std::ostream & operator<< (std::ostream & out, UnsignedBigInt_t ubi) {
    out << ubi.to_string();
    return out;
}

