#include"Threading/native_thread.h"

#include<vector>
//https://c9x.me/x86/html/file_module_x86_id_26.html
native::mutex Mtx;
std::vector<uint32_t> ThreadVector;

void Thread_TestFunction()
{
	//Print("Active Thread: " << native::thread::ActiveThread());
	//Print("Get Thread: "    << native::thread::GetThread());

 	std::mutex A;
	A.lock();
	Print("Hello");
	A.unlock();
	Print("Stack Size: " << native::thread::get_StackSize() / 1024 << " In Thread" <<  native::thread::ActiveThread());
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
	Thread0.wait();                                /*       Make sure they                  */
	Thread1.wait();                                /*       are Finished                    */
	Thread2.wait();                                /*                                       */
	Thread3.wait();                                /*                                       */
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


#include<shared_mutex>
#include<future>
#include<mutex>