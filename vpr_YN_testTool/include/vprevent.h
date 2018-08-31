#ifndef VPREVENT_H
#define VPREVENT_H

#include<QEvent>

class VPREVENT : public QEvent
{
public:
   explicit VPREVENT(quint32 dataMsgID, int nIndex);

public:
    quint32 m_nDataMsgId;
    int m_nIndex;
};

#endif // VPREVENT_H
