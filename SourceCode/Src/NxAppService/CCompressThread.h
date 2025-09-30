/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_COMPRESS_THREAD_H
#define __C_COMPRESS_THREAD_H
#include "nxappservice_global.h"

#include <QThread>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFile>

class QuaZip;
BEGIN_NX_NAMESPACE
class CCompressThread  : public QThread
{
    Q_OBJECT

public:
    enum CompressType
    {
        compress_file,
        compress_Dir,
        extract_Dir,
    };

    CCompressThread(CompressType _type, QString _fileCompressed, QString _selectPath, bool _encryption,QObject *parent);

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

signals:
    void signalCompressProgress(QString fileName, int currentIndex, int allIndex);
    void signalExtractProgress(QString fileName, int currentIndex, int allIndex);

protected:
    void run() override;

private:
    QStringList jsonlist;
    int m_iCompressFileCount = 0;
    int m_iAllFileCount = 0;
    CompressType m_Type;
    QString m_pFileCompressed;
    QString m_pSelectPath;
    bool m_bEncryption;
};

END_NX_NAMESPACE
#endif
