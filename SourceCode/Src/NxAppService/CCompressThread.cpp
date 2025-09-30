#include "CCompressThread.h"
#include <quazip.h>
#include <quazipfile.h>
#include <quazipfileinfo.h>
#include <QDebug>
#include<QCryptographicHash>

BEGIN_NX_NAMESPACE

static bool copyData(QIODevice& inFile, QIODevice& outFile)
{
	auto data = inFile.readAll();
	outFile.write(data);

	//outFile.close();
	//inFile.close();
	return true;
}

int getCompressFileCount(QString fileCompressed, QString dir_path, int& fileCount)
{
	QDir dir(dir_path);
	if (!dir.exists()) {
		return -1;
	}
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::DirsFirst);
	QFileInfoList list = dir.entryInfoList();
	QStringList infolist = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
	if (list.size() < 1) {
		return -1;
	}
	int i = 0;
	do {
		QFileInfo fileInfo = list.at(i);
		bool bisDir = fileInfo.isDir();
		if (bisDir) {
			getCompressFileCount(fileCompressed, fileInfo.filePath(), fileCount);
		}
		else {
			for (int m = 0; m < infolist.size(); m++) {
				if (infolist.at(m) != fileCompressed)
				{
					fileCount++;
				}
			}
			break;
		}
		i++;
	} while (i < list.size());
	return 0;
}

CCompressThread::CCompressThread(CompressType _type, QString _fileCompressed, QString _selectPath, bool _encryption, QObject *parent)
    : QThread(parent)
	, m_bEncryption(_encryption)
{
	m_Type = _type;
	m_pFileCompressed = _fileCompressed;
	m_pSelectPath = _selectPath;
	m_iAllFileCount = 0;
	m_iCompressFileCount = 0;
}

void CCompressThread::compressProgress(QString fileName)
{
	if (m_Type == compress_Dir)
		emit signalCompressProgress(fileName, ++m_iCompressFileCount, m_iAllFileCount);
	else if (m_Type == extract_Dir)
		emit signalExtractProgress(fileName, ++m_iCompressFileCount, m_iAllFileCount);
}

bool CCompressThread::compressFile(QuaZip* zip, QString fileName, QString fileDest) {
	// zip: oggetto dove aggiungere il file
	// fileName: nome del file reale
	// fileDest: nome del file all'interno del file compresso
	// Controllo l'apertura dello zip
	if (!zip) return false;
	if (zip->getMode() != QuaZip::mdCreate &&
		zip->getMode() != QuaZip::mdAppend &&
		zip->getMode() != QuaZip::mdAdd) return false;
	// Apro il file originale
	QString strMD5 = QCryptographicHash::hash(QByteArray("123456"), QCryptographicHash::Md5).toHex();
	QFile inFile;
	inFile.setFileName(fileName);
	if (!inFile.open(QIODevice::ReadOnly)) return false;
	// Apro il file risulato
	QuaZipFile outFile(zip);

	if (m_bEncryption) {
		if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, inFile.fileName()),strMD5.toUtf8().constData())) return false;
	}
	else {
		if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileDest, inFile.fileName()))) return false;
	}
	
	// Copio i dati
	if (!copyData(inFile, outFile) || outFile.getZipError() != UNZ_OK) {
		return false;
	}
	// Chiudo i file
	outFile.close();
	if (outFile.getZipError() != UNZ_OK) return false;
	inFile.close();
	return true;
}

bool CCompressThread::compressSubDir(QuaZip* zip, QString dir, QString origDir, bool recursive) {
	// zip: oggetto dove aggiungere il file
	// dir: cartella reale corrente
	// origDir: cartella reale originale
	// (path(dir)-path(origDir)) = path interno all'oggetto zip
	// Controllo l'apertura dello zip
	if (!zip) return false;
	if (zip->getMode() != QuaZip::mdCreate &&
		zip->getMode() != QuaZip::mdAppend &&
		zip->getMode() != QuaZip::mdAdd) return false;
	// Controllo la cartella
	QDir directory(dir);
	if (!directory.exists()) return false;
	QDir origDirectory(origDir);
	if (dir != origDir) {
		QuaZipFile dirZipFile(zip);
		if (!dirZipFile.open(QIODevice::WriteOnly,
			QuaZipNewInfo(origDirectory.relativeFilePath(dir) + "/", dir), 0, 0)) {
			return false;
		}
		dirZipFile.close();
	}
	// Se comprimo anche le sotto cartelle
	if (recursive) {
		// Per ogni sotto cartella
		QFileInfoList files = directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		Q_FOREACH(QFileInfo file, files) {
			// Comprimo la sotto cartella
			if (!compressSubDir(zip, file.absoluteFilePath(), origDir, recursive)) return false;
		}
	}
	// Per ogni file nella cartella
	QFileInfoList files = directory.entryInfoList(QDir::Files);
	Q_FOREACH(QFileInfo file, files) {
		// Se non e un file o e il file compresso che sto creando
		if (!file.isFile() || file.absoluteFilePath() == zip->getZipName()) continue;
		// Creo il nome relativo da usare all'interno del file compresso
		QString filename = origDirectory.relativeFilePath(file.absoluteFilePath());
		// Comprimo il file
		if (!compressFile(zip, file.absoluteFilePath(), filename)) return false;
		else {
			compressProgress(filename);
		}
	}
	return true;
}

