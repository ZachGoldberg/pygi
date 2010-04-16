/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */
#ifndef __PYGI_PRIVATE_H__
#define __PYGI_PRIVATE_H__

#ifdef __PYGI_H__
#   error "Import pygi.h or pygi-private.h, but not both"
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <Python.h>

#include "pygi.h"

#include "pygobject-external.h"

#include "pygi-repository.h"
#include "pygi-info.h"
#include "pygi-struct.h"
#include "pygi-boxed.h"
#include "pygi-argument.h"
#include "pygi-type.h"
#include "pygi-closure.h"

G_BEGIN_DECLS

#define _PyGI_ERROR_PREFIX(format, ...) G_STMT_START { \
    PyObject *py_error_prefix; \
    py_error_prefix = PyString_FromFormat(format, ## __VA_ARGS__); \
    if (py_error_prefix != NULL) { \
        PyObject *py_error_type, *py_error_value, *py_error_traceback; \
        PyErr_Fetch(&py_error_type, &py_error_value, &py_error_traceback); \
        if (PyString_Check(py_error_value)) { \
            PyString_ConcatAndDel(&py_error_prefix, py_error_value); \
            if (py_error_prefix != NULL) { \
                py_error_value = py_error_prefix; \
            } \
        } \
        PyErr_Restore(py_error_type, py_error_value, py_error_traceback); \
    } \
} G_STMT_END

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION == 20
/* Private stuff copied from glib-2.20.x sources */
struct _GRealArray
{
    guint8 *data;
    guint   len;
    guint   alloc;
    guint   elt_size;
    guint   zero_terminated : 1;
    guint   clear : 1;
};

static inline guint
g_array_get_element_size (GArray *array)
{
    struct _GRealArray *rarray = (struct _GRealArray*) array;
    return rarray->elt_size;
}
#endif

/* Redefine g_array_index because we want it to return the i-th element, casted
 * to the type t, of the array a, and not the i-th element of the array a
 * casted to the type t. */
#define _g_array_index(a,t,i) \
    *(t *)((a)->data + g_array_get_element_size(a) * (i))


G_END_DECLS

#endif /* __PYGI_PRIVATE_H__ */
