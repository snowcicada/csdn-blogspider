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
    //初始化界面
    initSetupUi();
    //初始化系统图标
    initSystemTrayIcon();
    //初始化控件的提示信息
    initWidgetStatusTip();
    //初始化下载信号槽连接
    initDownloadSignalSlot();
    //开启背景音乐
    initBackGroundMusic();
}

BlogSpider::~BlogSpider()
{
    if (m_dealThread.isRunning())
    {
        m_dealThread.stop();
    }

    //释放数据库指针
    deleteBlogSqlDb();

    //删除音乐对象
    if (m_pMediaObject)
    {
        delete m_pMediaObject;
    }
    if (m_pAudioOutput)
    {
        delete m_pAudioOutput;
    }

    //将博客列表的内存释放出来
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
    m_pTrayIcon->showMessage(this->windowTitle(), tr("哈喽,我在这呢! 您可以右键图标退出软件."), QSystemTrayIcon::Information);
    //要忽略掉窗口的关闭事件
    event->ignore();
}

void BlogSpider::initSetupUi()
{
    //变量初始化
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

    //指针初始化
    m_pBlogSqlDb = 0;
    m_pHttpReply = 0;
    m_pDownloadFile = 0;

    m_pBlogInfoDialog = 0;
    m_pBlogInfoSqlModel = 0;
    m_pBlogInfoView = 0;

    m_pMediaObject = 0;
    m_pAudioOutput = 0;

    //设置窗口图标和标题
    this->setWindowIcon(QIcon(QString(":/images/icon.png")));
    this->setWindowTitle(QString("CSDN博客下载器v%1-(作者:gzshun)").arg(m_spiderVersion));

    //固定窗口大小
    this->setFixedSize(500, 620);

    //屏蔽最大化按钮的使用
    this->setWindowFlags(Qt::WindowMinimizeButtonHint);

    //菜单栏
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

    //输入框
    ui->lineEditUser->setMaxLength(20);
    ui->lineEditUser->setPlaceholderText(tr("请输入用户名"));//默认值:鼠标点击消失的
    connect(ui->lineEditUser, SIGNAL(textChanged(QString)), this, SLOT(slotUserTextChanged(QString)));
    connect(ui->lineEditUser, SIGNAL(returnPressed()), this, SLOT(slotGetBlogInfo()));//回车信号

    ui->lineEditUrl->setReadOnly(true);
    ui->lineEditDir->setReadOnly(true);

    //按钮
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

    //csdn默认头像
    setBlogImage(":/images/default.jpg");

    //博客信息框
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

    //进度条标签
    ui->labelPgsBarAnalyse->setAlignment(Qt::AlignCenter);
    ui->labelPgsBarDownload->setAlignment(Qt::AlignCenter);

    //状态栏
    m_pBlogUrlLabel = new QLabel(QString("<a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun </a>"), this);
    ui->statusBar->addPermanentWidget(m_pBlogUrlLabel);
    connect(m_pBlogUrlLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotOpenMyBlog(QString)));
    ui->statusBar->setSizeGripEnabled(false);//去掉状态栏的大小控制点

    //设置tableWidgetBlog控件的字体
//    ui->tableWidgetBlog->setFont(QFont(tr("宋体"), 10, QFont::Normal));
    ui->tableWidgetBlog->setColumnCount(1);//只有一列
    ui->tableWidgetBlog->horizontalHeader()->setHidden(true);//隐藏第一行的标签
    ui->tableWidgetBlog->verticalHeader()->setShown(true);//显示第一列的序号
    //固定住控件,不让拖宽
    ui->tableWidgetBlog->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidgetBlog->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidgetBlog->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置为只读
    //自适应长度,bug:每增加一个新行，都会重新适应长度
