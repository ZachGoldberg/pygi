/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 *
 *   pygi-closure.c: PyGI C Closure functions
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

static PyGICClosure *global_destroy_notify;

/* This maintains a list of callbacks which can be free'd whenever
   as they have been called.  We will free them on the next
   library function call.
 */
static GSList* async_free_list;

void
_pygi_closure_handle (ffi_cif *cif,
                      void    *result,
                      void   **args,
                      void    *data)
{
    PyGILState_STATE state;
    PyGICClosure *closure = data;
    gint n_args, i;
    GIArgInfo  *arg_info;
    GIDirection arg_direction;
    GITypeInfo *arg_type;
    GITransfer arg_transfer;
    GITypeTag  arg_tag;
    GITypeTag  return_tag;
    GITransfer return_transfer;
    GITypeInfo *return_type;
    PyObject *retval;
    PyObject *py_args;
    PyObject *pyarg;
    gint n_in_args, n_out_args;


    /* Lock the GIL as we are coming into this code without the lock and we
      may be executing python code */
    state = PyGILState_Ensure();
    return_type = g_callable_info_get_return_type(closure->info);
    return_tag = g_type_info_get_tag(return_type);
    return_transfer = g_callable_info_get_caller_owns(closure->info);

    n_args = g_callable_info_get_n_args (closure->info);  

    py_args = PyTuple_New(n_args);
    if (py_args == NULL) {
        PyErr_Clear();
        goto end;
    }

    n_in_args = 0;
    n_out_args = 0;

    for (i = 0; i < n_args; i++) {
        arg_info = g_callable_info_get_arg (closure->info, i);
        arg_type = g_arg_info_get_type (arg_info);
        arg_transfer = g_arg_info_get_ownership_transfer(arg_info);
        arg_tag = g_type_info_get_tag(arg_type);
        arg_direction = g_arg_info_get_direction(arg_info);
        g_print("Arg: Name: %s, %p\n", g_base_info_get_name(arg_info), arg_info);
        g_print("Tag: %d\n", arg_tag);
               
             
        switch (arg_tag){
            case GI_TYPE_TAG_VOID:
                {
                    if (g_type_info_is_pointer(arg_type)) {
                        PyTuple_SetItem(py_args, i, closure->user_data);
                        n_in_args++;
                        g_print("Skipping because we had %p!\n", closure->user_data);
                        continue;
                    }
                }
            case GI_TYPE_TAG_ERROR:
                 {
                     if (arg_direction == GI_DIRECTION_OUT){
                         n_out_args++;
                         break;
                     }else if(arg_direction == GI_DIRECTION_INOUT){
                         n_out_args++;
                     }
                 }
            default:
                {
                    n_in_args++;
                    pyarg = _pygi_argument_to_object (args[i],
                                                      arg_type,
                                                      arg_transfer);
                    
                    PyTuple_SetItem(py_args, i, pyarg);
                    g_base_info_unref((GIBaseInfo*)arg_info);
                    g_base_info_unref((GIBaseInfo*)arg_type);                                    
                }
        }
        
    }
    
    _PyTuple_Resize (&py_args, n_in_args);

    retval = PyObject_CallObject((PyObject *)closure->function, py_args);

    /*
      If python exception occured
       and we have an out gerror
       then make new GIOError with string from python excpetion
    */

    Py_DECREF(py_args);

    if (retval == NULL){
        goto end;
    }

    *(GArgument*)result = _pygi_argument_from_object(retval, return_type, return_transfer);

end:
    g_base_info_unref((GIBaseInfo*)return_type);

    PyGILState_Release(state);

    /* Now that the callback has finished we can make a decision about how
       to free it.  Scope call gets free'd now, scope notified will be freed
       when the notify is called and we can free async anytime we want
       once we return from this function */
    switch (closure->scope) {
    case GI_SCOPE_TYPE_CALL:
        _pygi_callback_invoke_closure_free(closure);
        break;
    case GI_SCOPE_TYPE_NOTIFIED:        
        break;
    case GI_SCOPE_TYPE_ASYNC:
        /* Append this PyGICClosure to a list of closure that we will free
           after we're done with this function invokation */
        async_free_list = g_slist_prepend(async_free_list, closure);
        break;
    default:
        g_assert_not_reached();
    }
}

