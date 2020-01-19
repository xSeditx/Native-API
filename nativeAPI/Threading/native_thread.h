#pragma once
/*======================================================================================================================================*/
/*                                 Copyright 2020 Sedit
/*
/*        Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/*    and associated documentation files(the "Software"), to deal in the Software without restriction,
/*    including without limitation the rights to use, copy, modify, merge, publish, distribute,
/*    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
/*    is furnished to do so, subject to the following conditions :
/*
/*    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/*
/*        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
/*    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
/*    PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
/*    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
/*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
/*======================================================================================================================================*/

/*======================================================================================================================================*/
/*  This Library is Intended to be a bear bones drop in replacement for STL threading and Synchronization Library
/*  This is a WIP and all code is subject to change.
/*  Anyone considering using this in its current form in their project is highly discouraged from doing so
/*======================================================================================================================================*/
#include <cstdint>
#include <iostream>
#include <Windows.h>

#ifdef _DEBUG
#    define    DEBUG_CODE(x)   x
#else
#    define    DEBUG_CODE(x)
#endif

#define Print(x)              std::cout << x << "\n"
#define THREAD_ERROR(x)       DEBUG_CODE(Print("Error: ~" << x << "~"); Print("Code: " << GetLastError() << "In Thread: " << GetCurrentThreadId()); __debugbreak())                         //                                    //
#define SIZEOF_WIN_MUTEX      80
#define DEFAULT_SPIN_COUNT    1024

#if __cplusplus > 201703L
#    define NO_DISCARD            [[nodiscard]]
#else
#    define NO_DISCARD  
#endif




namespace native
{// Namespace for various system specific stuff
#ifdef WIN32 // The following is WINDOWS Specific API
	enum
	{   /* Return codes */
		Thread_success,
		Thread_nomemory,
		Thread_timedout,
		Thread_busy,
		Thread_error
	};
	enum
	{	/* Mutex types */
		Mutex_plain = 0x01,
		Mutex_try = 0x02,
		Mutex_timed = 0x04,
		Mutex_recursive = 0x100
	};

	/* Class for Thread Handling
	/* Creates and Utilizes a Thread Object on
	/* Windows to allow multi processor execution */
	class thread
	{
		/* Remove the ability to copy this object */
		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;
	public:

		/* Create Default Empty thread object */
		thread() noexcept
		{
			id = 0;
			Handle = 0;
			StackSize = 0;
		}

		/* Waits for the Thread to return then Destroys it */
		~thread() noexcept
		{
			wait();
			ExitThread(0);
		}

		/* Move constructor */
		thread(thread&& _other) noexcept
		{
			if (joinable())
			{// Why are we testing this?
				THREAD_ERROR("Attempting to overwrite Initialized thread object");
			}
			*this = move(_other);
			_other.id = 0;
			_other.Handle = 0;
			_other.StackSize = 0;
		}

		/* Assign this thread by moving another into its location */
		thread& operator=(thread&& other) noexcept
		{
			Handle = move(Handle);
			StackSize = move(StackSize);
			id = move(id);
		}

