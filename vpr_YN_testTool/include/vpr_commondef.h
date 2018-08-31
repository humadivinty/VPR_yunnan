#ifndef VPR_COMMONDEF_H
#define VPR_COMMONDEF_H
#include<QObject>




//车牌识别事件回调函数定义。
typedef void (*Func_VPR_NotifyFunc)(quint32 uIndex);

#pragma pack(push, 1)
 typedef struct
{
    int nColor;					//	车牌颜色代码
    char* lpszPlate;			//	车牌号
    quint32 nRating;				//	车牌的可信度
    quint8* lpBigImgBuf;			//	车辆大图图像数据缓冲区
    quint32 nBigImgSize;			//	车辆大图图像大小
    quint8* lpSmallImgBuf;		//	车牌小图图像数据缓冲区
    quint32 nSmallImgSize;			//	车牌小图图像大小
    quint8* lpBinImgBuf;			//	二值化图像数据缓冲区
    quint32 nBinImgSize;			//	二值化图像数据大小
    quint32 nBinImgWidth;			//	二值化图像宽度
    quint32 nBinImgHeight;			//	二值化图像高度
}CAutoDetectedVehInfo;
#pragma pack(pop)



#endif // VPR_COMMONDEF_H
