// Trackball.h
#pragma once


namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
class FRAMEWORK_UI_OSG_API TrackBall : public Producer_Trackball
{
protected:
	bool _max_distance_is_set;
	float _max_distance;

public:
	TrackBall()
	{
		setOrientation( Producer_Trackball::Z_UP);
		_fixedDX = _fixedDY = 0.0f;
		_max_distance_is_set = false;
		_max_distance = 0.0;
	}

	void setMaximumDistance( float dist )
	{
		if (dist > 0.0)
		{
			_max_distance_is_set = true;
			_max_distance = dist;
		}
		else _max_distance_is_set = false;
	}

	void ResetFixed(bool do_update=true)
	{
		_fixedDX = _fixedDY = 0.0f;
		reset(do_update);
	}

	void SetReference
		(const osg::Matrix& R_mat, const osg::Matrix& T_mat, double dist)
	{
		Tr = T_mat;
		Rr = R_mat;
		_distance_ref = dist;
		T.makeIdentity();
		R.makeIdentity();
		reset();
	};

	void GetCurrent
		(osg::Matrix& R_mat, osg::Matrix& T_mat, double& dist)
	{
		R_mat = R;
		T_mat = T;
		dist = _distance;
	}


	//! I want to check for max dist as well, and do better with fixed axis 
	virtual void input( float mx, float my, unsigned int mbutton, float dTime_s, bool do_update);

protected:
	float _fixedDX;
	float _fixedDY;
};
		}
	}
}