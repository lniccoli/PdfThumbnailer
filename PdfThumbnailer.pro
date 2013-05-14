#-------------------------------------------------
#
# Project created by QtCreator 2013-05-08T18:03:35
#
#-------------------------------------------------

QT       -= core gui


TARGET = PdfThumbnailer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Build configuration
CONFIG(debug  , debug|release) {
  DEFINES += DEBUG _DEBUG
  BUILD = debug
  SUFFIX = d
}

CONFIG(release, debug|release) {
  BUILD = release
  SUFFIX =
}

SOURCES += main.cpp \
    handlerfactory.cpp \
    thumbnailrequesthandler.cpp \
    pdfrasterizer.cpp \
    thumbnailserver.cpp

LIBS += -lPocoFoundation$$SUFFIX$$POCO_SUFFIX \
-lPocoUtil$$SUFFIX$$POCO_SUFFIX \
-lPocoNet$$SUFFIX$$POCO_SUFFIX \
-lPocoXML$$SUFFIX$$POCO_SUFFIX \
-lfitz \
-lz \
-lfreetype \
-ljbig2dec \
-ljpeg \
-lopenjpeg

LIBS += -L$$PWD/lib/
INCLUDEPATH += $$PWD/include/
HEADERS += \
    handlerfactory.h \
    thumbnailrequesthandler.h \
    pdfrasterizer.h \
    thumbnailserver.h

OTHER_FILES += \
    README.md \
    COPYING
