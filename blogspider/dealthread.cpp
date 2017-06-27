#include "dealthread.h"

QString DealThread::m_destBlogFile = "_dstblog.html";

DealThread::DealThread() : m_stopped(false)
{
}

DealThread::~DealThread()
{
    wait();
}

void DealThread::run()
{
    switch(m_ctrlThreadOp)
    {
    case CtrlThread_AnalyseBlogRank:
        analyseBlogRank();
        break;

    case CtrlThread_AnalyseBlogPage:
        analyseAllPage();
        break;

    case CtrlThread_Analyseblog:
        analyseBlog();
        break;

    case CtrlThread_CreatePdf:
        createPdf();
        break;
    case CtrlThread_UpdateSqlDb:
        updateSqlDb();
        break;

    default:
        break;
    }
}

void DealThread::stop()
{
    m_stopped = true;
}


void DealThread::setCtrlThreadOp(CtrlThread op)
{
    m_ctrlThreadOp = op;
}

void DealThread::setCsdnUserId(const QString &id)
{
    m_csdnUserId = id;
}

void DealThread::setBlogSaveDir(const QString &dir)
{
    m_blogSaveDir = dir;
}

void DealThread::setBlogImageDir(const QString &dir)
{
    m_blogImageDir = dir;
}

void DealThread::setBlogPdfDir(const QString &dir)
{
    m_blogPdfDir = dir;
}

void DealThread::setAnalyseFile(const QString &file)
{
    m_analyseFile = file;
}

void DealThread::setBlogSqlDatabase(QSqlDatabase *db)
{
    m_pBlogSqlDatabase = db;
}

void DealThread::setDownloadStateMap(DownloadStateMap &map)
{
    m_downloadBlogMap = map;
}

void DealThread::setBlogImageInfoVector(BlogImageInfoVector *vec)
{
    m_blogimageVector = vec;
}

void DealThread::setBlogToPdfName(const QString &name)
{
    m_blogPdfName = name;
}