void _pygi_callback_invoke_closure_free(gpointer data)
{
    PyGICClosure* invoke_closure = (PyGICClosure *)data;
    

    Py_DECREF(invoke_closure->function);

    g_callable_info_free_closure(invoke_closure->info,
                                 invoke_closure->closure);

    if (invoke_closure->info)
        g_base_info_unref((GIBaseInfo*)invoke_closure->info);

    g_slice_free(PyGICClosure, invoke_closure);
}

static void
_pygi_destroy_notify_callback_closure(ffi_cif *cif,
                                      void *result,
                                      void **args,
                                      void *data)
{
    PyGICClosure *info = *(void**)(args[0]);

    g_assert(info);

    _pygi_callback_invoke_closure_free(info);    
}


PyGICClosure*
_pygi_destroy_notify_create(void)
{
    if (!global_destroy_notify){
        
        ffi_status status;
        PyGICClosure *destroy_notify = g_slice_new0(PyGICClosure);        

        g_assert(destroy_notify);
        
        GIBaseInfo* glib_destroy_notify = g_irepository_find_by_name(NULL, "GLib", "DestroyNotify");
        g_assert(glib_destroy_notify != NULL);
        g_assert(g_base_info_get_type(glib_destroy_notify) == GI_INFO_TYPE_CALLBACK);
        
        destroy_notify->closure = g_callable_info_prepare_closure((GICallableInfo*)glib_destroy_notify,
                                                                  &destroy_notify->cif,
                                                                  _pygi_destroy_notify_callback_closure,
                                                                  NULL);

        global_destroy_notify = destroy_notify;
    }
    
    return global_destroy_notify;
}


gboolean
_pygi_scan_for_callbacks (PyGIBaseInfo  *function_info,
                          gboolean       is_method,
                          guint8        *callback_index,
                          guint8        *user_data_index,
                          guint8        *destroy_notify_index)
{
    guint i, n_args;

    *callback_index = G_MAXUINT8;
    *user_data_index = G_MAXUINT8;
    *destroy_notify_index = G_MAXUINT8;

    n_args = g_callable_info_get_n_args((GICallableInfo *)function_info->info);
    for (i = 0; i < n_args; i++) {
        GIDirection direction;
        GIArgInfo *arg_info;
        GITypeInfo *type_info;
        guint8 destroy, closure;
        GITypeTag type_tag;

        arg_info = g_callable_info_get_arg((GICallableInfo*) function_info->info, i);
        type_info = g_arg_info_get_type(arg_info);
        type_tag = g_type_info_get_tag(type_info);    

        if (type_tag == GI_TYPE_TAG_INTERFACE) {
            GIBaseInfo* interface_info;
            GIInfoType interface_type;

            interface_info = g_type_info_get_interface(type_info);
            interface_type = g_base_info_get_type(interface_info);
            if (interface_type == GI_INFO_TYPE_CALLBACK &&
                !(strcmp(g_base_info_get_namespace( (GIBaseInfo*) interface_info), "GLib") == 0 &&
                  strcmp(g_base_info_get_name( (GIBaseInfo*) interface_info), "DestroyNotify") == 0)) {
                if (*callback_index != G_MAXUINT8) {
                    PyErr_Format(PyExc_TypeError, "Function %s.%s has multiple callbacks, not supported",
                              g_base_info_get_namespace( (GIBaseInfo*) function_info->info),
                              g_base_info_get_name( (GIBaseInfo*) function_info->info));
                    g_base_info_unref(interface_info);
                    return FALSE;
                }
                *callback_index = i;
            }
            g_base_info_unref(interface_info);
        }
        destroy = g_arg_info_get_destroy(arg_info);
        if (is_method)
            --destroy;
        closure = g_arg_info_get_closure(arg_info);
        if (is_method)
            --closure;
        direction = g_arg_info_get_direction(arg_info);

        if (destroy > 0 && destroy < n_args) {
            if (*destroy_notify_index != G_MAXUINT8) {
                PyErr_Format(PyExc_TypeError, "Function %s has multiple GDestroyNotify, not supported",
                             g_base_info_get_name((GIBaseInfo*)function_info->info));
                return FALSE;
            }
            *destroy_notify_index = destroy;
        }

        if (closure > 0 && closure < n_args) {
            if (*user_data_index != G_MAXUINT8) {
                 PyErr_Format(PyExc_TypeError, "Function %s has multiple user_data arguments, not supported",
                          g_base_info_get_name((GIBaseInfo*)function_info->info));
                return FALSE;
            }
            *user_data_index = closure;
        }

        g_base_info_unref((GIBaseInfo*)arg_info);
        g_base_info_unref((GIBaseInfo*)type_info);
    }

    return TRUE;
}

