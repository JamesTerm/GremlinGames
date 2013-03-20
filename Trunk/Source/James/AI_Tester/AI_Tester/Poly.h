#pragma once

struct PolynomialEquation_forth_Props
{
	void Init()
	{
		memset(&Term,0,sizeof(PolynomialEquation_forth_Props));
		Term[1]=1.0;
	}
	double Term[5];

	virtual void LoadFromScript(Scripting::Script& script,const char FieldName[])
	{
		const char* err=NULL;
		err = script.GetFieldTable(FieldName);
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&Term[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&Term[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&Term[2]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&Term[3]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&Term[4]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
	}
};

class PolynomialEquation_forth
{
	public:
		PolynomialEquation_forth()
		{
			memset(&m_PolyProps,0,sizeof(PolynomialEquation_forth_Props));
			m_PolyProps.Term[1]=1.0;
		}
		void Initialize(const PolynomialEquation_forth_Props *props=NULL)
		{
			if (props)
				m_PolyProps=*props;
		}
		__inline double operator() (double x)
		{
			double y=fabs(x);
			const double *c=m_PolyProps.Term;
			const double x2=y*y;
			const double x3=y*x2;
			const double x4=x2*x2;
			y = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*y) + c[0]; 
			const double result=(x<0)?-y:y;
			return result;
		}
		/// \param clip_value used to auto clip before restoring sign this is typically set to 1.0
		__inline double operator() (double x,double clip_value)
		{
			double y=fabs(x);
			const double *c=m_PolyProps.Term;
			const double x2=y*y;
			const double x3=y*x2;
			const double x4=x2*x2;
			y = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*y) + c[0]; 
			y=(y<clip_value)?y:clip_value;  //min operation
			const double result=(x<0)?-y:y;
			return result;
		}

	private:
		PolynomialEquation_forth_Props m_PolyProps;
};
