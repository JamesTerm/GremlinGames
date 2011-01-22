// GG_Framework.UI CameraParentedTransforms.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

		class FRAMEWORK_UI_API CameraParentedTransform : public osg::PositionAttitudeTransform
		{
		public:
			CameraParentedTransform(GG_Framework::UI::OSG::ICamera* cam);
			void CameraUpdateCallback(const osg::Matrix& camMatrix);

		private:
			IEvent::HandlerList ehl;
		};

		/// An ScaledTransform is a PositionAttitudeTransform that is used for large objects in the
		/// scene that would cause clipping and precision errors when in the same scene with other
		/// objects.  The object is pseudo-parented to the camera's position, in that its position is
		/// adjusted in the update callback to be an offset from the camera's position.  The scale
		/// is used to make it so the camera may eventually reach the object by tricking the object's
		/// position.  If the scale is 0, the object will never be reached (like a distant star field.
		/// The render bin is used to make sure these large objects are rendered before the rest
		/// of the scene.  ScaledTransform with a scale of 0 are in render bin -2, and the others
		/// are in render bin -1.
		class FRAMEWORK_UI_API ScaledTransform : public osg::PositionAttitudeTransform
		{
		public:
			ScaledTransform(GG_Framework::UI::OSG::ICamera& cam, double adjScale);
			void CameraUpdateCallback(const osg::Matrix& camMatrix);

		private:
			double m_adjScale;
			IEvent::HandlerList ehl;
		};

		/// An LightBloomTransform is a PositionAttitudeTransform that is used for engine glows, explosions, etc.
		/// When the camera is closer to the object than the nominal distance, no scaling is performed.  When the camera is between the nominal
		/// distance and the far distance, the object is scaled so that its apparent size on the screen is the same as at the
		/// nominal distance.  Beyond the far distance, the object will scale down as normal.  For instance, if the nominal distance
		/// is 10 meters and the far distance is 1,000 meters, and the camera is 300 meters away, the object will be scaled to 
		/// 30x its original size.
		class FRAMEWORK_UI_API LightBloomTransform : public osg::PositionAttitudeTransform
		{
		public:
			LightBloomTransform(GG_Framework::UI::OSG::ICamera& cam, double nomDist, double farDist);
			void CameraUpdateCallback(const osg::Matrix& camMatrix);

		private:
			double m_nomDist, m_farDist;
			IEvent::HandlerList ehl;
		};
	}
}