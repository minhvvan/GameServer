﻿#include "pch.h"
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

condition_variable cv;

void Producer()
{
	while (true)
	{
		//!1) Lock 잡기
		//! 2) 공유변수 값 수정
		//! 3) Lock 풀기
		//! 4) 조건변수 통해 다른 쓰레드에게 통지
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		cv.notify_one();//wait 중인 thread 하나만 깨우기

		this_thread::sleep_for(10000ms);
	}
}

void Consumer()
{
	while (true)
	{
		//Manual Reset (false) --> 자동으로 signal이 꺼짐
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() { return q.empty() == false; });
		//!1) Lock 잡기
		//! 2) 조건확인 --> 만족하면 진행
		//!                    --> 만족하지 않으면 locK 풀고 대기

		//while(q.empty() == false){
		{
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