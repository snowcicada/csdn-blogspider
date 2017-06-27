#include "blogspider.h"
#include "ui_blogspider.h"

QString BlogSpider::m_indexName = "index.html";
QString BlogSpider::m_blogFileName = "_srcblog.html";
QString BlogSpider::m_blogPdfDir = "blog";
QString BlogSpider::m_blogImageDir = "image";
QString BlogSpider::m_blogSqlName = "blogspider.db";
QString BlogSpider::m_blogImageName = "blogimage.jpg";
QString BlogSpider::m_spiderVersion = "2.0";

BlogSpider::BlogSpider(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BlogSpider)
{
    ui->setupUi(this);

    //Lucky
    //��ʼ������
    initSetupUi();
    //��ʼ��ϵͳͼ��
    initSystemTrayIcon();
    //��ʼ���ؼ�����ʾ��Ϣ
    initWidgetStatusTip();
    //��ʼ�������źŲ�����
    initDownloadSignalSlot();
    //������������
    initBackGroundMusic();
}

BlogSpider::~BlogSpider()
{
    if (m_dealThread.isRunning())
    {
        m_dealThread.stop();
    }

    //�ͷ����ݿ�ָ��
    deleteBlogSqlDb();

    //ɾ�����ֶ���
    if (m_pMediaObject)
    {
        delete m_pMediaObject;
    }
    if (m_pAudioOutput)
    {
        delete m_pAudioOutput;
    }

    //�������б���ڴ��ͷų���
    ui->tableWidgetBlog->clearContents();

    m_blogSelMap.clear();
    //free memory
    for (BlogImageInfoVector::Iterator it = m_blogimageVector.begin();
         it != m_blogimageVector.end();
         ++it)
    {
        free(*it);
    }
    m_blogimageVector.clear();

    delete m_pMaximizeAction;
    delete m_pMinimizeAction;
    delete m_pHideAction;
    delete m_pRestoreAction;
    delete m_pQuitAction;
    delete m_pTrayIconMenu;
    delete m_pTrayIcon;
    delete m_pBlogUrlLabel;
    delete ui;
}

void BlogSpider::closeEvent(QCloseEvent *event)
{
    hide();
    m_pTrayIcon->showMessage(this->windowTitle(), tr("���,��������! �������Ҽ�ͼ���˳����."), QSystemTrayIcon::Information);
    //Ҫ���Ե����ڵĹر��¼�
    event->ignore();
}

void BlogSpider::initSetupUi()
{
    //������ʼ��
    m_blogUrl.clear();
    m_blogImageUrl.clear();
    m_csdnUserId.clear();
    m_blogSaveDir.clear();
    m_blogTitle.clear();
    m_blogDescription.clear();
    m_iBlogTotal = 0;
    m_iBlogPageTotal = 0;
    m_iBlogView = 0;
    m_iBlogScore = 0;
    m_iBlogRanking = 0;
    m_iBlogOriginal = 0;
    m_iBlogReship = 0;
    m_iBlogTranslation = 0;
    m_iBlogComment = 0;
    m_iBlogPageTotalCount = 0;
    m_iBlogDealCount = 0;
    m_iBlogDownloadCount = 0;
    m_curDownloadBlogName.clear();

    setMainThreadOp(MainThreadOp_Init);
    setMainThreadCtrl(CtrlMainThread_Init);

    //ָ���ʼ��
    m_pBlogSqlDb = 0;
    m_pHttpReply = 0;
    m_pDownloadFile = 0;

    m_pBlogInfoDialog = 0;
    m_pBlogInfoSqlModel = 0;
    m_pBlogInfoView = 0;

    m_pMediaObject = 0;
    m_pAudioOutput = 0;

    //���ô���ͼ��ͱ���
    this->setWindowIcon(QIcon(QString(":/images/icon.png")));
    this->setWindowTitle(QString("CSDN����������v%1-(����:gzshun)").arg(m_spiderVersion));

    //�̶����ڴ�С
    this->setFixedSize(500, 620);

    //������󻯰�ť��ʹ��
    this->setWindowFlags(Qt::WindowMinimizeButtonHint);

    //�˵���
    connect(ui->actImportUser, SIGNAL(triggered()), this, SLOT(slotActImportUser()));

    ui->actOpenBlog->setEnabled(false);
    ui->actShowBlogInfo->setEnabled(false);
    connect(ui->actOpenBlog, SIGNAL(triggered()), this, SLOT(slotActOpenPerBlog()));

    ui->actQuit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(slotActQuit()));

    connect(ui->actAboutQt, SIGNAL(triggered()), this, SLOT(slotActAboutQt()));
    connect(ui->actBlogSpider, SIGNAL(triggered()), this, SLOT(slotActAboutBlogSpider()));
    connect(ui->actShowBlogInfo, SIGNAL(triggered()), this, SLOT(slotShowBloginfo()));

    connect(ui->actPlayMusic, SIGNAL(triggered()), this, SLOT(slotPlayMusic()));
    connect(ui->actStopMusic, SIGNAL(triggered()), this, SLOT(slotStopMusic()));

    //�����
    ui->lineEditUser->setMaxLength(20);
    ui->lineEditUser->setPlaceholderText(tr("�������û���"));//Ĭ��ֵ:�������ʧ��
    connect(ui->lineEditUser, SIGNAL(textChanged(QString)), this, SLOT(slotUserTextChanged(QString)));
    connect(ui->lineEditUser, SIGNAL(returnPressed()), this, SLOT(slotGetBlogInfo()));//�س��ź�

    ui->lineEditUrl->setReadOnly(true);
    ui->lineEditDir->setReadOnly(true);

    //��ť
    ui->btnGetBlogInfo->setEnabled(false);
    connect(ui->btnGetBlogInfo, SIGNAL(clicked()), this, SLOT(slotGetBlogInfo()));

    ui->btnDownloadBlog->setEnabled(false);
    connect(ui->btnDownloadBlog, SIGNAL(clicked()), this, SLOT(slotDownloadBlog()));

    ui->btnPauseOrContinue->setEnabled(false);
    connect(ui->btnPauseOrContinue, SIGNAL(clicked()), this, SLOT(slotPauseOrContinue()));

    ui->btnFinish->setEnabled(false);
    connect(ui->btnFinish, SIGNAL(clicked()), this, SLOT(slotFinish()));

    connect(ui->btnScanDir, SIGNAL(clicked()), this, SLOT(slotScanDir()));
    connect(ui->btnClearSetting, SIGNAL(clicked()), this, SLOT(slotClearSetting()));
    connect(ui->btnSelectAll, SIGNAL(clicked()), this, SLOT(slotSelectAll()));
    connect(ui->btnUnselectAll, SIGNAL(clicked()), this, SLOT(slotUnselectAll()));
    connect(ui->btnClearSelect, SIGNAL(clicked()), this, SLOT(slotClearSelect()));

    //csdnĬ��ͷ��
    setBlogImage(":/images/default.jpg");

    //������Ϣ��
    ui->lineEditBlogTitle->setReadOnly(true);
    ui->lineEditBlogDescription->setReadOnly(true);
    ui->lineEditView->setReadOnly(true);
    ui->lineEditScore->setReadOnly(true);
    ui->lineEditRanking->setReadOnly(true);
    ui->lineEditBlogTotal->setReadOnly(true);
    ui->lineEditOriginal->setReadOnly(true);
    ui->lineEditReship->setReadOnly(true);
    ui->lineEditTranslation->setReadOnly(true);
    ui->lineEditComments->setReadOnly(true);

    //��������ǩ
    ui->labelPgsBarAnalyse->setAlignment(Qt::AlignCenter);
    ui->labelPgsBarDownload->setAlignment(Qt::AlignCenter);

    //״̬��
    m_pBlogUrlLabel = new QLabel(QString("<a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun </a>"), this);
    ui->statusBar->addPermanentWidget(m_pBlogUrlLabel);
    connect(m_pBlogUrlLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotOpenMyBlog(QString)));
    ui->statusBar->setSizeGripEnabled(false);//ȥ��״̬���Ĵ�С���Ƶ�

    //����tableWidgetBlog�ؼ�������
