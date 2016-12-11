#include <iostream>
#include <windows.h>

using namespace std;
int number = 0;

class Event{
    HANDLE hEvent;
public:
    Event() {
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        cout << endl << "ThreadCreate ID: " << GetCurrentThreadId() << endl;
    }

    ~Event() {
        cout << endl << "ThreadDestoyd ID: " << GetCurrentThreadId() << endl;
        CloseHandle(hEvent);
    }

    void set() {
        SetEvent(hEvent);
        cout << endl << "ThreadSet ID: " << GetCurrentThreadId() << endl;
    }

    void reset(){
        ResetEvent(hEvent);
    }

    void wait() {
        cout << endl << "WaitThread ID: " << GetCurrentThreadId() << endl;
        WaitForSingleObject(hEvent, INFINITE);
    }

};


class Lock{
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual ~Lock() {}
};

class CriticalSection : public Lock {
private:
    CRITICAL_SECTION CS;
public:
    CriticalSection() {
        InitializeCriticalSection(&CS);
    }

    ~CriticalSection() {
        //
    }


    void lock() {
        EnterCriticalSection(&CS);
    }

    void unlock() {
        LeaveCriticalSection(&CS);
    }

};



class Semaphor : public Lock{
private:
    int maxCount;
    int count;
    CriticalSection *tCS;
    Event *event;
public:
    Semaphor(int maxCount) : maxCount(maxCount), count(0){
        tCS = new CriticalSection();
        event = new Event();
    }

    ~Semaphor(){
        delete event;
        delete tCS;
    }

    void lock(){
        tCS->lock();
        if(count++ < maxCount){
            tCS->unlock();
            return;
        }
        tCS->unlock();
        event->wait();
    }

    void unlock(){
        tCS->lock();
        if(count-- >= maxCount){
            event->set();
        }
        tCS->unlock();
    }
};


Semaphor *SEMAPHOR;

void inc(void){
    cout << endl << "INC = " << number++ << endl;
}

DWORD WINAPI Foo(PVOID){ // PVOID = void*
    while(int i = 5){
        SEMAPHOR->lock();
        inc();
        Sleep(2000);
        SEMAPHOR->unlock();
        i--;
    }
    return 0;
}

int main(){
    cout << "Enter number: ";
    cin >> number;

    HANDLE ThreadArray[20];
    SEMAPHOR = new Semaphor(5);
    for(register int i=0; i<20; i++){
        ThreadArray[i] = CreateThread(NULL, 0, Foo, NULL, 0, NULL);
    }

    DWORD result_stop_thread = WaitForMultipleObjects(20, ThreadArray, true, INFINITE);

    switch (result_stop_thread){
        case WAIT_TIMEOUT:{
            cout << "Stopping timeout!" << endl;
            break;
        }

        case WAIT_OBJECT_0:{
            cout << "Thread " << GetCurrentThreadId() << "reading from buffer" << endl;
            break;
        }
        default:{
            cout << "Error: " << GetLastError() << endl;
            break;
        }
    }
    /*
    DWORD WINAPI WaitForMultipleObjects(
        __in  DWORD nCount, число элементов в массиве объектов(перехода в сигнальное состояние которых надо ждать),
        переданном на месте следующего параметра lpHandles
        максимальное число таких объектов определяется константой MAXIMUM_WAIT_OBJECTS.
        Данный параметр не может быть=0
        __in  const HANDLE *lpHandles, // массив объектов
        __in  BOOL bWaitAll, // если установлено TRUE , то функция будет ждать включения в сигнальное состояние
        всех объектов, если же FALSE  -то любого  одного, который включится первым
        __in  DWORD dwMilliseconds максимальное время ожидания
    );
    */

    /*
    Сначала при помощи Semaphor *SEMAPHOR; создали семафор, и его дескриптор присваивается глобальной переменной.
    Пред попыткой обращения к ресурсам доступ, к которым необходимо ограничить, поток должен вызвать функцию WaitForSingleObject.
    При открытии доступа функция возвращает 0. По окончании работы с ресурсом следует вызвать функцию SEMAPHOR.unlock();
    Тем самым увеличивается счётчик доступа на 1.
    */

    for (register int i=0; i<20; i++){
        CloseHandle(ThreadArray[i]);
    }

    delete SEMAPHOR;
    SEMAPHOR = NULL;

    return 0;
}
