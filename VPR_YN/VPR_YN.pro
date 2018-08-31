#-------------------------------------------------
#
# Project created by QtCreator 2018-08-17T09:24:57
#
#-------------------------------------------------

QT       -= gui

TARGET = VPR_YN
TEMPLATE = lib

DEFINES += VPR_YN_LIBRARY

SOURCES += vpr_yn.cpp \
    coredump/MiniDumper.cpp \
    Camera/BaseCamera.cpp \
    Camera/Camera6467_plate.cpp \
    Camera/CameraResult.cpp \
    tinyxml/tinystr.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxmlparser.cpp \
    utilityTool/log4z.cpp \
    utilityTool/ToolFunction.cpp \
    Camera/ResultListManager.cpp

HEADERS += vpr_yn.h\
        vpr_yn_global.h \
    vpr_commondef_yn.h \
    coredump/MiniDumper.h \
    Camera/BaseCamera.h \
    Camera/Camera6467_plate.h \
    Camera/CameraResult.h \
    HvDevice/HvCamera.h \
    HvDevice/HvDeviceBaseDef.h \
    HvDevice/HvDeviceBaseType.h \
    HvDevice/HvDeviceCommDef.h \
    HvDevice/HvDeviceNew.h \
    tinyxml/tinystr.h \
    tinyxml/tinyxml.h \
    utilityTool/log4z.h \
    utilityTool/ToolFunction.h \
    Camera/ResultListManager.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -lshlwapi -lUser32 -ldbghelp -lshlwapi -lWS2_32  -L./HvDevice -lHvDevice

#鍔犲叆璋冭瘯淇℃�QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_RELEASE += -g

#绂佹浼樺寲
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O2

#release鍦ㄦ渶鍚巐ink鏃� �粯璁ゆ�-s鈥濆弬鏁帮紝琛ㄧ�Omit all symbol information from the output file"锛屽洜姝よ鍘绘帀璇ュ弬鏁QMAKE_LFLAGS_RELEASE = -mthreads -Wl

DISTFILES += \
   VPR_YN.rc

win32:RC_FILE = VPR_YN.rc