//    ui->tableWidgetBlog->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidgetBlog->setColumnWidth(0, 355);//设置固定宽度
    //只能选一行
    ui->tableWidgetBlog->setSelectionMode(QAbstractItemView::SingleSelection);
    //当选中时,直接选中一整行
    ui->tableWidgetBlog->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void BlogSpider::initSystemTrayIcon()
{
    //增加行为
    //最大化
    m_pMaximizeAction = new QAction(tr("最大化"), this);
    connect(m_pMaximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
    //最小化
    m_pMinimizeAction = new QAction(tr("最小化"), this);
    connect(m_pMinimizeAction, SIGNAL(triggered()), this, SLOT(showMinimized()));
    //隐藏
    m_pHideAction = new QAction(tr("隐藏"), this);
    connect(m_pHideAction, SIGNAL(triggered()), this, SLOT(hide()));
    //还原
    m_pRestoreAction = new QAction(tr("还原"), this);
    connect(m_pRestoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    //退出
    m_pQuitAction = new QAction(tr("退出"), this);
    connect(m_pQuitAction, SIGNAL(triggered()), this, SLOT(slotActQuit()));

    //增加菜单
    m_pTrayIconMenu = new QMenu(this);
    m_pTrayIconMenu->addAction(m_pHideAction);
    m_pTrayIconMenu->addAction(m_pRestoreAction);
    m_pTrayIconMenu->addAction(m_pMinimizeAction);
    m_pTrayIconMenu->addAction(m_pMaximizeAction);
    m_pTrayIconMenu->addSeparator();
    m_pTrayIconMenu->addAction(m_pQuitAction);

    //开启系统任务栏图标
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
    ui->actImportUser->setStatusTip(tr("导入已存在的用户"));
    ui->actOpenBlog->setStatusTip(tr("打开我的博客"));
    ui->actShowBlogInfo->setStatusTip(tr("显示博客详细信息"));
    ui->actPlayMusic->setStatusTip(tr("播放音乐"));
    ui->actStopMusic->setStatusTip(tr("停止播放音乐"));
    ui->actQuit->setStatusTip(tr("退出"));
    ui->actAboutQt->setStatusTip(tr("关于Qt"));
    ui->actBlogSpider->setStatusTip(tr("关于BlogSpider"));

    ui->lineEditUser->setStatusTip(tr("用户名"));
    ui->lineEditUser->setToolTip(tr("用户名"));
    ui->lineEditUrl->setStatusTip(tr("博客网址"));
    ui->lineEditUrl->setToolTip(tr("博客网址"));
    ui->lineEditDir->setStatusTip(tr("博客下载目录"));
    ui->lineEditDir->setToolTip(tr("博客下载目录"));

    ui->btnGetBlogInfo->setStatusTip(tr("获取博客列表信息"));
    ui->btnGetBlogInfo->setToolTip(tr("获取博客列表信息"));
    ui->btnDownloadBlog->setStatusTip(tr("下载博客"));
    ui->btnDownloadBlog->setToolTip(tr("下载博客"));
    ui->btnPauseOrContinue->setStatusTip(tr("暂停或继续处理"));
    ui->btnPauseOrContinue->setToolTip(tr("暂停或继续处理"));
    ui->btnFinish->setStatusTip(tr("停止处理"));
    ui->btnFinish->setToolTip(tr("停止处理"));
    ui->btnScanDir->setStatusTip(tr("浏览选择目录"));
    ui->btnScanDir->setToolTip(tr("浏览选择目录"));
    ui->btnClearSetting->setStatusTip(tr("重置所有设置"));
    ui->btnClearSetting->setToolTip(tr("重置所有设置"));

    ui->labelBlogImage->setStatusTip(tr("用户头像"));
    ui->labelBlogImage->setToolTip(tr("用户头像 "));

    ui->tableWidgetBlog->setStatusTip(tr("博客列表"));
    ui->tableWidgetBlog->setToolTip(tr("博客列表"));

    ui->btnSelectAll->setStatusTip(tr("全选博客"));
    ui->btnSelectAll->setToolTip(tr("全选博客"));
    ui->btnUnselectAll->setStatusTip(tr("反选博客"));
    ui->btnUnselectAll->setToolTip(tr("反选博客"));
    ui->btnClearSelect->setStatusTip(tr("重置选择"));
    ui->btnClearSelect->setToolTip(tr("重置选择"));

    m_pBlogUrlLabel->setStatusTip(tr("我的博客"));
    m_pBlogUrlLabel->setToolTip(tr("我的博客"));
}

void BlogSpider::initDownloadSignalSlot()
{
    connect(&m_dealThread, SIGNAL(updateSqlDbProgress()), this, SLOT(slotUpdateSqlDbProgress()));
    connect(&m_dealThread, SIGNAL(updateSqlDbDone()), this, SLOT(slotUpdateSqlDbDone()));
    //下载博客用户头像
    connect(this, SIGNAL(startDownloadBlogImage()), this, SLOT(slotDownloadBlogHeadImage()));
    //开始下载每个页面
    connect(this, SIGNAL(startDownloadAllPage()), this, SLOT(slotDownloadAllPage()));
    connect(&m_dealThread, SIGNAL(startDownloadAllPage()), this, SLOT(slotDownloadAllPage()));
    //开始下载每篇博客
    connect(this, SIGNAL(startDownloadAllBlog()), this, SLOT(slotDownloadAllBlog()));
    //分析完页面发送信号通知
    connect(this, SIGNAL(downloadPageDone()), this, SLOT(slotDownloadPageDone()));
    //下载完博客
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


    //关于生成pdf的信号槽
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

    //循环播放
    connect(m_pMediaObject, SIGNAL(finished()), this, SLOT(slotRepeatPlay()));
}

void BlogSpider::setBlogImage(const QString &img)
{
    QPixmap pixmap(img);

    //若图片打开失败,报错:"QPixmap::scaled: Pixmap is a null pixmap",原因可能是不同格式的图片以不同格式的名称命名
    //比如:a.gif是一张gif格式的图片,如果直接重命名为a.jpg,那么就会读取失败
    //如果失败,直接读取一张默认的图片显示
    if (pixmap.isNull())
    {
        pixmap.load(":/images/default.jpg");
    }

    //按照原来的比例显示
    ui->labelBlogImage->setPixmap(pixmap.scaled(ui->labelBlogImage->size(),
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

bool BlogSpider::isValidCsdnId(const QString &id)
{
    quint32 idLen;
    QChar ch;

    //首字符必须为字母
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
    //博客信息框
    ui->lineEditBlogTitle->setText(m_blogTitle);
    ui->lineEditBlogTitle->setCursorPosition(0);//设置光标位置

    ui->lineEditBlogDescription->setText(m_blogDescription);
    ui->lineEditBlogDescription->setCursorPosition(0);//设置光标位置

    ui->lineEditView->setText(QString("%1次").arg(m_iBlogView));
    ui->lineEditScore->setText(QString("%1分").arg(m_iBlogScore));
    if (m_iBlogRanking != 0)
    {
        ui->lineEditRanking->setText(QString("第%1名").arg(m_iBlogRanking));
    }
    else
    {
        ui->lineEditRanking->setText(QString("千里之外"));
    }
    ui->lineEditBlogTotal->setText(QString("%1篇").arg(m_iBlogTotal));
    ui->lineEditOriginal->setText(QString("%1篇").arg(m_iBlogOriginal));
    ui->lineEditReship->setText(QString("%1篇").arg(m_iBlogReship));
    ui->lineEditTranslation->setText(QString("%1篇").arg(m_iBlogTranslation));
    ui->lineEditComments->setText(QString("%1条").arg(m_iBlogComment));
}

bool BlogSpider::initBlogSqlDb()
{
    QString sqlStr;
    QString blogSqlDbPath = m_blogSaveDir + QDir::separator() + m_blogSqlName;

    m_pBlogSqlDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));

    m_pBlogSqlDb->setDatabaseName(blogSqlDbPath);
    if (!m_pBlogSqlDb->open())
    {
        QMessageBox::critical(this, "数据库初始化", "数据库初始化失败!", QMessageBox::Ok);
        return false;
    }

    QSqlQuery query(*m_pBlogSqlDb);

    //创建博客界面的一些信息表
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

    //创建所有博客文章的信息表
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
    //必须注意指针的释放顺序
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

    //重复使用数据库,必须这样释放,否则会提示重复使用数据库的警告
    QString sqlName;
    //删除数据库
    if (m_pBlogSqlDb)
    {
        m_pBlogSqlDb->close();
        delete m_pBlogSqlDb;
        m_pBlogSqlDb = 0;

        //在该作用域中，得到QSqlDatabase的名字
        {
            sqlName = QSqlDatabase::database().connectionName();
        }//超出作用域，隐含对象QSqlDatabase::database()被删除
        QSqlDatabase::removeDatabase(sqlName);
    }
}

bool BlogSpider::isMainThreadBusy()
{
    if (MainThreadOp_AnalyseBlog == getMainThreadOp())
    {
        QMessageBox::warning(this, tr("警告"), tr("正在获取博客列表,操作无效!"), QMessageBox::Ok);
        return false;
    }
    else if (MainThreadOp_DownloadBlog == getMainThreadOp())
    {
        QMessageBox::warning(this, tr("警告"), tr("正在下载博客,操作无效!"), QMessageBox::Ok);
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
        m_pTrayIcon->showMessage(this->windowTitle(), tr("哈喽,我在这呢! 您可以右键图标退出软件."), QSystemTrayIcon::Information);
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

    //判断用户的输入信息
    if (ui->lineEditUser->text().isEmpty())
    {
        QMessageBox::warning(this, tr("警告"), tr("请输入用户名!"), QMessageBox::Ok);
        return;
    }
    if (!isValidCsdnId(ui->lineEditUser->text()))
    {
        QMessageBox::warning(this, tr("警告"), tr("请输入有效的用户名!"), QMessageBox::Ok);
        return;
    }
    if (ui->lineEditDir->text().isEmpty())
    {
        QMessageBox::warning(this, tr("警告"), tr("请选择博客要保存的目录!"), QMessageBox::Ok);
        return;
    }

    m_blogUrl = ui->lineEditUrl->text();
    m_csdnUserId = ui->lineEditUser->text();
    m_blogSaveDir = ui->lineEditDir->text();

    //下载博客主页
    //创建目录
    QDir downloadDir(m_blogSaveDir);
    if (!downloadDir.exists())
    {
        if (!downloadDir.mkpath(m_blogSaveDir))
        {
            QMessageBox::critical(this, tr("错误"), tr("创建目录失败,请重试!"), QMessageBox::Ok);
            return;
        }
    }

    //释放数据库指针
    deleteBlogSqlDb();

    //获取博客时,必须删除原有数据库
    blogSqlDbPath = m_blogSaveDir + QDir::separator() + m_blogSqlName;
    if (QFile::exists(blogSqlDbPath))
    {
        QFile::remove(blogSqlDbPath);
    }

    //释放列表内存
    ui->tableWidgetBlog->clearContents();
    ui->tableWidgetBlog->setRowCount(0);

    //初始化数据库
    if (!initBlogSqlDb())
    {
        slotClearSetting();
        return;
    }

    //创建文件
    QString fileName(m_blogSaveDir + QDir::separator() + m_indexName);
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("错误"), tr("创建文件失败,请重试!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
//        slotClearSetting();
        return;
    }

    //开始请求
    indexStartRequest(m_blogUrl);

    //设置按钮和输入框
    ui->lineEditUser->setReadOnly(true);
    ui->actOpenBlog->setEnabled(true);
    ui->btnGetBlogInfo->setEnabled(false);
    ui->btnPauseOrContinue->setEnabled(true);
    ui->btnFinish->setEnabled(true);
    ui->btnScanDir->setEnabled(false);

    //设置当前处理的操作
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
            QMessageBox::critical(this, tr("错误"), tr("创建目录失败,请重试!"), QMessageBox::Ok);
            return;
        }
    }
    //图片目录
    path = m_blogSaveDir + separator + m_blogImageDir;
    QDir imageDir(path);
    if (!imageDir.exists())
    {
        if (!imageDir.mkpath(path))
        {
            QMessageBox::critical(this, tr("错误"), tr("创建目录失败,请重试!"), QMessageBox::Ok);
            return;
        }
    }

    //获取tableWidgetBlog控件中被选择的项
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
        QMessageBox::warning(this, ui->btnDownloadBlog->text(), tr("请选择要下载的博客!"), QMessageBox::Ok);
        return;
    }

    //设置当前处理的操作
    setMainThreadOp(MainThreadOp_DownloadBlog);

    //设置一些按钮的可用性
    ui->btnPauseOrContinue->setEnabled(true);
    ui->btnFinish->setEnabled(true);
    ui->btnDownloadBlog->setEnabled(false);
    ui->actShowBlogInfo->setEnabled(false);

    //设置进度条
    ui->pgsBarDownload->setMaximum(m_iBlogDownloadCount);
    ui->pgsBarDownload->setValue(0);
    ui->labelPgsBarDownload->setText(QString("0%"));

    //开始下载所有博客
    emit startDownloadAllBlog();
}

void BlogSpider::slotPauseOrContinue()
{
    if ((CtrlMainThread_Init == getMainThreadCtrl())
            || (CtrlMainThread_Continue == getMainThreadCtrl()))
    {
        setMainThreadCtrl(CtrlMainThread_Pause);
        ui->btnPauseOrContinue->setText(tr("继续"));
        QMessageBox::information(this, tr("暂停任务"), tr("暂停任务成功!"), QMessageBox::Ok);
    }
    else if (CtrlMainThread_Pause == getMainThreadCtrl())
    {
        setMainThreadCtrl(CtrlMainThread_Continue);
        ui->btnPauseOrContinue->setText(tr("暂停"));
        //继续分析或下载
        if (MainThreadOp_AnalyseBlog == getMainThreadOp())
        {
            emit startDownloadAllPage();
        }
        else if (MainThreadOp_DownloadBlog == getMainThreadOp())
        {
            emit startDownloadAllBlog();
        }

        QMessageBox::information(this, tr("继续任务"), tr("继续执行任务成功!"), QMessageBox::Ok);
    }
    else if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        QMessageBox::warning(this, ui->btnPauseOrContinue->text() + tr("任务"), tr("任务已经停止,操作无效!"), QMessageBox::Ok);
    }
}

