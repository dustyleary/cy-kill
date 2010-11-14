#pragma once

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