//    ui->tableWidgetBlog->setFont(QFont(tr("����"), 10, QFont::Normal));
    ui->tableWidgetBlog->setColumnCount(1);//ֻ��һ��
    ui->tableWidgetBlog->horizontalHeader()->setHidden(true);//���ص�һ�еı�ǩ
    ui->tableWidgetBlog->verticalHeader()->setShown(true);//��ʾ��һ�е����
    //�̶�ס�ؼ�,�����Ͽ�
    ui->tableWidgetBlog->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidgetBlog->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidgetBlog->setEditTriggers(QAbstractItemView::NoEditTriggers);//����Ϊֻ��
    //����Ӧ����,bug:ÿ����һ�����У�����������Ӧ����
//    ui->tableWidgetBlog->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidgetBlog->setColumnWidth(0, 355);//���ù̶����
    //ֻ��ѡһ��
    ui->tableWidgetBlog->setSelectionMode(QAbstractItemView::SingleSelection);
    //��ѡ��ʱ,ֱ��ѡ��һ����
    ui->tableWidgetBlog->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void BlogSpider::initSystemTrayIcon()
{
    //������Ϊ
    //���
    m_pMaximizeAction = new QAction(tr("���"), this);
    connect(m_pMaximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
    //��С��
    m_pMinimizeAction = new QAction(tr("��С��"), this);
    connect(m_pMinimizeAction, SIGNAL(triggered()), this, SLOT(showMinimized()));
    //����
    m_pHideAction = new QAction(tr("����"), this);
    connect(m_pHideAction, SIGNAL(triggered()), this, SLOT(hide()));
    //��ԭ
    m_pRestoreAction = new QAction(tr("��ԭ"), this);
    connect(m_pRestoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    //�˳�
    m_pQuitAction = new QAction(tr("�˳�"), this);
    connect(m_pQuitAction, SIGNAL(triggered()), this, SLOT(slotActQuit()));

    //���Ӳ˵�
    m_pTrayIconMenu = new QMenu(this);
    m_pTrayIconMenu->addAction(m_pHideAction);
    m_pTrayIconMenu->addAction(m_pRestoreAction);
    m_pTrayIconMenu->addAction(m_pMinimizeAction);
    m_pTrayIconMenu->addAction(m_pMaximizeAction);
    m_pTrayIconMenu->addSeparator();
    m_pTrayIconMenu->addAction(m_pQuitAction);

    //����ϵͳ������ͼ��
    m_pTrayIcon = new QSystemTrayIcon(this);
    m_pTrayIcon->setIcon(QIcon(":/images/icon.ico"));
    m_pTrayIcon->setContextMenu(m_pTrayIconMenu);
    m_pTrayIcon->setToolTip(this->windowTitle());
    m_pTrayIcon->show();

    connect(m_pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));
}

void BlogSpider::initWidgetStatusTip()
{
    ui->actImportUser->setStatusTip(tr("�����Ѵ��ڵ��û�"));
    ui->actOpenBlog->setStatusTip(tr("���ҵĲ���"));
    ui->actShowBlogInfo->setStatusTip(tr("��ʾ������ϸ��Ϣ"));
    ui->actPlayMusic->setStatusTip(tr("��������"));
    ui->actStopMusic->setStatusTip(tr("ֹͣ��������"));
    ui->actQuit->setStatusTip(tr("�˳�"));
    ui->actAboutQt->setStatusTip(tr("����Qt"));
    ui->actBlogSpider->setStatusTip(tr("����BlogSpider"));

    ui->lineEditUser->setStatusTip(tr("�û���"));
    ui->lineEditUser->setToolTip(tr("�û���"));
    ui->lineEditUrl->setStatusTip(tr("������ַ"));
    ui->lineEditUrl->setToolTip(tr("������ַ"));
    ui->lineEditDir->setStatusTip(tr("��������Ŀ¼"));
    ui->lineEditDir->setToolTip(tr("��������Ŀ¼"));

    ui->btnGetBlogInfo->setStatusTip(tr("��ȡ�����б���Ϣ"));
    ui->btnGetBlogInfo->setToolTip(tr("��ȡ�����б���Ϣ"));
    ui->btnDownloadBlog->setStatusTip(tr("���ز���"));
    ui->btnDownloadBlog->setToolTip(tr("���ز���"));
    ui->btnPauseOrContinue->setStatusTip(tr("��ͣ���������"));
    ui->btnPauseOrContinue->setToolTip(tr("��ͣ���������"));
    ui->btnFinish->setStatusTip(tr("ֹͣ����"));
    ui->btnFinish->setToolTip(tr("ֹͣ����"));
    ui->btnScanDir->setStatusTip(tr("���ѡ��Ŀ¼"));
    ui->btnScanDir->setToolTip(tr("���ѡ��Ŀ¼"));
    ui->btnClearSetting->setStatusTip(tr("������������"));
    ui->btnClearSetting->setToolTip(tr("������������"));

    ui->labelBlogImage->setStatusTip(tr("�û�ͷ��"));
    ui->labelBlogImage->setToolTip(tr("�û�ͷ�� "));

    ui->tableWidgetBlog->setStatusTip(tr("�����б�"));
    ui->tableWidgetBlog->setToolTip(tr("�����б�"));

    ui->btnSelectAll->setStatusTip(tr("ȫѡ����"));
    ui->btnSelectAll->setToolTip(tr("ȫѡ����"));
    ui->btnUnselectAll->setStatusTip(tr("��ѡ����"));
    ui->btnUnselectAll->setToolTip(tr("��ѡ����"));
    ui->btnClearSelect->setStatusTip(tr("����ѡ��"));
    ui->btnClearSelect->setToolTip(tr("����ѡ��"));

    m_pBlogUrlLabel->setStatusTip(tr("�ҵĲ���"));
    m_pBlogUrlLabel->setToolTip(tr("�ҵĲ���"));
}

void BlogSpider::initDownloadSignalSlot()
{
    connect(&m_dealThread, SIGNAL(updateSqlDbProgress()), this, SLOT(slotUpdateSqlDbProgress()));
    connect(&m_dealThread, SIGNAL(updateSqlDbDone()), this, SLOT(slotUpdateSqlDbDone()));
    //���ز����û�ͷ��
    connect(this, SIGNAL(startDownloadBlogImage()), this, SLOT(slotDownloadBlogHeadImage()));
    //��ʼ����ÿ��ҳ��
    connect(this, SIGNAL(startDownloadAllPage()), this, SLOT(slotDownloadAllPage()));
    connect(&m_dealThread, SIGNAL(startDownloadAllPage()), this, SLOT(slotDownloadAllPage()));
    //��ʼ����ÿƪ����
    connect(this, SIGNAL(startDownloadAllBlog()), this, SLOT(slotDownloadAllBlog()));
    //������ҳ�淢���ź�֪ͨ
    connect(this, SIGNAL(downloadPageDone()), this, SLOT(slotDownloadPageDone()));
    //�����격��
    connect(this, SIGNAL(downloadBlogDone()), this, SLOT(slotDownloadBlogDone()));

    connect(&m_dealThread, SIGNAL(sendBlogRank(QString,QString,QString,QString,
                                             QString,QString,QString,QString,
                                             QString,QString,QString,QString,
                                             QString)), this,
            SLOT(slotRecvBlogRank(QString,QString,QString,QString,QString,
                                  QString,QString,QString,QString,QString,
                                  QString,QString,QString)));

    connect(&m_dealThread, SIGNAL(sendBlogInfo(QString)), this, SLOT(slotRecvBlogInfo(QString)));

    connect(&m_dealThread, SIGNAL(dealThreadError(QString)), this, SLOT(slotDealThreadError(QString)));


    //��������pdf���źŲ�
    connect(&m_dealThread, SIGNAL(startDownloadAllImage()), this, SLOT(slotDownloadAllImage()));
    connect(this, SIGNAL(startDownloadAllImage()), this, SLOT(slotDownloadAllImage()));
    connect(&m_dealThread, SIGNAL(createPdfDone()), this, SLOT(slotCreatePdfDone()));
    connect(&m_dealThread, SIGNAL(startDownloadAllBlog()), this, SLOT(slotDownloadAllBlog()));
}

void BlogSpider::initBackGroundMusic()
{
    QString file("default.mp3");
    if (!QFile::exists(file))
    {
        return;
    }
    m_pMediaObject = new Phonon::MediaObject(this);
    m_pAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

    Phonon::MediaSource music(file);

    Phonon::createPath(m_pMediaObject, m_pAudioOutput);

    m_pMediaObject->setCurrentSource(music);

    m_pMediaObject->play();

    //ѭ������
    connect(m_pMediaObject, SIGNAL(finished()), this, SLOT(slotRepeatPlay()));
}

void BlogSpider::setBlogImage(const QString &img)
{
    QPixmap pixmap(img);

    //��ͼƬ��ʧ��,����:"QPixmap::scaled: Pixmap is a null pixmap",ԭ������ǲ�ͬ��ʽ��ͼƬ�Բ�ͬ��ʽ����������
    //����:a.gif��һ��gif��ʽ��ͼƬ,���ֱ��������Ϊa.jpg,��ô�ͻ��ȡʧ��
    //���ʧ��,ֱ�Ӷ�ȡһ��Ĭ�ϵ�ͼƬ��ʾ
    if (pixmap.isNull())
    {
        pixmap.load(":/images/default.jpg");
    }

    //����ԭ���ı�����ʾ
    ui->labelBlogImage->setPixmap(pixmap.scaled(ui->labelBlogImage->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

bool BlogSpider::isValidCsdnId(const QString &id)
{
    quint32 idLen;
    QChar ch;

    //���ַ�����Ϊ��ĸ
    if (!id.at(0).isLetter())
    {
        return false;
    }

    idLen = id.length();
    for (quint8 i = 0; i < idLen; i++)
    {
        ch = id.at(i);
        if ((!ch.isLetterOrNumber()) && (ch != '_'))
        {
            return false;
        }
    }

    return true;
}

void BlogSpider::indexStartRequest(const QUrl &url)
{
    m_pHttpReply = m_httpManager.get(QNetworkRequest(url));
    connect(m_pHttpReply, SIGNAL(readyRead()), this, SLOT(slotIndexReadyWrite()));
    connect(m_pHttpReply, SIGNAL(finished()), this, SLOT(slotIndexFinish()));
}

void BlogSpider::headImageStartRequest(const QUrl &url)
{
    m_pHttpReply = m_httpManager.get(QNetworkRequest(url));
    connect(m_pHttpReply, SIGNAL(readyRead()), this, SLOT(slotBlogHeadImageReadyWrite()));
    connect(m_pHttpReply, SIGNAL(finished()), this, SLOT(slotBlogHeadImageFinish()));
}

void BlogSpider::infoStartRequest(const QUrl &url)
{
    m_pHttpReply = m_httpManager.get(QNetworkRequest(url));
    connect(m_pHttpReply, SIGNAL(readyRead()), this, SLOT(slotInfoReadyWrite()));
    connect(m_pHttpReply, SIGNAL(finished()), this, SLOT(slotInfoFinish()));
}

void BlogSpider::blogStartRequest(const QUrl &url)
{
    m_pHttpReply = m_httpManager.get(QNetworkRequest(url));
    connect(m_pHttpReply, SIGNAL(readyRead()), this, SLOT(slotBlogReadyRead()));
    connect(m_pHttpReply, SIGNAL(finished()), this, SLOT(slotBlogFinish()));
}

void BlogSpider::blogImageStartRequest(const QUrl &url)
{
    m_pHttpReply = m_httpManager.get(QNetworkRequest(url));
    connect(m_pHttpReply, SIGNAL(readyRead()), this, SLOT(slotBlogImageReadyWrite()));
    connect(m_pHttpReply, SIGNAL(finished()), this, SLOT(slotBlogImageFinish()));
}

void BlogSpider::setBlogInfoOnLineEdit()
{
    //������Ϣ��
    ui->lineEditBlogTitle->setText(m_blogTitle);
    ui->lineEditBlogTitle->setCursorPosition(0);//���ù��λ��

    ui->lineEditBlogDescription->setText(m_blogDescription);
    ui->lineEditBlogDescription->setCursorPosition(0);//���ù��λ��

    ui->lineEditView->setText(QString("%1��").arg(m_iBlogView));
    ui->lineEditScore->setText(QString("%1��").arg(m_iBlogScore));
    if (m_iBlogRanking != 0)
    {
        ui->lineEditRanking->setText(QString("��%1��").arg(m_iBlogRanking));
    }
    else
    {
        ui->lineEditRanking->setText(QString("ǧ��֮��"));
    }
    ui->lineEditBlogTotal->setText(QString("%1ƪ").arg(m_iBlogTotal));
    ui->lineEditOriginal->setText(QString("%1ƪ").arg(m_iBlogOriginal));
    ui->lineEditReship->setText(QString("%1ƪ").arg(m_iBlogReship));
    ui->lineEditTranslation->setText(QString("%1ƪ").arg(m_iBlogTranslation));
    ui->lineEditComments->setText(QString("%1��").arg(m_iBlogComment));
}

bool BlogSpider::initBlogSqlDb()
{
    QString sqlStr;
    QString blogSqlDbPath = m_blogSaveDir + QDir::separator() + m_blogSqlName;

    m_pBlogSqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));

    m_pBlogSqlDb->setDatabaseName(blogSqlDbPath);
    if (!m_pBlogSqlDb->open())
    {
        QMessageBox::critical(this, "���ݿ��ʼ��", "���ݿ��ʼ��ʧ��!", QMessageBox::Ok);
        return false;
    }

    QSqlQuery query(*m_pBlogSqlDb);

    //�������ͽ����һЩ��Ϣ��
    sqlStr = "create table blog_rank("
            "user varchar primary key, "
            "url varchar, "
            "dir varchar, "
            "title varchar, "
            "description varchar, "
            "view int, "
            "score int, "
            "ranking int, "
            "total int, "
            "original int, "
            "reship int, "
            "translation int, "
            "comment int)";

    query.exec(sqlStr);

    //�������в������µ���Ϣ��
    sqlStr = "create table blog_info("
            "num int primary key, "
            "title varchar, "
            "date varchar, "
            "view int, "
            "comment int, "
            "url varchar, "
            "download varchar)";

    query.exec(sqlStr);

    return true;
}

void BlogSpider::insertBlogInfoToSql()
{
    QString sqlStr;

    QSqlQuery query(*m_pBlogSqlDb);

    sqlStr = QString("insert into blog_rank values("
                     "'%1', '%2', '%3', '%4', "
                     "'%5', %6, %7, %8, "
                     "%9, %10, %11, %12, "
                     "%13)")
            .arg(m_csdnUserId, m_blogUrl.toString(), m_blogSaveDir, m_blogTitle, m_blogDescription)
            .arg(m_iBlogView).arg(m_iBlogScore)
            .arg(m_iBlogRanking).arg(m_iBlogTotal)
            .arg(m_iBlogOriginal).arg(m_iBlogReship)
            .arg(m_iBlogTranslation).arg(m_iBlogComment);

    query.exec(sqlStr);
}

void BlogSpider::deleteBlogSqlDb()
{
    //����ע��ָ����ͷ�˳��
    if (m_pBlogInfoSqlModel)
    {
        delete m_pBlogInfoSqlModel;
        m_pBlogInfoSqlModel = 0;
    }
    if (m_pBlogInfoView)
    {
        delete m_pBlogInfoView;
        m_pBlogInfoView = 0;
    }
    if (m_pBlogInfoDialog)
    {
        delete m_pBlogInfoDialog;
        m_pBlogInfoDialog = 0;
    }

    //�ظ�ʹ�����ݿ�,���������ͷ�,�������ʾ�ظ�ʹ�����ݿ�ľ���
    QString sqlName;
    //ɾ�����ݿ�
    if (m_pBlogSqlDb)
    {
        m_pBlogSqlDb->close();
        delete m_pBlogSqlDb;
        m_pBlogSqlDb = 0;

        //�ڸ��������У��õ�QSqlDatabase������
        {
            sqlName = QSqlDatabase::database().connectionName();
        }//������������������QSqlDatabase::database()��ɾ��
        QSqlDatabase::removeDatabase(sqlName);
    }
}

bool BlogSpider::isMainThreadBusy()
{
    if (MainThreadOp_AnalyseBlog == getMainThreadOp())
    {
        QMessageBox::warning(this, tr("����"), tr("���ڻ�ȡ�����б�,������Ч!"), QMessageBox::Ok);
        return false;
    }
    else if (MainThreadOp_DownloadBlog == getMainThreadOp())
    {
        QMessageBox::warning(this, tr("����"), tr("�������ز���,������Ч!"), QMessageBox::Ok);
        return false;
    }
    else if (MainThreadOp_Init == getMainThreadOp())
    {
        return true;
    }

    return true;
}

MainThreadOp BlogSpider::getMainThreadOp()
{
    return m_mainThreadCurrentOp;
}

CtrlMainThread BlogSpider::getMainThreadCtrl()
{
    return m_ctrlMainThreadProgress;
}

void BlogSpider::setMainThreadOp(MainThreadOp op)
{
    m_mainThreadCurrentOp = op;
}

void BlogSpider::setMainThreadCtrl(CtrlMainThread ctrl)
{
    m_ctrlMainThreadProgress = ctrl;
}

/**********slots**********/
void BlogSpider::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
        this->showNormal();
        break;

    case QSystemTrayIcon::DoubleClick:
        this->hide();
        m_pTrayIcon->showMessage(this->windowTitle(), tr("���,��������! �������Ҽ�ͼ���˳����."), QSystemTrayIcon::Information);
        break;

    default:
        break;
    }
}

