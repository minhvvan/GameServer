#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

class SpinLock {
public:
	void lock() 
	{
		bool expected = false;
		bool desired = true;

		while (_locked.compare_exchange_strong(expected, desired) == false) {
			expected = false;

			//!100ms 대기
			this_thread::sleep_for(100ms);

			//!일단 지금은 양보하고 대기큐로
			//this_thread::yield();
		}
	}

	void unlock()
	{
		_locked.store(false);
	}

private:
	atomic<bool> _locked = false;
};

mutex m;
queue<int32> q;
HANDLE handle;


void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		::SetEvent(handle);

		this_thread::sleep_for(10000ms);
	}
}

void Consumer()
{
	while (true)
	{
		//Manual Reset (false) --> 자동으로 signal이 꺼짐
		::WaitForSingleObject(handle, INFINITE);

		unique_lock<mutex> lock(m);
		if(q.empty() == false){
			int32 data = q.front();
			q.pop();
			cout << data << "\n";
		}
	}
}

int main()
{
	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*ManualReset*/, FALSE/*bInitalState*/, NULL);

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}