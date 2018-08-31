#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include <string>
#include <list>
#include <map>
#include<windef.h>
//#define MY_SPRINTF sprintf_s

#define SAFE_DELETE_OBJ(obj) \
if (NULL != obj)                                  \
{                                           \
    delete obj;                        \
    obj = NULL;                      \
}

#define SAFE_DELETE_ARRAY(arrayObj) \
if (NULL != arrayObj)                                  \
{                                           \
    delete[] arrayObj;                        \
    arrayObj = NULL;                      \
}

//通过节点名查找并返回相应节点
//注：XMLTYPE 为1时，InputInfo为XML路径，当为2时,InputInfo为二进制文件内容
TiXmlElement Tool_SelectElementByName(const char* InputInfo, const char* pName, int iXMLType);

TiXmlElement* Tool_ReadElememt(TiXmlElement* InputElement, const char* pName);

bool Tool_GetElementTextByName(const char* InputInfo, const char* pName, int iXMLType, char* chTextValueBuffer, size_t& bufferLength);

bool Tool_InsertElementByName(const char* InputInfo, const char* pName, int iXMLType, const char* nodeName, const char* textValue, std::string& outputString);

void Tool_ReadKeyValueFromConfigFile(const char* FileName, const char* nodeName, const char* keyName, char* keyValue, size_t bufferSize);

void Tool_ReadIntValueFromConfigFile(const char* FileName, const char* nodeName, const char* keyName, int&keyValue);

void Tool_WriteKeyValueFromConfigFile(const char* FileName, const char* nodeName, const char* keyName, char* keyValue, size_t bufferSize);

//检查IP的有效性
int Tool_checkIP(const char* p);

bool Tool_IsFileExist(const char* FilePath);

bool Tool_IsDirExist(const char* Dir);

bool Tool_MakeDir(const char* chImgPath);

long Tool_GetFileSize(const char *FileName);

bool Tool_PingIPaddress(const char* IpAddress);

//bool Tool_Img_ScaleJpg(PBYTE pbSrc, int iSrcLen, PBYTE pbDst, size_t *iDstLen, int iDstWidth, int iDstHeight, int compressQuality);

//int Tool_GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

void Tool_ExcuteShellCMD(char* pChCommand);

bool Tool_ExcuteCMDbyCreateProcess(const char* CmdName);

//bool Tool_OverlayStringToImg(unsigned char** pImgsrc, long srcSize,
//    unsigned char** pImgDest, long& DestSize,
//    const wchar_t* DestString, int FontSize,
//    int x, int y, int colorR, int colorG, int colorB,
//    int compressQuality);

bool Tool_GetDataFromAppenedInfo(char *pszAppendInfo, std::string strItemName, char *pszRstBuf, int *piRstBufLen);

void Tool_ExcuteCMD(char* pChCommand);

std::wstring Tool_string2wstring(std::string strSrc);

#ifdef  USE_MFC
//删除指定文件夹
bool DeleteDirectory(const char* strDirName);

//删除指定目录中以 'yyyy-mm-dd' 方式命名的文件夹，其中限定的条件为时间早于指定天数
int CirclelaryDelete(const char* folderPath, int iBackUpDays);
#endif


int Tool_SafeCloseThread(HANDLE& threadHandle);

const char* Tool_GetCurrentPath();
#ifdef USE_MSVC
SYSTEMTIME Tool_GetCurrentTime();
#endif

bool Tool_DimCompare(const char *szSrcPlateNo, const char *szDesPlateNo);

void Tool_WriteLog(const char*);

void Tool_WriteFormatLog(const char* szfmt, ...);

bool Tool_SaveFileToPath(const char* szPath, void* fileData, size_t fileSize);

std::list<std::string> getFilesPath(const std::string& cate_dir, const std::string& filter);

bool Tool_LoadFile(const char* fileName, void* pBuffer, size_t& fileSize);

bool Tool_LoadCamerXml(const char* fileName, std::map<std::string, int>& myMap);

void Tool_DeleteFileByCMD(const char* chFileName);

bool Tool_FindMapAndGetValue(std::map<std::string, int>& myMap, std::string keyName, int& value);

#ifdef WIN32
std::string Tool_GetSoftVersion(const char* exepath);
#endif
