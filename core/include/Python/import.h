#include "Add_decl.h"

/* Module definition and import interface */

#ifndef Py_IMPORT_H
#define Py_IMPORT_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(void) __PYDECL  _PyImportZip_Init(void);

PyMODINIT_FUNC PyInit_imp(void);
PyAPI_FUNC(long) __PYDECL  PyImport_GetMagicNumber(void);
PyAPI_FUNC(const char *) __PYDECL  PyImport_GetMagicTag(void);
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ExecCodeModule(
    const char *name,           /* UTF-8 encoded string */
    PyObject *co
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ExecCodeModuleEx(
    const char *name,           /* UTF-8 encoded string */
    PyObject *co,
    const char *pathname        /* decoded from the filesystem encoding */
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ExecCodeModuleWithPathnames(
    const char *name,           /* UTF-8 encoded string */
    PyObject *co,
    const char *pathname,       /* decoded from the filesystem encoding */
    const char *cpathname       /* decoded from the filesystem encoding */
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ExecCodeModuleObject(
    PyObject *name,
    PyObject *co,
    PyObject *pathname,
    PyObject *cpathname
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_GetModuleDict(void);
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_AddModuleObject(
    PyObject *name
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_AddModule(
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ImportModule(
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ImportModuleNoBlock(
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ImportModuleLevel(
    const char *name,           /* UTF-8 encoded string */
    PyObject *globals,
    PyObject *locals,
    PyObject *fromlist,
    int level
    );
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ImportModuleLevelObject(
    PyObject *name,
    PyObject *globals,
    PyObject *locals,
    PyObject *fromlist,
    int level
    );

#define PyImport_ImportModuleEx(n, g, l, f) \
    PyImport_ImportModuleLevel(n, g, l, f, 0)

PyAPI_FUNC(PyObject *) __PYDECL  PyImport_GetImporter(PyObject *path);
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_Import(PyObject *name);
PyAPI_FUNC(PyObject *) __PYDECL  PyImport_ReloadModule(PyObject *m);
PyAPI_FUNC(void) __PYDECL  PyImport_Cleanup(void);
PyAPI_FUNC(int) __PYDECL  PyImport_ImportFrozenModuleObject(
    PyObject *name
    );
PyAPI_FUNC(int) __PYDECL  PyImport_ImportFrozenModule(
    const char *name            /* UTF-8 encoded string */
    );

#ifndef Py_LIMITED_API
#ifdef WITH_THREAD
PyAPI_FUNC(void) __PYDECL  _PyImport_AcquireLock(void);
PyAPI_FUNC(int) __PYDECL  _PyImport_ReleaseLock(void);
#else
#define _PyImport_AcquireLock()
#define _PyImport_ReleaseLock() 1
#endif

PyAPI_FUNC(void) __PYDECL  _PyImport_ReInitLock(void);

PyAPI_FUNC(PyObject *) __PYDECL  _PyImport_FindBuiltin(
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(PyObject *) __PYDECL  _PyImport_FindExtensionObject(PyObject *, PyObject *);
PyAPI_FUNC(int) __PYDECL  _PyImport_FixupBuiltin(
    PyObject *mod,
    const char *name            /* UTF-8 encoded string */
    );
PyAPI_FUNC(int) __PYDECL  _PyImport_FixupExtensionObject(PyObject*, PyObject *, PyObject *);

struct _inittab {
    const char *name;           /* ASCII encoded string */
    PyObject* (*initfunc)(void);
};
PyAPI_DATA(struct _inittab *) PyImport_Inittab;
PyAPI_FUNC(int) __PYDECL  PyImport_ExtendInittab(struct _inittab *newtab);
#endif /* Py_LIMITED_API */

PyAPI_DATA(PyTypeObject) PyNullImporter_Type;

PyAPI_FUNC(int) __PYDECL  PyImport_AppendInittab(
    const char *name,           /* ASCII encoded string */
    PyObject* (*initfunc)(void)
    );

#ifndef Py_LIMITED_API
struct _frozen {
    const char *name;                 /* ASCII encoded string */
    const unsigned char *code;
    int size;
};

/* Embedding apps may change this pointer to point to their favorite
   collection of frozen modules: */

PyAPI_DATA(const struct _frozen *) PyImport_FrozenModules;
#endif

#ifdef __cplusplus
}
#endif
#endif /* !Py_IMPORT_H */