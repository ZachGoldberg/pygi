/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 *
 * Copyright (C) 2009 Simon van der Linden <svdlinden@src.gnome.org>
 *
 *   pygi-struct.c: wrapper to handle non-registered structures.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "pygi-private.h"

#include <pygobject.h>
#include <girepository.h>

static void
_struct_dealloc (PyGIStruct *self)
{
    PyObject_GC_UnTrack((PyObject *)self);

    PyObject_ClearWeakRefs((PyObject *)self);

    if (self->free_on_dealloc) {
        g_free(((PyGPointer *)self)->pointer);
    }

    ((PyGPointer *)self)->ob_type->tp_free((PyObject *)self);
}

static PyObject *
_struct_new (PyTypeObject *type,
             PyObject     *args,
             PyObject     *kwargs)
{
    static char *kwlist[] = { NULL };

    GIBaseInfo *info;
    gboolean is_simple;
    gsize size;
    gpointer pointer;
    PyObject *self = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "", kwlist)) {
        return NULL;
    }

    info = _pygi_object_get_gi_info((PyObject *)type, &PyGIStructInfo_Type);
    if (info == NULL) {
        if (PyErr_ExceptionMatches(PyExc_AttributeError)) {
            PyErr_Format(PyExc_TypeError, "missing introspection information");
        }
        return NULL;
    }

    size = g_struct_info_get_size((GIStructInfo *)info);
    pointer = g_try_malloc0(size);
    if (pointer == NULL) {
        PyErr_NoMemory();
        goto out;
    }

    self = _pygi_struct_new(type, pointer, TRUE);
    if (self == NULL) {
        g_free(pointer);
    }

out:
    g_base_info_unref(info);

    return (PyObject *)self;
}

static int
_struct_init (PyObject *self,
              PyObject *args,
              PyObject *kwargs)
{
    /* Don't call PyGPointer's init, which raises an exception. */
    return 0;
}


PyTypeObject PyGIStruct_Type = {
    PyObject_HEAD_INIT(NULL)
    0,
    "gi.Struct",                               /* tp_name */
    sizeof(PyGIStruct),                        /* tp_basicsize */
    0,                                         /* tp_itemsize */
    (destructor)_struct_dealloc,               /* tp_dealloc */
    (printfunc)NULL,                           /* tp_print */
    (getattrfunc)NULL,                         /* tp_getattr */
    (setattrfunc)NULL,                         /* tp_setattr */
    (cmpfunc)NULL,                             /* tp_compare */
    (reprfunc)NULL,                            /* tp_repr */
    NULL,                                      /* tp_as_number */
    NULL,                                      /* tp_as_sequence */
    NULL,                                      /* tp_as_mapping */
    (hashfunc)NULL,                            /* tp_hash */
    (ternaryfunc)NULL,                         /* tp_call */
    (reprfunc)NULL,                            /* tp_str */
    (getattrofunc)NULL,                        /* tp_getattro */
    (setattrofunc)NULL,                        /* tp_setattro */
    NULL,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    NULL,                                     /* tp_doc */
    (traverseproc)NULL,                       /* tp_traverse */
    (inquiry)NULL,                            /* tp_clear */
    (richcmpfunc)NULL,                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    (getiterfunc)NULL,                        /* tp_iter */
    (iternextfunc)NULL,                       /* tp_iternext */
    NULL,                                     /* tp_methods */
    NULL,                                     /* tp_members */
    NULL,                                     /* tp_getset */
    (PyTypeObject *)NULL,                     /* tp_base */
};

PyObject *
_pygi_struct_new (PyTypeObject *type,
                  gpointer      pointer,
                  gboolean      free_on_dealloc)
{
    PyGIStruct *self;
    GType g_type;

    if (!PyType_IsSubtype(type, &PyGIStruct_Type)) {
        PyErr_SetString(PyExc_TypeError, "must be a subtype of gi.Struct");
        return NULL;
    }

    self = (PyGIStruct *)type->tp_alloc(type, 0);
    if (self == NULL) {
        return NULL;
    }

    g_type = pyg_type_from_object((PyObject *)type);

    ((PyGPointer *)self)->gtype = g_type;
    ((PyGPointer *)self)->pointer = pointer;
    self->free_on_dealloc = free_on_dealloc;

    return (PyObject *)self;
}

void
_pygi_struct_register_types (PyObject *m)
{
    PyGIStruct_Type.ob_type = &PyType_Type;
    PyGIStruct_Type.tp_base = &PyGPointer_Type;
    PyGIStruct_Type.tp_new = (newfunc)_struct_new;
    PyGIStruct_Type.tp_init = (initproc)_struct_init;
    if (PyType_Ready(&PyGIStruct_Type))
        return;
    if (PyModule_AddObject(m, "Struct", (PyObject *)&PyGIStruct_Type))
        return;
}
