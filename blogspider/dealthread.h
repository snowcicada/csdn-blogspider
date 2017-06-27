#ifndef DEALTHREAD_H
#define DEALTHREAD_H

#include <QtCore>
#include <QtNetwork>
#include <QtSql>
#include <QThread>
#include <QPrinter>
#include <QTextDocument>

#include "common.h"

class DealThread : public QThread
{
    Q_OBJECT

public:
    DealThread();
    ~DealThread();
    void stop();

    void setCtrlThreadOp(CtrlThread op);
    void setCsdnUserId(const QString &id);
    void setBlogSaveDir(const QString &dir);
    void setBlogImageDir(const QString &dir);
    void setBlogPdfDir(const QString &dir);
    void setAnalyseFile(const QString &file);
    void setBlogSqlDatabase(QSqlDatabase *db);
    void setDownloadStateMap(DownloadStateMap &map);
    void setBlogImageInfoVector(BlogImageInfoVector *vec);
    void setBlogToPdfName(const QString &name);

protected:
    void run();

signals:
    void startDownloadAllPage();
    void startDownloadAllImage();
    void startDownloadAllBlog();

    void sendBlogRank(const QString &title, const QString &description,
                      const QString &imageUrl, const QString &imageName,
                      const QString &total, const QString &pageTotal,
                      const QString &view, const QString &score,
                      const QString &ranking, const QString &original,
                      const QString &reship, const QString &translation,
                      const QString &comment);
    void sendBlogInfo(const QString &title);
    void dealThreadError(const QString &);
    void updateSqlDbProgress();
    void updateSqlDbDone();
    void createPdfDone();

private:
    void analyseBlogRank();
    void analyseAllPage();
    void analyseBlog();
    void createPdf();
    void updateSqlDb();
    void restoreImageFormat(const QString &file);
    bool getImageFormat(const QString &file, QString &output);

    QSqlDatabase *m_pBlogSqlDatabase;
    QString m_csdnUserId;
    QString m_blogSaveDir;
    QString m_blogImageDir;
    QString m_blogPdfDir;
    QString m_analyseFile;
    QString m_blogPdfName;
    CtrlThread m_ctrlThreadOp;
    DownloadStateMap m_downloadBlogMap;
    BlogImageInfoVector *m_blogimageVector;

    static QString m_destBlogFile;

    volatile bool m_stopped;
};

#endif // DEALTHREAD_H
