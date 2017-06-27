#ifndef COMMON_H
#define COMMON_H

class QString;

//�������߳�
enum CtrlMainThread {
    CtrlMainThread_Init,
    CtrlMainThread_Pause,
    CtrlMainThread_Continue,
    CtrlMainThread_Finished
};

//�̴߳�������
enum CtrlThread {
    CtrlThread_AnalyseBlogRank,
    CtrlThread_AnalyseBlogPage,
    CtrlThread_Analyseblog,
    CtrlThread_CreatePdf,
    CtrlThread_UpdateSqlDb
};

//����״̬
enum DownloadState {
    DownloadState_Init,
    DownloadState_Current,
    DownloadState_Failed,
    DownloadState_Success
};

//����
enum MainThreadOp {
    MainThreadOp_Init,
    MainThreadOp_AnalyseBlog,
    MainThreadOp_DownloadBlog
};

//ͼ����Ϣ�Ľṹ��
typedef struct tagBlogImageInfo {
    QString imageUrl;
    QString imageName;
    bool downloadState;
} BlogImageInfo;

typedef QMap<int, DownloadState> DownloadStateMap;
typedef QVector<BlogImageInfo*> BlogImageInfoVector;

#endif // COMMON_H
