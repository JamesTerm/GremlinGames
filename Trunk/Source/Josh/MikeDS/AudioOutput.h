#pragma once

//******************************************************************************************************************
//******************************************************************************************************************

struct RENDERTREE_SWONLY_API TheSoundCard
{
	static ISoundCard& Get(void);	// Singleton support if you wanna go that way
	
	static const char* VerifyRequiredHardware(void);

	// Multi-instance support if you wanna bat for the other team
	static ISoundCard* NewInst(bool bUseDirectSound=true);
	static void DeleteInst(ISoundCard* pInst);
};

//******************************************************************************************************************
//******************************************************************************************************************