/**OK
 * Estrae il file fileName, contenuto nell'oggetto zip, con il nome fileDest.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato di estrarre.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip e stato aperto in una modalita non compatibile con l'estrazione di file;
 * * non e possibile aprire il file all'interno dell'oggetto zip;
 * * non e possibile creare il file estratto;
 * * si e rilevato un errore nella copia dei dati (1);
 * * non e stato possibile chiudere il file all'interno dell'oggetto zip (1);
 *
 * (1): prima di uscire dalla funzione cancella il file estratto.
 */
bool CCompressThread::extractFile(QuaZip* zip, QString fileName, QString fileDest) {
	// zip: oggetto dove aggiungere il file
	// filename: nome del file reale
	// fileincompress: nome del file all'interno del file compresso
	// Controllo l'apertura dello zip

	/*QString strMD5 = QCryptographicHash::hash(QByteArray("123456"), QCryptographicHash::Md5).toHex();*/
	if (!zip) return false;
	if (zip->getMode() != QuaZip::mdUnzip) return false;

	// Apro il file compresso
	if (!fileName.isEmpty())
		zip->setCurrentFile(fileName);
	QuaZipFile inFile(zip);
	if (!inFile.open(QIODevice::ReadOnly) || inFile.getZipError() != UNZ_OK) return false;

	// Controllo esistenza cartella file risultato
	QDir curDir;
	if (fileDest.endsWith('/')) {
		if (!curDir.mkpath(fileDest)) {
			return false;
		}
	}
	else {
		if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
			return false;
		}
	}

	QuaZipFileInfo64 info;
	if (!zip->getCurrentFileInfo(&info))
		return false;

	QFile::Permissions srcPerm = info.getPermissions();
	if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
		if (srcPerm != 0) {
			QFile(fileDest).setPermissions(srcPerm);
		}
		return true;
	}

	// Apro il file risultato
	QFile outFile;
	outFile.setFileName(fileDest);
	if (!outFile.open(QIODevice::WriteOnly)) return false;
	//QFile::copy(inFile, outFile);
	copyData(inFile, outFile);
	//Copio i dati
	if (!copyData(inFile, outFile) || inFile.getZipError() != UNZ_OK) {
		outFile.close();
		removeFile(QStringList(fileDest));
		return false;
	}
	if (srcPerm != 0) {
		outFile.setPermissions(srcPerm);
	}
	if (inFile.getZipError() != UNZ_OK) {
		removeFile(QStringList(fileDest));
		return false;
	}
	outFile.close();
	inFile.close();
	return true;
}
/**
 * Rimuove i file il cui nome e specificato all'interno di listFile.
 * Restituisce true se tutti i file sono stati cancellati correttamente, attenzione
 * perche puo restituire false anche se alcuni file non esistevano e si e tentato
 * di cancellarli.
 */
