######################################################################
#
#  TSPSG: TSP Solver and Generator
#  Copyright (C) 2007-2011 Lёppa <contacts[at]oleksii[dot]name>
#
#  $Id$
#  $URL$
#
#  This file is part of TSPSG.
#
######################################################################

#Header files
HEADERS += src/globals.h \
    src/version.h \
    src/os.h \
    src/defaults.h \
    src/mainwindow.h \
    src/settingsdialog.h \
    src/tspmodel.h \
    src/tspsolver.h \
    src/resource.h \
    src/qtwin.h

#Source files
SOURCES += src/main.cpp \
    src/globals.cpp \
    src/mainwindow.cpp \
    src/settingsdialog.cpp \
    src/tspmodel.cpp \
    src/tspsolver.cpp \
    src/qtwin.cpp

#Forms
FORMS += ui/mainwindow.ui \
    ui/settingsdialog.ui

#Resource file(s)
RESOURCES += resources/common.qrc \
    l10n/l10n.qrc

handheld {
    RESOURCES += resources/handheld.qrc
} else {
    RESOURCES += resources/desktop.qrc
}

win* {
    RC_FILE = resources/tspsg.rc
}

#Translations
CODECFORTR = UTF-8
TRANSLATIONS += l10n/template.ts \
    l10n/tspsg_en.ts \
    l10n/tspsg_ru.ts \
    l10n/tspsg_uk.ts

blackberry {
    OTHER_FILES += \
        bar-descriptor.xml
}

!nopch {
    # Precompiled header
    PRECOMPILED_HEADER = src/pch.h
}

_t_t_l_ {
    DEFINES += _T_T_L_
    HEADERS += src/_.h
}
