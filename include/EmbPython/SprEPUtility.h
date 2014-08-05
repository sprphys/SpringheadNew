﻿#ifndef SPREPUTILITY_H
#define SPREPUTILITY_H

#if _DEBUG
#define __EPDECL __cdecl
#else
#define __EPDECL __fastcall
#endif

#include "Utility/SprUTCriticalSection.h"//CriticalSection用

#include <EmbPython\/SprEPVersion.h>
#include PYTHON_H_PATH

#define EP_MODULE_NAME "Spr"
//#define EP_USE_SUBMODULE

extern Spr::UTCriticalSection EPCriticalSection;
extern PyObject* PyErr_Spr_NullReference;
extern PyObject* PyErr_Spr_OSException;

// Class for containing information on a Win32 Structured Exception
class SEH_Exception: public std::exception{
private:
    SEH_Exception() {}
    unsigned int seCode;
	void* eptr;
public:
    SEH_Exception(unsigned int uSECode, void* ep) : seCode(uSECode), eptr(ep) {}
    ~SEH_Exception() {}
	virtual const char* what() const throw();
};

void initUtility(PyObject *rootModule = NULL);
#endif
