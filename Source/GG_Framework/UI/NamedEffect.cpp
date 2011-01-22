// GG_Framework.UI NamedEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

std::list<INamedEffectClassFactory*> INamedEffectClassFactory::s_list;

///// PLACE ALL OTHER GLOBAL CLASS FACTORY DECLARATIONS HERE
SoundEffect::SoundEffectClassFactory SoundEffect::s_classFactory;
FireEffect::FireEffectClassFactory FireEffect::s_classFactory;
SmokeTrailEffect::SmokeTrailEffectClassFactory SmokeTrailEffect::s_classFactory;
CustomParticleEffect::CustomParticleEffectClassFactory CustomParticleEffect::s_classFactory;
SmokeEffect::SmokeEffectClassFactory SmokeEffect::s_classFactory;
ExplosionEffect::ExplosionEffectClassFactory ExplosionEffect::s_classFactory;
ExplosionDebrisEffect::ExplosionDebrisEffectClassFactory ExplosionDebrisEffect::s_classFactory;
MapFramesEffect::MapFramesEffectClassFactory MapFramesEffect::s_classFactory;
OsgFile_ParticleEffect::OsgFile_ParticleEffectClassFactory OsgFile_ParticleEffect::s_classFactory;

INamedEffect* INamedEffectClassFactory::CreateEffect(
	ActorScene* actorScene, GG_Framework::UI::EventMap& localMap, osg::Node& parent, const char* lineFromFile)
{
	char s[3][32];
	if (sscanf(lineFromFile, "%31s %31s %31s", s[0], s[1], s[2]) != 3)
		throw std::exception("Error in INamedEffectClassFactory Line");
	std::list<INamedEffectClassFactory*>::iterator pos;
	for (pos = s_list.begin(); pos != s_list.end(); ++pos)
	{
		INamedEffectClassFactory* cf = *pos;
		INamedEffect* effect = cf->CreateEffect(actorScene, s[2], parent);
		
		if (effect)
		{
			effect->m_classFactory = cf;
			effect->Initialize(localMap, lineFromFile);
			return effect;
		}
	}
	// Nothing else was found
	ASSERT_MSG(false, GG_Framework::Base::BuildString("Could not create effect of name %s.\n", s[2]).c_str());
	return NULL;
}
//////////////////////////////////////////////////////////////////////////

void INamedEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	IEffect::ReadParameters(localMap, remLineFromFile);
	char name[32];
	if (sscanf(remLineFromFile, "%31s", name) != 1)
		throw std::exception("Error in INamedEffect Line");
	m_name = name;
	remLineFromFile += strlen(name) + 1;
}
//////////////////////////////////////////////////////////////////////////

void INamedEffect::Delete()
{
	if (m_classFactory)
		m_classFactory->DeleteEffect(this);
	else
		delete this;
}
//////////////////////////////////////////////////////////////////////////
