// GG_Framework.UI ParticleEffect.h
#pragma once
#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Node>
#include <osgParticle/ParticleEffect>
#include <osgParticle/SmokeTrailEffect>

namespace GG_Framework
{
	namespace UI
	{

class FRAMEWORK_UI_API IParticleEffect : public INamedEffect
{	
public:
	IParticleEffect(ActorScene* actorScene, osg::Node& parent);
	virtual void LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt);

	void OnTimer(double time_s);

protected:
	virtual ~IParticleEffect(){}
	virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity) = 0;
	virtual bool UseLocalParticleSystem() = 0;
	virtual double GetDefaultIntensity(){return m_intensity;}
	virtual osg::Vec3d GetDefaultPoint(){return m_position;}
	virtual double GetDefaultDuration() = 0;

private:
	osg::Vec3d m_position;
	float m_scale;
	float m_intensity;
	IEvent::HandlerList ehl;

	struct ParticleStruct
	{
		ParticleStruct() : EndTime(0.0) {}
		osg::ref_ptr<osgParticle::ParticleEffect> OnEffect;
		osg::ref_ptr<osg::Geode> OnGeode;
		double EndTime;	// Only applies for the ON event
	};
	ParticleStruct m_loopingPG;
	std::list<ParticleStruct> m_onParticleStruct;

	void FireParticleEffect(ParticleStruct& pg, double intensity, const osg::Vec3d& pt);
	void RemoveParticleEffect(ParticleStruct& pg);
};
//////////////////////////////////////////////////////////////////////////

class FireEffect : public IParticleEffect
{
public:
	FireEffect(ActorScene* actorScene, osg::Node& parent) : IParticleEffect(actorScene, parent) {}

protected:
	virtual bool UseLocalParticleSystem(){return true;}
	virtual double GetDefaultDuration(){return 6.0;}
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
	class FireEffectClassFactory : public INamedEffectClassFactory
	{
	protected:
		virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
		{
			if (!stricmp(name, "FireEffect")) return new FireEffect(actorScene, parent);
			else return NULL;
		}
		virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<FireEffect*>(effect);}
	};

private:
	static FireEffectClassFactory s_classFactory;
	friend FireEffectClassFactory;
};
//////////////////////////////////////////////////////////////////////////

class ExplosionEffect : public IParticleEffect
{
public:
	ExplosionEffect(ActorScene* actorScene, osg::Node& parent) : IParticleEffect(actorScene, parent) {}

protected:
	virtual bool UseLocalParticleSystem(){return false;}
	virtual double GetDefaultDuration(){return 3.0;}
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
	class ExplosionEffectClassFactory : public INamedEffectClassFactory
	{
	protected:
		virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
		{
			if (!stricmp(name, "ExplosionEffect")) return new ExplosionEffect(actorScene, parent);
			else return NULL;
		}
		virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<ExplosionEffect*>(effect);}
	};

private:
	static ExplosionEffectClassFactory s_classFactory;
	friend ExplosionEffectClassFactory;
};
//////////////////////////////////////////////////////////////////////////

class ExplosionDebrisEffect : public IParticleEffect
{
public:
	ExplosionDebrisEffect(ActorScene* actorScene, osg::Node& parent) : IParticleEffect(actorScene, parent) {}

protected:
	virtual bool UseLocalParticleSystem(){return false;}
	virtual double GetDefaultDuration(){return 7.0;}
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
	class ExplosionDebrisEffectClassFactory : public INamedEffectClassFactory
	{
	protected:
		virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
		{
			if (!stricmp(name, "ExplosionDebrisEffect")) return new ExplosionDebrisEffect(actorScene, parent);
			else return NULL;
		}
		virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<ExplosionDebrisEffect*>(effect);}
	};

private:
	static ExplosionDebrisEffectClassFactory s_classFactory;
	friend ExplosionDebrisEffectClassFactory;
};
//////////////////////////////////////////////////////////////////////////

class SmokeEffect : public IParticleEffect
{
public:
	SmokeEffect(ActorScene* actorScene, osg::Node& parent) : IParticleEffect(actorScene, parent) {}

protected:
	virtual bool UseLocalParticleSystem(){return false;}
	virtual double GetDefaultDuration(){return 7.0;}
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
	class SmokeEffectClassFactory : public INamedEffectClassFactory
	{
	protected:
		virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
		{
			if (!stricmp(name, "SmokeEffect")) return new SmokeEffect(actorScene, parent);
			else return NULL;
		}
		virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<SmokeEffect*>(effect);}
	};

private:
	static SmokeEffectClassFactory s_classFactory;
	friend SmokeEffectClassFactory;
};
//////////////////////////////////////////////////////////////////////////

class SmokeTrailEffect : public IParticleEffect
{
public:
	SmokeTrailEffect(ActorScene* actorScene, osg::Node& parent);

protected:
	virtual bool UseLocalParticleSystem(){return false;}
	virtual double GetDefaultDuration(){return m_duration;}
	virtual osgParticle::ParticleEffect* CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity);
	class SmokeTrailEffectClassFactory : public INamedEffectClassFactory
	{
	protected:
		virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
		{
			if (!stricmp(name, "SmokeTrailEffect")) return new SmokeTrailEffect(actorScene, parent);
			else return NULL;
		}
		virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<SmokeTrailEffect*>(effect);}
	};

private:
	double m_duration;
	IEvent::HandlerList ehl;

	// Listen for Camera performance index to dynamically set the duration
	void PerformanceIndexChanged(int oldIndex, int newIndex);
	static SmokeTrailEffectClassFactory s_classFactory;
	friend SmokeTrailEffectClassFactory;
};
//////////////////////////////////////////////////////////////////////////
	}
}
