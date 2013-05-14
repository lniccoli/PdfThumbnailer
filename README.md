PdfThumbnailer
==============

**PdfThumbnailer** is a very simple RESTful server providing PDF rasterization to PNG.

Just POST your PDF to it using the URL query string to choose the page you want
rasterized and optionally the dimensions of the PNG.

Dependencies
------------

**PdfThumbnailer** depends on libmupdf 1.1 for PDF rasterization and on libpoco
tested with 1.3p6) for the HTTP server framework. It uses qmake as the build system,
but does not depend on Qt otherwise.

Licence
-------

**PdfThumbnailer** is Copyright (C) 2013 AwTech s.r.l. and is distributed under the
terms of the GNU GPL v3.
For full terms see the file COPYING
