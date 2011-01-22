// GG_Framework.UI CustomParticleEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;
using namespace osgParticle;

Custom_OsgParticleEffect::Custom_OsgParticleEffect(
	const osg::Vec3& position, float scale, float intensity, const CustomParticleEffect::Settings& settings) :
m_settings(settings)
{
	setDefaults();

	_position = position;
	_scale = scale;
	_intensity = intensity;

	if (_automaticSetup) buildEffect();
}

Custom_OsgParticleEffect::Custom_OsgParticleEffect(const Custom_OsgParticleEffect& copy, const osg::CopyOp& copyop):
ParticleEffect(copy,copyop)
{
	m_settings = copy.m_settings;
	if (_automaticSetup) buildEffect();
}

void Custom_OsgParticleEffect::setDefaults()
{
	ParticleEffect::setDefaults();

	_textureFileName = m_settings._textureFileName;    
	_emitterDuration = m_settings._emitterDuration;
	_useLocalParticleSystem = m_settings._useLocalParticleSystem;

	// set up unit particle.
	_defaultParticleTemplate.setLifeTime(m_settings._lifeConst+m_settings._lifeScale*_scale);
	_defaultParticleTemplate.setSizeRange(m_settings._sizeRange);
	_defaultParticleTemplate.setAlphaRange(m_settings._alphaRange);
	_defaultParticleTemplate.setColorRange(m_settings._colorRange);

}


void Custom_OsgParticleEffect::setUpEmitterAndProgram()
{
	// set up particle system
	if (!_particleSystem)
	{
		_particleSystem = new osgParticle::ParticleSystem;
	}

	if (_particleSystem.valid())
	{
		_particleSystem->setDefaultAttributes(_textureFileName, false, false);

		osgParticle::Particle& ptemplate = _particleSystem->getDefaultParticleTemplate();

		float radius = m_settings._radiusScale*_scale; 

		ptemplate.setLifeTime(_defaultParticleTemplate.getLifeTime());

		// the following ranges set the envelope of the respective 
		// graphical properties in time.
		ptemplate.setSizeRange(osgParticle::rangef(radius*_defaultParticleTemplate.getSizeRange().minimum,
			radius*_defaultParticleTemplate.getSizeRange().maximum));
		ptemplate.setAlphaRange(_defaultParticleTemplate.getAlphaRange());
		ptemplate.setColorRange(_defaultParticleTemplate.getColorRange());

		// these are physical properties of the particle
		ptemplate.setRadius(radius);
		ptemplate.setMass(m_settings._density*radius*radius*radius*osg::PI*4.0f/3.0f);

	} 


	// set up emitter
	if (!_emitter)
	{
		_emitter = new osgParticle::ModularEmitter;
		_emitter->setNumParticlesToCreateMovementCompensationRatio(m_settings._movementCompensationRatio);
		_emitter->setCounter(new osgParticle::RandomRateCounter);
		_emitter->setPlacer(new osgParticle::SectorPlacer);
		_emitter->setShooter(new osgParticle::RadialShooter);
	}

	if (_emitter.valid())
	{
		_emitter->setParticleSystem(_particleSystem.get());
		_emitter->setReferenceFrame(_useLocalParticleSystem?
			osgParticle::ParticleProcessor::ABSOLUTE_RF:
		osgParticle::ParticleProcessor::RELATIVE_RF);

		_emitter->setStartTime(_startTime);
		_emitter->setLifeTime(_emitterDuration);
		_emitter->setEndless(m_settings._endless);

		osgParticle::RandomRateCounter* counter = dynamic_cast<osgParticle::RandomRateCounter*>(_emitter->getCounter());
		if (counter)
		{
			counter->setRateRange(
				m_settings._counterRateRangeIntensityMultMin*_intensity,
				m_settings._counterRateRangeIntensityMultMax*_intensity);
		}

		osgParticle::SectorPlacer* placer = dynamic_cast<osgParticle::SectorPlacer*>(_emitter->getPlacer());
		if (placer)
		{
			placer->setCenter(_position);
			placer->setRadiusRange(
				m_settings._placerRadiusChangeScaleMultMin*_scale,
				m_settings._placerRadiusChangeScaleMultMax*_scale);
		}

		osgParticle::RadialShooter* shooter = dynamic_cast<osgParticle::RadialShooter*>(_emitter->getShooter());
		if (shooter)
		{
			shooter->setThetaRange(m_settings._shooterThetaMin, m_settings._shooterThetaMax);
			shooter->setInitialSpeedRange(m_settings._shooterInitSpeedMultMin*_scale,m_settings._shooterInitSpeedMultMax*_scale);
		}
	}

	// set up the program
	if (!_program)
	{        
		_program = new osgParticle::FluidProgram;
	}

	if (_program.valid())
	{
		_program->setParticleSystem(_particleSystem.get());
		_program->setWind(_wind);
	}

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

osgParticle::ParticleEffect* CustomParticleEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{
	// return new osgParticle::ExplosionEffect(pos, 5, 5);
	return new Custom_OsgParticleEffect(pos, scale, intensity, m_settings);
}
//////////////////////////////////////////////////////////////////////////

void CustomParticleEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	// Base class goes first
	IParticleEffect::ReadParameters(localMap, remLineFromFile);

	if (!remLineFromFile)
	{
		throw std::exception("Failed to read custom filename from CustomParticleEffect");
	}

	// Read in the name of the settings file
	if (!m_settings.ReadFromFile(remLineFromFile))
	{
		std::string msg("Could not read CustomParticleEffect settings from ");
		msg.append(remLineFromFile);
		throw std::exception(msg.c_str());
	}

	// Only pass on more info if there is a '~'
	const char* tilde = strchr(remLineFromFile, '~');
	remLineFromFile = tilde ? tilde+1 : NULL;
}
//////////////////////////////////////////////////////////////////////////