void DealThread::analyseBlogRank()
{
    int pos = 0;
    int count = 0;
    bool hasBlog = false;
    QRegExp rx;
    QString line;
    QString tmp;
    QString title(QString("<a href=\"/%1\">").arg(m_csdnUserId));//<a href="/gzshun">
    QString linkTitle("<span class=\"link_title\">");//<span class="link_title">
    QString page("class=\"pagelist\">");
    QString image("<div id=\"blog_userface\">");
    QString rank("<ul id=\"blog_rank\">");
    QString statistics("<ul id=\"blog_statistics\">");
    QString noPageError("Object moved");

    //保存数据的变量
    QString blogImageUrl;
    QString blogTitle;
    QString blogDescription;
    QString blogImageName;
    QString blogTotal;
    QString blogPageTotal;
    QString blogView;
    QString blogScore;
    QString blogRanking;
    QString blogOriginal;
    QString blogReship;
    QString blogTranslation;
    QString blogComment;

    QFile file(m_blogSaveDir + QDir::separator() + m_analyseFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit dealThreadError(QString(file.errorString()));
        return;
    }

    QTextStream in(&file);
    //必须设置,否则乱码,必须是utf8格式
    in.setCodec("UTF-8");

    //判断是否存在该博客
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(noPageError))
        {
            emit dealThreadError(tr("该用户尚未开通博客"));
            return;
        }
    }
    in.seek(0);

    //获取博客标题
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(title))
        {
            rx.setPattern(">(.*)</a>");
            pos = 0;
            if (rx.indexIn(line, pos) >= 0)
            {
                blogTitle = rx.cap(1);
                break;
            }
        }
    }

    //获取博客描述
    line = in.readLine();
    rx.setPattern("<h2>(.*)</h2>");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogDescription = rx.cap(1);
    }

    //获取博客总页数和博客数量
    while(!in.atEnd())
    {
        line = in.readLine();
        if(line.contains(page))
        {
            //博客数量与总页数
            line = in.readLine();
            //得到 <span> 54条数据  共6页</span>
            rx.setPattern("<span>(.*)</span>");
            pos = 0;
            if ((pos = rx.indexIn(line, pos)) >= 0)
            {
                tmp = rx.cap(1);
            }
            //从 "<span> 54条数据  共6页</span>"里面分析数据
            rx.setPattern("\\d+");
            pos = 0;
            //数据
            if ((pos = rx.indexIn(tmp, pos)) >= 0)
            {
                blogTotal = rx.cap(0);
                pos += rx.matchedLength();
            }
            //页数
            if ((pos = rx.indexIn(tmp, pos)) >= 0)
            {
                blogPageTotal = rx.cap(0);
            }

            hasBlog = true;

            break;
        }
    }
    //找不到"总页数",那么就要判断该页面有几篇博客
    if (!hasBlog)
    {
        in.seek(0);
        count = 0;
        while(!in.atEnd())
        {
            line = in.readLine();
            if (line.contains(linkTitle))
            {
                count++;
                hasBlog = true;
            }
        }
        //存在博客
        if (hasBlog)
        {
            blogTotal.setNum(count);
            blogPageTotal.setNum(1);
        }
        in.seek(0);
    }

    //获取博客头像网址
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(image))
        {
            break;
        }
    }
    //开启最小匹配,下面的正则表达式需要改设置
    rx.setMinimal(true);
    //达到头像网址的前2行
    line = in.readLine();//跳过
    line = in.readLine();
    rx.setPattern("<img src=\"(.*)\"");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogImageUrl = rx.cap(1);
        //http://avatar.csdn.net/B/A/E/1_gzshun.jpg 获取出 1_gzshun.jpg
        pos = blogImageUrl.lastIndexOf('/');
        blogImageName = blogImageUrl.right(blogImageUrl.length()-pos-1);
    }

    //获取博客排名信息
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(rank))
        {
            //到达排名信息的位置，跳出循环开始分析
            break;
        }
    }
    //访问
    line = in.readLine();
    // <li>访问：<span>87698次</span></li>
    rx.setPattern("<span>(.*)次");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogView = rx.cap(1);
    }
    //积分
    line = in.readLine();
    // <li>积分：<span>1753分</span></li>
    rx.setPattern("<span>(.*)分");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogScore = rx.cap(1);
    }
    //排名
    line = in.readLine();
    // <li>排名：<span>第2946名</span></li>
    rx.setPattern("<span>第(.*)名");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {

        blogRanking = rx.cap(1);
    }
    else
    {
        blogRanking.setNum(0);
    }
    //跳过中间几行无用信息
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(statistics))
        {
            break;
        }
    }
    //原创
    line = in.readLine();
    // <li>原创：<span>48篇</span></li>
    rx.setPattern("<span>(.*)篇");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogOriginal = rx.cap(1);
    }
    //转载
    line = in.readLine();
    // <li>转载：<span>6篇</span></li>
    rx.setPattern("<span>(.*)篇");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogReship = rx.cap(1);
    }
    //译文
    line = in.readLine();
    // <li>译文：<span>0篇</span></li>
    rx.setPattern("<span>(.*)篇");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogTranslation = rx.cap(1);
    }
    //评论
    line = in.readLine();
    // <li>评论：<span>349条</span></li>
    rx.setPattern("<span>(.*)条");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogComment = rx.cap(1);
    }

    file.close();

    //将分析得到的信息发送给主线程
    emit sendBlogRank(blogTitle,
                      blogDescription,
                      blogImageUrl,
                      blogImageName,
                      blogTotal,
                      blogPageTotal,
                      blogView,
                      blogScore,
                      blogRanking,
                      blogOriginal,
                      blogReship,
                      blogTranslation,
                      blogComment);
}

