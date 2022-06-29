#ifndef LIME_DEVICE_EXCEPTIONS_H
#define LIME_DEVICE_EXCEPTIONS_H

#include <stdexcept>

class OperationNotSupported : public std::logic_error {
    using std::logic_error::logic_error;
};

#endif