bool CustomParticleEffect::Settings::ReadFromFile(std::string filename)
{
	StringMap settingsMap;
	if (!ReadStringMapFromIniFile(filename, settingsMap)) return false;

	// textureFileName=Images/smoke.rgb
	if (settingsMap["textureFileName"] != "")
		_textureFileName = settingsMap["textureFileName"];

	// lifeConst=0.5
	sscanf(settingsMap["lifeConst"].c_str(), "%f", &_lifeConst);

	// lifeScale=0.1
	sscanf(settingsMap["lifeScale"].c_str(), "%f", &_lifeScale);

	// sizeRange=(0.75, 3.0)
	sscanf(settingsMap["sizeRange"].c_str(), "(%f, %f)", &(_sizeRange.minimum), &(_sizeRange.maximum));

	// alphaRange=(0.1, 1.0)
	sscanf(settingsMap["alphaRange"].c_str(), "(%f, %f)", &(_alphaRange.minimum), &(_alphaRange.maximum));

	// colorRange=(1.0, 0.8, 0.2, 1.0), (1.0, 0.4, 0.1, 0.0)
	sscanf(settingsMap["colorRange"].c_str(), "(%f, %f, %f, %f), (%f, %f, %f, %f)", 
		&(_colorRange.minimum[0]), &(_colorRange.minimum[1]), &(_colorRange.minimum[2]), &(_colorRange.minimum[3]),
		&(_colorRange.maximum[0]), &(_colorRange.maximum[1]), &(_colorRange.maximum[2]), &(_colorRange.maximum[3]));

	// radiusScale=0.4
	sscanf(settingsMap["radiusScale"].c_str(), "%f", &_radiusScale);

	// density=1.2 # kg/m^3
	sscanf(settingsMap["density"].c_str(), "%f", &_density);

	// movementCompensationRatio=1.5
	sscanf(settingsMap["movementCompensationRatio"].c_str(), "%f", &_movementCompensationRatio);

	// useLocalParticleSystem=false 
	if (settingsMap["useLocalParticleSystem"] != "")
		_useLocalParticleSystem = ParseBooleanFromChar(settingsMap["useLocalParticleSystem"][0]);
	
	// endless=false
	if (settingsMap["endless"] != "")
		_endless = ParseBooleanFromChar(settingsMap["endless"][0]);

	// placerRadiusChangeScaleMultMin=0.0
	sscanf(settingsMap["placerRadiusChangeScaleMultMin"].c_str(), "%f", &_placerRadiusChangeScaleMultMin);

	// placerRadiusChangeScaleMultMax=0.25
	sscanf(settingsMap["placerRadiusChangeScaleMultMax"].c_str(), "%f", &_placerRadiusChangeScaleMultMax);

	// counterRateRangeIntensityMultMin=50.0
	sscanf(settingsMap["counterRateRangeIntensityMultMin"].c_str(), "%f", &_counterRateRangeIntensityMultMin);

	// counterRateRangeIntensityMultMax=100.0
	sscanf(settingsMap["counterRateRangeIntensityMultMax"].c_str(), "%f", &_counterRateRangeIntensityMultMax);

	// shooterThetaMin=0.0
	sscanf(settingsMap["shooterThetaMin"].c_str(), "%f", &_shooterThetaMin);

	// shooterThetaMax=1.5708 # PI/2
	sscanf(settingsMap["shooterThetaMax"].c_str(), "%f", &_shooterThetaMax);

	// shooterInitSpeedMultMin=1.0
	sscanf(settingsMap["shooterInitSpeedMultMin"].c_str(), "%f", &_shooterInitSpeedMultMin);

	// shooterInitSpeedMultMax=10.0
	sscanf(settingsMap["shooterInitSpeedMultMax"].c_str(), "%f", &_shooterInitSpeedMultMax);

	// duration=6.0
	sscanf(settingsMap["duration"].c_str(), "%f", &_duration);

	return true;
}
//////////////////////////////////////////////////////////////////////////