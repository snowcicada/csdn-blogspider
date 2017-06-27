#ifndef COMMON_H
#define COMMON_H

class QString;

//控制主线程
enum CtrlMainThread {
    CtrlMainThread_Init,
    CtrlMainThread_Pause,
    CtrlMainThread_Continue,
    CtrlMainThread_Finished
};

//线程处理事情
enum CtrlThread {
    CtrlThread_AnalyseBlogRank,
    CtrlThread_AnalyseBlogPage,
    CtrlThread_Analyseblog,
    CtrlThread_CreatePdf,
    CtrlThread_UpdateSqlDb
};

//下载状态
enum DownloadState {
    DownloadState_Init,
    DownloadState_Current,
    DownloadState_Failed,
    DownloadState_Success
};

//操作
enum MainThreadOp {
    MainThreadOp_Init,
    MainThreadOp_AnalyseBlog,
    MainThreadOp_DownloadBlog
};

//图像信息的结构体
typedef struct tagBlogImageInfo {
    QString imageUrl;
    QString imageName;
    bool downloadState;
} BlogImageInfo;

typedef QMap<int, DownloadState> DownloadStateMap;
typedef QVector<BlogImageInfo*> BlogImageInfoVector;

#endif // COMMON_H