bool CCompressThread::removeFile(QStringList listFile) {
	bool ret = true;
	// Per ogni file
	for (int i = 0; i < listFile.count(); i++) {
		// Lo elimino
		ret = ret && QFile::remove(listFile.at(i));
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Comprime il file fileName nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool CCompressThread::compressFile(QString fileCompressed, QString file) {
	// Creo lo zip
	QuaZip zip(fileCompressed);
	QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
	if (!zip.open(QuaZip::mdCreate)) {
		QFile::remove(fileCompressed);
		return false;
	}
	// Aggiungo il file
	if (!compressFile(&zip, file, QFileInfo(file).fileName())) {
		QFile::remove(fileCompressed);
		return false;
	}
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		QFile::remove(fileCompressed);
		return false;
	}
	return true;
}
/**OK
 * Comprime i file specificati in files nel file fileCompressed.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool CCompressThread::compressFiles(QString fileCompressed, QStringList files) {
	// Creo lo zip
	QuaZip zip(fileCompressed);
	QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
	if (!zip.open(QuaZip::mdCreate)) {
		QFile::remove(fileCompressed);
		return false;
	}
	// Comprimo i file
	QFileInfo info;
	Q_FOREACH(QString file, files) {
		info.setFile(file);
		if (!info.exists() || !compressFile(&zip, file, info.fileName())) {
			QFile::remove(fileCompressed);
			return false;
		}
	}
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		QFile::remove(fileCompressed);
		return false;
	}
	return true;
}
/**OK
 * Comprime la cartella dir nel file fileCompressed, se recursive e true allora
 * comprime anche le sotto cartelle.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato
 * di creare.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool CCompressThread::compressDir(QString fileCompressed, QString dir, bool recursive) {
	// Creo lo zip
	QuaZip zip(fileCompressed);
	QDir().mkpath(QFileInfo(fileCompressed).absolutePath());
	if (!zip.open(QuaZip::mdCreate)) {
		QFile::remove(fileCompressed);
		return false;
	}
	getCompressFileCount(fileCompressed, dir, m_iAllFileCount);
	// Aggiungo i file e le sotto cartelle
	if (!compressSubDir(&zip, dir, dir, recursive)) {
		QFile::remove(fileCompressed);
		return false;
	}
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		QFile::remove(fileCompressed);
		return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**OK
 * Estrae il file fileName, contenuto nel file fileCompressed, con il nome fileDest.
 * Se fileDest = "" allora il file viene estratto con lo stesso nome con cui e
 * stato compresso.
 * Se la funzione fallisce cancella il file che si e tentato di estrarre.
 * Restituisce il nome assoluto del file estratto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione del file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QString CCompressThread::extractFile(QString fileCompressed, QString fileName, QString fileDest) {
	// Apro lo zip
	QuaZip zip(fileCompressed);
	if (!zip.open(QuaZip::mdUnzip)) {
		return QString();
	}
	// Estraggo il file
	if (fileDest.isEmpty())
		fileDest = fileName;
	if (!extractFile(&zip, fileName, fileDest)) {
		return QString();
	}
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		removeFile(QStringList(fileDest));
		return QString();
	}
	return QFileInfo(fileDest).absoluteFilePath();
}
/**OK
 * Estrae i file specificati in files, contenuti nel file fileCompressed, nella
 * cartella dir. La struttura a cartelle del file compresso viene rispettata.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si e tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * l'estrazione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList CCompressThread::extractFiles(QString fileCompressed, QStringList files, QString dir) {
	// Creo lo zip
	QuaZip zip(fileCompressed);
	if (!zip.open(QuaZip::mdUnzip)) {
		return QStringList();
	}
	// Estraggo i file
	QStringList extracted;
	for (int i = 0; i < files.count(); i++) {
		QString absPath = QDir(dir).absoluteFilePath(files.at(i));
		if (!extractFile(&zip, files.at(i), absPath)) {
			removeFile(extracted);
			return QStringList();
		}
		extracted.append(absPath);
	}
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		removeFile(extracted);
		return QStringList();
	}
	return extracted;
}
/**OK
 * Estrae il file fileCompressed nella cartella dir.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si e tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList CCompressThread::extractDir(QString fileCompressed, QString dir) {
	qDebug() << "extractDir:" << fileCompressed << dir;
	QuaZip zip(fileCompressed);
	if (!zip.open(QuaZip::mdUnzip)) {
		qDebug() << "extractDir open zip error";
		return QStringList();
	}
	QDir directory(dir);
	QStringList extracted;
	if (!zip.goToFirstFile()) {
		return QStringList();
	}
	do {
		m_iAllFileCount++;
	} while (zip.goToNextFile());
	qDebug() << "extractDir:" << m_iAllFileCount;
	zip.goToFirstFile();
	do {
		QString name = zip.getCurrentFileName();
		QString absFilePath = directory.absoluteFilePath(name);
		if (!extractFile(&zip, "", absFilePath)) {
			removeFile(extracted);
			return QStringList();
		}
		else {
			compressProgress(absFilePath);
		}
	/*	if (absFilePath.contains(".json"))
		{
			jsonlist.append(absFilePath);
		}*/
		extracted.append(absFilePath);
	} while (zip.goToNextFile());
	// Chiudo il file zip
	zip.close();
	if (zip.getZipError() != 0) {
		removeFile(extracted);
		return QStringList();
	}
	return extracted;
}
/**OK
 * Restituisce la lista dei file resenti nel file compresso fileCompressed.
 * Se la funzione fallisce, restituisce un elenco vuoto.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la richiesta di informazioni di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
QStringList CCompressThread::getFileList(QString fileCompressed) {
	// Apro lo zip
	QuaZip* zip = new QuaZip(QFileInfo(fileCompressed).absoluteFilePath());
	if (!zip->open(QuaZip::mdUnzip)) {
		delete zip;
		return QStringList();
	}
	// Estraggo i nomi dei file
	QStringList lst;
	QuaZipFileInfo64 info;
	for (bool more = zip->goToFirstFile(); more; more = zip->goToNextFile()) {
		if (!zip->getCurrentFileInfo(&info)) {
			delete zip;
			return QStringList();
		}
		lst << info.name;
		//info.name.toLocal8Bit().constData()
	}
	// Chiudo il file zip
	zip->close();
	if (zip->getZipError() != 0) {
		delete zip;
		return QStringList();
	}
	delete zip;
	return lst;
}
void CCompressThread::run()
{
	if (m_Type == compress_Dir)
	{
		compressDir(m_pFileCompressed, m_pSelectPath);
	}
	if (m_Type == extract_Dir)
	{
		extractDir(m_pFileCompressed, m_pSelectPath);
	}
}
END_NX_NAMESPACE