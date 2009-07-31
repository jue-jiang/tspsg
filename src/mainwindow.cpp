	/*
 *  TSPSG: TSP Solver and Generator
 *  Copyright (C) 2007-2009 Lёppa <contacts[at]oleksii[dot]name>
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

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"TSPSG","tspsg");
	loadLanguage();
	setupUi(this);
	initDocStyleSheet();
	solutionText->document()->setDefaultFont(settings->value("Output/Font",QFont(DEF_FONT_FAMILY,DEF_FONT_SIZE)).value<QFont>());
	solutionText->setTextColor(settings->value("Output/Color",DEF_FONT_COLOR).value<QColor>());
	solutionText->setWordWrapMode(QTextOption::WordWrap);
#ifdef Q_OS_WINCE
	// A little hack for toolbar icons to have sane size.
int s = qMin(QApplication::desktop()->screenGeometry().width(),QApplication::desktop()->screenGeometry().height());
	toolBar->setIconSize(QSize(s / 10,s / 10));
#endif
#ifndef Q_OS_WINCE
	printer = new QPrinter();
#endif // Q_OS_WINCE
	groupSettingsLanguageList = new QActionGroup(this);
	actionSettingsLanguageEnglish->setData("en");
	actionSettingsLanguageEnglish->setActionGroup(groupSettingsLanguageList);
	loadLangList();
	spinCities->setValue(settings->value("NumCities",5).toInt());
	actionSettingsLanguageAutodetect->setChecked(settings->value("Language","").toString().isEmpty());
	connect(actionFileNew,SIGNAL(triggered()),this,SLOT(actionFileNewTriggered()));
	connect(actionFileOpen,SIGNAL(triggered()),this,SLOT(actionFileOpenTriggered()));
	connect(actionFileSaveAsTask,SIGNAL(triggered()),this,SLOT(actionFileSaveAsTaskTriggered()));
	connect(actionFileSaveAsSolution,SIGNAL(triggered()),this,SLOT(actionFileSaveAsSolutionTriggered()));
	connect(actionSettingsPreferences,SIGNAL(triggered()),this,SLOT(actionSettingsPreferencesTriggered()));
	connect(actionSettingsLanguageAutodetect,SIGNAL(triggered(bool)),this,SLOT(actionSettingsLanguageAutodetectTriggered(bool)));
	connect(groupSettingsLanguageList,SIGNAL(triggered(QAction *)),this,SLOT(groupSettingsLanguageListTriggered(QAction *)));
	connect(actionHelpAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
	connect(actionHelpAbout,SIGNAL(triggered()),this,SLOT(actionHelpAboutTriggered()));
#ifndef Q_OS_WINCE
	connect(actionFilePrintSetup,SIGNAL(triggered()),this,SLOT(actionFilePrintSetupTriggered()));
#endif // Q_OS_WINCE
	connect(buttonSolve,SIGNAL(clicked()),this,SLOT(buttonSolveClicked()));
	connect(buttonRandom,SIGNAL(clicked()),this,SLOT(buttonRandomClicked()));
	connect(spinCities,SIGNAL(valueChanged(int)),this,SLOT(spinCitiesValueChanged(int)));
QRect rect = geometry();
	setCentralWidget(tabWidget);
#ifndef Q_OS_WINCE
	if (settings->value("SavePos",false).toBool()) {
		// Loading of saved window state
		settings->beginGroup("MainWindow");
		resize(settings->value("Size",size()).toSize());
		move(settings->value("Position",pos()).toPoint());
		if (settings->value("Maximized",false).toBool())
			setWindowState(windowState() | Qt::WindowMaximized);
		settings->endGroup();
	} else {
		// Centering main window
		rect.moveCenter(QApplication::desktop()->availableGeometry(this).center());
		setGeometry(rect);
	}
#endif // Q_OS_WINCE
	qsrand(QDateTime().currentDateTime().toTime_t());
	tspmodel = new CTSPModel();
	tspmodel->setNumCities(spinCities->value());
	taskView->setModel(tspmodel);
	connect(tspmodel,SIGNAL(numCitiesChanged(int)),this,SLOT(numCitiesChanged(int)));
	connect(tspmodel,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(dataChanged()));
	connect(tspmodel,SIGNAL(layoutChanged()),this,SLOT(dataChanged()));
#ifdef Q_OS_WINCE
	taskView->resizeColumnsToContents();
	taskView->resizeRowsToContents();
#endif // Q_OS_WINCE
}

void MainWindow::enableSolutionActions(bool enable)
{
	actionFileSaveAsSolution->setEnabled(enable);
	solutionText->setEnabled(enable);
	if (!enable)
		output.clear();
}

bool MainWindow::loadLanguage(QString lang)
{
// i18n
bool ad = false;
	if (lang.isEmpty()) {
		ad = settings->value("Language","").toString().isEmpty();
		lang = settings->value("Language",QLocale::system().name()).toString();
	}
static QTranslator *qtTranslator;
	if (qtTranslator) {
		qApp->removeTranslator(qtTranslator);
		delete qtTranslator;
		qtTranslator = NULL;
	}
	qtTranslator = new QTranslator();
static QTranslator *translator;
	if (translator) {
		qApp->removeTranslator(translator);
		delete translator;
	}
	translator = new QTranslator();
	if (lang.compare("en") && !lang.startsWith("en_")) {
		// Trying to load system Qt library translation...
		if (qtTranslator->load("qt_" + lang,QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
			qApp->installTranslator(qtTranslator);
		else
			// No luck. Let's try to load bundled one.
			if (qtTranslator->load("qt_" + lang,"i18n"))
				qApp->installTranslator(qtTranslator);
			else {
				delete qtTranslator;
				qtTranslator = NULL;
			}
		// Now let's load application translation.
		if (translator->load(lang,"i18n"))
			qApp->installTranslator(translator);
		else {
			if (!ad)
				QMessageBox(QMessageBox::Warning,trUtf8("Language change"),trUtf8("Unable to load translation language."),QMessageBox::Ok,this).exec();
			delete translator;
			translator = NULL;
			return false;
		}
	}
	return true;
}

void MainWindow::initDocStyleSheet()
{
QColor color = settings->value("Output/Color",DEF_FONT_COLOR).value<QColor>();
QColor hilight;
	if (color.value() < 192)
		hilight.setHsv(color.hue(),color.saturation(),127 + qRound(color.value() / 2));
	else
		hilight.setHsv(color.hue(),color.saturation(),color.value() / 2);
	solutionText->document()->setDefaultStyleSheet("* {color: " + color.name() +";} p {margin: 0px 10px;} table {margin: 5px;} td {padding: 1px 5px;} .hasalts {color: " + hilight.name() + ";} .selected {color: #A00000; font-weight: bold;} .alternate {color: #008000; font-weight: bold;}");
	solutionText->document()->setDefaultFont(settings->value("Output/Font",QFont(DEF_FONT_FAMILY,DEF_FONT_SIZE)).value<QFont>());
}

void MainWindow::spinCitiesValueChanged(int n)
{
#ifdef Q_OS_WINCE
int count = tspmodel->numCities();
#endif // Q_OS_WINCE
	tspmodel->setNumCities(n);
#ifdef Q_OS_WINCE
	if (n > count)
		for (int k = count; k < n; k++) {
			taskView->resizeColumnToContents(k);
			taskView->resizeRowToContents(k);
		}
#endif // Q_OS_WINCE
}


void MainWindow::actionFileNewTriggered()
{
	if (isWindowModified()) {
int res = QMessageBox(QMessageBox::Warning,trUtf8("New Task"),trUtf8("Would you like to save changes in current task?"),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,this).exec();
		if ((res == QMessageBox::Cancel) || ((res == QMessageBox::Yes) && !saveTask()))
			return;
	}
	tspmodel->clear();
	setWindowModified(false);
	tabWidget->setCurrentIndex(0);
	solutionText->clear();
	enableSolutionActions(false);
}

void MainWindow::actionFileOpenTriggered()
{
	if (isWindowModified()) {
int res = QMessageBox(QMessageBox::Warning,trUtf8("Task Open"),trUtf8("Would you like to save changes in current task?"),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,this).exec();
		if ((res == QMessageBox::Cancel) || ((res == QMessageBox::Yes) && !saveTask()))
			return;
	}
QFileDialog od(this);
	od.setAcceptMode(QFileDialog::AcceptOpen);
	od.setFileMode(QFileDialog::ExistingFile);
QStringList filters(trUtf8("All Supported Formats") + " (*.tspt *.zkt)");
	filters.append(QString(trUtf8("%1 Task Files")).arg("TSPSG") + " (*.tspt)");
	filters.append(QString(trUtf8("%1 Task Files")).arg("ZKomModRd") + " (*.zkt)");
	filters.append(trUtf8("All Files") + " (*)");
	od.setNameFilters(filters);
	if (od.exec() != QDialog::Accepted)
		return;
QStringList files = od.selectedFiles();
	if (files.size() < 1)
		return;
	tspmodel->loadTask(files.first());
	setWindowModified(false);
	solutionText->clear();
	enableSolutionActions(false);
}

void MainWindow::actionFileSaveAsTaskTriggered()
{
	saveTask();
}

void MainWindow::actionFileSaveAsSolutionTriggered()
{
static QString selectedFile;
	if (selectedFile.isEmpty())
		selectedFile = "solution.html";
QFileDialog sd(this);
	sd.setAcceptMode(QFileDialog::AcceptSave);
QStringList filters(trUtf8("HTML Files") + " (*.html *.htm)");
	filters.append(trUtf8("OpenDocument Files") + " (*.odt)");
	filters.append(trUtf8("All Files") + " (*)");
	sd.setNameFilters(filters);
	sd.selectFile(selectedFile);
	if (sd.exec() != QDialog::Accepted)
		return;
QStringList files = sd.selectedFiles();
	if (files.empty())
		return;
	selectedFile = files.first();
QTextDocumentWriter dw(selectedFile);
	if (!(selectedFile.endsWith(".htm",Qt::CaseInsensitive) || selectedFile.endsWith(".html",Qt::CaseInsensitive) || selectedFile.endsWith(".odt",Qt::CaseInsensitive) || selectedFile.endsWith(".txt",Qt::CaseInsensitive)))
		dw.setFormat("plaintext");
	dw.write(solutionText->document());
}

bool MainWindow::saveTask() {
QFileDialog sd(this);
	sd.setAcceptMode(QFileDialog::AcceptSave);
QStringList filters(QString(trUtf8("%1 Task File")).arg("TSPSG") + " (*.tspt)");
	filters.append(trUtf8("All Files") + " (*)");
	sd.setNameFilters(filters);
	sd.setDefaultSuffix("tspt");
	if (sd.exec() != QDialog::Accepted)
		return false;
QStringList files = sd.selectedFiles();
	if (files.size() < 1)
		return false;
	if (tspmodel->saveTask(files.first())) {
		setWindowModified(false);
		return true;
	} else
		return false;
}

void MainWindow::actionSettingsPreferencesTriggered()
{
SettingsDialog sd(this);
	if (sd.exec() != QDialog::Accepted)
		return;
	if (sd.colorChanged() || sd.fontChanged()) {
		initDocStyleSheet();
		if (!output.isEmpty() && sd.colorChanged() && (QMessageBox(QMessageBox::Question,trUtf8("Settings Changed"),trUtf8("You have changed color settings.\nDo you wish to apply them to current solution text?"),QMessageBox::Yes | QMessageBox::No,this).exec() == QMessageBox::Yes)) {
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			solutionText->clear();
			solutionText->setHtml(output.join(""));
			QApplication::restoreOverrideCursor();
		}
	}
}

#ifndef Q_OS_WINCE
void MainWindow::actionFilePrintSetupTriggered()
{
QPrintDialog pd(printer,this);
#if QT_VERSION >= 0x040500
	// No such methods in Qt < 4.5
	pd.setOption(QAbstractPrintDialog::PrintSelection,false);
	pd.setOption(QAbstractPrintDialog::PrintPageRange,false);
#endif
	pd.exec();
}
#endif // Q_OS_WINCE

void MainWindow::buttonRandomClicked()
{
	tspmodel->randomize();
	setWindowModified(true);
#ifdef Q_OS_WINCE
	taskView->resizeColumnsToContents();
	taskView->resizeRowsToContents();
#endif // Q_OS_WINCE
}

void MainWindow::outputMatrix(tMatrix matrix, QStringList &output, int nRow, int nCol)
{
int n = spinCities->value();
QString line="";
	output.append("<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">");
	for (int r = 0; r < n; r++) {
		line = "<tr>";
		for (int c = 0; c < n; c++) {
			if (matrix[r][c] == INFINITY)
				line += "<td align=\"center\">"INFSTR"</td>";
			else if ((r == nRow) && (c == nCol))
				line += "<td align=\"center\" class=\"selected\">" + QVariant(matrix[r][c]).toString() + "</td>";
			else
				line += "<td align=\"center\">" + QVariant(matrix[r][c]).toString() + "</td>";
		}
		line += "</tr>";
		output.append(line);
	}
	output.append("</table>");
}

void MainWindow::buttonSolveClicked()
{
tMatrix matrix;
QList<double> row;
int n = spinCities->value();
bool ok;
	for (int r = 0; r < n; r++) {
		row.clear();
		for (int c = 0; c < n; c++) {
			row.append(tspmodel->index(r,c).data(Qt::UserRole).toDouble(&ok));
			if (!ok) {
				QMessageBox(QMessageBox::Critical,trUtf8("Data error"),QString(trUtf8("Error in cell [Row %1; Column %2]: Invalid data format.")).arg(r + 1).arg(c + 1),QMessageBox::Ok,this).exec();
				return;
			}
		}
		matrix.append(row);
	}
CTSPSolver solver;
sStep *root = solver.solve(spinCities->value(),matrix);
	if (!root)
		return;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
QColor color = settings->value("Output/Color",DEF_FONT_COLOR).value<QColor>();
	output.clear();
	output.append("<p>" + trUtf8("Variant #%1").arg(spinVariant->value()) + "</p>");
	output.append("<p>" + trUtf8("Task:") + "</p>");
	outputMatrix(matrix,output);
	output.append("<hr>");
	output.append("<p>" + trUtf8("Solution of Variant #%1 task").arg(spinVariant->value()) + "</p>");
sStep *step = root;
	n = 1;
QString path = "";
	while (n <= spinCities->value()) {
		if (step->prNode->prNode != NULL || (step->prNode->prNode == NULL && step->plNode->prNode == NULL)) {
			if (n != spinCities->value()) {
				output.append("<p>" + trUtf8("Step #%1").arg(n++) + "</p>");
				outputMatrix(step->matrix,output,step->candidate.nRow,step->candidate.nCol);
				if (step->alts)
					output.append("<p class=\"hasalts\">" + trUtf8("This step has alternate candidates for branching.") + "</p>");
				output.append("<p>&nbsp;</p>");
			}
			path += QString(" (%1,%2)").arg(step->candidate.nRow + 1).arg(step->candidate.nCol + 1);
		}
		if (step->prNode->prNode != NULL)
			step = step->prNode;
		else if (step->plNode->prNode != NULL)
			step = step->plNode;
		else
			break;
	}
	output.append("<p>" + trUtf8("Optimal path:") + "</p>");
	output.append("<p>&nbsp;&nbsp;" + path + "</p>");
	output.append("<p>" + trUtf8("The price is <b>%1</b> units.").arg(step->price) + "</p>");
	solutionText->setHtml(output.join(""));
	solutionText->setDocumentTitle(trUtf8("Solution of Variant #%1 task").arg(spinVariant->value()));
	enableSolutionActions();
	tabWidget->setCurrentIndex(1);
	QApplication::restoreOverrideCursor();
}

void MainWindow::actionHelpAboutTriggered()
{
	// TODO: Normal about window :-)
QString about = QString::fromUtf8("TSPSG - TSP Solver and Generator\n");
	about += QString::fromUtf8("    Version: "BUILD_VERSION" ("BUILD_STATUS")\n");
	about += QString::fromUtf8("    Copyright (C) 2007-%1 Lёppa <contacts[at]oleksii[dot]name>\n").arg(QDate::currentDate().toString("yyyy"));
	about += QString::fromUtf8("Target OS: %1\n").arg(OS);
	about += "Qt library:\n";
	about += QString::fromUtf8("    Compile time: %1\n").arg(QT_VERSION_STR);
	about += QString::fromUtf8("    Runtime: %1\n").arg(qVersion());
	about += QString::fromUtf8("Built on %1 at %2\n").arg(__DATE__).arg(__TIME__);
	about += "\n";
	about += "TSPSG is licensed under the terms of the GNU General Public License. You should have received a copy of the GNU General Public License along with TSPSG.";
	QMessageBox(QMessageBox::Information,"About",about,QMessageBox::Ok,this).exec();
}

void MainWindow::loadLangList()
{
QSettings langinfo("i18n/languages.ini",QSettings::IniFormat);
#if QT_VERSION >= 0x040500
	// In Qt < 4.5 QSettings doesn't have method setIniCodec.
	langinfo.setIniCodec("UTF-8");
#endif
QDir dir("i18n","*.qm",QDir::Name | QDir::IgnoreCase,QDir::Files);
	if (!dir.exists())
		return;
QFileInfoList langs = dir.entryInfoList();
	if (langs.size() <= 0)
		return;
QAction *a;
	for (int k = 0; k < langs.size(); k++) {
		QFileInfo lang = langs.at(k);
		if (!lang.completeBaseName().startsWith("qt_") && lang.completeBaseName().compare("en")) {
#if QT_VERSION >= 0x040500
			a = menuSettingsLanguage->addAction(langinfo.value(lang.completeBaseName() + "/NativeName",lang.completeBaseName()).toString());
#else
			// We use Name if Qt < 4.5 because NativeName is in UTF-8, QSettings
			// reads .ini file as ASCII and there is no way to set file encoding.
			a = menuSettingsLanguage->addAction(langinfo.value(lang.completeBaseName() + "/Name",lang.completeBaseName()).toString());
#endif
			a->setData(lang.completeBaseName());
			a->setCheckable(true);
			a->setActionGroup(groupSettingsLanguageList);
			if (settings->value("Language",QLocale::system().name()).toString().startsWith(lang.completeBaseName()))
				a->setChecked(true);
		}
	}
}

void MainWindow::actionSettingsLanguageAutodetectTriggered(bool checked)
{
	if (checked) {
		settings->remove("Language");
		QMessageBox(QMessageBox::Information,trUtf8("Language change"),trUtf8("Language will be autodetected on next application start."),QMessageBox::Ok,this).exec();
	} else
		settings->setValue("Language",groupSettingsLanguageList->checkedAction()->data().toString());
}

void MainWindow::groupSettingsLanguageListTriggered(QAction *action)
{
	if (actionSettingsLanguageAutodetect->isChecked()) {
		// We have language autodetection. It needs to be disabled to change language.
		if (QMessageBox(QMessageBox::Question,trUtf8("Language change"),trUtf8("You have language autodetection turned on.\nIt needs to be off.\nDo you wish to turn it off?"),QMessageBox::Yes | QMessageBox::No,this).exec() == QMessageBox::Yes) {
			actionSettingsLanguageAutodetect->trigger();
		} else
			return;
	}
	if (loadLanguage(action->data().toString())) {
		settings->setValue("Language",action->data().toString());
		retranslateUi(this);
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isWindowModified()) {
int res = QMessageBox(QMessageBox::Warning,trUtf8("Application Close"),trUtf8("Would you like to save changes in current task?"),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,this).exec();
		if ((res == QMessageBox::Cancel) || ((res == QMessageBox::Yes) && !saveTask())) {
			event->ignore();
			return;
		}
	}
	settings->setValue("NumCities",spinCities->value());
#ifndef Q_OS_WINCE
	// Saving windows state
	if (settings->value("SavePos",false).toBool()) {
		settings->beginGroup("MainWindow");
		settings->setValue("Maximized",isMaximized());
		if (!isMaximized()) {
			settings->setValue("Size",size());
			settings->setValue("Position",pos());
		}
		settings->endGroup();
	}
#endif // Q_OS_WINCE
	QMainWindow::closeEvent(event);
}

void MainWindow::dataChanged()
{
	setWindowModified(true);
}

void MainWindow::numCitiesChanged(int nCities)
{
	spinCities->setValue(nCities);
}
