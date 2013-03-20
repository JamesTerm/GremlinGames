#pragma once

struct PolynomialEquation_forth_Props
{
	double Term[5];
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
			double *c=m_PolyProps.Term;
			double x2=y*y;
			double x3=y*x2;
			double x4=x2*x2;
			y = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*y) + c[0]; 
			const double result=(x<0)?-y:y;
			return result;
		}
	private:
		PolynomialEquation_forth_Props m_PolyProps;
};

class PolynomialEquation_forth_Properties
{
	public:
		/// \param FieldName this is the name chosen in lua to represent field table to obtain the terms when using LoadFromScript
		void Init()
		{
			memset(&m_PolyProps,0,sizeof(PolynomialEquation_forth_Props));
			m_PolyProps.Term[1]=1.0;
		}
		virtual void LoadFromScript(Scripting::Script& script,const char FieldName[])
		{
			const char* err=NULL;
			err = script.GetFieldTable(FieldName);
			if (!err)
			{
				err = script.GetField("c", NULL, NULL,&m_PolyProps.Term[0]);
				ASSERT_MSG(!err, err);
				err = script.GetField("t1", NULL, NULL,&m_PolyProps.Term[1]);
				ASSERT_MSG(!err, err);
				err = script.GetField("t2", NULL, NULL,&m_PolyProps.Term[2]);
				ASSERT_MSG(!err, err);
				err = script.GetField("t3", NULL, NULL,&m_PolyProps.Term[3]);
				ASSERT_MSG(!err, err);
				err = script.GetField("t4", NULL, NULL,&m_PolyProps.Term[4]);
				ASSERT_MSG(!err, err);
				script.Pop();
			}
		}

		const PolynomialEquation_forth_Props &GetPolyProps() const {return m_PolyProps;}
		//Get and Set the properties
		PolynomialEquation_forth_Props &PolyProps() {return m_PolyProps;}

	protected:
		PolynomialEquation_forth_Props m_PolyProps;
};
