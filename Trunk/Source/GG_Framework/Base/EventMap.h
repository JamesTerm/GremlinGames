// GG_Framework.Base EventMap.h
#pragma once
#include <map>
#include <vector>
#include <stdio.h>
#include <xmlParser.h>
#include <osgGA\GUIEventAdapter>

namespace GG_Framework
{
	namespace Base
	{
		//! Writes a File out with debug Frame Details
		//! Other things can register themselves with the information
		//! Allows other registering items to register to EACH of the events
		//! Writes a CSV (comma separated values) file that can be opened and graphed in EXCEL
		class FRAMEWORK_BASE_API FrameLogger
		{
		public:
			FrameLogger(std::string logFileName);
			~FrameLogger();
			void WriteLog();

			//! Called between time updates, with prior time and current time, ONLY when active
			Event2<double, double> TimerEvent;

			//! Write out your title, using a comma before 
			Event1<FILE*> WriteTitles;
			Event3<FILE*, double, double> WriteValues;

			void ToggleActive(){m_active = !m_active;}
			bool IsActive(){return m_active;}

			void ListenForToggleEvent(Event0& ev);
			void ListenForTimerUpdate(Event1<double>& timerEv);

		private:
			void TimeChanged(double t);
			double m_lastTime;
			std::vector<double>* m_currRecordedTimeSet;
			std::vector<std::vector<double>*> m_allRecordedTimes;
			bool m_active;
			IEvent::HandlerList ehl;
			std::string m_logFileName;
		};
		//////////////////////////////////////////////////////////////////////////

		//! Use this to log any value on a TimerLogger and write its value
		class FRAMEWORK_BASE_API ValueLogger
		{
		public:
			ValueLogger(std::string itemName) : m_itemName(itemName), V(0.0), m_writeIndex(0) {}
			double V; //! Set this value to write on the next frame
			void SetLogger(FrameLogger& fl);

		private:
			void WriteTitles(FILE* logFile);
			void TimerEvent(double lastTime, double currTime);
			void WriteValues(FILE* logFile, double lastTime, double currTime);

			IEvent::HandlerList ehl;
			std::string m_itemName;
			std::vector<double> m_valSet;
			unsigned m_writeIndex;
		};
		//////////////////////////////////////////////////////////////////////////

		class FRAMEWORK_BASE_API ProfilingLogger : public ValueLogger
		{
		public:
			ProfilingLogger(std::string itemName);

			// Call these to write on the next frame
			void Start();
			void End();

		private:
			__int64 begC, endC, freq;
		};
		//////////////////////////////////////////////////////////////////////////

		class FRAMEWORK_BASE_API Timer
		{
		public:
			Timer(std::string logFileName);
			virtual ~Timer();

			//! Returns the time increment since the last time Fired
			virtual double FireTimer() = 0;

			virtual double ConvertFromNetTime(unsigned int netTime_ms) = 0;
			virtual unsigned int ConvertToNetTime(double time_s) = 0;

			// All of these return the current time plus the scrub
			double GetCurrTime_s(){return _currentTime_s;}
			Event1<double> CurrTimeChanged;

			// FOr debugging only, Sync TImer may be smoothed
			virtual double GetSynchronizedActualTime() {return GetCurrTime_s();}

			// Does not fire the timer event on the change.  Expects the next update to handle it
			// This should be atomic across threads if needed
			void SetCurrTime_NoEvent(double currTime_s){_currentTime_s = currTime_s;}

			FrameLogger Logger;

		protected:
			double SetCurrTime_s(double t_s);
			double IncTime_s(double i_s);

		private:
			double _currentTime_s;
		};
		//////////////////////////////////////////////////////////////////////////

      class FRAMEWORK_BASE_API Key
      {
         public:
            int key, flags;
         Key (int _key, int _flags = 0) : key(_key), flags(_flags)
         {
            if (isShiftChar(key))
            {
               key = unShiftChar(key);
               flags |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
            }
         }

         enum { DBL = 0x4000 }; // next bit up from osgGA::GUIEventAdapter::MODKEY_CAPS_LOCK = 0x2000

         class KeyStringMaps
         {
            public:
               std::map<int, std::string> keyStringMap;
               std::map<std::string, int> stringKeyMap;
            KeyStringMaps ();
         };
         static KeyStringMaps KEY_STRING_MAPS;

         std::string getKeyString ()
         {
            if (KEY_STRING_MAPS.keyStringMap.find(key) != KEY_STRING_MAPS.keyStringMap.end())
               return KEY_STRING_MAPS.keyStringMap[key];
            else
               return "";
         }

         bool isShiftChar (char c); // returns true if the character requires a shift
         char unShiftChar (char c); // returns the unshifted version of a shift character

         bool operator >  (const Key& rhs) const { return ((key == rhs.key) ? (flags > rhs.flags) : (key > rhs.key)); }
         bool operator == (const Key& rhs) const { return (key == rhs.key) && (flags == rhs.flags); }
      };

      struct FRAMEWORK_BASE_API UserInputEvents
		{
			Event2<Key, bool> KBCB_KeyDnUp;
			Event1<Key> KBCB_KeyDn;
			
			Event2<float, float> MouseMove;
			Event1<int> MouseScroll;
		};
		//////////////////////////////////////////////////////////////////////////

		class FRAMEWORK_BASE_API EventMap
		{
		public:
			EventMap(bool listOwned = false) : 
			m_listOwned(listOwned), m_KB_Controlled(false), AlternateEventTime(NULL)
			{}

			bool IsListOwned(){return m_listOwned;}

			Event2<EventMap*, bool> KB_Controlled;
			bool IsKB_Controlled(){return m_KB_Controlled;}
			void SetKB_Controlled(bool controlled);

			// These are the events that get fired by the KBMCB when we are connected
			UserInputEvents KBM_Events;

			std::map<std::string,Event0,std::greater<std::string> > Event_Map;
			std::map<std::string,Event1<bool>,std::greater<std::string> > EventOnOff_Map;
			std::map<std::string,Event1<double>,std::greater<std::string> > EventValue_Map;

			// Some things are interested in the TIME that an event is fired.  
			// Usually, this TIME is the current time on whatever timer is being used, BUT
			// In situations when events are being fired due to some network event, we want to be able to 
			// specify an event time that is earlier than the current time.
			// 
			// Rather than the huge amount of re-write that would be required to pass in the time to every event fired
			// This public member variable can hold a different event time for those that might be interested.
			// This value should be a pointer to the appropriate time, then reset back to NULL immediately after
			// firing said events.
			double* AlternateEventTime;

		private:
			// Fired for key press and key release from the KBMCB
			bool m_KB_Controlled;
			bool m_listOwned;
		};	// EventMap
		//////////////////////////////////////////////////////////////////////////

	}
}


//! This class is used to hold a collection of EventMaps (onr for each ActorParent) and 
//! Takes care of deleting them when done.
class FRAMEWORK_BASE_API EventMapList : public std::vector<GG_Framework::Base::EventMap*>
{
public:
	~EventMapList()
	{
		// Delete all of the Event Maps this list owns
		for (unsigned i = 0; i < size(); ++i)
		{
			if ((*this)[i]->IsListOwned())
				delete (*this)[i];
		}
	}
};