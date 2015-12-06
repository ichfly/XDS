#include "Platform.h"

#if EMU_PLATFORM == PLATFORM_LINUX
#include <pthread.h>
#elif EMU_PLATFORM == PLATFORM_WINDOWS
#include <windows.h>
#endif

class PMutex {
public:
    PMutex();
    ~PMutex();

    void Lock();
    bool IsLocked();
    void Unlock();
private:
    bool m_locked;

#if EMU_PLATFORM == PLATFORM_LINUX
    pthread_mutex_t m_mutex;
#elif EMU_PLATFORM == PLATFORM_WINDOWS
    HANDLE m_mutex;
#endif
};
