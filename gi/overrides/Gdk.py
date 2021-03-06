# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab
#
# Copyright (C) 2009 Johan Dahlin <johan@gnome.org>
#               2010 Simon van der Linden <svdlinden@src.gnome.org>
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

from ..types import override
from ..importer import modules

Gdk = modules['Gdk']


class Rectangle(Gdk.Rectangle):

    def __init__(self, x, y, width, height):
        Gdk.Rectangle.__init__(self)
        self.x = x
        self.y = y
        self.width = width
        self.height = height

    def __new__(cls, *args, **kwargs):
        return Gdk.Rectangle.__new__(cls)

    def __repr__(self):
        return '<Gdk.Rectangle(x=%d, y=%d, width=%d, height=%d)>' % (
		    self.x, self.y, self.width, self.height)

Rectangle = override(Rectangle)


__all__ = [Rectangle]


import sys

initialized, argv = Gdk.init_check(sys.argv)
if not initialized:
    raise RuntimeError("Gdk couldn't be initialized")
