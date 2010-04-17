# -*- Mode: Python; py-indent-offset: 4 -*-
# vim: tabstop=4 shiftwidth=4 expandtab

import unittest

import sys
sys.path.insert(0, "../")

import gobject
import cairo

from gi.repository import Everything

class TestEverything(unittest.TestCase):

    def test_cairo_context(self):
        context = Everything.test_cairo_context_full_return()
        self.assertTrue(isinstance(context, cairo.Context))

        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 10, 10)
        context = cairo.Context(surface)
        Everything.test_cairo_context_none_in(context)

    def test_cairo_surface(self):
        surface = Everything.test_cairo_surface_none_return()
        self.assertTrue(isinstance(surface, cairo.ImageSurface))
        self.assertTrue(isinstance(surface, cairo.Surface))
        self.assertEquals(surface.get_format(), cairo.FORMAT_ARGB32)
        self.assertEquals(surface.get_width(), 10)
        self.assertEquals(surface.get_height(), 10)

        surface = Everything.test_cairo_surface_full_return()
        self.assertTrue(isinstance(surface, cairo.ImageSurface))
        self.assertTrue(isinstance(surface, cairo.Surface))
        self.assertEquals(surface.get_format(), cairo.FORMAT_ARGB32)
        self.assertEquals(surface.get_width(), 10)
        self.assertEquals(surface.get_height(), 10)

        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 10, 10)
        Everything.test_cairo_surface_none_in(surface)

        surface = Everything.test_cairo_surface_full_out()
        self.assertTrue(isinstance(surface, cairo.ImageSurface))
        self.assertTrue(isinstance(surface, cairo.Surface))
        self.assertEquals(surface.get_format(), cairo.FORMAT_ARGB32)
        self.assertEquals(surface.get_width(), 10)
        self.assertEquals(surface.get_height(), 10)

class TestCallbacks(unittest.TestCase):
    called = False
    def testCallback(self):
        TestCallbacks.called = False
        def callback():
            TestCallbacks.called = True
        
        Everything.test_simple_callback(callback)
        self.assertTrue(TestCallbacks.called)

    def testCallbackException(self):
        """
        This test ensures that we get errors from callbacks correctly
        and in particular that we do not segv when callbacks fail
        """
        def callback():
            x = 1 / 0
            
        try:
            Everything.test_simple_callback(callback)
        except ZeroDivisionError:
            pass

    def testDoubleCallbackException(self):
        """
        This test ensures that we get errors from callbacks correctly
        and in particular that we do not segv when callbacks fail
        """
        def badcallback():
            x = 1 / 0

        def callback():
            Everything.test_boolean(True)
            Everything.test_boolean(False)
            Everything.test_simple_callback(badcallback())

        try:
            Everything.test_simple_callback(callback)
        except ZeroDivisionError:
            pass

    def testReturnValueCallback(self):
        TestCallbacks.called = False
        def callback():
            TestCallbacks.called = True
            return 44

        self.assertEquals(Everything.test_callback(callback), 44)
        self.assertTrue(TestCallbacks.called)
    
    def testCallbackAsync(self):
        TestCallbacks.called = False
        def callback(foo):
            TestCallbacks.called = True
            return foo

        Everything.test_callback_async(callback, 44);
        i = Everything.test_callback_thaw_async();
        self.assertEquals(44, i);
        self.assertTrue(TestCallbacks.called)
