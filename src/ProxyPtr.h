/**
    @file ProxyPtr.h
    @author Lime Microsystems
    @brief Templated class to redirect to a pointer
*/

#pragma once

/*!
 * The ProxyPtr contains an internal pointer
 * that can be swapped out at runtime via reset().
 * The ProxyPtr does not own the pointer.
 */
template <typename T>
class ProxyPtr
{
public:

    ProxyPtr(void);

    //! Create a proxy from a pointer pointer
    ProxyPtr(T *ptr);

    /*!
     * Create a proxy from a pointer to a pointer pointer.
     * This is used for swapping out the underlying pointer.
     */
    ProxyPtr(T **ptr);

    //! Set the internal pointer to null
    void reset(void);

    //! Set the internal pointer to internal
    void reset(T *ptr);

    //! Get the internal pointer
    const T *get(void) const;

    //! Get the internal pointer
    T *get(void);

    //! True when the pointer is not null
    operator bool(void) const;

    const T* operator->(void) const;

    T* operator->(void);

    const T& operator*(void) const;

    T& operator*(void);

private:
    T **_internalRef;
    T *_internal;
};

template <typename T>
ProxyPtr<T>::ProxyPtr(void):
    _internalRef(nullptr),
    _internal(nullptr)
{
    return;
}

template <typename T>
ProxyPtr<T>::ProxyPtr(T *ptr):
    _internalRef(nullptr),
    _internal(ptr)
{
    return;
}

template <typename T>
ProxyPtr<T>::ProxyPtr(T **ptr):
    _internalRef(ptr),
    _internal(nullptr)
{
    return;
}

template <typename T>
void ProxyPtr<T>::reset(void)
{
    _internal = nullptr;
}

template <typename T>
void ProxyPtr<T>::reset(T *ptr)
{
    _internal = ptr;
}

template <typename T>
const T *ProxyPtr<T>::get(void) const
{
    if (_internalRef != nullptr) return *_internalRef;
    return _internal;
}

template <typename T>
T *ProxyPtr<T>::get(void)
{
    if (_internalRef != nullptr) return *_internalRef;
    return _internal;
}

template <typename T>
ProxyPtr<T>::operator bool(void) const
{
    return this->get() != nullptr;
}

template <typename T>
const T* ProxyPtr<T>::operator->(void) const
{
    return this->get();
}

template <typename T>
T* ProxyPtr<T>::operator->(void)
{
    return this->get();
}

template <typename T>
const T& ProxyPtr<T>::operator*(void) const
{
    return *this->get();
}

template <typename T>
T& ProxyPtr<T>::operator*(void)
{
    return *this->get();
}
