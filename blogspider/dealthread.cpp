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

    //�������ݵı���
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
    //��������,��������,������utf8��ʽ
    in.setCodec("UTF-8");

    //�ж��Ƿ���ڸò���
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(noPageError))
        {
            emit dealThreadError(tr("���û���δ��ͨ����"));
            return;
        }
    }
    in.seek(0);

    //��ȡ���ͱ���
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

    //��ȡ��������
    line = in.readLine();
    rx.setPattern("<h2>(.*)</h2>");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogDescription = rx.cap(1);
    }

    //��ȡ������ҳ���Ͳ�������
    while(!in.atEnd())
    {
        line = in.readLine();
        if(line.contains(page))
        {
            //������������ҳ��
            line = in.readLine();
            //�õ� <span> 54������  ��6ҳ</span>
            rx.setPattern("<span>(.*)</span>");
            pos = 0;
            if ((pos = rx.indexIn(line, pos)) >= 0)
            {
                tmp = rx.cap(1);
            }
            //�� "<span> 54������  ��6ҳ</span>"�����������
            rx.setPattern("\\d+");
            pos = 0;
            //����
            if ((pos = rx.indexIn(tmp, pos)) >= 0)
            {
                blogTotal = rx.cap(0);
                pos += rx.matchedLength();
            }
            //ҳ��
            if ((pos = rx.indexIn(tmp, pos)) >= 0)
            {
                blogPageTotal = rx.cap(0);
            }

            hasBlog = true;

            break;
        }
    }
    //�Ҳ���"��ҳ��",��ô��Ҫ�жϸ�ҳ���м�ƪ����
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
        //���ڲ���
        if (hasBlog)
        {
            blogTotal.setNum(count);
            blogPageTotal.setNum(1);
        }
        in.seek(0);
    }

    //��ȡ����ͷ����ַ
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(image))
        {
            break;
        }
    }
    //������Сƥ��,�����������ʽ��Ҫ������
    rx.setMinimal(true);
    //�ﵽͷ����ַ��ǰ2��
    line = in.readLine();//����
    line = in.readLine();
    rx.setPattern("<img src=\"(.*)\"");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogImageUrl = rx.cap(1);
        //http://avatar.csdn.net/B/A/E/1_gzshun.jpg ��ȡ�� 1_gzshun.jpg
        pos = blogImageUrl.lastIndexOf('/');
        blogImageName = blogImageUrl.right(blogImageUrl.length()-pos-1);
    }

    //��ȡ����������Ϣ
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(rank))
        {
            //����������Ϣ��λ�ã�����ѭ����ʼ����
            break;
        }
    }
    //����
    line = in.readLine();
    // <li>���ʣ�<span>87698��</span></li>
    rx.setPattern("<span>(.*)��");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogView = rx.cap(1);
    }
    //����
    line = in.readLine();
    // <li>���֣�<span>1753��</span></li>
    rx.setPattern("<span>(.*)��");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogScore = rx.cap(1);
    }
    //����
    line = in.readLine();
    // <li>������<span>��2946��</span></li>
    rx.setPattern("<span>��(.*)��");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {

        blogRanking = rx.cap(1);
    }
    else
    {
        blogRanking.setNum(0);
    }
    //�����м伸��������Ϣ
    while(!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(statistics))
        {
            break;
        }
    }
    //ԭ��
    line = in.readLine();
    // <li>ԭ����<span>48ƪ</span></li>
    rx.setPattern("<span>(.*)ƪ");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogOriginal = rx.cap(1);
    }
    //ת��
    line = in.readLine();
    // <li>ת�أ�<span>6ƪ</span></li>
    rx.setPattern("<span>(.*)ƪ");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogReship = rx.cap(1);
    }
    //����
    line = in.readLine();
    // <li>���ģ�<span>0ƪ</span></li>
    rx.setPattern("<span>(.*)ƪ");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogTranslation = rx.cap(1);
    }
    //����
    line = in.readLine();
    // <li>���ۣ�<span>349��</span></li>
    rx.setPattern("<span>(.*)��");
    pos = 0;
    if (rx.indexIn(line, pos) >= 0)
    {
        blogComment = rx.cap(1);
    }

    file.close();

    //�������õ�����Ϣ���͸����߳�
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
    int sqlCount = 0;//���ݿ��б�����
    QRegExp rx;
    QString line;
    QString tmp;
    QString title("<span class=\"link_title\">");//<span class="link_title">
    QString date("<span class=\"link_postdate\">");//<span class="link_postdate">

    //��������
    QString blogUrl;
    QString blogTitle;
    QString blogDate;
    QString blogView;
    QString blogComment;

    //�ж��߳��Ƿ�Ҫֹͣ
    if (m_stopped)
    {
        return;
    }

    QFile file(m_blogSaveDir + QDir::separator() + m_analyseFile);

    if (!m_pBlogSqlDatabase->isOpen())
    {
        emit dealThreadError(tr("�߳��޷���ʼ�����ݿ�,���߳����ݿ�û�д�"));
        return;
    }
    QSqlQuery query(*m_pBlogSqlDatabase);
    QString sqlStr;

    sqlStr = QString("select count(*) from blog_info");
    query.exec(sqlStr);
    query.next();
    sqlCount = query.value(0).toInt();//�õ���ǰ���ݿ��б�ĸ���

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit dealThreadError(file.errorString());
        return;
    }

    QTextStream in(&file);
    //��������,��������
    in.setCodec("UTF-8");

    while(!in.atEnd())
    {
        line = in.readLine();
        //�õ�������ַ
        if (line.contains(title))
        {
            rx.setPattern("href=\"(.*)\">");
            pos = title.length();//�����ж���γ���
            if (rx.indexIn(line, pos) >= 0)
            {
                blogUrl = "http://blog.csdn.net" + rx.cap(1);
            }

            //�õ����ͱ���
            line = in.readLine();
            if (line.contains(tr("[�ö�]")))
            {
                line = in.readLine();
            }
            blogTitle = line.trimmed();
            //��windowsƽ̨��,����һЩ�ַ�Ϊ�������ַ�
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
                //�õ����ͷ�������
                if (line.contains(date))
                {
                    rx.setPattern(">(.*)</");
                    pos = date.length();
                    if (rx.indexIn(line, pos) >= 0)
                    {
                        blogDate = rx.cap(1);
                    }

                    //�õ������Ķ�����
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
                    //�õ��������۴���
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

                    //ÿƪ���¶�Ҫ����һ��,����ȡ������Ϣ���͵����߳�
                    emit sendBlogInfo(blogTitle);

                    //��ӽ����ݿ���
                    sqlStr = QString("insert into blog_info values(%1, '%2', '%3', %4, %5, '%6', '%7')")
                            .arg(1 + sqlCount++).arg(blogTitle).arg(blogDate)
                            .arg(blogView).arg(blogComment).arg(blogUrl).arg(tr("δ����"));
                    query.exec(sqlStr);

                    //�������ѭ��
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
                //����ͼƬ��ַ <img src="http://my.csdn.net/uploads/201204/11/1334158895_1149.jpg" alt="">
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
                        m_blogimageVector->push_back(image);//���ͼƬ��Ϣ

                        line = line.replace(tmpImageUrl, m_blogImageDir + QDir::separator() + tmpImageName);
                    }
                }

                //ѭ���ж�ͬһ�еĴ���ע��,������ͬһ�оͽ����˴�������
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
                //��ͬ����Ļ��з���ͬ
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

    //��û��ͼƬ,��ֱ������pdf
    if (m_blogimageVector->isEmpty())
    {
        createPdf();
    }
    else
    {
        //��ʼ����ͼƬ
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

    //����ͼƬ�ļ���ʽ��ƥ����ʵ��ʽ������
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

    //֪ͨ���߳�����pdf�ɹ�
    emit createPdfDone();
}

void DealThread::updateSqlDb()
{
    int state;
    QString tmp, sqlStr;

    //������״̬���뵽���ݿ�
    QSqlQuery query(*m_pBlogSqlDatabase);

    foreach(int key, m_downloadBlogMap.keys())
    {
        //�ж��߳��Ƿ�Ҫֹͣ
        if (m_stopped)
        {
            return;
        }

        state = m_downloadBlogMap.value(key);

        if (DownloadState_Failed == state)
        {
            tmp = tr("����ʧ��");
        }
        else if (DownloadState_Success == state)
        {
            tmp = tr("���سɹ�");
        }
        else
        {
            continue;
        }

        sqlStr = QString("select download from blog_info where num=%2").arg(key);
        query.exec(sqlStr);
        query.next();

        //ֻ�в������д���ݿ�,�������ظ�д
        if (query.value(0).toString().compare(tr("���سɹ�")) != 0)
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
        //��html�ļ��е�ͼƬ�����滻����ȷ��
        htmlContent.replace(imageName, imageRootName);

        //�������ļ�
        QFile::rename(m_blogImageDir + separator + imageName,
                      m_blogImageDir + separator + imageRootName);
    }

    //���ļ���������Ϊ0,������д��
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