void BlogSpider::slotFinish()
{
    if (CtrlMainThread_Finished == getMainThreadCtrl())
    {
        QMessageBox::warning(this, tr("停止任务"), tr("任务已经停止!"), QMessageBox::Ok);
    }
    else
    {
        if (MainThreadOp_AnalyseBlog == getMainThreadOp())
        {
            //分析过程中
            setMainThreadCtrl(CtrlMainThread_Finished);
            //恢复主线程处理状态
            setMainThreadOp(MainThreadOp_Init);
            ui->actShowBlogInfo->setEnabled(true);
            QMessageBox::information(this, tr("停止任务"), tr("停止任务成功!"), QMessageBox::Ok);
        }
        else if (MainThreadOp_DownloadBlog == getMainThreadOp())
        {
            // 下载过程中
            //清空当前主线程的操作
            setMainThreadCtrl(CtrlMainThread_Finished);
            //恢复主线程处理状态
            setMainThreadOp(MainThreadOp_Init);
            ui->actShowBlogInfo->setEnabled(true);
            //删除不需要的文件
            QFile::remove(m_blogSaveDir + QDir::separator() + m_blogFileName);
            QFile::remove(m_blogSaveDir + QDir::separator() + "_dstblog.html");
            QFile::remove(m_blogSaveDir + QDir::separator() + "index.html");
            QMessageBox::information(this, tr("停止任务"), tr("停止任务成功!"), QMessageBox::Ok);
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
    //转换为本地需要的路径分隔符,'/' or '\'
    m_blogSaveDir = QDir::toNativeSeparators(m_blogSaveDir);
    if (!m_blogSaveDir.isEmpty())
    {
        ui->lineEditDir->setText(m_blogSaveDir);
    }
}

//重置按钮
void BlogSpider::slotClearSetting()
{
    if (!isMainThreadBusy())
    {
        return;
    }

    //删除数据库
    deleteBlogSqlDb();

    //控制主线程的全局变量
    setMainThreadCtrl(CtrlMainThread_Init);

    //重置变量
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

    //重置输入框
    ui->lineEditUser->setReadOnly(false);
    ui->lineEditUser->clear();
    ui->lineEditUrl->clear();
    ui->lineEditDir->clear();

    //设置输入框焦点
    ui->lineEditUser->setFocus();

    //设置控件可用性
    ui->actOpenBlog->setEnabled(false);
    ui->actShowBlogInfo->setEnabled(false);

    ui->btnGetBlogInfo->setEnabled(false);
    ui->btnDownloadBlog->setEnabled(false);
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);
    ui->btnScanDir->setEnabled(true);

    setBlogImage(":/images/default.jpg");

    //博客信息框
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

    //博客列表
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
                                                tr("选择用户下载目录"),
                                                QDir::rootPath(),
                                                QFileDialog::ShowDirsOnly);

    if (!userdir.isEmpty())
    {
        userdir = QDir::toNativeSeparators(userdir);

        if (!QFile::exists(userdir + separator + m_blogSqlName))
        {
            QMessageBox::critical(this, tr("错误"), tr("用户数据库已被删除, 用户导入失败!"), QMessageBox::Ok);
            return;
        }

        slotClearSetting();

        m_blogSaveDir = userdir;

        initBlogSqlDb();
        QSqlQuery query(*m_pBlogSqlDb);
        //将数据库内容读取到界面
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
        //将博客信息显示到信息框中
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
            //默认为黑色
            if (query.value(1).toString().compare(tr("下载成功")) == 0)
            {
                item->setTextColor(QColor(Qt::blue));
            }
            else if (query.value(1).toString().compare(tr("下载失败")) == 0)
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

//设置字体需要用到HTML文件的编辑格式
void BlogSpider::slotActAboutBlogSpider()
{
    if (m_spiderVersion.compare("1.0") == 0)
    {
        QMessageBox::about(this, tr("关于BlogSpider"),
                           tr("<b><font size=5>CSDN博客下载器-BlogSpider</font></b>"
                              "<font size=4>"
                              "<p>版本: v%1</p>"
                              "<p>作者: gzshun</p>"
                              "<p>邮箱: gzshuns@163.com</p>"
                              "<p>QQ群: 210563904</p>"
                              "<p>博客: <a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun</a></p>"
                              "<p>功能: 一个简易并人性化的CSDN博客下载器</p>"
                              "<p>版权: 完全免费,自由传播</p>"
                              "<p>日期: 2012-04-11</p>"
                              "</font>").arg(m_spiderVersion));
    }
    else if (m_spiderVersion.compare("2.0") == 0)
    {
        QMessageBox::about(this, tr("关于BlogSpider"),
                           tr("<b><font size=5>CSDN博客下载器-BlogSpider</font></b>"
                              "<font size=4>"
                              "<p>版本: v%1</p>"
                              "<p>作者: gzshun</p>"
                              "<p>邮箱: gzshuns@163.com</p>"
                              "<p>博客: <a href=\"http://blog.csdn.net/gzshun\">http://blog.csdn.net/gzshun</a></p>"
                              "<p>功能: 一个简易并人性化的CSDN博客下载器</p>"
                              "<p>版权: 完全免费,自由传播</p>"
                              "<p>日期: 2012-05-01</p>"
                              "</font>").arg(m_spiderVersion));
    }
}

void BlogSpider::slotActQuit()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, this->windowTitle(), tr("确定要退出该软件?"), QMessageBox::Ok, QMessageBox::Cancel);
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
    //若指针已被释放, 则要重新初始化,否则直接show即可
    if (!m_pBlogInfoDialog)
    {
        m_pBlogInfoDialog = new QDialog(this);
        m_pBlogInfoSqlModel = new QSqlTableModel(m_pBlogInfoDialog, *m_pBlogSqlDb);
        m_pBlogInfoView = new QTableView(m_pBlogInfoDialog);

        //信号槽
        connect(m_pBlogInfoView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotOpenBlogUrl(QModelIndex)));

        //模态显示
        m_pBlogInfoDialog->setModal(true);

        //获取info表
        m_pBlogInfoSqlModel->setTable("blog_info");
        m_pBlogInfoSqlModel->select();

        //设置列标题
        m_pBlogInfoSqlModel->setHeaderData(0, Qt::Horizontal, tr("序号"));
        m_pBlogInfoSqlModel->setHeaderData(1, Qt::Horizontal, tr("标题"));
        m_pBlogInfoSqlModel->setHeaderData(2, Qt::Horizontal, tr("日期"));
        m_pBlogInfoSqlModel->setHeaderData(3, Qt::Horizontal, tr("阅读"));
        m_pBlogInfoSqlModel->setHeaderData(4, Qt::Horizontal, tr("评论"));
        m_pBlogInfoSqlModel->setHeaderData(5, Qt::Horizontal, tr("网址"));
        m_pBlogInfoSqlModel->setHeaderData(6, Qt::Horizontal, tr("下载状态"));
        //窗口标题
        m_pBlogInfoDialog->setWindowTitle(tr("博客详细信息"));

        //添加到blogInfoView中
        m_pBlogInfoView->setModel(m_pBlogInfoSqlModel);
        m_pBlogInfoView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_pBlogInfoView->verticalHeader()->setHidden(true);//隐藏第一列的行号
        //设置标题栏不会被拖动
        m_pBlogInfoView->horizontalHeader()->setMovable(false);
        m_pBlogInfoView->verticalHeader()->setMovable(false);
        //设置标题栏为固定宽度,不被改变
        m_pBlogInfoView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
        m_pBlogInfoView->verticalHeader()->setResizeMode(QHeaderView::Fixed);

        m_pBlogInfoView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pBlogInfoView->setSelectionMode(QAbstractItemView::SingleSelection);

        //固定位置
        m_pBlogInfoView->move(0, 0);
    }

    //固定列大小
    m_pBlogInfoView->setColumnWidth(0, 60);//num
    m_pBlogInfoView->setColumnWidth(1, 300);//title
    m_pBlogInfoView->setColumnWidth(2, 110);//date
    m_pBlogInfoView->setColumnWidth(3, 60);//read
    m_pBlogInfoView->setColumnWidth(4, 50);//comment
    m_pBlogInfoView->setColumnWidth(5, 340);//url
    m_pBlogInfoView->setColumnWidth(6, 60);//download state
    //固定大小
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
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //开始分析博客
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

    //如果下载到最后一页，退出，通知界面
    if (++m_iBlogPageTotalCount > m_iBlogPageTotal)
    {
        emit downloadPageDone();
        return;
    }

    //http://blog.csdn.net/gzshun/article/list/1
    QUrl pageUrl(QString("%1/article/list/%2").arg(m_blogUrl.toString()).arg(m_iBlogPageTotalCount));

    //下载页面
    QString fileName(m_blogSaveDir + QDir::separator() + m_indexName);
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), tr("创建文件失败,请重试!"), QMessageBox::Ok);
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
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //通知线程,开始分析页面
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
        QMessageBox::information(this, tr("获取博客列表"), tr("用户博客没有发表过文章"), QMessageBox::Ok);
        setMainThreadOp(MainThreadOp_Init);
    }
    else
    {
        ui->actShowBlogInfo->setEnabled(true);
        ui->btnDownloadBlog->setEnabled(true);
        //恢复
        setMainThreadOp(MainThreadOp_Init);
        m_pTrayIcon->showMessage(this->windowTitle(),
                              tr("获取博客列表成功,可以开始下载博客"), QSystemTrayIcon::Information);
    }

    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //清空当前主线程的控制
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

    //如果下载到最后一页，退出，通知界面
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

    //下载博客
    if (QFile::exists(fileName))
    {
        //得到当前下载的行
        item = ui->tableWidgetBlog->item(currentKey-1, 0);
        item->setTextColor(QColor(Qt::blue));
        ui->tableWidgetBlog->setCurrentItem(item);
        ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
        ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
        m_blogSelMap[currentKey] = DownloadState_Success;//下载成功
        emit startDownloadAllBlog();
        return;
    }

    m_pDownloadFile = new QFile(m_blogSaveDir + separator + m_blogFileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), tr("创建文件失败,请重试!"), QMessageBox::Ok);
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

    //得到当前下载的行
    item = ui->tableWidgetBlog->item(currentKey-1, 0);

    m_pDownloadFile->flush();
    m_pDownloadFile->close();

    if (m_pHttpReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();

        m_blogSelMap[currentKey] = DownloadState_Failed;//下载失败
        item->setTextColor(QColor(Qt::red));
        ui->tableWidgetBlog->setCurrentItem(item);

        //设置进度条
        ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
        ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
        return;
    }
    m_blogSelMap[currentKey] = DownloadState_Success;//下载成功
    item->setTextColor(QColor(Qt::blue));
    ui->tableWidgetBlog->setCurrentItem(item);

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //开启线程分析博客
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
    //释放博客详细信息界面的内存
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

    //删除不需要的文件
    QFile::remove(m_blogSaveDir + QDir::separator() + m_blogFileName);
    QFile::remove(m_blogSaveDir + QDir::separator() + "_dstblog.html");
    QFile::remove(m_blogSaveDir + QDir::separator() + "index.html");

    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("博客下载成功"), QSystemTrayIcon::Information);
    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("开始更新数据库"), QSystemTrayIcon::Information);


    //屏蔽按钮
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //开启线程,用来更新数据库
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

    //设置进度条
    ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
    ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));

    //继续下载下一篇文章
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

    //没有未下载的图片
    if (!hasImage)
    {
        m_dealThread.setCtrlThreadOp(CtrlThread_CreatePdf);
        m_dealThread.start();
        return;
    }

    //下载图片
    QString fileName(m_blogSaveDir + separator + m_blogImageDir + separator + imageName);
    if (QFile::exists(fileName))
    {
        emit startDownloadAllImage();
        return;
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), tr("创建文件失败,请重试!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        ui->btnDownloadBlog->setEnabled(true);
        ui->btnPauseOrContinue->setEnabled(false);
        ui->btnFinish->setEnabled(false);
        setMainThreadOp(MainThreadOp_Init);
        slotClearSetting();
        return;
    }

    //开始请求下载图片
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
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), m_pHttpReply->errorString(), QMessageBox::Ok);
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
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), m_pHttpReply->errorString(), QMessageBox::Ok);
        m_pDownloadFile->remove();
        m_pHttpReply->deleteLater();
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        return;
    }

    m_pHttpReply->deleteLater();

    delete m_pDownloadFile;
    m_pDownloadFile = 0;

    //设置头像
    setBlogImage(m_blogSaveDir + QDir::separator() + m_blogImageName);

    //下载完图片，开始下载所有页面
    emit startDownloadAllPage();
}

