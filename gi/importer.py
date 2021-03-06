# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab
#
# Copyright (C) 2005-2009 Johan Dahlin <johan@gnome.org>
#
#   importer.py: dynamic importer for introspected libraries.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

from __future__ import absolute_import

import sys
import gobject

from ._gi import Repository, RepositoryError
from .module import DynamicModule, ModuleProxy


repository = Repository.get_default()
modules = {}


class DynamicImporter(object):

    # Note: see PEP302 for the Importer Protocol implemented below.

    def __init__(self, path):
        self.path = path

    def find_module(self, fullname, path=None):
        if not fullname.startswith(self.path):
            return

        path, namespace = fullname.rsplit('.', 1)
        if path != self.path:
            return
        try:
            repository.require(namespace)
        except RepositoryError:
            pass
        else:
            return self

    def load_module(self, fullname):
        if fullname in sys.modules:
            return sys.modules[fullname]

        path, namespace = fullname.rsplit('.', 1)

        # Workaround for GObject
        if namespace == 'GObject':
            sys.modules[fullname] = gobject
            return gobject

        dynamic_module = DynamicModule(namespace)
        modules[namespace] = dynamic_module

        overrides_modules = __import__('gi.overrides', fromlist=[namespace])
        overrides_module = getattr(overrides_modules, namespace, None)

        if overrides_module is not None:
            module = ModuleProxy(fullname, namespace, dynamic_module, overrides_module)
        else:
            module = dynamic_module

        module.__file__ = '<%s>' % fullname
        module.__loader__ = self

        sys.modules[fullname] = module

        return module