		/* Create a thread that runs a specified function with the provided arguments */
		template<typename _F, typename... ARGS>
		explicit thread(_F&& _func, ARGS&&...args)
		{
			Handle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)_func, NULL, NULL, (LPDWORD)&id);
		}

		/* Waits for Function to return before moving on */
		void wait()
		{
			WaitForSingleObject(Handle, INFINITE);
		}

		/* Waits for N seconds before proceeding */
		void wait(uint32_t _N)
		{
			WaitForSingleObject(Handle, (DWORD)_N);
		}

		/* Gets pointer to OS Thread Object */
		NO_DISCARD void* GetThread()
		{
			return (void*)GetCurrentThread();
		}

		/* Test to see if we can join our thread */
		NO_DISCARD bool joinable()
		{
			return (
				!(
					id == NULL &&
					Handle == NULL &&
					StackSize == NULL)
				);
		}

		/* Waits for thread to Return before proceeding */
		void join()
		{
			wait();
		}

		/* Detaches Thread from currently running process */
		bool detach()
		{
			if (!joinable())
			{
				THREAD_ERROR("Invalid Argument");
			}
			bool results = CloseHandle(Handle);
			id = 0;
			Handle = 0;
			StackSize = 0;
			return results;
		}

		/* Swaps other thread with this one */
		void swap(thread& _other)
		{
			std::swap(*this, _other);// std::swap(_Thr, _Other._Thr);
		}

		/* Returns the OS id of the Thread */
		NO_DISCARD uint32_t get_id()
		{
			return id;
		}

		/* Returns the OS Handle of the Thread */
		NO_DISCARD void* native_handle()
		{
			return Handle;
		}

		/* Check the amount of cores the system has to use */
		NO_DISCARD static unsigned int hardware_concurrency() noexcept
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			return sysinfo.dwNumberOfProcessors;
		}

		/* Return the Currently Active Calling Thread
		~WARNING~: Currently Produces Errors Not Functional */
		NO_DISCARD static uint32_t ActiveThread() noexcept
		{
			return GetCurrentThreadId();
		}

		NO_DISCARD static size_t get_StackSize()
		{
			size_t H{ 0 }, L{ 0 };
			GetCurrentThreadStackLimits(&L, &H);
			return H - L;
		}
		 
			/* STL Freedom */
		template< class T > struct remove_reference      { typedef T type; };
		template< class T > struct remove_reference<T&>  { typedef T type; };
		template< class T > struct remove_reference<T&&> { typedef T type; };
		template< class T > constexpr typename remove_reference<T>::type&& move(T&& t) noexcept;
	private:

		uint32_t id;
		HANDLE Handle;
		size_t StackSize;
		//mutex Mtx;
	};


	/* CRITICAL SECTION
	/* Provides user with exclusion abilities on a
	/* Thread to ensure data is not stomped on   */
	struct CriticalSection
	{
		PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
		uint32_t LockCount;
		uint32_t RecursionCount;
		//HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
		uint32_t OwningThread;        // from the thread's ClientId->UniqueThread
		HANDLE LockSemaphore;
		size_t SpinCount;

		/* Constructs Critical section */
		CriticalSection() noexcept
			:
			SpinCount(0),
			RecursionCount(0),
			LockCount(0)
		{
			OwningThread = thread::ActiveThread();
			InitializeCriticalSectionEx(&Handle, DEFAULT_SPIN_COUNT, NULL);
		}

		/* Create a critical section with a specific spin count */
		CriticalSection(size_t _spin, uint32_t _flags = NULL) noexcept
		{
			OwningThread = thread::ActiveThread();
			InitializeCriticalSectionAndSpinCount(&Handle, _spin);
			InitializeCriticalSectionEx(&Handle, _spin, _flags);

			SpinCount       = Handle.SpinCount;
			RecursionCount  = Handle.RecursionCount;
			LockCount       = Handle.LockCount;
			LockSemaphore   = Handle.LockSemaphore;
			DebugInfo       = Handle.DebugInfo;

			//if (OwningThread != Handle.OwningThread)
			//{
			//	THREAD_ERROR("Critical sections Owning thread does not match record");
			//}
		}

		/* Exits and Destroys our Critical section object */
		~CriticalSection()
		{
			while (RecursionCount--)
			{// Exits inorder the amount of times our thread has been locked to ensure no deadlocks
				exit();
			}
			DeleteCriticalSection(&Handle);
		}

		/* Attempts to enter the critical section returns false on fail */
		bool try_enter()
		{
			if (TryEnterCriticalSection(&Handle) == true)
			{
				++RecursionCount;
				return true;
			}
			return false;
		}

		/* Enters Critical section */
		void enter()
		{
			EnterCriticalSection(&Handle);
			++RecursionCount;
		}

		/* Exits the critical section */
		void exit()
		{
			--RecursionCount;
			LeaveCriticalSection(&Handle);
		}

		/* Sets the sections Spin count */
		size_t set_spincount(size_t _spin)
		{
			return SetCriticalSectionSpinCount(&Handle, _spin);
		}

		/* Implement the Locked Callback system with LeaveCritical Section when Callback returns */

		bool isOwned()
		{
			return !(Handle.OwningThread == nullptr);
		}

		bool hasOwnership(uint32_t _tid)
		{
			return (native::thread::ActiveThread() == GetThreadId(Handle.OwningThread));
		}

		void Clear()
		{
			SpinCount = 0;
			RecursionCount = 0;
			LockCount = 0;
			LockSemaphore = 0;
			DebugInfo = 0;

			Handle.SpinCount = 0;
			Handle.RecursionCount = 0;
			Handle.LockCount = 0;
			Handle.LockSemaphore = 0;
			Handle.DebugInfo = 0;
 		}

		CriticalSection& Copy(const CriticalSection& _other)
		{
			CriticalSection results;
			results = _other;
			return results;
		}	
		CriticalSection& Move(CriticalSection& _other)
		{    
			CriticalSection results;
			results = _other;
			_other.Clear();
			return results;
		}

 

		CRITICAL_SECTION Handle;// Basically all the above Information.

	};


	/* MUTEX BASEOBJECT
	/* Parent of all Native Mutex Objects which
	/* are designed to ensure mutual exclusion   */
	struct MutexBase
	{	// base class for all mutex types

		/* Construct Default with flags */
		MutexBase(int _flags = 0) noexcept
		{
			Handle = CriticalSection(DEFAULT_SPIN_COUNT, _flags);
		}

		/* Will handle the destruction of our Critical Section */
		virtual ~MutexBase() noexcept
		{}

		MutexBase(MutexBase&& _other)
		{
			__debugbreak();// WAIT I DONT THINK I WANT TO DO THIS YET MIGHT NEED TO DELETE THE PREVIOUS
		}
		/* Locks the Mutex, Waits if can not Lock */
		void lock()
		{
			Handle.enter();
		}

		/* Attempts to Lock our Mutex if not currently Locked */
		NO_DISCARD bool try_lock()
		{
			return Handle.try_enter();
		}

		/* Unlocks our Mutex Object */
		void unlock()
		{
			Handle.exit();
		}

		CriticalSection& get_handle()
		{
			return Handle;
		}



		MutexBase& Copy(const MutexBase& _other)
		{
			MutexBase results;
		//	results.Handle = move(_other.Handle.Copy(_other.Handle);
			return results;
		}

		MutexBase&  Move(MutexBase& _other)
		{	// move stored associated asynchronous state object from _Other
			MutexBase results;
			results.Handle = _other.Handle;
			_other.Handle.Clear();
		}

	private:
 		MutexBase(const MutexBase&) = delete;
		MutexBase& operator=(const MutexBase&) = delete;

		CriticalSection Handle;
	};


	/* Class for Mutual Exclusion
	/* Creates and Utilizes a Critical Section on
	/* Windows to ensure Data remains protected   */
	struct mutex
		: MutexBase
	{
		mutex() noexcept
			:
			MutexBase()
		{}

		mutex(const mutex&) = delete;
		mutex& operator=(const mutex&) = delete;
	};




	/*
	/* Semaphore class
	/**/
	struct semaphore
	{
		semaphore() {
			Handle = CreateSemaphore(
				NULL,           // default security attributes
				Count,  // initial count
				100,  // maximum count
				NULL);          // unnamed semaphore

		}


		uint32_t release()
		{
			uint32_t result;
			ReleaseSemaphore(Handle, 1, (LPLONG)result);
			return result;
		}


		size_t Count{ 0 };
		HANDLE Handle{ nullptr };
	};






