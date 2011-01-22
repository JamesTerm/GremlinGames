// GG_Framework.UI.OSG PosAttTransCallback.h
#pragma once
#include <osg/PositionAttitudeTransform>
#include <osg/NodeCallback>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{

			// this class is used to safely add actors during the Update Callback
			class FRAMEWORK_UI_OSG_API PosAttTransCallback : public osg::NodeCallback
			{
			private:
				GG_Framework::Base::AtomicT<osg::Vec3d> m_Position;
				GG_Framework::Base::AtomicT<osg::Quat>  m_Attitude;
				GG_Framework::Base::AtomicT<osg::Vec3d> m_Scale;
				osg::PositionAttitudeTransform* m_sceneParent;

			protected:
				virtual ~PosAttTransCallback() {}

			public:
				PosAttTransCallback(osg::PositionAttitudeTransform* sceneParent);
				void SetPosition(const osg::Vec3d& pos){m_Position = pos;}
				void SetAttitude(const osg::Quat&  att){m_Attitude = att;}
				void SetScale(const osg::Vec3d& scale){m_Scale = scale;}

				virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
			};

			class FRAMEWORK_UI_OSG_API ThreadUpdatedPosAttTransform : public osg::PositionAttitudeTransform
			{
			private:
				osg::ref_ptr<OSG::PosAttTransCallback> m_posAttTransCallback;
			public:
				ThreadUpdatedPosAttTransform()
				{
					m_posAttTransCallback = new PosAttTransCallback(this);
				}

				void SetPositionNextUpdate(const osg::Vec3& pos){m_posAttTransCallback->SetPosition(pos);}
				void SetAttitudeNextUpdate(const osg::Quat& att){m_posAttTransCallback->SetAttitude(att);}
				void SetScaleNextUpdate(const osg::Vec3& scale){m_posAttTransCallback->SetScale(scale);}
			};
		}
	}
}