void BlogSpider::slotUserTextChanged(const QString &str)
{
    QString tmpText = ui->lineEditUser->text();

    if (tmpText.trimmed().isEmpty())
    {
        ui->lineEditUrl->clear();
        ui->lineEditDir->setText(m_blogSaveDir);

        ui->btnGetBlogInfo->setEnabled(false);
    }
    else
    {
        QString url("http://blog.csdn.net/" + str.trimmed());
        ui->lineEditUrl->setText(url);
        ui->btnGetBlogInfo->setEnabled(true);
    }
}

void BlogSpider::slotGetBlogInfo()
{
    QString blogSqlDbPath;

    //�ж��û���������Ϣ
    if (ui->lineEditUser->text().isEmpty())
    {
        QMessageBox::warning(this, tr("����"), tr("�������û���!"), QMessageBox::Ok);
        return;
    }
    if (!isValidCsdnId(ui->lineEditUser->text()))
    {
        QMessageBox::warning(this, tr("����"), tr("��������Ч���û���!"), QMessageBox::Ok);
        return;
    }
    if (ui->lineEditDir->text().isEmpty())
    {
        QMessageBox::warning(this, tr("����"), tr("��ѡ�񲩿�Ҫ�����Ŀ¼!"), QMessageBox::Ok);
        return;
    }

    m_blogUrl = ui->lineEditUrl->text();
    m_csdnUserId = ui->lineEditUser->text();
    m_blogSaveDir = ui->lineEditDir->text();

    //���ز�����ҳ
    //����Ŀ¼
    QDir downloadDir(m_blogSaveDir);
    if (!downloadDir.exists())
    {
        if (!downloadDir.mkpath(m_blogSaveDir))
        {
            QMessageBox::critical(this, tr("����"), tr("����Ŀ¼ʧ��,������!"), QMessageBox::Ok);
            return;
        }
    }

    //�ͷ����ݿ�ָ��
    deleteBlogSqlDb();

    //��ȡ����ʱ,����ɾ��ԭ�����ݿ�
    blogSqlDbPath = m_blogSaveDir + QDir::separator() + m_blogSqlName;
    if (QFile::exists(blogSqlDbPath))
    {
        QFile::remove(blogSqlDbPath);
    }

    //�ͷ��б��ڴ�
    ui->tableWidgetBlog->clearContents();
    ui->tableWidgetBlog->setRowCount(0);

    //��ʼ�����ݿ�
    if (!initBlogSqlDb())
    {
        slotClearSetting();
        return;
    }

    //�����ļ�
    QString fileName(m_blogSaveDir + QDir::separator() + m_indexName);
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("����"), tr("�����ļ�ʧ��,������!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
//        slotClearSetting();
        return;
    }

    //��ʼ����
    indexStartRequest(m_blogUrl);

    //���ð�ť�������
    ui->lineEditUser->setReadOnly(true);
    ui->actOpenBlog->setEnabled(true);
    ui->btnGetBlogInfo->setEnabled(false);
    ui->btnPauseOrContinue->setEnabled(true);
    ui->btnFinish->setEnabled(true);
    ui->btnScanDir->setEnabled(false);

    //���õ�ǰ����Ĳ���
    setMainThreadOp(MainThreadOp_AnalyseBlog);
}

