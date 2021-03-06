/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 *
 * Copyright (C) 2005-2009 Johan Dahlin <johan@gnome.org>
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

#ifndef __PYGI_ARGUMENT_H__
#define __PYGI_ARGUMENT_H__

#include <Python.h>

#include <girepository.h>

G_BEGIN_DECLS


/* Private */

gint _pygi_g_type_info_check_object (GITypeInfo *type_info,
                                     PyObject   *object);

gint _pygi_g_registered_type_info_check_object (GIRegisteredTypeInfo *info,
                                                gboolean              is_instance,
                                                PyObject             *object);


GArray* _pygi_argument_to_array (GArgument  *arg,
                                 GArgument  *args[],
                                 GITypeInfo *type_info,
                                 gboolean    is_method);

GArgument _pygi_argument_from_object (PyObject   *object,
                                      GITypeInfo *type_info,
                                      GITransfer  transfer);

PyObject* _pygi_argument_to_object (GArgument  *arg,
                                    GITypeInfo *type_info,
                                    GITransfer  transfer);


void _pygi_argument_release (GArgument   *arg,
                             GITypeInfo  *type_info,
                             GITransfer   transfer,
                             GIDirection  direction);

void _pygi_argument_init (void);

G_END_DECLS

#endif /* __PYGI_ARGUMENT_H__ */
