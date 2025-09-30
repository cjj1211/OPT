#ifndef ZKCOMPRESSFOLDER_H_
#define ZKCOMPRESSFOLDER_H_
#include <QThread>
#include "quazip.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFile>
class zkCompressThread : public QThread
{
    Q_OBJECT
signals:
    void sig_compressProgress(QString fileName ,int currentIndex, int allIndex);
    void sig_extractProgress(QString fileName ,int currentIndex, int allIndex);
public:
    enum zkCompressType
    {
        compress_file,
        compress_Dir,
        extract_Dir,
    };
    QStringList jsonlist;
    zkCompressThread(zkCompressType type,QString fileCompressed, QString selectPath);
    void compressProgress(QString fileName);
    bool compressFile(QString fileCompressed, QString file);
    bool compressFiles(QString fileCompressed, QStringList files);
    bool compressDir(QString fileCompressed = QString(), QString dir = QString(), bool recursive = true);
    QString extractFile(QString fileCompressed, QString fileName, QString fileDest);
    QStringList extractFiles(QString fileCompressed, QStringList files, QString dir);
    QStringList extractDir(QString fileCompressed, QString dir);
    QStringList getFileList(QString fileCompressed);
    bool compressFile(QuaZip* zip, QString fileName, QString fileDest);
    bool compressSubDir(QuaZip* parentZip, QString dir, QString parentDir, bool recursive = true);
    bool extractFile(QuaZip* zip, QString fileName, QString fileDest);
    bool removeFile(QStringList listFile);
protected:
    void run();

private:
    int m_iCompressFileCount = 0;
    int m_iAllFileCount = 0;
    zkCompressType m_iType;
    QString m_pFileCompressed;
    QString m_pSelectPath;
};
#endif /* zkCompressFOLDER_H_ */