void BlogSpider::slotDownloadBlog()
{
    int count = 0;
    QTableWidgetItem *item;
    QChar separator = QDir::separator();

    QString path(m_blogSaveDir + separator + m_blogPdfDir);

    QDir pdfDir(path);
    if (!pdfDir.exists())
    {
        if (!pdfDir.mkpath(path))
        {
            QMessageBox::critical(this, tr("����"), tr("����Ŀ¼ʧ��,������!"), QMessageBox::Ok);
            return;
        }
    }
    //ͼƬĿ¼
    path = m_blogSaveDir + separator + m_blogImageDir;
    QDir imageDir(path);
    if (!imageDir.exists())
    {
        if (!imageDir.mkpath(path))
        {
            QMessageBox::critical(this, tr("����"), tr("����Ŀ¼ʧ��,������!"), QMessageBox::Ok);
            return;
        }
    }

    //��ȡtableWidgetBlog�ؼ��б�ѡ�����
    count = ui->tableWidgetBlog->rowCount();

    m_iBlogDownloadCount = 0;

    for (int i = 0; i < count; i++)
    {
        item = ui->tableWidgetBlog->item(i, 0);
        if (item->checkState() == Qt::Checked)
        {
            m_iBlogDownloadCount++;
            m_blogSelMap.insert(i+1, DownloadState_Init);
        }
    }

    if (0 == m_iBlogDownloadCount)
    {
        QMessageBox::warning(this, ui->btnDownloadBlog->text(), tr("��ѡ��Ҫ���صĲ���!"), QMessageBox::Ok);
        return;
    }

    //���õ�ǰ����Ĳ���
    setMainThreadOp(MainThreadOp_DownloadBlog);

    //����һЩ��ť�Ŀ�����
    ui->btnPauseOrContinue->setEnabled(true);
    ui->btnFinish->setEnabled(true);
    ui->btnDownloadBlog->setEnabled(false);
    ui->actShowBlogInfo->setEnabled(false);

    //���ý�����
    ui->pgsBarDownload->setMaximum(m_iBlogDownloadCount);
    ui->pgsBarDownload->setValue(0);
    ui->labelPgsBarDownload->setText(QString("0%"));

    //��ʼ�������в���
    emit startDownloadAllBlog();
}

