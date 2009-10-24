/*!
 * \file mainwindow.h
 * \author Copyright &copy; 2007-2009 Lёppa <contacts[at]oleksii[dot]name>
 *
 *  $Id$
 *  $URL$
 *
 * \brief Defines MainWindow class.
 *
 *  <b>TSPSG: TSP Solver and Generator</b>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "globals.h"

#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include "tspsolver.h"
#include "tspmodel.h"

/*!
 * \brief Class for handling Main Window UI and logic.
 * \author Copyright &copy; 2007-2009 Lёppa <contacts[at]oleksii[dot]name>
 */
class MainWindow: public QMainWindow, Ui::MainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	void closeEvent(QCloseEvent *);

private slots:
// Actions
	void actionFileNewTriggered();
	void actionFileOpenTriggered();
	void actionFileSaveTriggered();
	void actionFileSaveAsTaskTriggered();
	void actionFileSaveAsSolutionTriggered();
#ifndef QT_NO_PRINTER
	void actionFilePrintPreviewTriggered();
	void actionFilePrintTriggered();
#endif // QT_NO_PRINTER
	void actionSettingsPreferencesTriggered();
	void actionSettingsLanguageAutodetectTriggered(bool);
	void groupSettingsLanguageListTriggered(QAction *);
	void actionHelpAboutTriggered();
// Buttons
	void buttonBackToTaskClicked();
	void buttonRandomClicked();
	void buttonSolveClicked();

	void dataChanged();
	void dataChanged(const QModelIndex &, const QModelIndex &);
	void numCitiesChanged(int);
#ifndef QT_NO_PRINTER
	void printPreview(QPrinter *printer);
#endif // QT_NO_PRINTER
	void spinCitiesValueChanged(int);

private:
	QString fileName;
	QActionGroup *groupSettingsLanguageList;
	QStringList output;
#ifndef QT_NO_PRINTER
	QPrinter *printer;
#endif // QT_NO_PRINTER
	QSettings *settings;
	CTSPModel *tspmodel;

	void enableSolutionActions(bool enable = true);
	void initDocStyleSheet();
	void loadLangList();
	bool loadLanguage(QString lang = "");
	bool maybeSave();
	void outputMatrix(tMatrix, QStringList &, int nRow = -1, int nCol = -1);
	bool saveTask();
	void setFileName(QString fileName = trUtf8("Untitled") + ".tspt");
};

#endif // MAINWINDOW_H
