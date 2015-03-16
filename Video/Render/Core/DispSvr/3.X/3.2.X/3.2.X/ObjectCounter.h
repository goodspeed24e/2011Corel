#ifndef _DISPSVR_OBJECT_COUNTER_H_
#define _DISPSVR_OBJECT_COUNTER_H_

namespace DispSvr
{
    template<typename T>
    class CObjectCounter
    {
    public:
        CObjectCounter()
        {
            InterlockedIncrement(&s_nObjectCount);
        }

        ~CObjectCounter()
        {
            InterlockedDecrement(&s_nObjectCount);
        }

        static LONG GetObjectCount()
        {
            return s_nObjectCount;
        }

    protected:
        static LONG s_nObjectCount;
    };

    template<typename T> LONG CObjectCounter<T>::s_nObjectCount = 0;
}

#endif  // _DISPSVR_OBJECT_COUNTER_H_