void BlogSpider::slotPauseOrContinue()
{
    if ((CtrlMainThread_Init == getMainThreadCtrl())
            || (CtrlMainThread_Continue == getMainThreadCtrl()))
    {
        setMainThreadCtrl(CtrlMainThread_Pause);
        ui->btnPauseOrContinue->setText(tr("����"));
        QMessageBox::information(this, tr("��ͣ����"), tr("��ͣ����ɹ�!"), QMessageBox::Ok);
    }
    else if (CtrlMainThread_Pause == getMainThreadCtrl())
    {
        setMainThreadCtrl(CtrlMainThread_Continue);
        ui->btnPauseOrContinue->setText(tr("��ͣ"));
        //��������������
        if (MainThreadOp_AnalyseBlog == getMainThreadOp())
        {
            emit startDownloadAllPage();
        }
        else if (MainThreadOp_DownloadBlog == getMainThreadOp())
        {
            emit startDownloadAllBlog();
        }

        QMessageBox::information(this, tr("��������"), tr("����ִ������ɹ�!"), QMessageBox::Ok);
    }
    else if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        QMessageBox::warning(this, ui->btnPauseOrContinue->text() + tr("����"), tr("�����Ѿ�ֹͣ,������Ч!"), QMessageBox::Ok);
    }
}

void BlogSpider::slotFinish()
{
    if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        QMessageBox::warning(this, tr("ֹͣ����"), tr("�����Ѿ�ֹͣ!"), QMessageBox::Ok);
    }
    else
    {
        if (MainThreadOp_AnalyseBlog == getMainThreadOp())
        {
            //����������
            setMainThreadCtrl(CtrlMainThread_Finished);
            //�ָ����̴߳���״̬
            setMainThreadOp(MainThreadOp_Init);
            ui->actShowBlogInfo->setEnabled(true);
            QMessageBox::information(this, tr("ֹͣ����"), tr("ֹͣ����ɹ�!"), QMessageBox::Ok);
        }
        else if (MainThreadOp_DownloadBlog == getMainThreadOp())
        {
            // ���ع�����
            //��յ�ǰ���̵߳Ĳ���
            setMainThreadCtrl(CtrlMainThread_Finished);
            //�ָ����̴߳���״̬
            setMainThreadOp(MainThreadOp_Init);
            ui->actShowBlogInfo->setEnabled(true);
            //ɾ������Ҫ���ļ�
            QFile::remove(m_blogSaveDir + QDir::separator() + m_blogFileName);
            QFile::remove(m_blogSaveDir + QDir::separator() + "_dstblog.html");
            QFile::remove(m_blogSaveDir + QDir::separator() + "index.html");
            QMessageBox::information(this, tr("ֹͣ����"), tr("ֹͣ����ɹ�!"), QMessageBox::Ok);
        }
    }
}

void BlogSpider::slotScanDir()
{
    //QDir::rootPath(): 1.Linux/Unix->"/"; 2.Windows->"C:/"
    m_blogSaveDir = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Directory"),
                                                    QDir::rootPath(),
                                                    QFileDialog::ShowDirsOnly);
    //ת��Ϊ������Ҫ��·���ָ���,'/' or '\'
    m_blogSaveDir = QDir::toNativeSeparators(m_blogSaveDir);
    if (!m_blogSaveDir.isEmpty())
    {
        ui->lineEditDir->setText(m_blogSaveDir);
    }
}

//���ð�ť
void BlogSpider::slotClearSetting()
{
    if (!isMainThreadBusy())
    {
        return;
    }

    //ɾ�����ݿ�
    deleteBlogSqlDb();

    //�������̵߳�ȫ�ֱ���
    setMainThreadCtrl(CtrlMainThread_Init);

    //���ñ���
    m_blogUrl.clear();
    m_blogImageUrl.clear();
    m_csdnUserId.clear();
    m_blogSaveDir.clear();
    m_blogTitle.clear();
    m_blogDescription.clear();
    m_iBlogTotal = 0;
    m_iBlogPageTotal = 0;
    m_iBlogView = 0;
    m_iBlogScore = 0;
    m_iBlogRanking = 0;
    m_iBlogOriginal = 0;
    m_iBlogReship = 0;
    m_iBlogTranslation = 0;
    m_iBlogComment = 0;
    m_iBlogPageTotalCount = 0;
    m_iBlogDealCount = 0;
    m_iBlogDownloadCount = 0;
    m_curDownloadBlogName.clear();

    m_blogSelMap.clear();
    //free memory
    for (BlogImageInfoVector::Iterator it = m_blogimageVector.begin();
         it != m_blogimageVector.end();
         ++it)
    {
        free(*it);
    }
    m_blogimageVector.clear();

    setMainThreadOp(MainThreadOp_Init);

    //���������
    ui->lineEditUser->setReadOnly(false);
    ui->lineEditUser->clear();
    ui->lineEditUrl->clear();
    ui->lineEditDir->clear();

    //��������򽹵�
    ui->lineEditUser->setFocus();

    //���ÿؼ�������
    ui->actOpenBlog->setEnabled(false);
    ui->actShowBlogInfo->setEnabled(false);

    ui->btnGetBlogInfo->setEnabled(false);
    ui->btnDownloadBlog->setEnabled(false);
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);
    ui->btnScanDir->setEnabled(true);

    setBlogImage(":/images/default.jpg");

    //������Ϣ��
    ui->lineEditBlogTitle->clear();
    ui->lineEditBlogDescription->clear();
    ui->lineEditView->clear();
    ui->lineEditScore->clear();
    ui->lineEditRanking->clear();
    ui->lineEditBlogTotal->clear();
    ui->lineEditOriginal->clear();
    ui->lineEditReship->clear();
    ui->lineEditTranslation->clear();
    ui->lineEditComments->clear();

    //�����б�
    ui->tableWidgetBlog->clearContents();
    ui->tableWidgetBlog->setRowCount(0);

    ui->pgsBarAnalyse->setValue(0);
    ui->pgsBarDownload->setValue(0);
    ui->labelPgsBarAnalyse->setText("0%");
    ui->labelPgsBarDownload->setText("0%");
}

void BlogSpider::slotSelectAll()
{
    if (!isMainThreadBusy())
    {
        return;
    }

    int i = 0;
    int count = ui->tableWidgetBlog->rowCount();

    if (count)
    {
        for(i = 0; i < count; i++)
        {
            ui->tableWidgetBlog->item(i, 0)->setCheckState(Qt::Checked);
        }
    }
}

void BlogSpider::slotUnselectAll()
{
    if (!isMainThreadBusy())
    {
        return;
    }

    int i = 0;
    int count = ui->tableWidgetBlog->rowCount();
    QTableWidgetItem *tmpItem;

    if (count)
    {
        for(i = 0; i < count; i++)
        {
            tmpItem = ui->tableWidgetBlog->item(i, 0);
            if (tmpItem->checkState() == Qt::Checked)
            {
                tmpItem->setCheckState(Qt::Unchecked);
            }
            else if (tmpItem->checkState() == Qt::Unchecked)
            {
                tmpItem->setCheckState(Qt::Checked);
            }
        }
    }
}

void BlogSpider::slotClearSelect()
{
    if (!isMainThreadBusy())
    {
        return;
    }

    int i = 0;
    int count = ui->tableWidgetBlog->rowCount();
    QTableWidgetItem *tmpItem;

    if (count)
    {
        for(i = 0; i < count; i++)
        {
            tmpItem = ui->tableWidgetBlog->item(i, 0);
            tmpItem->setCheckState(Qt::Unchecked);
            tmpItem->setTextColor(Qt::black);
        }
    }
}

