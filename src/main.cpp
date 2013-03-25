/*
 *  TSPSG: TSP Solver and Generator
 *  Copyright (C) 2007-2011 Lёppa <contacts[at]oleksii[dot]name>
 *
 *  $Id$
 *  $URL$
 *
 *  This file is part of TSPSG.
 *
 *  TSPSG is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TSPSG is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TSPSG.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#if QT_VERSION < 0x040600
#   ifdef Q_CC_MSVC
#       pragma message("WARNING: You are using Qt version < 4.6. Application will not support some non-critical features.")
#   elif defined(Q_CC_GNU)
#       warning WARNING: You are using Qt version < 4.6. Application will not support some non-critical features.
#   else
#       error You are using Qt version < 4.6. Application will not support some non-critical features. To continue, please, comment line 31 at main.cpp.
#   endif
#endif

#ifdef Q_OS_BLACKBERRY

#include <QProxyStyle>

class BB10ProxyStyle: public QProxyStyle
{
public:
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
    {
        switch (metric) {
        case QStyle::PM_ButtonIconSize:
            return 42;
        case QStyle::PM_SmallIconSize:
            return 42;
        case QStyle::PM_LargeIconSize:
            return 64;
        default:
        return QProxyStyle::pixelMetric(metric, option, widget);
        }
    }
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
    {
        if (hint == QStyle::SH_UnderlineShortcut)
            return 0;
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

#endif // Q_OS_BLACKBERRY

//#ifdef STATIC_BUILD
//	#ifndef NOSVG
//		Q_IMPORT_PLUGIN(qsvgicon)
//	#endif
//#endif

int main(int argc, char *argv[])
{
#ifdef Q_WS_S60
    // No enough memory for solution graph generation with tasks
    // of 20 and more cities if we use non-raster graphics system.
    QApplication::setGraphicsSystem("raster");
#endif
QApplication app(argc, argv);
    app.setOverrideCursor(QCursor(Qt::WaitCursor));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    app.setOrganizationName("Oleksii \"Lёppa\" Serdiuk");
    app.setOrganizationDomain("oleksii.name");
    app.setApplicationName("TSP Solver and Generator");
    app.setApplicationVersion(BUILD_VERSION);

#ifdef Q_OS_BLACKBERRY
    QApplication::setStartDragDistance(42);
    QApplication::setStyle(new BB10ProxyStyle);
    app.setStyleSheet(
                "QMenuBar {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #444 stop:0.65 #333, stop:1 #444);"
                "    border-bottom: 1px solid darkgray;"
                "    spacing: 1px;"
                "}"
                "QMenuBar::item {"
                "    padding: 0.3em 0.5em;"
                "}"
                "QMenuBar::item:selected {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00a8df stop:0.65 #00889f, stop:1 #00a8df);"
                "}"
                "QMenu {"
                "    border: 1px solid darkgray;"
                "}"
                "QMenu::item {"
                "    padding: 0.4em 0.5em 0.4em 1.4em;"
                "    color: white;"
                "    font-size: 9pt;"
                "    background-color: #121212;"
                "}"
                "QMenu::item:selected {"
                "    background-color: #00a8df;"
                "}"
                "QMenu::item:disabled {"
                "    color: darkgray;"
                "    background-color: #121212;"
                "}"
                "QMenu::icon {"
                "    margin-left: 0.85em;"
                "}"
                "QTabBar {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #444 stop:0.65 #333, stop:1 #444);"
                "    border-bottom: 1px solid darkgray;"
                "    spacing: 1px;"
                "}"
                "QTabBar::tab {"
                "    padding: 0.3em 0.5em;"
                "}"
                "QTabBar::tab:selected {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00a8df stop:0.65 #00889f, stop:1 #00a8df);"
                "    padding: 0.3em 0.5em;"
                "}"
                "QTableView {"
                "    gridline-color: darkgray;"
                "}"
                "QTableView::item {"
                "    padding: 0 0.5em;"
                "}"
                "QComboBox {"
                "    padding: 0.5em 1.3em 0.5em 0.5em;"
                "}"
                "QTextBrowser {"
                "    background-color: lightgray;"
                "    color: black;"
                "}"
    );
#endif // Q_OS_BLACKBERRY

    // Seeding random number generator
    qsrand(QDateTime::currentDateTime().toTime_t() ^ QCursor::pos().x() ^ QCursor::pos().y());

#ifdef Q_WS_WINCE_WM
    // Qt "leaves" unpacked .ttf files after running - let's try to delete them.
QStringList files = QDir(app.applicationDirPath(), "*.ttf").entryList();
    foreach (QString file, files) {
        QFile::remove(file);
    }
#endif
    // Don't load the font if it is already available
    if (!QFontDatabase().families().contains(DEF_FONT_FACE))
        QFontDatabase::addApplicationFont(":/files/DejaVuLGCSansMono.ttf");

QTranslator en;
    if (en.load("tspsg_en", PATH_L10N))
        app.installTranslator(&en);
    else if (en.load("tspsg_en", ":/l10n"))
        app.installTranslator(&en);

MainWindow mainwindow;
#ifdef Q_WS_S60
    //! \hack HACK: A workaround to hide Actions menu item in Symbian.
QWidgetList widgets = QApplication::allWidgets();
QWidget *w = 0;
    foreach(w, widgets) {
        w->setContextMenuPolicy(Qt::NoContextMenu);
    }
#endif

#ifdef HANDHELD
    mainwindow.showMaximized();
#else // HANDHELD
    mainwindow.show();
#endif // HANDHELD
    app.restoreOverrideCursor();
    return app.exec();
}
