#pragma once

#include "FrameWork_Threads.h"

// Enumeration for synchronization operations
enum SYNC_OPS
{
	SYNC_RETURN = 0,
	SYNC_JUMP = 1,
	SYNC_LOCK = 2,
	SYNC_TRYLOCK = 3,
	SYNC_UNLOCK = 4
};

// struct sync_op
// A struct containing an operator and 2 operands, to be used with the synchronization classes.
struct sync_op
{
	short m_op;
	short m_param_1;
	short m_param_2;
};

// template class thread_sync
// Template for class that manages starting and stopping of Thread objects.
template <typename type>
class thread_sync
{
public:
	thread_sync();
	virtual ~thread_sync();

	const Thread<type> *THREAD_START(HANDLE *p_thread_event_handle, type *thread_type);
	void THREAD_STOP();
	bool THREAD_CHECK(DWORD wait_time_if_locked_ms);
	bool THREAD_LOCK(DWORD wait_time_if_locked_ms);
	void THREAD_UNLOCK();

protected:
	HANDLE *m_p_thread_event_handle;
	CriticalSection m_lock;
	Thread<type> *m_p_thread;
};

// Constructor
template <typename type>
thread_sync<type>::thread_sync():
	m_p_thread_event_handle(NULL),
	m_p_thread(NULL)
{
	m_lock.Lock();
}

// Destructor
template <typename type>
thread_sync<type>::~thread_sync()
{
}

// THREAD_START
// Starts a Thread.
template <typename type>
const Thread<type> *thread_sync<type>::THREAD_START(HANDLE *p_thread_event_handle, type *thread_type)
{
	*p_thread_event_handle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_p_thread_event_handle = p_thread_event_handle;

	m_lock.UnLock();

	return (m_p_thread = new Thread<type>(thread_type));
}

// THREAD_STOP
// Stops a Thread.
template <typename type>
void thread_sync<type>::THREAD_STOP()
{
	m_lock.Lock();

	::SetEvent(*m_p_thread_event_handle);

	delete m_p_thread;
	m_p_thread = NULL;

	::CloseHandle(*m_p_thread_event_handle);
	*m_p_thread_event_handle = NULL;
	m_p_thread_event_handle = NULL;
}

// THREAD_CHECK
// Calls THREAD_LOCK and returns whether it succeeded. If it succeeded, calls THREAD_UNLOCK
// before returning.
template <typename type>
bool thread_sync<type>::THREAD_CHECK(DWORD wait_time_if_locked_ms)
{
	if (!THREAD_LOCK(wait_time_if_locked_ms))
		return false;

	THREAD_UNLOCK();

	return true;
}

// THREAD_LOCK
// Returns whether a CriticalSection associated with a Thread is locked. If it's locked,
// optionally waits before returning.
template <typename type>
bool thread_sync<type>::THREAD_LOCK(DWORD wait_time_if_locked_ms)
{
	if (!m_lock.TryLock())
	{
		if (wait_time_if_locked_ms > 0)
			::Sleep(wait_time_if_locked_ms);

		return false;
	}

	return true;
}

// THREAD_UNLOCK
// Unlocks a CriticalSection associated with a Thread.
template <typename type>
void thread_sync<type>::THREAD_UNLOCK()
{
	m_lock.UnLock();
}

// template class synchronization
// Template for a class that manages an array of CriticalSection objects and an array of
// thread_sync objects.
template <typename type>
class synchronization
{
public:
	synchronization(short num_locks = 0, short num_thread_syncs = 0);
	virtual ~synchronization();

	short synchronize(const sync_op *sync_sequence);

	const Thread<type> *THREAD_START(short thread_index, HANDLE *p_thread_event_handle, type *thread_type);
	void THREAD_STOP(short thread_index);
	bool THREAD_CHECK(short thread_index, DWORD wait_time_if_locked_ms);
	bool THREAD_LOCK(short thread_index, DWORD wait_time_if_locked_ms);
	void THREAD_UNLOCK(short thread_index);

protected:
	CriticalSection *m_locks;
	short m_num_locks;

	thread_sync<type> *m_thread_syncs;
	short m_num_thread_syncs;
};

// Constructor
template <typename type>
synchronization<type>::synchronization(short num_locks = 0, short num_thread_syncs = 0):
	m_locks(NULL),
	m_num_locks(max(num_locks, 0)),
	m_thread_syncs(NULL),
	m_num_thread_syncs(max(num_thread_syncs, 0))
{
	if (m_num_locks > 0)
		m_locks = new CriticalSection[m_num_locks];
	if (m_num_thread_syncs > 0)
		m_thread_syncs = new thread_sync<type>[m_num_thread_syncs];
}

// Destructor
template <typename type>
synchronization<type>::~synchronization()
{
	if (m_thread_syncs != NULL)
		delete[] m_thread_syncs;
	if (m_locks != NULL)
		delete[] m_locks;
}

// synchronize
// Executes a series of locks and unlocks on CriticalSection objects.
template <typename type>
short synchronization<type>::synchronize(const sync_op *sync_sequence)
{
	short op_offset, op, param_1, param_2;

	for (short i = 0; true; i+= op_offset)
	{
		op = sync_sequence[i].m_op;
		param_1 = sync_sequence[i].m_param_1;
		param_2 = sync_sequence[i].m_param_2;
		op_offset = 1;

		switch (op)
		{
		case SYNC_RETURN:
			return param_1;
		case SYNC_JUMP:
			op_offset = param_1;
			break;
		case SYNC_LOCK:
			m_locks[param_1].Lock();
			break;
		case SYNC_TRYLOCK:
			if (!m_locks[param_1].TryLock())
				op_offset = param_2;
			break;
		case SYNC_UNLOCK:
			m_locks[param_1].UnLock();
			break;
		default:
			break;
		}
	}

	return 1;
}

// THREAD_START
// Calls THREAD_START on the thread_sync object located at a specified index.
template <typename type>
const Thread<type> *synchronization<type>::THREAD_START(short thread_index, HANDLE *p_thread_event_handle, type *thread_type)
{
	return m_thread_syncs[thread_index].THREAD_START(p_thread_event_handle, thread_type);
}

// THREAD_STOP
// Calls THREAD_STOP on the thread_sync object located at a specified index.
template <typename type>
void synchronization<type>::THREAD_STOP(short thread_index)
{
	m_thread_syncs[thread_index].THREAD_STOP();
}

// THREAD_CHECK
// Calls THREAD_CHECK on the thread_sync object located at a specified index.
template <typename type>
bool synchronization<type>::THREAD_CHECK(short thread_index, DWORD wait_time_if_locked_ms)
{
	return m_thread_syncs[thread_index].THREAD_CHECK(wait_time_if_locked_ms);
}

// THREAD_LOCK
// Calls THREAD_LOCK on the thread_sync object located at a specified index.
template <typename type>
bool synchronization<type>::THREAD_LOCK(short thread_index, DWORD wait_time_if_locked_ms)
{
	return m_thread_syncs[thread_index].THREAD_LOCK(wait_time_if_locked_ms);
}

// THREAD_UNLOCK
// Calls THREAD_UNLOCK on the thread_sync object located at a specified index.
template <typename type>
void synchronization<type>::THREAD_UNLOCK(short thread_index)
{
	m_thread_syncs[thread_index].THREAD_UNLOCK();
}