void BlogSpider::slotActImportUser()
{
    QChar separator = QDir::separator();
    QString userdir, sqlStr, imagePath;
    int count = 0;

    if (!isMainThreadBusy())
    {
        return;
    }

    userdir = QFileDialog::getExistingDirectory(this,
                                                tr("ѡ���û�����Ŀ¼"),
                                                QDir::rootPath(),
                                                QFileDialog::ShowDirsOnly);

    if (!userdir.isEmpty())
    {
        userdir = QDir::toNativeSeparators(userdir);

        if (!QFile::exists(userdir + separator + m_blogSqlName))
        {
            QMessageBox::critical(this, tr("����"), tr("�û����ݿ��ѱ�ɾ��, �û�����ʧ��!"), QMessageBox::Ok);
            return;
        }

        slotClearSetting();

        m_blogSaveDir = userdir;

        initBlogSqlDb();
        QSqlQuery query(*m_pBlogSqlDb);
        //�����ݿ����ݶ�ȡ������
        sqlStr = "select * from blog_rank";
        query.exec(sqlStr);
        query.next();
        m_csdnUserId = query.value(0).toString();
        m_blogUrl = QUrl(query.value(1).toString());
        m_blogSaveDir = query.value(2).toString();
        m_blogTitle = query.value(3).toString();
        m_blogDescription = query.value(4).toString();
        m_iBlogView = query.value(5).toInt();
        m_iBlogScore = query.value(6).toInt();
        m_iBlogRanking = query.value(7).toInt();
        m_iBlogTotal = query.value(8).toInt();
        m_iBlogOriginal = query.value(9).toInt();
        m_iBlogReship = query.value(10).toInt();
        m_iBlogTranslation = query.value(11).toInt();
        m_iBlogComment = query.value(12).toInt();

        ui->lineEditUser->setText(m_csdnUserId);
        ui->lineEditUrl->setText(m_blogUrl.toString());
        ui->lineEditDir->setText(m_blogSaveDir);
        //��������Ϣ��ʾ����Ϣ����
        setBlogInfoOnLineEdit();
        imagePath = m_blogSaveDir + separator + m_blogImageName;
        if (QFile::exists(imagePath))
        {
            setBlogImage(imagePath);
        }
        else
        {
            setBlogImage(":/images/default.jpg");
        }

        sqlStr = "select title, download from blog_info";
        query.exec(sqlStr);

        count = 0;
        while(query.next())
        {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(0).toString());
            //Ĭ��Ϊ��ɫ
            if (query.value(1).toString().compare(tr("���سɹ�")) == 0)
            {
                item->setTextColor(QColor(Qt::blue));
            }
            else if (query.value(1).toString().compare(tr("����ʧ��")) == 0)
            {
                item->setTextColor(QColor(Qt::red));
            }
            item->setCheckState(Qt::Unchecked);
            ui->tableWidgetBlog->insertRow(count);
            ui->tableWidgetBlog->setItem(count, 0, item);
            ui->tableWidgetBlog->scrollToBottom();
            count++;
        }

        ui->lineEditUser->setReadOnly(true);
        ui->btnDownloadBlog->setEnabled(true);
        ui->actShowBlogInfo->setEnabled(true);
        ui->actOpenBlog->setEnabled(true);
    }
}

void BlogSpider::slotActOpenPerBlog()
{
    QDesktopServices::openUrl(QUrl(m_blogUrl));
}

void BlogSpider::slotActAboutQt()
{
    QMessageBox::aboutQt(this);
}

//����������Ҫ�õ�HTML�ļ��ı༭��ʽ
void BlogSpider::slotActAboutBlogSpider()
{
    if (m_spiderVersion.compare("1.0") == 0)
    {
        QMessageBox::about(this, tr("����BlogSpider"),
                           tr("<b><font size=5>CSDN����������-BlogSpider</font></b>"
                              "<font size=4>"
                              "<p>�汾: v%1</p>"
                              "<p>����: gzshun</p>"
                              "<p>����: gzshuns@163.com</p>"
                              "<p>QQȺ: 210563904</p>"
                              "<p>����: <a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun</a></p>"
                              "<p>����: һ�����ײ����Ի���CSDN����������</p>"
                              "<p>��Ȩ: ��ȫ���,���ɴ���</p>"
                              "<p>����: 2012-04-11</p>"
                              "</font>").arg(m_spiderVersion));
    }
    else if (m_spiderVersion.compare("2.0") == 0)
    {
        QMessageBox::about(this, tr("����BlogSpider"),
                           tr("<b><font size=5>CSDN����������-BlogSpider</font></b>"
                              "<font size=4>"
                              "<p>�汾: v%1</p>"
                              "<p>����: gzshun</p>"
                              "<p>����: gzshuns@163.com</p>"
                              "<p>����: <a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun</a></p>"
                              "<p>����: һ�����ײ����Ի���CSDN����������</p>"
                              "<p>��Ȩ: ��ȫ���,���ɴ���</p>"
                              "<p>����: 2012-05-01</p>"
                              "</font>").arg(m_spiderVersion));
    }
}

void BlogSpider::slotActQuit()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, this->windowTitle(), tr("ȷ��Ҫ�˳������?"), QMessageBox::Ok, QMessageBox::Cancel);
    if (btn == QMessageBox::Ok)
    {
        qApp->quit();
    }
}

void BlogSpider::slotOpenMyBlog(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void BlogSpider::slotShowBloginfo()
{
    //��ָ���ѱ��ͷ�, ��Ҫ���³�ʼ��,����ֱ��show����
    if (!m_pBlogInfoDialog)
    {
        m_pBlogInfoDialog = new QDialog(this);
        m_pBlogInfoSqlModel = new QSqlTableModel(m_pBlogInfoDialog, *m_pBlogSqlDb);
        m_pBlogInfoView = new QTableView(m_pBlogInfoDialog);

        //�źŲ�
        connect(m_pBlogInfoView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotOpenBlogUrl(QModelIndex)));

        //ģ̬��ʾ
        m_pBlogInfoDialog->setModal(true);

        //��ȡinfo��
        m_pBlogInfoSqlModel->setTable("blog_info");
        m_pBlogInfoSqlModel->select();

        //�����б���
        m_pBlogInfoSqlModel->setHeaderData(0, Qt::Horizontal, tr("���"));
        m_pBlogInfoSqlModel->setHeaderData(1, Qt::Horizontal, tr("����"));
        m_pBlogInfoSqlModel->setHeaderData(2, Qt::Horizontal, tr("����"));
        m_pBlogInfoSqlModel->setHeaderData(3, Qt::Horizontal, tr("�Ķ�"));
        m_pBlogInfoSqlModel->setHeaderData(4, Qt::Horizontal, tr("����"));
        m_pBlogInfoSqlModel->setHeaderData(5, Qt::Horizontal, tr("��ַ"));
        m_pBlogInfoSqlModel->setHeaderData(6, Qt::Horizontal, tr("����״̬"));
        //���ڱ���
        m_pBlogInfoDialog->setWindowTitle(tr("������ϸ��Ϣ"));

        //��ӵ�blogInfoView��
        m_pBlogInfoView->setModel(m_pBlogInfoSqlModel);
        m_pBlogInfoView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_pBlogInfoView->verticalHeader()->setHidden(true);//���ص�һ�е��к�
        //���ñ��������ᱻ�϶�
        m_pBlogInfoView->horizontalHeader()->setMovable(false);
        m_pBlogInfoView->verticalHeader()->setMovable(false);
        //���ñ�����Ϊ�̶����,�����ı�
        m_pBlogInfoView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
        m_pBlogInfoView->verticalHeader()->setResizeMode(QHeaderView::Fixed);

        m_pBlogInfoView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pBlogInfoView->setSelectionMode(QAbstractItemView::SingleSelection);

        //�̶�λ��
        m_pBlogInfoView->move(0, 0);
    }

    //�̶��д�С
    m_pBlogInfoView->setColumnWidth(0, 60);//num
    m_pBlogInfoView->setColumnWidth(1, 300);//title
    m_pBlogInfoView->setColumnWidth(2, 110);//date
    m_pBlogInfoView->setColumnWidth(3, 60);//read
    m_pBlogInfoView->setColumnWidth(4, 50);//comment
    m_pBlogInfoView->setColumnWidth(5, 340);//url
    m_pBlogInfoView->setColumnWidth(6, 60);//download state
    //�̶���С
    m_pBlogInfoView->setFixedSize(1000, 600);
    m_pBlogInfoDialog->setFixedSize(1000, 600);

    m_pBlogInfoDialog->show();
}

