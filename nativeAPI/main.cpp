#include"Threading/native_thread.h"

#include<vector>

native::mutex Mtx;
std::vector<uint32_t> ThreadVector;
void Thread_TestFunction()
{
	for (uint32_t i{ 0 }; i < 100; ++i)
	{
		Mtx.lock();
		ThreadVector.push_back(ThreadVector.back() + 1);
		Mtx.unlock();
	}
}

int main()
{
	ThreadVector.push_back(1);
	native::thread Thread0(Thread_TestFunction);   /*                                       */
	native::thread Thread1(Thread_TestFunction);   /*       Spawn a Number of Threads       */
	native::thread Thread2(Thread_TestFunction);   /*                                       */
	native::thread Thread3(Thread_TestFunction);   /*                                       */
	native::thread Thread4(Thread_TestFunction);   /*                                       */
	native::thread Thread5(Thread_TestFunction);   /*                                       */
	Thread0.wait();                                /*                                       */
	Thread1.wait();                                /*                                       */
	Thread2.wait();                                /*       Make sure they                  */
	Thread3.wait();                                /*       are Finished                    */
	Thread4.wait();                                /*                                       */
	Thread5.wait();                                /*                                       */
	for (int i{ 1 }; i < ThreadVector.size(); ++i) /*   Test Order to Ensure Functionality  */
	{
		if (ThreadVector[i] != ThreadVector[i - 1] + 1)
		{
			THREAD_ERROR("Threading Test Failed. Elements Out Of Order ");
		}
	}
}