void DealThread::analyseAllPage()
{
    int pos = 0;
    int sqlCount = 0;//数据库列表的序号
    QRegExp rx;
    QString line;
    QString tmp;
    QString title("<span class=\"link_title\">");//<span class="link_title">
    QString date("<span class=\"link_postdate\">");//<span class="link_postdate">

    //保存数据
    QString blogUrl;
    QString blogTitle;
    QString blogDate;
    QString blogView;
    QString blogComment;

    //判断线程是否要停止
    if (m_stopped)
    {
        return;
    }

    QFile file(m_blogSaveDir + QDir::separator() + m_analyseFile);

    if (!m_pBlogSqlDatabase->isOpen())
    {
        emit dealThreadError(tr("线程无法初始化数据库,主线程数据库没有打开"));
        return;
    }
    QSqlQuery query(*m_pBlogSqlDatabase);
    QString sqlStr;

    sqlStr = QString("select count(*) from blog_info");
    query.exec(sqlStr);
    query.next();
    sqlCount = query.value(0).toInt();//得到当前数据库列表的个数

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit dealThreadError(file.errorString());
        return;
    }

    QTextStream in(&file);
    //必须设置,否则乱码
    in.setCodec("UTF-8");

    while(!in.atEnd())
    {
        line = in.readLine();
        //得到博客网址
        if (line.contains(title))
        {
            rx.setPattern("href=\"(.*)\">");
            pos = title.length();//不用判断这段长度
            if (rx.indexIn(line, pos) >= 0)
            {
                blogUrl = "http://blog.csdn.net" + rx.cap(1);
            }

            //得到博客标题
            line = in.readLine();
            if (line.contains(tr("[置顶]")))
            {
                line = in.readLine();
            }
            blogTitle = line.trimmed();
            //在windows平台下,以下一些字符为不规则字符
            blogTitle.replace('/', '-');
            blogTitle.replace('\\', '-');
            blogTitle.replace(':', '-');
            blogTitle.replace('*', '-');
            blogTitle.replace('?', '-');
            blogTitle.replace('"', '-');
            blogTitle.replace('<', '-');
            blogTitle.replace('>', '-');
            blogTitle.replace('|', '-');

            while(!in.atEnd())
            {
                line = in.readLine();
                //得到博客发表日期
                if (line.contains(date))
                {
                    rx.setPattern(">(.*)</");
                    pos = date.length();
                    if (rx.indexIn(line, pos) >= 0)
                    {
                        blogDate = rx.cap(1);
                    }

                    //得到博客阅读次数
                    line = in.readLine();
                    rx.setPattern("</a>(.*)</span>");
                    pos = 0;
                    if (rx.indexIn(line, pos) >= 0)
                    {
                        tmp = rx.cap(1);//(145)
                        rx.setPattern("\\d+");
                        pos = 0;
                        if (rx.indexIn(tmp, pos) >= 0)
                        {
                            blogView = rx.cap(0);
                        }
                    }
                    //得到博客评论次数
                    line = in.readLine();
                    rx.setPattern("</a>((.*))</span>");
                    pos = 0;
                    if (rx.indexIn(line, pos) >= 0)
                    {
                        tmp = rx.cap(1);//(145)
                        rx.setPattern("\\d+");
                        pos = 0;
                        if (rx.indexIn(tmp, pos) >= 0)
                        {
                            blogComment = rx.cap(0);
                        }
                    }

                    //每篇文章都要发送一次,将获取到的信息发送到主线程
                    emit sendBlogInfo(blogTitle);

                    //添加进数据库中
                    sqlStr = QString("insert into blog_info values(%1, '%2', '%3', %4, %5, '%6', '%7')")
                            .arg(1 + sqlCount++).arg(blogTitle).arg(blogDate)
                            .arg(blogView).arg(blogComment).arg(blogUrl).arg(tr("未下载"));
                    query.exec(sqlStr);

                    //跳到外层循环
                    break;
                }
            }
        }
    }

    emit startDownloadAllPage();
}

