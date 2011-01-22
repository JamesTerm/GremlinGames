#ifndef __Event_Included_
#define __Event_Included_

#include <list>

class IEvent
{
protected:
	class IEventHandler;
public:
	virtual ~IEvent(){}

	class HandlerList
	{

		friend IEvent::IEventHandler;

	public:
		~HandlerList()
		{
			GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
			std::list<IEvent::IEventHandler*>::iterator pos;
			for (pos = _handlerList.begin(); pos != _handlerList.end(); ++pos)
			{
				IEventHandler* eh = *pos;
				eh->_ehl = NULL;		// Set to NULL to avoid circular problems
				IEvent& event = eh->_event;
				event.RemoveEventHandler(eh);	// Note that this will DELETE the eh
			}
			_handlerList.clear();
		}
	protected:
		OpenThreads::Mutex m_mutex;

	private:
		std::list<IEvent::IEventHandler*>	_handlerList;
		void AddEventHandler(IEvent::IEventHandler* eh){
			GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.push_back(eh);}
		void RemoveEventHandler(IEvent::IEventHandler* eh){
			GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.remove(eh);}
	};

protected:

	class IEventHandler
	{
	public:
		IEventHandler(IEvent& event, IEvent::HandlerList& ehl) : 
		  _event(event), _ehl(&ehl) {_ehl->AddEventHandler(this);}
		  virtual ~IEventHandler() {if (_ehl) _ehl->RemoveEventHandler(this);}
	private:
		IEvent::HandlerList* _ehl;
		IEvent& _event;

		friend HandlerList;
	};

	virtual void RemoveEventHandler(IEventHandler* eh) = 0;

	friend HandlerList;
	OpenThreads::Mutex m_mutex;

};

class Event0 : public IEvent
{
public:

	virtual ~Event0() {ClearAllHandlers();}

	void ClearAllHandlers()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler0*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); ++pos)
		{
			IEventHandler0* eh = *pos;
			delete eh;
		}
		_handlerList.clear();
	}

	void Fire()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler0*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); )
		{
			IEventHandler0* eh = *pos;
			++pos;	// Placed here in case Firing the event removes this element
			eh->Fire();
		}
	}

	template<class T>
		void Subscribe(IEvent::HandlerList& ehl, T& client, void (T::*delegate)())
	{GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.push_back(new EventHandler0<T>(*this, ehl, client, delegate));}

	template<class T>
		void Remove(T& client, void (T::*delegate)())
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler0*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			EventHandler0<T>* posPtr = dynamic_cast<EventHandler0<T>*>(*pos);
			if ((posPtr) && (&posPtr->_client == &client) && (posPtr->_delegate == delegate))
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

protected:
	/// Only called from EventHandlerList when it is destroyed
	virtual void RemoveEventHandler(IEvent::IEventHandler* eh)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler0*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			IEventHandler0* posPtr = *pos;
			if (posPtr == eh)
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

private:
	class IEventHandler0 : public IEventHandler
	{
	public:
		IEventHandler0(Event0& event, IEvent::HandlerList& ehl) : IEventHandler(event, ehl) {}
		virtual void Fire() = 0;
	};

	template<class T>
	class EventHandler0 : public IEventHandler0
	{
	public:
		EventHandler0(Event0& event, IEvent::HandlerList& ehl, T& client, void (T::*delegate)()) : 
		  IEventHandler0(event, ehl), _client(client), _delegate(delegate) {}
		  virtual void Fire(){GG_Framework::Base::RefMutexWrapper rmw(m_mutex); (_client.*_delegate)();}

		  T& _client;
		  void (T::*_delegate)();
	};

	std::list<IEventHandler0*>	_handlerList;
};




template<class P1>
class Event1 : public IEvent
{
public:

	virtual ~Event1() {ClearAllHandlers();}

	void ClearAllHandlers()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler1*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); ++pos)
		{
			IEventHandler1* eh = *pos;
			delete eh;
		}
		_handlerList.clear();
	}

	void Fire(P1 p1)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler1*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); )
		{
			IEventHandler1* eh = *pos;
			++pos;	// Placed here in case Firing the event removes this element
			eh->Fire(p1);
		}
	}

	template<class T>
		void Subscribe(IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1))
	{GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.push_back(new EventHandler1<T>(*this, ehl, client, delegate));}

	template<class T>
		void Remove(T& client, void (T::*delegate)(P1 p1))
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler1*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			EventHandler1<T>* posPtr = dynamic_cast<EventHandler1<T>*>(*pos);
			if ((posPtr) && (&posPtr->_client == &client) && (posPtr->_delegate == delegate))
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

protected:
	/// Only called from EventHandlerList when it is destroyed
	virtual void RemoveEventHandler(IEvent::IEventHandler* eh)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler1*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			IEventHandler1* posPtr = *pos;
			if (posPtr == eh)
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