gboolean
_pygi_create_c_closure (PyGIBaseInfo  *function_info,
                        gboolean       is_method,
                        int            n_args,
                        Py_ssize_t     py_argc,
                        PyObject      *py_argv,
                        guint8         callback_index,
                        guint8         user_data_index,
                        guint8         destroy_notify_index,
                        PyGICClosure **closure_out)
{
    GIArgInfo *callback_arg;
    GITypeInfo *callback_type;
    GICallbackInfo *callback_info;
    GIScopeType scope;
    gboolean found_py_function;
    PyObject *py_function;
    guint8 i, py_argv_pos;
    ffi_closure *fficlosure;
    PyGICClosure *closure;
    PyObject *py_user_data;

    /* Begin by cleaning up old async functions */
    g_slist_foreach(async_free_list, (GFunc)_pygi_callback_invoke_closure_free, NULL);
    g_slist_free(async_free_list);
    async_free_list = NULL;

    callback_arg = g_callable_info_get_arg( (GICallableInfo*) function_info->info, callback_index);
    scope = g_arg_info_get_scope(callback_arg);

    callback_type = g_arg_info_get_type(callback_arg);
    g_assert(g_type_info_get_tag(callback_type) == GI_TYPE_TAG_INTERFACE);

    callback_info = (GICallbackInfo*)g_type_info_get_interface(callback_type);
    g_assert(g_base_info_get_type((GIBaseInfo*)callback_info) == GI_INFO_TYPE_CALLBACK);

    /* Find the Python function passed for the callback */
    found_py_function = FALSE;
    py_function = Py_None;
    py_user_data = NULL;
    
    /* if its a method then we need to skip over 'function_info' */
    if (is_method)
        py_argv_pos = 1;
    else
        py_argv_pos = 0;
    for (i = 0; i < n_args && i < py_argc; i++) {
        if (i == callback_index) {
            py_function = PyTuple_GetItem(py_argv, py_argv_pos);
            found_py_function = TRUE;
            continue;
        } else if (i == user_data_index){
            py_user_data = PyTuple_GetItem(py_argv, py_argv_pos);
        }else if (i == destroy_notify_index) {
            continue;
        }
        py_argv_pos++;
    }

    if (!found_py_function
        || (py_function == Py_None || !PyCallable_Check(py_function))) {
        PyErr_Format(PyExc_TypeError, "Error invoking %s.%s: Invalid callback given for argument %s",
                  g_base_info_get_namespace( (GIBaseInfo*) function_info->info),
                  g_base_info_get_name( (GIBaseInfo*) function_info->info),
                  g_base_info_get_name( (GIBaseInfo*) callback_arg));
        g_base_info_unref( (GIBaseInfo*) callback_info);
        return FALSE;
    }


    /** Now actually build the closure **/

    /* Build the closure itfunction_info */
    *closure_out = g_slice_new0(PyGICClosure);
    closure = *closure_out;
    closure->info = (GICallableInfo *) g_base_info_ref ((GIBaseInfo *) callback_info);  
    closure->function = py_function;
    closure->user_data = py_user_data;

    Py_INCREF(py_function);
    if (closure->user_data)
        Py_INCREF(closure->user_data);

    fficlosure =
        g_callable_info_prepare_closure (callback_info, &closure->cif, _pygi_closure_handle,
                                         closure);
    closure->closure = fficlosure;
    
    /* Give the closure the information it needs to determine when
       to free itfunction_info later */
    closure->scope = g_arg_info_get_scope(callback_arg);

    g_base_info_unref( (GIBaseInfo*) callback_info);

    return TRUE;
}
