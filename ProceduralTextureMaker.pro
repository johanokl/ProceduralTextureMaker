TEMPLATE = app
DESTDIR = bin

QT += xml \
    widgets \
    gui \
    qml \
    script \
    core

CONFIG += qt

INCLUDEPATH += . \
    include

SOURCES = main.cpp \
    core/texturenode.cpp \
    core/textureimage.cpp \
    core/texturerenderthread.cpp \
    core/settingsmanager.cpp \
    gui/nodesettingswidget.cpp \
    gui/mainwindow.cpp \
    gui/addnodepanel.cpp \
    gui/previewpanel.cpp \
    gui/settingspanel.cpp \
    sceneview/viewnodeview.cpp \
    sceneview/viewnodeitem.cpp \
    sceneview/viewnodescene.cpp \
    sceneview/viewnodeline.cpp \
    generators/blending.cpp \
    generators/boxblur.cpp \
    generators/circle.cpp \
    generators/fill.cpp \
    generators/gaussianblur.cpp \
    generators/greyscale.cpp \
    generators/modifyalpha.cpp \
    generators/normalmap.cpp \
    generators/perlinnoise.cpp \
    generators/sineplasma.cpp \
    generators/square.cpp \
    generators/star.cpp \
    generators/transform.cpp \
    texgenapplication.cpp \
    gui/menuactions.cpp \
    core/textureproject.cpp \
    gui/connectionwidget.cpp \
    gui/sceneinfowidget.cpp \
    gui/iteminfopanel.cpp \
    generators/setchannels.cpp \
    generators/mirror.cpp \
    generators/gradient.cpp \
    generators/empty.cpp \
    generators/javascript.cpp \
    generators/lines.cpp \
    generators/whirl.cpp \
    generators/sinetransform.cpp \
    generators/invert.cpp


HEADERS += core/texturenode.h \
    core/textureimage.h \
    core/texturerenderthread.h \
    core/settingsmanager.h \
    global.h \
    sceneview/viewnodeitem.h \
    sceneview/viewnodescene.h \
    sceneview/viewnodeline.h \
    sceneview/viewnodeview.h \
    gui/addnodepanel.h \
    gui/qdoubleslider.h \
    gui/nodesettingswidget.h \
    gui/previewpanel.h \
    gui/settingspanel.h \
    gui/mainwindow.h \
    generators/texturegenerator.h \
    generators/blending.h \
    generators/boxblur.h \
    generators/circle.h \
    generators/empty.h \
    generators/fill.h \
    generators/gaussianblur.h \
    generators/greyscale.h \
    generators/modifyalpha.h \
    generators/normalmap.h \
    generators/perlinnoise.h \
    generators/sineplasma.h \
    generators/square.h \
    generators/star.h \
    generators/transform.h \
    texgenapplication.h \
    gui/menuactions.h \
    core/textureproject.h \
    gui/connectionwidget.h \
    gui/sceneinfowidget.h \
    gui/iteminfopanel.h \
    generators/setchannels.h \
    generators/mirror.h \
    generators/gradient.h \
    generators/javascript.h \
    generators/lines.h \
    generators/whirl.h \
    generators/sinetransform.h \
    generators/invert.h

TARGET = "ProceduralTextureMaker"

macx {
  # Possible to include spaces in the file name if on Mac OS X.
  TARGET = "ProceduralTextureMaker"
}

RESOURCES += texgen.qrc
RC_ICONS += images/mainicon.ico
ICON += images/mainicon.icns

QMAKE_INFO_PLIST += Info.plist
OTHER_FILES += Info.plist

QMAKE_MAC_SDK = 10.13