//class shared_mutex
//{	// class for mutual exclusion shared across threads
//public:
//
//	shared_mutex() noexcept
//			:
//			Handle(nullptr)
//		{}
//	~shared_mutex() noexcept
//		{}
//
//
//	void lock() noexcept
//	{	// lock exclusive
//		_Smtx_lock_exclusive(&Handle);
//	}
//
//	NO_DISCARD bool try_lock() noexcept
//		{	// try to lock exclusive
//			return (_Smtx_try_lock_exclusive(&Handle) != 0);
//		}
//
//	void unlock() noexcept
//		{	// unlock exclusive
//			_Smtx_unlock_exclusive(&Handle);
//		}
//
//	void lock_shared() noexcept
//		{	// lock non-exclusive
//			_Smtx_lock_shared(&Handle);
//		}
//
//	NO_DISCARD bool try_lock_shared() noexcept
//		{	// try to lock non-exclusive
//			return (_Smtx_try_lock_shared(&Handle) != 0);
//		}
//
//	void unlock_shared() noexcept
//	{	// unlock non-exclusive
//		_Smtx_unlock_shared(&Handle);
//	}
//
//	NO_DISCARD HANDLE native_handle() noexcept
//	{	// get native handle
//		return (&Handle);
//	}
//
//	shared_mutex(const shared_mutex&) = delete;
//	shared_mutex& operator=(const shared_mutex&) = delete;
//private:
//	HANDLE Handle; // typedef void * _Smtx_t;
//
//};

	namespace {

		int    Mutex_init(CriticalSection*, int);
		void   Mutex_destroy(CriticalSection&);
		void   Mutex_init_in_situ(CriticalSection&, int);
		void   Mutex_destroy_in_situ(CriticalSection&);
		int    Mutex_current_owns(CriticalSection&);
		int    Mutex_lock(CriticalSection&);
		int    Mutex_trylock(CriticalSection&);
		int    Mutex_timedlock(CriticalSection&, const size_t);
		int    Mutex_unlock(CriticalSection&);
		void * Mutex_getconcrtcs(CriticalSection&);
		void   Mutex_clear_owner(CriticalSection&);
		void   Mutex_reset_owner(CriticalSection&);
	}