private:
	class IEventHandler1 : public IEventHandler
	{
	public:
		IEventHandler1(Event1& event, IEvent::HandlerList& ehl) : IEventHandler(event, ehl) {}
		virtual void Fire(P1 p1) = 0;
	};

	template<class T>
	class EventHandler1 : public IEventHandler1
	{
	public:
		EventHandler1(Event1& event, IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1)) : 
		  IEventHandler1(event, ehl), _client(client), _delegate(delegate) {}
		  virtual void Fire(P1 p1){GG_Framework::Base::RefMutexWrapper rmw(m_mutex); (_client.*_delegate)(p1);}

		  T& _client;
		  void (T::*_delegate)(P1 p1);
	};

	std::list<IEventHandler1*>	_handlerList;
};





template<class P1, class P2>
class Event2 : public IEvent
{
public:

	virtual ~Event2() {ClearAllHandlers();}

	void ClearAllHandlers()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler2*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); ++pos)
		{
			IEventHandler2* eh = *pos;
			delete eh;
		}
		_handlerList.clear();
	}

	void Fire(P1 p1, P2 p2)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler2*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); )
		{
			IEventHandler2* eh = *pos;
			++pos;	// Placed here in case Firing the event removes this element
			eh->Fire(p1, p2);
		}
	}

	template<class T>
		void Subscribe(IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1, P2 p2))
	{GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.push_back(new EventHandler2<T>(*this, ehl, client, delegate));}

	template<class T>
		void Remove(T& client, void (T::*delegate)(P1 p1, P2 p2))
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler2*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			EventHandler2<T>* posPtr = dynamic_cast<EventHandler2<T>*>(*pos);
			if ((posPtr) && (&posPtr->_client == &client) && (posPtr->_delegate == delegate))
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

protected:
	/// Only called from EventHandlerList when it is destroyed
	virtual void RemoveEventHandler(IEvent::IEventHandler* eh)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler2*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			IEventHandler2* posPtr = *pos;
			if (posPtr == eh)
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

private:
	class IEventHandler2 : public IEventHandler
	{
	public:
		IEventHandler2(Event2& event, IEvent::HandlerList& ehl) : IEventHandler(event, ehl) {}
		virtual void Fire(P1 p1, P2 p2) = 0;
	};

	template<class T>
	class EventHandler2 : public IEventHandler2
	{
	public:
		EventHandler2(Event2& event, IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1, P2 p2)) : 
		  IEventHandler2(event, ehl), _client(client), _delegate(delegate) {}
		  virtual void Fire(P1 p1, P2 p2){GG_Framework::Base::RefMutexWrapper rmw(m_mutex); (_client.*_delegate)(p1, p2);}

		  T& _client;
		  void (T::*_delegate)(P1 p1, P2 p2);
	};

	std::list<IEventHandler2*>	_handlerList;
};



template<class P1, class P2, class P3>
class Event3 : public IEvent
{
public:

	virtual ~Event3() {ClearAllHandlers();}

	void ClearAllHandlers()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler3*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); ++pos)
		{
			IEventHandler3* eh = *pos;
			delete eh;
		}
		_handlerList.clear();
	}

	void Fire(P1 p1, P2 p2, P3 p3)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler3*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end(); )
		{
			IEventHandler3* eh = *pos;
			++pos;	// Placed here in case Firing the event removes this element
			eh->Fire(p1, p2, p3);
		}
	}

	template<class T>
		void Subscribe(IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1, P2 p2, P3 p3))
	{GG_Framework::Base::RefMutexWrapper rmw(m_mutex); _handlerList.push_back(new EventHandler3<T>(*this, ehl, client, delegate));}

	template<class T>
		void Remove(T& client, void (T::*delegate)(P1 p1, P2 p2, P3 p3))
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler3*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			EventHandler3<T>* posPtr = dynamic_cast<EventHandler3<T>*>(*pos);
			if ((posPtr) && (&posPtr->_client == &client) && (posPtr->_delegate == delegate))
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

protected:
	/// Only called from EventHandlerList when it is destroyed
	virtual void RemoveEventHandler(IEvent::IEventHandler* eh)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex); 
		std::list<IEventHandler3*>::iterator pos;
		for (pos = _handlerList.begin(); pos != _handlerList.end();)
		{
			IEventHandler3* posPtr = *pos;
			if (posPtr == eh)
			{
				pos = _handlerList.erase(pos);
				delete posPtr;
			}
			else
				pos++;
		}
	}

private:
	class IEventHandler3 : public IEventHandler
	{
	public:
		IEventHandler3(Event3& event, IEvent::HandlerList& ehl) : IEventHandler(event, ehl) {}
		virtual void Fire(P1 p1, P2 p2, P3 p3) = 0;
	};

	template<class T>
	class EventHandler3 : public IEventHandler3
	{
	public:
		EventHandler3(Event3& event, IEvent::HandlerList& ehl, T& client, void (T::*delegate)(P1 p1, P2 p2, P3 p3)) : 
		  IEventHandler3(event, ehl), _client(client), _delegate(delegate) {}
		  virtual void Fire(P1 p1, P2 p2, P3 p3){GG_Framework::Base::RefMutexWrapper rmw(m_mutex); (_client.*_delegate)(p1, p2, p3);}

		  T& _client;
		  void (T::*_delegate)(P1 p1, P2 p2, P3 p3);
	};

	std::list<IEventHandler3*>	_handlerList;
};



#endif // __Event_Included_