void DealThread::analyseBlog()
{
    int pos1, pos2;
    bool isCode = false;
    QTextStream in, out;
    QString line, tmp;
//    QString csdnBlogUrl("http://blog.csdn.net");
//    QString tmpHref = "<a href=\"";
    QString tmpTitle = "<div id=\"container\">";
    QString tmpTitleEnd = "<div id=\"navigator\">";
    QString tmpArticle = "<div id=\"article_details\"";
    QString tmpArticleEnd = "<div class=\"share_buttons\"";
    QString tmpImage = "<img";
    QString tmpImage1 = "src=\"";
    QString tmpCode = "<textarea readonly name=\"code\"";
    QString tmpCodeend = "</textarea>";
    QString tmpImageUrl, tmpImageName;
    QFile srcBlogFile(m_blogSaveDir + QDir::separator() + m_analyseFile);
    QFile dstBlogFile(m_blogSaveDir + QDir::separator() + m_destBlogFile);

    if (!srcBlogFile.open(QIODevice::ReadOnly))
    {
        emit dealThreadError(srcBlogFile.errorString());
        return;
    }
    if (!dstBlogFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        emit dealThreadError(dstBlogFile.errorString());
        return;
    }

    out.setDevice(&srcBlogFile);
    out.setCodec("UTF-8");

    in.setDevice(&dstBlogFile);
    in.setCodec("UTF-8");

    while(!out.atEnd())
    {
        line = out.readLine();
        if (line.contains(tmpTitle))
        {
            line = line.trimmed();
            in << line << '\n';
            while(!out.atEnd())
            {
                line = out.readLine();
                line = line.trimmed();
                if (line.contains(tmpTitleEnd))
                {
                    break;
                }
//                if ((pos = line.indexOf(tmpHref)) != -1)
//                {
//                    line.insert(pos + tmpHref.length(), csdnBlogUrl);
//                }
                in << line << '\n';
            }
            break;
        }
    }

    while(!out.atEnd())
    {
        line = out.readLine();
        if (line.contains(tmpArticle))
        {
            in << line << '\n';
            while(!out.atEnd())
            {
                line = out.readLine();
                if (line.contains(tmpArticleEnd))
                {
                    break;
                }
                //搜索图片网址 <img src="http://my.csdn.net/uploads/201204/11/1334158895_1149.jpg" alt="">
                if (line.contains(tmpImage))
                {
                    if ((pos1 = line.indexOf(tmpImage1)) != -1)
                    {
                        pos1 += tmpImage1.length();
                        tmp = line.right(line.length() - pos1);
                        pos2 = tmp.indexOf('"');
                        tmpImageUrl = tmp.left(pos2);
                        if (tmpImageUrl.isEmpty())
                        {
                            continue;
                        }
                        //http://my.csdn.net/uploads/201204/11/1334158895_1149.jpg
                        pos1 = tmpImageUrl.lastIndexOf('/');
                        tmpImageName = tmpImageUrl.right(tmpImageUrl.length() - pos1 - 1);

                        BlogImageInfo *image = new BlogImageInfo;
                        image->imageUrl = tmpImageUrl;
                        image->imageName = tmpImageName;
                        image->downloadState = false;
                        m_blogimageVector->push_back(image);//添加图片信息

                        line = line.replace(tmpImageUrl, m_blogImageDir + QDir::separator() + tmpImageName);
                    }
                }

                //循环判断同一行的代码注释,可能在同一行就结束了代码区域
                while(1)
                {
                    if ((pos1 = line.indexOf(tmpCode)) != -1)
                    {
                        isCode = true;
                        line = line.right(line.length() - pos1);
                    }
                    else
                    {
                        break;
                    }

                    if ((pos2 = line.indexOf(tmpCodeend)) != -1)
                    {
                        isCode = false;
                        line = line.right(line.length() - pos2);
                    }
                    else
                    {
                        break;
                    }
                }
                //不同区域的换行符不同
                if (isCode)
                {
                    in << line << "<br>";
                }
                else
                {
                    in << line << '\n';
                }
            }
            break;
        }
    }

    srcBlogFile.close();
    dstBlogFile.close();

    //若没有图片,则直接生成pdf
    if (m_blogimageVector->isEmpty())
    {
        createPdf();
    }
    else
    {
        //开始下载图片
        emit startDownloadAllImage();
    }
}

void DealThread::createPdf()
{
    QChar separator = QDir::separator();
    QString blogHtmlName(m_blogSaveDir + separator + m_destBlogFile);
    QString blogPdfName(m_blogPdfDir + separator + m_blogPdfName);
    QFile blogHtmlFile(blogHtmlName);
    QPrinter outputPdf;
    QTextDocument outputHtml;
    QString str;

    //处理图片文件格式不匹配真实格式的问题
    restoreImageFormat(blogHtmlName);

    if (!blogHtmlFile.open(QIODevice::ReadOnly))
    {
        emit dealThreadError(blogHtmlFile.errorString());
        return;
    }

    QTextStream out(&blogHtmlFile);
    out.setCodec("UTF-8");

    outputPdf.setPageSize(QPrinter::A3);
    outputPdf.setOutputFormat(QPrinter::PdfFormat);
    outputPdf.setOutputFileName(blogPdfName);

    str = out.readAll();
    outputHtml.setHtml(str);
    outputHtml.print(&outputPdf);

    blogHtmlFile.close();

    //通知主线程生成pdf成功
    emit createPdfDone();
}

