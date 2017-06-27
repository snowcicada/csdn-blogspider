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

    //���ز��͵�һ��ҳ��
    void slotIndexReadyWrite();
    void slotIndexFinish();

    //���ز�������ҳ��
    void slotDownloadAllPage();
    void slotInfoReadyWrite();
    void slotInfoFinish();
    void slotDownloadPageDone();
    //��������ѡ�еĲ���
    void slotDownloadAllBlog();
    void slotBlogReadyRead();
    void slotBlogFinish();
    void slotDownloadBlogDone();
    void slotCreatePdfDone();
    //���ز���������ͼƬ
    void slotDownloadAllImage();
    void slotBlogImageReadyWrite();
    void slotBlogImageFinish();

    //����ͷ��
    void slotDownloadBlogHeadImage();
    void slotBlogHeadImageReadyWrite();
    void slotBlogHeadImageFinish();

    //�����̵߳��ź�
    void slotRecvBlogRank(const QString &title, const QString &description,
                          const QString &imageUrl, const QString &imageName,
                          const QString &total, const QString &pageTotal,
                          const QString &view, const QString &score,
                          const QString &ranking, const QString &original,
                          const QString &reship, const QString &translation,
                          const QString &comment);
    void slotRecvBlogInfo(const QString &title);
    void slotDealThreadError(const QString &);

    //�������ݿ�
    void slotUpdateSqlDbProgress();
    void slotUpdateSqlDbDone();

    //����
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
    //http����
    void indexStartRequest(const QUrl &);
    void headImageStartRequest(const QUrl &);
    void infoStartRequest(const QUrl &);
    void blogStartRequest(const QUrl &);
    void blogImageStartRequest(const QUrl &);
    void setBlogInfoOnLineEdit();
    //���ݿ�
    bool initBlogSqlDb();
    void insertBlogInfoToSql();
    void deleteBlogSqlDb();
    //�жϵ�ǰ���߳��Ƿ���æ
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
    int m_iBlogPageTotalCount;//���ڱ���仯��ҳ��
    int m_iBlogDealCount;//��¼���ͷ������ڼ�ƪ
    int m_iBlogDownloadCount;//��¼�������ص��ڼ�ƪ
    DownloadStateMap m_blogSelMap;//���汻ѡ�е�����,Ҫ����
    BlogImageInfoVector m_blogimageVector;//����ÿ�����������ͼƬ��Ϣ
    QString m_curDownloadBlogName;

    MainThreadOp m_mainThreadCurrentOp;//���̵߳�ǰ����ִ�еĲ���
    CtrlMainThread m_ctrlMainThreadProgress;//���̵߳Ŀ���:��ͣ,ֹͣ

    //���ݿ�
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

    //����dialog��ʾ���͵���ϸ��Ϣ
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

    //����
    Phonon::MediaObject *m_pMediaObject;
    Phonon::AudioOutput *m_pAudioOutput;
};

#endif // BLOGSPIDER_H
