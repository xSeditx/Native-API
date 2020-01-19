#include"native_thread.h"

/*
#ifdef _CRT_WINDOWS
#ifdef _WIN64
#define _Mtx_internal_imp_size		32
#define _Mtx_internal_imp_alignment	8
#define _Cnd_internal_imp_size		16
#define _Cnd_internal_imp_alignment	8
#else // _WIN64 
#define _Mtx_internal_imp_size		20
#define _Mtx_internal_imp_alignment	4
#define _Cnd_internal_imp_size		8
#define _Cnd_internal_imp_alignment	4
#endif // _WIN64 
#else // _CRT_WINDOWS 
#ifdef _WIN64
#define _Mtx_internal_imp_size		80
#define _Mtx_internal_imp_alignment	8
#define _Cnd_internal_imp_size		72
#define _Cnd_internal_imp_alignment	8
*/
namespace native {
	namespace {

		int Mutex_init(CriticalSection *_mtx, int)
		{
			_mtx = new CriticalSection();
			return 0;
		}
		void Mutex_destroy(CriticalSection& _mtx) 
		{
			delete(&_mtx);
		}
		void    Mutex_init_in_situ(CriticalSection& _mtx, int) 
		{

		}
		void    Mutex_destroy_in_situ(CriticalSection& _mtx) {}
		int     Mutex_current_owns(CriticalSection& _mtx) { return 0; }
		int     Mutex_lock(CriticalSection& _mtx) { return 0; }
		int     Mutex_trylock(CriticalSection& _mtx) { return 0; }
		int     Mutex_timedlock(CriticalSection& _mtx, const size_t *) { return 0; }
		int     Mutex_unlock(CriticalSection& _mtx) { return 0; }
		void *  Mutex_getconcrtcs(CriticalSection& _mtx) { return nullptr; }
		void    Mutex_clear_owner(CriticalSection& _mtx) {}
		void    Mutex_reset_owner(CriticalSection& _mtx) {}
	}
}