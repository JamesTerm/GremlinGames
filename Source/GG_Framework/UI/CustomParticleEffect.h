// GG_Framework.UI CustomParticleEffect.h
#pragma once
#include <osgParticle/SectorPlacer>

namespace GG_Framework
{
	namespace UI
	{
		class CustomParticleEffect : public IParticleEffect
		{
		public:
			struct Settings
			{
				float _emitterDuration;
				std::string _textureFileName;
				float _lifeConst, _lifeScale;
				osgParticle::rangef _sizeRange;
				osgParticle::rangef _alphaRange;
				osgParticle::rangev4 _colorRange;
				float _radiusScale;
				float _density; // 1.2kg/m^3
				float _movementCompensationRatio;
				bool _useLocalParticleSystem;
				bool _endless;
				float _placerRadiusChangeScaleMultMin,_placerRadiusChangeScaleMultMax;
				float _counterRateRangeIntensityMultMin, _counterRateRangeIntensityMultMax;
				float _shooterThetaMin, _shooterThetaMax;
				float _shooterInitSpeedMultMin, _shooterInitSpeedMultMax;
				float _duration;

				bool ReadFromFile(std::string filename);

				Settings() :
				_emitterDuration(1.0),
					_textureFileName("Images/smoke.rgb"),
					_lifeConst(0.5), _lifeScale(.1),
					_sizeRange(0.75f, 3.0f),
					_alphaRange(0.1f, 1.0f),
					_colorRange(osg::Vec4(1.0f, 0.8f, 0.2f, 1.0f), osg::Vec4(1.0f, 0.4f, 0.1f, 0.0f)),
					_radiusScale(0.4f),
					_density(1.2f), // 1.2kg/m^3
					_movementCompensationRatio(1.5f),
					_useLocalParticleSystem(true), 
					_endless(false),
					_placerRadiusChangeScaleMultMin(0.0f),
					_placerRadiusChangeScaleMultMax(0.25f),
					_counterRateRangeIntensityMultMin(50.f),
					_counterRateRangeIntensityMultMax(100.0f),
					_shooterThetaMin(0.0f),
					_shooterThetaMax(osg::PI_2),
					_shooterInitSpeedMultMin(1.0f),
					_shooterInitSpeedMultMax(10.0f),
					_duration(6.0)
				{
				}
			};

			CustomParticleEffect(ActorScene* actorScene, osg::Node& parent) : IParticleEffect(actorScene, parent)
			{
			}

		protected:
			virtual bool UseLocalParticleSystem(){return m_settings._useLocalParticleSystem;}
			virtual double GetDefaultDuration(){return m_settings._duration;}
			virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);
			virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
			class CustomParticleEffectClassFactory : public INamedEffectClassFactory
			{
			protected:
				virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
				{
					if (!stricmp(name, "CustomParticleEffect")) return new CustomParticleEffect(actorScene, parent);
					else return NULL;
				}
				virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<CustomParticleEffect*>(effect);}
			};

		private:
			Settings m_settings;
			static CustomParticleEffectClassFactory s_classFactory;
			friend CustomParticleEffectClassFactory;
		};
		//////////////////////////////////////////////////////////////////////////

		class Custom_OsgParticleEffect : public osgParticle::ParticleEffect
		{
		public:
			Custom_OsgParticleEffect(const osg::Vec3& position, float scale, float intensity, const CustomParticleEffect::Settings& settings);
			Custom_OsgParticleEffect(const Custom_OsgParticleEffect& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);


			virtual void setDefaults();
			virtual void setUpEmitterAndProgram();

			virtual osgParticle::Emitter* getEmitter() { return _emitter.get(); }
			virtual const osgParticle::Emitter* getEmitter() const { return _emitter.get(); }

			virtual osgParticle::Program* getProgram() { return _program.get(); }
			virtual const osgParticle::Program* getProgram() const { return _program.get(); }

		protected:

			virtual ~Custom_OsgParticleEffect() {}

			osg::ref_ptr<osgParticle::ModularEmitter> _emitter;
			osg::ref_ptr<osgParticle::FluidProgram> _program;
			CustomParticleEffect::Settings m_settings;
		};
		//////////////////////////////////////////////////////////////////////////
	}
}
