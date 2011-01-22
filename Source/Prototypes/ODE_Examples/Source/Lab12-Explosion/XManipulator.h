#ifndef SGI_DEMO_SIM_MANIPULATOR_H
#define SGI_DEMO_SIM_MANIPULATOR_H

#include <osgGA/GUIEventHandler>

#include "Application.h"
#include "Explosion.h"

class OSGGA_EXPORT XManipulator : public osgGA::GUIEventHandler
{
    public:

        XManipulator(Application &app): _app(app) {}

        virtual ~XManipulator() {}

        virtual const char* className() const { return "XManipulator"; }

        /** Handle events, return true if handled, false otherwise.*/
        virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
        {
            if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
            {
                switch( ea.getKey() )
                {
					case 'c':
						_app.toggleCow();
						break;

					case 'p':
						_app.togglePlane();
						break;

					case 'r':
						_app.trigger(false, ea.time());
						break;

                    case 'x':
						_app.trigger(true, ea.time());
                        break;
                }
                return false;
            }
            return false;
        }


        virtual void getUsage(osg::ApplicationUsage& usage) const
        {
		}

        /** Handle visitations */
        virtual void accept(osgGA::GUIEventHandlerVisitor&gehv)
        {
            gehv.visit(*this);
        }
		
	private:
		Application &_app;
};

#endif