void BlogSpider::slotDownloadBlogHeadImage()
{
    //下载头像
    QString fileName(m_blogSaveDir + QDir::separator() + m_blogImageName);
    if (QFile::exists(fileName))
    {
        //设置头像
        setBlogImage(m_blogSaveDir + QDir::separator() + m_blogImageName);
        //开始下载所有页面
        emit startDownloadAllPage();
        return;
    }

    m_pDownloadFile = new QFile(fileName);
    if (!m_pDownloadFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, ui->btnGetBlogInfo->text() + tr("错误"), tr("创建文件失败,请重试!"), QMessageBox::Ok);
        delete m_pDownloadFile;
        m_pDownloadFile = 0;
        ui->btnGetBlogInfo->setEnabled(true);
        ui->btnPauseOrContinue->setEnabled(false);
        ui->btnFinish->setEnabled(false);
        return;
    }

    //开始请求下载图片
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
    //初始化博客排名积分信息
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

    //将博客信息显示到信息框中
    setBlogInfoOnLineEdit();
    //将博客信息插入数据库中
    insertBlogInfoToSql();

    //设置进度条
    if (m_iBlogTotal != 0)
    {
        ui->pgsBarAnalyse->setMaximum(m_iBlogTotal);
    }

    //设置博客列表总行数, 提高效率