void DealThread::updateSqlDb()
{
    int state;
    QString tmp, sqlStr;

    //将下载状态导入到数据库
    QSqlQuery query(*m_pBlogSqlDatabase);

    foreach(int key, m_downloadBlogMap.keys())
    {
        //判断线程是否要停止
        if (m_stopped)
        {
            return;
        }

        state = m_downloadBlogMap.value(key);

        if (DownloadState_Failed == state)
        {
            tmp = tr("下载失败");
        }
        else if (DownloadState_Success == state)
        {
            tmp = tr("下载成功");
        }
        else
        {
            continue;
        }

        sqlStr = QString("select download from blog_info where num=%2").arg(key);
        query.exec(sqlStr);
        query.next();

        //只有不相等再写数据库,否则不用重复写
        if (query.value(0).toString().compare(tr("下载成功")) != 0)
        {
            sqlStr = QString("update blog_info set download='%1' where num=%2").arg(tmp).arg(key);

            query.exec(sqlStr);

            emit updateSqlDbProgress();
        }
        else
        {
            emit updateSqlDbProgress();
            continue;
        }
    }

    emit updateSqlDbDone();
}

void DealThread::restoreImageFormat(const QString &file)
{
    int pos;
    const char *BMP_TYPE = ".bmp";
    const char *JPG_TYPE = ".jpg";
    const char *GIF_TYPE = ".gif";
    const char *PNG_TYPE = ".png";
    QChar separator = QDir::separator();
    QString format, htmlContent, imageName, imageRootName;
    QFile blogHtmlFile(file);

    if (!blogHtmlFile.open(QIODevice::ReadWrite))
    {
        emit dealThreadError(blogHtmlFile.errorString());
        return;
    }

    QTextStream inout(&blogHtmlFile);
    inout.setCodec("UTF-8");
    htmlContent = inout.readAll();

    for(BlogImageInfoVector::Iterator it = m_blogimageVector->begin();
        it != m_blogimageVector->end();
        ++it)
    {
        imageName = (*it)->imageName;
        pos = imageName.lastIndexOf('.');
        imageRootName = imageName.left(pos);
        if (getImageFormat(m_blogImageDir + separator + imageName, format))
        {
            if (!format.compare(BMP_TYPE))
            {
                imageRootName += BMP_TYPE;
            }
            else if (!format.compare(JPG_TYPE))
            {
                imageRootName += JPG_TYPE;
            }
            else if (!format.compare(GIF_TYPE))
            {
                imageRootName += GIF_TYPE;
            }
            else if (!format.compare(PNG_TYPE))
            {
                imageRootName += PNG_TYPE;
            }
            else
            {
                continue;
            }
        }
        else
        {
            break;
        }
        //将html文件中的图片名称替换成正确的
        htmlContent.replace(imageName, imageRootName);

        //重命名文件
        QFile::rename(m_blogImageDir + separator + imageName,
                      m_blogImageDir + separator + imageRootName);
    }

    //将文件长度设置为0,再重新写入
    blogHtmlFile.resize(0);
    blogHtmlFile.flush();
    inout << htmlContent;
    blogHtmlFile.close();
}

bool DealThread::getImageFormat(const QString &file, QString &output)
{
    QByteArray line;
    QFile imageFile(file);
    const char *BMP_TYPE = "BM";
    const char *JPG_TYPE = "JFIF";
    const char *GIF_TYPE = "GIF";
    const char *PNG_TYPE = "PNG";
    const char *UNKNOWN_TYPE = "unknown";

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    line = imageFile.readLine();

    if (line.contains(BMP_TYPE))
    {
        output.clear();
        output.append(".bmp");
    }
    else if (line.contains(JPG_TYPE))
    {
        output.clear();
        output.append(".jpg");
    }
    else if (line.contains(GIF_TYPE))
    {
        output.clear();
        output.append(".gif");
    }
    else if (line.contains(PNG_TYPE))
    {
        output.clear();
        output.append(".png");
    }
    else
    {
        output.clear();
        output.append(UNKNOWN_TYPE);
    }

    imageFile.close();

    return true;
}
