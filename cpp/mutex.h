#pragma once

#ifdef _MSC_VER

class Mutex {
public:
    Mutex() {
        hmutex = CreateMutex(0, 0, 0);
    }
    void acquire() {
        while(WAIT_OBJECT_0 != WaitForSingleObject(hmutex, INFINITE)) {
        }
    }
    void release() {
        ReleaseMutex(hmutex);
    }
private:
    HANDLE hmutex;
};

#else

class Mutex {
    public:
        Mutex() {
            pthread_mutex_init(&mutex, NULL);
        }
        void acquire() {
            pthread_mutex_lock(&mutex);
        }
        void release() {
            pthread_mutex_unlock(&mutex);
        }
    private:
        pthread_mutex_t mutex;
};

#endif
