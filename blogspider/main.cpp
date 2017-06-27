#include <QtGui/QApplication>
#include <QTextCodec>
#include "blogspider.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    BlogSpider w;
    w.show();
    return a.exec();
}
