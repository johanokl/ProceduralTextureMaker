TEMPLATE = app
TARGET = "ProceduralTextureMaker"

macx {
  # Possible to include spaces in the file name if on Mac OS X.
  TARGET = "ProceduralTextureMaker"
  QMAKE_MAC_SDK = 10.13
  QMAKE_INFO_PLIST += Info.plist
  OTHER_FILES += Info.plist
}

# Select which Javascript engine to include
# -----------------------------------------
# Qt's QScriptEngine is deprecated and might be removed
# while QJSEngine (in Qt 5.9 and 5.10) still has some
# strange bugs releated to memory management.
# Uncomment the line "#defines USE_QJSENGINE" below if QtScript isn't
# available or QJSEngine is stable and enough to be a viable alternative.
# For QScriptEngine, remember to also include module script.
# For QJSEngine, remember to also include module qml.
# It is also possible to disable Javascript support, if both
# QML/QJSEngine and QtScript/QScriptEngine are missing from the system.
# To do so, uncomment the line "#defines DISABLE_JAVASCRIPT".
# -----------------------------------------
#DEFINES += "DISABLE_JAVASCRIPT"

qtHaveModule(qml) {
  QT += qml
  DEFINES += "USE_QJSENGINE"
}
else { qtHaveModule(script) {
  QT += script
}
else {
  DEFINES += "DISABLE_JAVASCRIPT"
}}

QT += xml \
    widgets \
    gui \
    core

CONFIG += c++11

INCLUDEPATH += src

SOURCES = \
    main.cpp \
    texgenapplication.cpp \
    base/texturenode.cpp \
    base/textureimage.cpp \
    base/texturerenderthread.cpp \
    base/settingsmanager.cpp \
    base/textureproject.cpp \
    gui/nodesettingswidget.cpp \
    gui/mainwindow.cpp \
    gui/addnodepanel.cpp \
    gui/settingspanel.cpp \
    gui/menuactions.cpp \
    gui/connectionwidget.cpp \
    gui/sceneinfowidget.cpp \
    gui/iteminfopanel.cpp \
    gui/previewimagepanel.cpp \
    gui/cubewidget.cpp \
    sceneview/viewnodeview.cpp \
    sceneview/viewnodeitem.cpp \
    sceneview/viewnodescene.cpp \
    sceneview/viewnodeline.cpp \
    generators/blending.cpp \
    generators/boxblur.cpp \
    generators/bricks.cpp \
    generators/checkboard.cpp \
    generators/circle.cpp \
    generators/cutout.cpp \
    generators/displacementmap.cpp \
    generators/empty.cpp \
    generators/fill.cpp \
    generators/fire.cpp \
    generators/gaussianblur.cpp \
    generators/glow.cpp \
    generators/greyscale.cpp \
    generators/gradient.cpp \
    generators/invert.cpp \
    generators/javascript.cpp \
    generators/lens.cpp \
    generators/lines.cpp \
    generators/mirror.cpp \
    generators/merge.cpp \
    generators/modifylevels.cpp \
    generators/noise.cpp \
    generators/normalmap.cpp \
    generators/perlinnoise.cpp \
    generators/pixelate.cpp \
    generators/pointillism.cpp \
    generators/shadow.cpp \
    generators/sinetransform.cpp \
    generators/sineplasma.cpp \
    generators/stackblur.cpp \
    generators/square.cpp \
    generators/star.cpp \
    generators/setchannels.cpp \
    generators/texturegenerator.cpp \
    generators/transform.cpp \
    generators/text.cpp \
    generators/whirl.cpp

HEADERS += \
    global.h \
    texgenapplication.h \
    base/texturenode.h \
    base/textureimage.h \
    base/texturerenderthread.h \
    base/settingsmanager.h \
    base/textureproject.h \
    gui/addnodepanel.h \
    gui/qdoubleslider.h \
    gui/nodesettingswidget.h \
    gui/settingspanel.h \
    gui/mainwindow.h \
    gui/menuactions.h \
    gui/connectionwidget.h \
    gui/sceneinfowidget.h \
    gui/iteminfopanel.h \
    gui/previewimagepanel.h \
    gui/cubewidget.h \
    sceneview/viewnodeitem.h \
    sceneview/viewnodescene.h \
    sceneview/viewnodeline.h \
    sceneview/viewnodeview.h \
    generators/texturegenerator.h \
    generators/blending.h \
    generators/bricks.h \
    generators/boxblur.h \
    generators/checkboard.h \
    generators/cutout.h \
    generators/circle.h \
    generators/displacementmap.h \
    generators/empty.h \
    generators/fill.h \
    generators/fire.h \
    generators/gaussianblur.h \
    generators/glow.h \
    generators/gradient.h \
    generators/greyscale.h \
    generators/invert.h \
    generators/javascript.h \
    generators/lens.h \
    generators/lines.h \
    generators/merge.h \
    generators/mirror.h \
    generators/modifylevels.h \
    generators/noise.h \
    generators/normalmap.h \
    generators/perlinnoise.h \
    generators/pixelate.h \
    generators/pointillism.h \
    generators/shadow.h \
    generators/sineplasma.h \
    generators/sinetransform.h \
    generators/setchannels.h \
    generators/square.h \
    generators/stackblur.h \
    generators/star.h \
    generators/text.h \
    generators/transform.h \
    generators/whirl.h

RESOURCES += \
    texgen.qrc \
    shaders.qrc

RC_ICONS += images/mainicon.ico
ICON += images/mainicon.icns
