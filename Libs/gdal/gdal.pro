include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = gdal
TARGET = $$qtLibraryName($$TARGET)

#根据不同的编译清空生成不同的输出路径
CONFIG(release, debug|release) {
  DEFINES *= RELEASE _RELEASE NDEBUG
  CONFIG_NAME = Release
} else {
  DEFINES *= DEBUG _DEBUG
  CONFIG_NAME = Debug
}

TargetRoot=$$dirname(PWD)
BINARIES_PATH = $$TargetRoot/out/$$CONFIG_NAME
DESTDIR = $$BINARIES_PATH

message("output:" + $$DESTDIR)
INCLUDEPATH += $${PWD}/include/gdal


SOURCES += \
    src/cpl_conv.cpp \
    src/cpl_csv.cpp \
    src/cpl_error.cpp \
    src/cpl_findfile.cpp \
    src/cpl_minixml.cpp \
    src/cpl_path.cpp \
    src/cpl_string.cpp \
    src/cpl_vsisimple.cpp \
    src/cplgetsymbol.cpp \
    src/gdal_misc.cpp \
    src/ograssemblepolygon.cpp \
    src/ogrcurve.cpp \
    src/ogrfeature.cpp \
    src/ogrfeaturedefn.cpp \
    src/ogrfielddefn.cpp \
    src/ogrgeometry.cpp \
    src/ogrgeometrycollection.cpp \
    src/ogrgeometryfactory.cpp \
    src/ogrlayer.cpp \
    src/ogrlinearring.cpp \
    src/ogrlinestring.cpp \
    src/ogrmultilinestring.cpp \
    src/ogrmultipoint.cpp \
    src/ogrmultipolygon.cpp \
    src/ogrpoint.cpp \
    src/ogrpolygon.cpp \
    src/ogrutils.cpp

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install;

