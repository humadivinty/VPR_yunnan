#pragma once

#include "Camera/CameraResult.h"
#include<windows.h>
#include <memory>
#include <deque>
#include <mutex>


class MYLocker
{
public:
    MYLocker(CRITICAL_SECTION* inHandle) : m_handle(inHandle)
    {
        if (NULL != m_handle)
        {
            EnterCriticalSection(m_handle);
        }
    }

    ~MYLocker()
    {
        if (NULL != m_handle)
        {
            LeaveCriticalSection(m_handle);
            m_handle = NULL;
        }
    }
private:
    CRITICAL_SECTION* m_handle;
};

class ResultListManager
{
    typedef std::shared_ptr<CameraResult> Result_Type;
    typedef std::deque<Result_Type> list_Type;
public:
    ResultListManager();
    ~ResultListManager();

public:
    size_t size();
    bool empty();

    void front(Result_Type& result);
    Result_Type GetOneByIndex(int iPosition);
    Result_Type GetOneByCarid(DWORD dwCarID);
    void pop_front();
    void pop_back();
    void push_back(Result_Type result);

    int GetPositionByPlateNo(const char* plateNo);
    void DeleteToPosition(int position);

    std::string GetAllPlateString();
    
    void ClearALLResult();

        
private:
    list_Type m_list;
    std::mutex m_mtx;
    CRITICAL_SECTION m_hcs;

    friend class MYLocker;
};

