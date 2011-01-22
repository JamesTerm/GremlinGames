// Threading Base Classes

class VariableBlock
{	private:
		CRITICAL_SECTION myBlock;
		long m_Locks;

	public:

		void Block(void);
		bool TryBlock(void);
		void UnBlock(void);

		VariableBlock(void);
		~VariableBlock(void);
};

class FunctionBlock
{	private:	VariableBlock	*Ptr;

	public:		FunctionBlock(VariableBlock *Fcn);
				FunctionBlock(VariableBlock &Fcn);
				~FunctionBlock(void);
};