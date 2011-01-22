#include "stdafx.h"

FunctionBlock::FunctionBlock(VariableBlock *Fcn)
{	Ptr = Fcn;
	if (Ptr) Ptr->Block();
}

FunctionBlock::FunctionBlock(VariableBlock &Fcn)
{	Ptr=&Fcn;
	Ptr->Block();
}

FunctionBlock::~FunctionBlock(void)
{	if (Ptr) Ptr->UnBlock();
}

bool VariableBlock::TryBlock(void)
{	if (!TryEnterCriticalSection(&myBlock)) return false;
	m_Locks++;
	return true;
}

//The non-debug version
/**/
void VariableBlock::Block(void)		
{	EnterCriticalSection(&myBlock);	
	m_Locks++;
}

void VariableBlock::UnBlock(void)		
{	m_Locks--;
	assert( m_Locks>=0 );
	LeaveCriticalSection(&myBlock);	
}

VariableBlock::VariableBlock(void)	
{	InitializeCriticalSection(&myBlock);
	m_Locks=0;
}

VariableBlock::~VariableBlock(void)	
{	DeleteCriticalSection(&myBlock);	 
}