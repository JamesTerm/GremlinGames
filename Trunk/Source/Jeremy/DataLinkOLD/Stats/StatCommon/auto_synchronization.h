#pragma once

#include "synchronization.h"

// Macro for automatic synchronization, including calls to sync_begin_result and additional_test_succeeded.
#ifndef AUTO_SYNCHRONIZE
#define AUTO_SYNCHRONIZE(type, name, p_synchronization, p_sync_begin, p_sync_if_failed_additional_test, p_sync_end, additional_test, if_failed) \
	auto_synchronization<type> auto_sync_##name(p_synchronization, p_sync_begin, p_sync_if_failed_additional_test, p_sync_end); \
	if (auto_sync_##name.sync_begin_result() == 0 || !auto_sync_##name.additional_test_succeeded(additional_test)) \
	{ \
		if_failed; \
	}
#endif // AUTO_SYNCHRONIZE

// template class auto_synchronization
// Template for class that automatically locks and unlocks a series of critical sections.
template <typename type>
class auto_synchronization
{
public:
	auto_synchronization(synchronization<type> *sync = NULL, const sync_op *sync_begin = NULL, const sync_op *sync_if_failed = NULL, const sync_op *sync_end = NULL);
	virtual ~auto_synchronization();

	short sync_begin_result() const;
	bool additional_test_succeeded(bool additional_test);

protected:
	synchronization<type> *m_sync;
	const sync_op *m_sync_begin;
	const sync_op *m_sync_if_failed_additional_test;
	const sync_op *m_sync_end;
	short m_sync_begin_result;
	bool m_additional_test;
};

// Constructor
template <typename type>
auto_synchronization<type>::auto_synchronization(synchronization<type> *sync = NULL, const sync_op *sync_begin = NULL,
												 const sync_op *sync_if_failed_additional_test = NULL, const sync_op *sync_end = NULL):
	m_sync(sync),
	m_sync_begin(sync_begin),
	m_sync_if_failed_additional_test(sync_if_failed_additional_test),
	m_sync_end(sync_end),
	m_sync_begin_result(1),
	m_additional_test(true)
{
	if (m_sync != NULL && m_sync_begin != NULL)
		m_sync_begin_result = m_sync->synchronize(m_sync_begin);
}

// Destructor
template <typename type>
auto_synchronization<type>::~auto_synchronization()
{
	if (m_sync != NULL)
	{
		if (!m_additional_test)
		{
			if (m_sync_if_failed_additional_test != NULL)
				m_sync->synchronize(m_sync_if_failed_additional_test);
		}
		else if (m_sync_begin_result != 0)
		{
			if (m_sync_end != NULL)
				m_sync->synchronize(m_sync_end);
		}
	}
}

// sync_begin_result
// Tells whether the sequence of locks and unlocks during construction succeeded.
template <typename type>
short auto_synchronization<type>::sync_begin_result() const
{
	return m_sync_begin_result;
}

// additional_test_succeeded
// Sets whether an additional condition checked outside of the auto_synchronization class succeeded.
template <typename type>
bool auto_synchronization<type>::additional_test_succeeded(bool additional_test)
{
	return (m_additional_test = additional_test);
}
