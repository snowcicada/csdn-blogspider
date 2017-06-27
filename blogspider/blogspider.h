#ifndef BLOGSPIDER_H
#define BLOGSPIDER_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include <phonon>

#include "dealthread.h"
#include "common.h"

namespace Ui {
class BlogSpider;
}

class BlogSpider : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit BlogSpider(QWidget *parent = 0);
    ~BlogSpider();

signals:
    void startAnalyseBlogRank();
    void startDownloadBlogImage();
    void startDownloadAllImage();
    void startDownloadAllPage();
    void startAnalyseAllPage();
    void startDownloadAllBlog();
    void downloadPageDone();
    void downloadBlogDone();

private slots:
    void slotIconActivated(QSystemTrayIcon::ActivationReason);
    void slotUserTextChanged(const QString &);
    void slotGetBlogInfo();
    void slotDownloadBlog();
    void slotPauseOrContinue();
    void slotFinish();
    void slotScanDir();
    void slotClearSetting();
    void slotSelectAll();
    void slotUnselectAll();
    void slotClearSelect();
    void slotActImportUser();
    void slotActQuit();
    void slotActOpenPerBlog();
    void slotActAboutQt();
    void slotActAboutBlogSpider();
    void slotOpenMyBlog(const QString &);
    void slotShowBloginfo();
    void slotOpenBlogUrl(const QModelIndex &index);

    //下载博客第一个页面
    void slotIndexReadyWrite();
    void slotIndexFinish();

    //下载博客所有页面
    void slotDownloadAllPage();
    void slotInfoReadyWrite();
    void slotInfoFinish();
    void slotDownloadPageDone();
    //下载所有选中的博客
    void slotDownloadAllBlog();
    void slotBlogReadyRead();
    void slotBlogFinish();
    void slotDownloadBlogDone();
    void slotCreatePdfDone();
    //下载博客中所有图片
    void slotDownloadAllImage();
    void slotBlogImageReadyWrite();
    void slotBlogImageFinish();

    //下载头像
    void slotDownloadBlogHeadImage();
    void slotBlogHeadImageReadyWrite();
    void slotBlogHeadImageFinish();

    //接收线程的信号
    void slotRecvBlogRank(const QString &title, const QString &description,
                          const QString &imageUrl, const QString &imageName,
                          const QString &total, const QString &pageTotal,
                          const QString &view, const QString &score,
                          const QString &ranking, const QString &original,
                          const QString &reship, const QString &translation,
                          const QString &comment);
    void slotRecvBlogInfo(const QString &title);
    void slotDealThreadError(const QString &);

    //更新数据库
    void slotUpdateSqlDbProgress();
    void slotUpdateSqlDbDone();

    //音乐
    void slotPlayMusic();
    void slotStopMusic();
    void slotRepeatPlay();

private:
    void closeEvent(QCloseEvent *);
    void initSetupUi();
    void initSystemTrayIcon();
    void initWidgetStatusTip();
    void initDownloadSignalSlot();
    void initBackGroundMusic();
    void setBlogImage(const QString &);
    bool isValidCsdnId(const QString &);
    //http请求
    void indexStartRequest(const QUrl &);
    void headImageStartRequest(const QUrl &);
    void infoStartRequest(const QUrl &);
    void blogStartRequest(const QUrl &);
    void blogImageStartRequest(const QUrl &);
    void setBlogInfoOnLineEdit();
    //数据库
    bool initBlogSqlDb();
    void insertBlogInfoToSql();
    void deleteBlogSqlDb();
    //判断当前主线程是否在忙
    bool isMainThreadBusy();
    MainThreadOp getMainThreadOp();
    CtrlMainThread getMainThreadCtrl();
    void setMainThreadOp(MainThreadOp op);
    void setMainThreadCtrl(CtrlMainThread ctrl);

    Ui::BlogSpider *ui;

    QUrl m_blogUrl;
    QUrl m_blogImageUrl;
    QString m_csdnUserId;
    QString m_blogSaveDir;
    QString m_blogTitle;
    QString m_blogDescription;
    int m_iBlogTotal;
    int m_iBlogPageTotal;
    int m_iBlogView;
    int m_iBlogScore;
    int m_iBlogRanking;
    int m_iBlogOriginal;
    int m_iBlogReship;
    int m_iBlogTranslation;
    int m_iBlogComment;
    int m_iBlogPageTotalCount;//用于保存变化的页数
    int m_iBlogDealCount;//记录博客分析到第几篇
    int m_iBlogDownloadCount;//记录博客下载到第几篇
    DownloadStateMap m_blogSelMap;//保存被选中的文章,要下载
    BlogImageInfoVector m_blogimageVector;//保存每个博客所需的图片信息
    QString m_curDownloadBlogName;

    MainThreadOp m_mainThreadCurrentOp;//主线程当前正在执行的操作
    CtrlMainThread m_ctrlMainThreadProgress;//主线程的控制:暂停,停止

    //数据库
    QSqlDatabase *m_pBlogSqlDb;

    //thread
    DealThread m_dealThread;

    QSystemTrayIcon *m_pTrayIcon;
    QMenu *m_pTrayIconMenu;
    QAction *m_pMaximizeAction;
    QAction *m_pMinimizeAction;
    QAction *m_pHideAction;
    QAction *m_pRestoreAction;
    QAction *m_pQuitAction;

    //弹出dialog显示博客的详细信息
    QDialog *m_pBlogInfoDialog;
    QSqlTableModel *m_pBlogInfoSqlModel;
    QTableView *m_pBlogInfoView;

    QLabel *m_pBlogUrlLabel;

    QNetworkAccessManager m_httpManager;
    QNetworkReply *m_pHttpReply;
    QFile *m_pDownloadFile;

    //static
    static QString m_indexName;
    static QString m_blogFileName;
    static QString m_blogPdfDir;
    static QString m_blogImageDir;
    static QString m_blogSqlName;
    static QString m_blogImageName;
    static QString m_spiderVersion;

    //音乐
    Phonon::MediaObject *m_pMediaObject;
    Phonon::AudioOutput *m_pAudioOutput;
};

#endif // BLOGSPIDER_H
