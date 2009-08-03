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

#include "tspmodel.h"

CTSPModel::CTSPModel(QObject *parent)
	: QAbstractTableModel(parent), nCities(0)
{
	settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"TSPSG","tspsg");
}

inline int CTSPModel::rand(int min, int max) const
{
	return min + (int)(((float)qrand() / RAND_MAX) * max);
}

int CTSPModel::rowCount(const QModelIndex &) const
{
	return nCities;
}

int CTSPModel::columnCount(const QModelIndex &) const
{
	return nCities;
}

QVariant CTSPModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Vertical)
			return trUtf8("City %1").arg(section + 1);
		else
			return trUtf8("%1").arg(section + 1);
	}
	return QVariant();
}

QVariant CTSPModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::TextAlignmentRole)
		return int(Qt::AlignCenter);
	else if (role == Qt::FontRole) {
QFont font;
		font.setBold(true);
		return font;
	} else if (role == Qt::DisplayRole || role == Qt::EditRole) {
		if (index.row() < nCities && index.column() < nCities)
			if (table[index.row()][index.column()] == INFINITY)
				return trUtf8(INFSTR);
			else
				// HACK: Converting to string to prevent spinbox in edit mode
				return QVariant(table[index.row()][index.column()]).toString();
		else
			return QVariant();
	} else if (role == Qt::UserRole)
		return table[index.row()][index.column()];
	return QVariant();
}

bool CTSPModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;
	if (role == Qt::EditRole && index.row() != index.column()) {
		if (value.toString().compare(INFSTR) == 0)
			table[index.row()][index.column()] = INFINITY;
		else {
bool ok;
double tmp = value.toDouble(&ok);
			if (!ok || tmp < 0)
				return false;
			else
				table[index.row()][index.column()] = tmp;
		}
		emit dataChanged(index,index);
		return true;
	}
	return false;
}

Qt::ItemFlags CTSPModel::flags(const QModelIndex &index) const
{
Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	if (index.row() != index.column())
		flags |= Qt::ItemIsEditable;
	return flags;
}

quint16 CTSPModel::numCities() const
{
	return nCities;
}

void CTSPModel::setNumCities(int n)
{
	if (n == nCities)
		return;
	emit layoutAboutToBeChanged();
	table.resize(n);
	for (int k = 0; k < n; k++) {
		table[k].resize(n);
	}
	if (n > nCities)
		for (int k = nCities; k < n; k++)
			table[k][k] = INFINITY;
	nCities = n;
	emit layoutChanged();
}

void CTSPModel::clear()
{
	for (int r = 0; r < nCities; r++)
		for (int c = 0; c < nCities; c++)
			if (r != c)
				table[r][c] = 0;
	emit dataChanged(index(0,0),index(nCities - 1,nCities - 1));
}

inline bool CTSPModel::loadError(QDataStream::Status status)
{
QString err;
	if (status == QDataStream::Ok)
		return false;
	else if (status == QDataStream::ReadPastEnd)
		err = trUtf8("Unexpected end of file.");
	else if (status == QDataStream::ReadCorruptData)
		err = trUtf8("Corrupt data read. File possibly corrupted.");
	else
		err = trUtf8("Unknown error.");
	QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + err,QMessageBox::Ok).exec();
	return true;
}

bool CTSPModel::loadTask(QString fname)
{
QFile f(fname);
	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),QString(trUtf8("Unable to open task file.\nError: %1")).arg(f.errorString()),QMessageBox::Ok).exec();
		return false;
	}
QDataStream ds(&f);
	ds.setVersion(QDataStream::Qt_4_4);
quint32 sig;
	ds >> sig;
	if (loadError(ds.status()))
		return false;
	ds.device()->reset();
	if (sig == TSPT) {
		if (!loadTSPT(&ds)) {
			f.close();
			return false;
		}
	} else if ((sig >> 16) == ZKT) {
		if (!loadZKT(&ds)) {
			f.close();
			return false;
		}
	} else {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + trUtf8("Unknown file format or file is corrupted."),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	f.close();
	return true;
}