void BlogSpider::slotOpenBlogUrl(const QModelIndex &index)
{
    int num = index.row() + 1;
    QString sqlStr;
    QSqlQuery query(*m_pBlogSqlDb);

    sqlStr = QString("select url from blog_info where num=%1").arg(num);
    query.exec(sqlStr);
    query.next();

    QDesktopServices::openUrl(query.value(0).toString());
}

void BlogSpider::slotIndexReadyWrite()
{
    if (m_pDownloadFile)
    {
        m_pDownloadFile->write(m_pHttpReply->readAll());
    }
}

void BlogSpider::slotIndexFinish()
{
    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //��ʼ��������
    m_dealThread.setCtrlThreadOp(CtrlThread_AnalyseBlogRank);
    m_dealThread.setCsdnUserId(m_csdnUserId);
    m_dealThread.setBlogSaveDir(m_blogSaveDir);
    m_dealThread.setAnalyseFile(m_indexName);
    m_dealThread.start();
}

void BlogSpider::slotDownloadAllPage()
{
    if (CtrlMainThread_Pause == getMainThreadCtrl())
    {
        return;
    }
    else if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        m_iBlogPageTotalCount = 0;
        return;
    }

    //������ص����һҳ���˳���֪ͨ����
    if (++m_iBlogPageTotalCount > m_iBlogPageTotal)
    {
        emit downloadPageDone();
        return;
    }

    //http://blog.csdn.net/gzshun/article/list/1
    QUrl pageUrl(QString("%1/article/list/%2").arg(m_blogUrl.toString()).arg(m_iBlogPageTotalCount));

    //����ҳ��
    QString fileName(m_blogSaveDir + QDir::separator() + m_indexName);
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), tr("�����ļ�ʧ��,������!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        return;
    }

    infoStartRequest(pageUrl);
}

void BlogSpider::slotInfoReadyWrite()
{
    if (m_pDownloadFile)
    {
        m_pDownloadFile->write(m_pHttpReply->readAll());
    }
}

void BlogSpider::slotInfoFinish()
{
    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //֪ͨ�߳�,��ʼ����ҳ��
    m_dealThread.setCtrlThreadOp(CtrlThread_AnalyseBlogPage);
    m_dealThread.setCsdnUserId(m_csdnUserId);
    m_dealThread.setBlogSaveDir(m_blogSaveDir);
    m_dealThread.setAnalyseFile(m_indexName);
    m_dealThread.setBlogSqlDatabase(m_pBlogSqlDb);
    m_dealThread.start();
}

void BlogSpider::slotDownloadPageDone()
{
    if (m_iBlogTotal == 0)
    {
        QMessageBox::information(this, tr("��ȡ�����б�"), tr("�û�����û�з��������"), QMessageBox::Ok);
        setMainThreadOp(MainThreadOp_Init);
    }
    else
    {
        ui->actShowBlogInfo->setEnabled(true);
        ui->btnDownloadBlog->setEnabled(true);
        //�ָ�
        setMainThreadOp(MainThreadOp_Init);
        m_pTrayIcon->showMessage(this->windowTitle(),
                              tr("��ȡ�����б�ɹ�,���Կ�ʼ���ز���"), QSystemTrayIcon::Information);
    }

    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //��յ�ǰ���̵߳Ŀ���
    setMainThreadCtrl(CtrlMainThread_Init);
}

void BlogSpider::slotDownloadAllBlog()
{
    QChar separator = QDir::separator();
    int currentKey = 0;
    QTableWidgetItem *item;

    if (CtrlMainThread_Pause == getMainThreadCtrl())
    {
        return;
    }
    else if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        m_iBlogDownloadCount = 0;
        return;
    }

    //������ص����һҳ���˳���֪ͨ����
    if ((m_iBlogDownloadCount--) == 0)
    {
        emit downloadBlogDone();
        return;
    }

    foreach(int key, m_blogSelMap.keys())
    {
        if (m_blogSelMap.value(key) == DownloadState_Init)
        {
            currentKey = key;
            m_blogSelMap[key] = DownloadState_Current;
            break;
        }
    }

    QSqlQuery query(*m_pBlogSqlDb);

    QString sqlStr = QString("select title,url from blog_info where num=%1").arg(currentKey);
    query.exec(sqlStr);
    query.next();

    //http://blog.csdn.net/gzshun/article/details/7371253
    QUrl blogUrl(query.value(1).toString());
    m_curDownloadBlogName = query.value(0).toString();
    QString fileName(m_blogSaveDir + separator + m_blogPdfDir + separator + m_curDownloadBlogName + ".pdf");

    //���ز���
    if (QFile::exists(fileName))
    {
        //�õ���ǰ���ص���
        item = ui->tableWidgetBlog->item(currentKey-1, 0);
        item->setTextColor(QColor(Qt::blue));
        ui->tableWidgetBlog->setCurrentItem(item);
        ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
        ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
        m_blogSelMap[currentKey] = DownloadState_Success;//���سɹ�
        emit startDownloadAllBlog();
        return;
    }

    m_pDownloadFile = new QFile(m_blogSaveDir + separator + m_blogFileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), tr("�����ļ�ʧ��,������!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        ui->btnDownloadBlog->setEnabled(true);
        ui->btnPauseOrContinue->setEnabled(false);
        ui->btnFinish->setEnabled(false);
        setMainThreadOp(MainThreadOp_Init);
        ui->actShowBlogInfo->setEnabled(true);
        m_blogSelMap.clear();
        return;
    }

    blogStartRequest(blogUrl);
}

void BlogSpider::slotBlogReadyRead()
{
    if (m_pDownloadFile)
    {
        m_pDownloadFile->write(m_pHttpReply->readAll());
    }
}

void BlogSpider::slotBlogFinish()
{
    int currentKey = 0;
    QTableWidgetItem *item;
    QChar separator = QDir::separator();

    foreach(int key, m_blogSelMap.keys())
    {
        if (m_blogSelMap.value(key) == DownloadState_Current)
        {
            currentKey = key;
            break;
        }
    }

    //�õ���ǰ���ص���
    item = ui->tableWidgetBlog->item(currentKey-1, 0);

    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();

        m_blogSelMap[currentKey] = DownloadState_Failed;//����ʧ��
        item->setTextColor(QColor(Qt::red));
        ui->tableWidgetBlog->setCurrentItem(item);

        //���ý�����
        ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
        ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
        return;
    }
    m_blogSelMap[currentKey] = DownloadState_Success;//���سɹ�
    item->setTextColor(QColor(Qt::blue));
    ui->tableWidgetBlog->setCurrentItem(item);

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //�����̷߳�������
    m_dealThread.setCtrlThreadOp(CtrlThread_Analyseblog);
    m_dealThread.setBlogSaveDir(m_blogSaveDir);
    m_dealThread.setBlogImageDir(m_blogSaveDir + separator + m_blogImageDir);
    m_dealThread.setBlogPdfDir(m_blogSaveDir + separator + m_blogPdfDir);
    m_dealThread.setAnalyseFile(m_blogFileName);
    m_dealThread.setBlogToPdfName(m_curDownloadBlogName + ".pdf");
    m_dealThread.setBlogImageInfoVector(&m_blogimageVector);
    m_dealThread.start();
}

