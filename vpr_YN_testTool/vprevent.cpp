#include "vprevent.h"


VPREVENT::VPREVENT(quint32 dataMsgID, int nIndex) : QEvent((Type)(User + 2)), m_nDataMsgId(dataMsgID), m_nIndex(nIndex)
{

}