bool CTSPModel::loadTSPT(QDataStream *ds)
{
	// Skipping signature
	ds->skipRawData(sizeof(TSPT));
	if (loadError(ds->status()))
		return false;
	// File version
quint8 version;
	*ds >> version;
	if (loadError(ds->status()))
		return false;
	if (version > TSPT_VERSION) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + trUtf8("File version is newer than application supports.\nPlease, try to update application."),QMessageBox::Ok).exec();
		return false;
	}
	// Skipping metadata
	ds->skipRawData(TSPT_META_SIZE);
	if (loadError(ds->status()))
		return false;
	// Number of cities
quint16 size;
	*ds >> size;
	if (loadError(ds->status()))
		return false;
	if ((size < 3) || (size > MAX_NUM_CITIES)) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + trUtf8("Unexpected data read.\nFile is possibly corrupted."),QMessageBox::Ok).exec();
		return false;
	}
	if (nCities != size) {
		setNumCities(size);
		emit numCitiesChanged(size);
	}
	// Travel costs
	for (int r = 0; r < size; r++)
		for (int c = 0; c < size; c++)
			if (r != c) {
				*ds >> table[r][c];
				if (loadError(ds->status())) {
					clear();
					return false;
				}
			}
	emit dataChanged(index(0,0),index(nCities - 1,nCities - 1));
	return true;
}

bool CTSPModel::loadZKT(QDataStream *ds)
{
	// Skipping signature
	ds->skipRawData(sizeof(ZKT));
	if (loadError(ds->status()))
		return false;
	// File version
quint16 version;
	ds->readRawData(reinterpret_cast<char *>(&version),2);
	if (loadError(ds->status()))
		return false;
	if (version > ZKT_VERSION) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + trUtf8("File version is newer than application supports.\nPlease, try to update application."),QMessageBox::Ok).exec();
		return false;
	}
	// Number of cities
quint8 size;
	ds->readRawData(reinterpret_cast<char *>(&size),1);
	if (loadError(ds->status()))
		return false;
	if ((size < 3) || (size > 5)) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Load"),trUtf8("Unable to load task:") + "\n" + trUtf8("Unexpected data read.\nFile is possibly corrupted."),QMessageBox::Ok).exec();
		return false;
	}
	if (nCities != size) {
		setNumCities(size);
		emit numCitiesChanged(size);
	}
	// Travel costs
double val;
	for (int r = 0; r < 5; r++)
		for (int c = 0; c < 5; c++)
			if ((r != c) && (r < size)) {
				ds->readRawData(reinterpret_cast<char *>(&val),8);
				if (loadError(ds->status())) {
					clear();
					return false;
				}
				table[r][c] = val;
			} else {
				ds->skipRawData(8);
				if (loadError(ds->status())) {
					clear();
					return false;
				}
			}
	emit dataChanged(index(0,0),index(nCities - 1,nCities - 1));
	return true;
}

bool CTSPModel::saveTask(QString fname)
{
QFile f(fname);
	if (!f.open(QIODevice::WriteOnly)) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),QString(trUtf8("Unable to create task file.\nError: %1\nMaybe, file is read-only?")).arg(f.errorString()),QMessageBox::Ok).exec();
		return false;
	}
QDataStream ds(&f);
	ds.setVersion(QDataStream::Qt_4_4);
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// File signature
	ds << TSPT;
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// File version
	ds << TSPT_VERSION;
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// File metadata version
	ds << TSPT_META_VERSION;
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// Metadata
	ds << OSID;
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// Number of cities
	ds << nCities;
	if (f.error() != QFile::NoError) {
		QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
		f.close();
		return false;
	}
	// Costs
	for (int r = 0; r < nCities; r++)
		for (int c = 0; c < nCities; c++)
			if (r != c) {
				ds << table[r][c];
				if (f.error() != QFile::NoError) {
					QMessageBox(QMessageBox::Critical,trUtf8("Task Save"),trUtf8("Unable to save task.\nError: %1").arg(f.errorString()),QMessageBox::Ok).exec();
					f.close();
					return false;
				}
			}
	f.close();
	return true;
}

void CTSPModel::randomize()
{
int randMin = settings->value("MinCost",DEF_RAND_MIN).toInt();
int randMax = settings->value("MaxCost",DEF_RAND_MAX).toInt();
	for (int r = 0; r < nCities; r++)
		for (int c = 0; c < nCities; c++)
			if (r != c)
				table[r][c] = rand(randMin,randMax);
	emit dataChanged(index(0,0),index(nCities - 1,nCities - 1));
}
