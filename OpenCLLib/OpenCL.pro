QMAKE_CXXFLAGS += -std=c++0x

LIBS+= -L /usr/lib64/nvidia/
LIBS+= -lOpenCL

INCLUDEPATH += /usr/include/CL/


SOURCES += \
    main.cpp
