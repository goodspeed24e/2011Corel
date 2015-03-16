#ifndef _DISPSVR_READER_WRITER_LOCK_H_
#define _DISPSVR_READER_WRITER_LOCK_H_

/// Simple and incomplete reader/writer lock:
/// The lock is to allow multiple readers to access a resource.
/// Writer can only gain access when there is no reader locking.
class CReaderWriterLock
{
public:
    CReaderWriterLock()
    {
        //InitializeSRWLock(&m_SRWLock);
        InitializeCriticalSection(&m_csLock);
    }

    ~CReaderWriterLock()
    {
    }

    void ReaderLock()
    {
        //AcquireSRWLockShared(&m_SRWLock);
        EnterCriticalSection(&m_csLock);
    }

    void ReaderUnlock()
    {
        //ReleaseSRWLockShared(&m_SRWLock);
        LeaveCriticalSection(&m_csLock);
    }

    void WriterLock()
    {
        //AcquireSRWLockExclusive(&m_SRWLock);
        EnterCriticalSection(&m_csLock);
    }

    void WriterUnlock()
    {
        //ReleaseSRWLockExclusive(&m_SRWLock);
        LeaveCriticalSection(&m_csLock);

    }

protected:
    //SRWLOCK m_SRWLock;
    CRITICAL_SECTION m_csLock;
};

#endif  // _DISPSVR_READER_WRITER_LOCK_H_