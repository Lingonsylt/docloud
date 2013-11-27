#ifndef DOCLOUDEXT_H
#define DOCLOUDEXT_H
#include <windows.h>
#include <windowsx.h>
#include <guiddef.h>
#include <shlobj.h>
#include <new>

class ClassFactory : public IClassFactory
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
    STDMETHODIMP LockServer(BOOL fLock);

    ClassFactory();

protected:
    ~ClassFactory();

private:
    long m_cRef;
};

#endif /* end of include guard: DOCLOUDEXT_H */