#endif // WIN32
}

#include <ppl.h>

/*
/*=========================================================================================================================================================================================
/*											  NOTES:
/*=========================================================================================================================================================================================
/*  TBB 
/*  https://software.intel.com/en-us/node/506168
/*
/*  Microsoft Docs on STD::Thread
/*  https://docs.microsoft.com/en-us/cpp/standard-library/thread-class?view=vs-2019
/* 
/*  FIBERS:
/*  http://www.1024cores.net/home/lock-free-algorithms/tricks/fibers
/* 
/*  CONCURRENT PROGRAMMING ON WINDOWS
/*  https://books.google.com/books?id=o4ohrd0_yA0C&pg=PT248&lpg=PT248&dq=example+InitializeCriticalSectionAndSpinCount(+LPCRITICAL_SECTION+lpCriticalSection,+DWORD+dwSpinCount+);&source=bl&ots=pftmjtlNNk&sig=ACfU3U2pTqo-RQ0SQcuIFA4Kc11C5LRIVg&hl=en&sa=X&ved=2ahUKEwi8so7Qof_mAhWkp1kKHfpyCUgQ6AEwAnoECAoQAQ#v=onepage&q=example%20InitializeCriticalSectionAndSpinCount(%20LPCRITICAL_SECTION%20lpCriticalSection%2C%20DWORD%20dwSpinCount%20)%3B&f=false
/*========================================================================================================================================================================================*/

//  #if _THREAD_CHECKX
//  #    define _THREAD_QUOTX(x)	#x
//  #    define _THREAD_QUOT(x)	_THREAD_QUOTX(x)
//  #    define _THREAD_ASSERT(expr, msg)	((expr) ? (void)0 : _Thrd_abort(__FILE__ "(" _THREAD_QUOT(__LINE__) "): " msg))
//  #else 
//  #    define _THREAD_ASSERT(expr, msg)	((void)0)
//  #endif 
  