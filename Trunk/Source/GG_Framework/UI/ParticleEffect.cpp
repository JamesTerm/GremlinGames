// GG_Framework.UI ParticleEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;
using namespace osgParticle;

IParticleEffect::IParticleEffect(ActorScene* actorScene, osg::Node& parent) : INamedEffect(actorScene, parent), 
	m_position(osg::Vec3(0.0f,0.0f,0.0f)), m_scale(1.0f), m_intensity(1.0f)
{
	actorScene->GetOsgTimer()->CurrTimeChanged.Subscribe(ehl, *this, &IParticleEffect::OnTimer);
}
//////////////////////////////////////////////////////////////////////////

void IParticleEffect::OnTimer(double time_s)
{
	// Assumes the durations are all the same, so they should be removed FIFO
	while (true)
	{
		if (!m_onParticleStruct.empty())
		{
			ParticleStruct& ps = m_onParticleStruct.front();
			if (ps.EndTime < time_s)
			{
				RemoveParticleEffect(ps);
				m_onParticleStruct.pop_front();
			}
			else return;
		}
		else
			return;
	}
}
//////////////////////////////////////////////////////////////////////////

void IParticleEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	INamedEffect::ReadParameters(localMap, remLineFromFile);
	sscanf(remLineFromFile, "(%lf,%lf,%lf) %f %f", &m_position[0], &m_position[1], &m_position[2], &m_scale, &m_intensity);

	// Only pass on more info if there is a '~'
	const char* tilde = strchr(remLineFromFile, '~');
	remLineFromFile = tilde ? tilde+1 : NULL;
}
//////////////////////////////////////////////////////////////////////////

void IParticleEffect::LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt)
{
	if (start)
	{
		if (GetMode() == IEffect::ON)
		{
			// Create a new one and place it in the list
			ParticleStruct pg;
			pg.EndTime = launchTime_s + GetDefaultDuration();
			FireParticleEffect(pg, intensity, pt);
			m_onParticleStruct.push_back(pg);
		}
		else
			FireParticleEffect(m_loopingPG, intensity, pt);
	}
	else
	{
		// Just stop it and put it on the stack, but do not remove it yet
		if (m_loopingPG.OnEffect.valid())
		{
			// Stop the particles from emmitting
			m_loopingPG.OnEffect->setStartTime(1e6);
			m_loopingPG.EndTime = launchTime_s + GetDefaultDuration();

			// Put it on the stack to remove when all the particles are dead
			m_onParticleStruct.push_back(m_loopingPG);

			// No longer watching this one
			m_loopingPG.OnEffect = NULL;
			m_loopingPG.OnGeode = NULL;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void IParticleEffect::FireParticleEffect(ParticleStruct& pg, double intensity, const osg::Vec3d& pt)
{
	ASSERT(!pg.OnEffect.valid());
	pg.OnEffect = CreateParticleEffectOSG(pt, m_scale, intensity);
	if (!pg.OnEffect.valid()) return;
	pg.OnEffect->setUseLocalParticleSystem(UseLocalParticleSystem());
	m_actorScene->AddChildNextUpdate(pg.OnEffect.get(), GetParentNode()->asGroup());

	if (!UseLocalParticleSystem())
	{
		ASSERT(!pg.OnGeode);
		pg.OnGeode = new osg::Geode;
		pg.OnGeode->addDrawable(pg.OnEffect->getParticleSystem());
		m_actorScene->AddChildNextUpdate(pg.OnGeode.get());
	}
}
//////////////////////////////////////////////////////////////////////////

void IParticleEffect::RemoveParticleEffect(ParticleStruct& pg)
{
	DEBUG_SMOKETRAILS("IParticleEffect::RemoveParticleEffect()\n");
	if (pg.OnGeode.valid())
		m_actorScene->RemoveChildNextUpdate(pg.OnGeode.get());
	m_actorScene->RemoveChildNextUpdate(pg.OnEffect.get(), GetParentNode()->asGroup());

	if (!UseLocalParticleSystem())
	{
		// Remove the child from the global scene
		m_actorScene->RemoveChildNextUpdate(pg.OnGeode.get());
	}
	pg.OnEffect = NULL;
	pg.OnGeode = NULL;
}
//////////////////////////////////////////////////////////////////////////

osgParticle::ParticleEffect* GG_Framework::UI::FireEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{
	return new osgParticle::FireEffect(pos, scale, intensity);
}
osgParticle::ParticleEffect* GG_Framework::UI::ExplosionDebrisEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{
	return new osgParticle::ExplosionDebrisEffect(pos, scale, intensity);
}
osgParticle::ParticleEffect* GG_Framework::UI::ExplosionEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{
	return new osgParticle::ExplosionEffect(pos, scale, intensity);
}
osgParticle::ParticleEffect* GG_Framework::UI::SmokeEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{
	return new osgParticle::SmokeEffect(pos, scale, intensity);
}
osgParticle::ParticleEffect* GG_Framework::UI::SmokeTrailEffect::CreateParticleEffectOSG(const osg::Vec3d& pos, double scale, double intensity)
{return NULL;
	osgParticle::SmokeTrailEffect* smokeTrailEffect = new osgParticle::SmokeTrailEffect(pos, scale, intensity);

	// Make the duration very long so that the trails do not go away
	smokeTrailEffect->setEmitterDuration(1e6);

	// Use the intensity number we have been given to 
	m_duration = intensity;
	smokeTrailEffect->setParticleDuration(m_duration);

	return smokeTrailEffect;
}
void GG_Framework::UI::SmokeTrailEffect::PerformanceIndexChanged(int oldIndex, int newIndex)
{
	// Only change the lengths if the newIndex is less than one.  
	// This means the maximum will never get higher than what was passed in.
	// Smoke trails will ONLY get shorter if we are having a performance problem
	if (newIndex < 1)
	{
		double newDur = m_duration + (newIndex-oldIndex);

		// Watch for durations that are less than 1 second (we always want to see a LITTLE)
		if (newDur >= 1.0)
			m_duration = newDur;
	}
}

GG_Framework::UI::SmokeTrailEffect::SmokeTrailEffect(ActorScene* actorScene, osg::Node& parent) : 
	m_duration(4.0), IParticleEffect(actorScene, parent)
{
	// Listen for performance changes to adjust the lengths
	GG_Framework::UI::MainWindow::GetMainWindow()->PerformanceIndexChange.Subscribe(ehl, *this, &SmokeTrailEffect::PerformanceIndexChanged);

	// Fire initial changes right away (in case a different default was passed in
	PerformanceIndexChanged(0, GG_Framework::UI::MainWindow::PERFORMANCE_INIT_INDEX);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////






