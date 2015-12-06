#include "Util.h"
#include "Platform.h"


PMutex::PMutex() {
    m_locked = false;
#if EMU_PLATFORM == PLATFORM_LINUX
    while(pthread_mutex_init(&m_mutex, NULL) != 0);
#elif EMU_PLATFORM == PLATFORM_WINDOWS
    while((m_mutex = CreateMutex(NULL, FALSE, NULL)) == NULL);
#endif
}

PMutex::~PMutex() {
#if EMU_PLATFORM == PLATFORM_LINUX
    while(pthread_mutex_destroy(&m_mutex) != 0);
#elif EMU_PLATFORM == PLATFORM_WINDOWS
    CloseHandle(m_mutex);
#endif
}

void PMutex::Lock() {
#if EMU_PLATFORM == PLATFORM_LINUX
    pthread_mutex_lock(&m_mutex);
#elif EMU_PLATFORM == PLATFORM_WINDOWS
    WaitForSingleObject(m_mutex, INFINITE);
#endif
    m_locked = true;
}

bool PMutex::IsLocked() {
    return m_locked;
}

void PMutex::Unlock() {
    m_locked = false;
#if EMU_PLATFORM == PLATFORM_LINUX
    pthread_mutex_unlock(&m_mutex);
#elif EMU_PLATFORM == PLATFORM_WINDOWS
    ReleaseMutex(m_mutex);
#endif
}