void BlogSpider::slotDownloadBlogDone()
{
    //�ͷŲ�����ϸ��Ϣ������ڴ�
    if (m_pBlogInfoSqlModel)
    {
        delete m_pBlogInfoSqlModel;
        m_pBlogInfoSqlModel = 0;
    }
    if (m_pBlogInfoView)
    {
        delete m_pBlogInfoView;
        m_pBlogInfoView = 0;
    }
    if (m_pBlogInfoDialog)
    {
        delete m_pBlogInfoDialog;
        m_pBlogInfoDialog = 0;
    }

    //ɾ������Ҫ���ļ�
    QFile::remove(m_blogSaveDir + QDir::separator() + m_blogFileName);
    QFile::remove(m_blogSaveDir + QDir::separator() + "_dstblog.html");
    QFile::remove(m_blogSaveDir + QDir::separator() + "index.html");

    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("�������سɹ�"), QSystemTrayIcon::Information);
    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("��ʼ�������ݿ�"), QSystemTrayIcon::Information);


    //���ΰ�ť
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //�����߳�,�����������ݿ�
    ui->pgsBarDownload->setValue(0);
    ui->pgsBarDownload->setMaximum(m_blogSelMap.count());
    ui->labelPgsBarDownload->setText(tr("0%"));

    m_dealThread.setCtrlThreadOp(CtrlThread_UpdateSqlDb);
    m_dealThread.setBlogSqlDatabase(m_pBlogSqlDb);
    m_dealThread.setDownloadStateMap(m_blogSelMap);
    m_dealThread.start();
}

void BlogSpider::slotCreatePdfDone()
{
    //free memory
    for (BlogImageInfoVector::Iterator it = m_blogimageVector.begin();
         it != m_blogimageVector.end();
         ++it)
    {
        free(*it);
    }
    m_blogimageVector.clear();

    //���ý�����
    ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
    ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));

    //����������һƪ����
    emit startDownloadAllBlog();
}

void BlogSpider::slotDownloadAllImage()
{
    bool hasImage = false;
    QString imageUrl, imageName;
    QChar separator = QDir::separator();
    for (BlogImageInfoVector::Iterator it = m_blogimageVector.begin();
         it != m_blogimageVector.end();
         ++it)
    {
        if (!(*it)->downloadState)
        {
            imageUrl = (*it)->imageUrl;
            imageName = (*it)->imageName;
            (*it)->downloadState = true;
            hasImage = true;
            break;
        }
    }

    //û��δ���ص�ͼƬ
    if (!hasImage)
    {
        m_dealThread.setCtrlThreadOp(CtrlThread_CreatePdf);
        m_dealThread.start();
        return;
    }

    //����ͼƬ
    QString fileName(m_blogSaveDir + separator + m_blogImageDir + separator + imageName);
    if (QFile::exists(fileName))
    {
        emit startDownloadAllImage();
        return;
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), tr("�����ļ�ʧ��,������!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        ui->btnDownloadBlog->setEnabled(true);
        ui->btnPauseOrContinue->setEnabled(false);
        ui->btnFinish->setEnabled(false);
        setMainThreadOp(MainThreadOp_Init);
        slotClearSetting();
        return;
    }

    //��ʼ��������ͼƬ
    blogImageStartRequest(imageUrl);
}

void BlogSpider::slotBlogImageReadyWrite()
{
    if (m_pDownloadFile)
    {
        m_pDownloadFile->write(m_pHttpReply->readAll());
    }
}

void BlogSpider::slotBlogImageFinish()
{
    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        m_pHttpReply->deleteLater();
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    emit startDownloadAllImage();
}

void BlogSpider::slotBlogHeadImageReadyWrite()
{
    if (m_pDownloadFile)
    {
        m_pDownloadFile->write(m_pHttpReply->readAll());
    }
}

void BlogSpider::slotBlogHeadImageFinish()
{
    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        m_pHttpReply->deleteLater();
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //����ͷ��
    setBlogImage(m_blogSaveDir + QDir::separator() + m_blogImageName);

    //������ͼƬ����ʼ��������ҳ��
    emit startDownloadAllPage();
}

void BlogSpider::slotDownloadBlogHeadImage()
{
    //����ͷ��
    QString fileName(m_blogSaveDir + QDir::separator() + m_blogImageName);
    if (QFile::exists(fileName))
    {
        //����ͷ��
        setBlogImage(m_blogSaveDir + QDir::separator() + m_blogImageName);
        //��ʼ��������ҳ��
        emit startDownloadAllPage();
        return;
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("����"), tr("�����ļ�ʧ��,������!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        ui->btnGetBlogInfo->setEnabled(true);
        ui->btnPauseOrContinue->setEnabled(false);
        ui->btnFinish->setEnabled(false);
        return;
    }

    //��ʼ��������ͼƬ
    headImageStartRequest(m_blogImageUrl);
}

void BlogSpider::slotRecvBlogRank(const QString &title,
                                  const QString &description,
                                  const QString &imageUrl,
                                  const QString &imageName,
                                  const QString &total,
                                  const QString &pageTotal,
                                  const QString &view,
                                  const QString &score,
                                  const QString &ranking,
                                  const QString &original,
                                  const QString &reship,
                                  const QString &translation,
                                  const QString &comment)
{
    //��ʼ����������������Ϣ
    m_blogTitle = title;
    m_blogDescription = description;
    m_blogImageUrl = QUrl(imageUrl);
//    m_blogImageName = imageName;
    m_iBlogTotal = total.toInt();
    m_iBlogPageTotal = pageTotal.toInt();
    m_iBlogView = view.toInt();
    m_iBlogScore = score.toInt();
    m_iBlogRanking = ranking.toInt();
    m_iBlogOriginal = original.toInt();
    m_iBlogReship = reship.toInt();
    m_iBlogTranslation = translation.toInt();
    m_iBlogComment = comment.toInt();

    //��������Ϣ��ʾ����Ϣ����
    setBlogInfoOnLineEdit();
    //��������Ϣ�������ݿ���
    insertBlogInfoToSql();

    //���ý�����
    if (m_iBlogTotal != 0)
    {
        ui->pgsBarAnalyse->setMaximum(m_iBlogTotal);
    }

    //���ò����б�������, ���Ч��
//    ui->tableWidgetBlog->setRowCount(m_iBlogTotal);

    //���ز���ͷ��
    emit startDownloadBlogImage();
}

void BlogSpider::slotRecvBlogInfo(const QString &title)
{
    //��������Ϣ��ӵ��б���
    QTableWidgetItem *item = new QTableWidgetItem(title);
    item->setCheckState(Qt::Unchecked);
    ui->tableWidgetBlog->insertRow(m_iBlogDealCount);
    ui->tableWidgetBlog->setItem(m_iBlogDealCount, 0, item);
//    ui->tableWidgetBlog->setCurrentItem(item);
    ui->tableWidgetBlog->scrollToBottom();//������������Ϊ�����

    m_iBlogDealCount++;

    //���·���������
    ui->pgsBarAnalyse->setValue(ui->pgsBarAnalyse->value()+1);
    ui->labelPgsBarAnalyse->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarAnalyse->value() / ui->pgsBarAnalyse->maximum())));
}

void BlogSpider::slotDealThreadError(const QString &errStr)
{
    QMessageBox::critical(this, tr("�������"), errStr, QMessageBox::Ok);
    setMainThreadOp(MainThreadOp_Init);
    slotClearSetting();
}

void BlogSpider::slotUpdateSqlDbProgress()
{
    //���·���������
    ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
    ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
}

void BlogSpider::slotUpdateSqlDbDone()
{

    ui->actShowBlogInfo->setEnabled(true);
    ui->btnDownloadBlog->setEnabled(true);
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //������ո�����
    m_blogSelMap.clear();

    //�ָ����̴߳���״̬
    setMainThreadOp(MainThreadOp_Init);

    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("���ݿ���³ɹ�"), QSystemTrayIcon::Information);
}

void BlogSpider::slotPlayMusic()
{
    if (m_pMediaObject->state() == Phonon::StoppedState)
    {
        m_pMediaObject->play();
    }
}

void BlogSpider::slotStopMusic()
{
    if (m_pMediaObject->state() == Phonon::PlayingState)
    {
        m_pMediaObject->stop();
    }
}

void BlogSpider::slotRepeatPlay()
{
    QString file("default.mp3");
    if (!QFile::exists(file))
    {
        return;
    }

    Phonon::MediaSource music(file);

    m_pMediaObject->setCurrentSource(music);

    m_pMediaObject->play();
}
