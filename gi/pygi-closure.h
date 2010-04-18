/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
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

#ifndef __PYGI_CLOSURE_H__
#define __PYGI_CLOSURE_H__

#include <Python.h>
#include <girffi.h>
#include <ffi.h>

G_BEGIN_DECLS


/* Private */

typedef struct _PyGICClosure
{
    GICallableInfo *info;
    PyObject *function;
    
    ffi_closure *closure;
    ffi_cif cif;
 
    GIScopeType scope;

    PyObject* user_data;
} PyGICClosure; 
 
void _pygi_closure_handle(ffi_cif *cif, void *result, void
                          **args, void *userdata);
 
void _pygi_invoke_closure_free(gpointer user_data);

PyGICClosure* _pygi_make_native_closure (GICallableInfo* info,
                                         GIScopeType scope,
                                         PyObject *function,
                                         gpointer user_data);

G_END_DECLS

#endif /* __PYGI_CLOSURE_H__ */