//    ui->tableWidgetBlog->setRowCount(m_iBlogTotal);

    //下载博客头像
    emit startDownloadBlogImage();
}

void BlogSpider::slotRecvBlogInfo(const QString &title)
{
    //将博客信息添加到列表中
    QTableWidgetItem *item = new QTableWidgetItem(title);
    item->setCheckState(Qt::Unchecked);
    ui->tableWidgetBlog->insertRow(m_iBlogDealCount);
    ui->tableWidgetBlog->setItem(m_iBlogDealCount, 0, item);
//    ui->tableWidgetBlog->setCurrentItem(item);
    ui->tableWidgetBlog->scrollToBottom();//将滚动栏设置为最底下

    m_iBlogDealCount++;

    //更新分析进度条
    ui->pgsBarAnalyse->setValue(ui->pgsBarAnalyse->value()+1);
    ui->labelPgsBarAnalyse->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarAnalyse->value() / ui->pgsBarAnalyse->maximum())));
}

void BlogSpider::slotDealThreadError(const QString &errStr)
{
    QMessageBox::critical(this, tr("处理错误"), errStr, QMessageBox::Ok);
    setMainThreadOp(MainThreadOp_Init);
    slotClearSetting();
}

void BlogSpider::slotUpdateSqlDbProgress()
{
    //更新分析进度条
    ui->pgsBarDownload->setValue(ui->pgsBarDownload->value()+1);
    ui->labelPgsBarDownload->setText(QString("%1%").arg((int)(100.0 * ui->pgsBarDownload->value() / ui->pgsBarDownload->maximum())));
}

void BlogSpider::slotUpdateSqlDbDone()
{

    ui->actShowBlogInfo->setEnabled(true);
    ui->btnDownloadBlog->setEnabled(true);
    ui->btnPauseOrContinue->setEnabled(false);
    ui->btnFinish->setEnabled(false);

    //必须清空该容器
    m_blogSelMap.clear();

    //恢复主线程处理状态
    setMainThreadOp(MainThreadOp_Init);

    m_pTrayIcon->showMessage(this->windowTitle(),
                          tr("数据库更新成功"), QSystemTrayIcon::Information);
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
