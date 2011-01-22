// Trackball.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace osg;

using namespace GG_Framework::UI::OSG;
		
void TrackBall::input( float mx, float my, unsigned int mbutton, float dTime_s, bool do_update )
{
#ifdef sun // Solaris X11 implementation delivers mouse events every 42 ms.
	// Note 11/03/02 - This turns out to be particular to Expert3D, and
	// was not evident in Zulu
	static long c = 0;
	c++;
	if( !(c%6) ) return;
#endif

	_distance_has_changed = false;

	if( _mbutton != mbutton )
	{
		_mbutton = mbutton;
		if( _mbutton ) 
		{
			restart( mx, my); 
		}
	}

	float dx, dy;
	if( _mbutton )
	{
		_update_mode = getMappedButtonState( _mbutton );
		dx = _dx = _lastx - mx;
		dy = _dy = _lasty - my;
	}
	else
	{
		dx = _dx * dTime_s * 60.0f;
		dy = _dy * dTime_s * 60.0f;
	}

	switch( _update_mode )
	{
	case UpdateRotation :
		if( _rotation == false ) 
		{
			_update_mode = UpdateNone;
			break;
		}

		if( _rotational_mode == FixedAxis )
		{
			// THIS IS THE PART WE ARE OVERRIDING
			_fixedDX += dx;
			_fixedDY += dy;
			if (_fixedDY > M_PIF*0.5f)
				_fixedDY = M_PIF*0.5f;
			else if (_fixedDY < M_PIF*-0.5f)
				_fixedDY = M_PIF*-0.5f;
			while (_fixedDX > M_PIF*2.0f)
				_fixedDX -= M_PIF*2.0f;
			while (_fixedDX < M_PIF*-2.0f)
				_fixedDX += M_PIF*2.0f;
			if( _orientation == Y_UP )
				R = Tr * Matrix::rotate(_fixedDX, 0, -1, 0) * Matrix::rotate(_fixedDY, 1, 0, 0);
			else if( _orientation == Z_UP )
				R = Tr * Matrix::rotate(_fixedDX, 0, 0, -1) * Matrix::rotate(_fixedDY, 1, 0, 0);
		}
		else if( _rotational_mode == Spherical )
		{
			float ry = -(_lastx - dx) * DEG_2_RAD(90);
			float rx =  (_lasty - dy) * DEG_2_RAD(90);
			Vec3 a( cos(rx) * -sin(ry), sin(rx), cos(rx) * cos(ry) ); 
			ry = -_lastx * DEG_2_RAD(90);
			rx =  _lasty * DEG_2_RAD(90);
			Vec3 b( cos(rx) * -sin(ry), sin(rx), cos(rx) * cos(ry) ); 

			Vec3 n = b ^ a;
			float angle = sin(n.length());
			if( angle > 0.0001 )
			{
				n.normalize();
				if( _orientation == Z_UP )
					n = n * Matrix::rotate( M_PIF*0.5f, 1.0f, 0.0f, 0.0f );
				/*
				* This is an optional way of doing the above generically,
				* but more expensive
				Matrix o;
				o.invert(O);
				n = n * o;
				*/

				angle *= _rscale;
				R *= Matrix::rotate( angle, n[0], n[1], n[2] );
			}
		}
		if( (_throw_mode & ThrowRotation) == 0 )
			_update_mode = UpdateNone;
		break;

	case UpdatePan :
		{
			if( _panning == false ) 
			{
				_update_mode = UpdateNone;
				break;
			}
			Vec3 pan;
			pan.set ( dx * _scale, dy * _scale, 0.0);

			/*
			Matrix o;
			o.invert(O);
			pan = pan * o;
			*/
			if( _orientation == Z_UP )
				pan = pan * Matrix::rotate( M_PIF*0.5f, 1.0f, 0.0f, 0.0f );

			if( _transform_order == TranslateRotate )
			{
				Matrix r;
				r.invert(R);
				pan = pan * r;
			}

			T(3,0) -= pan[0] / S(0,0);
			T(3,1) -= pan[1] / S(1,1);
			T(3,2) -= pan[2] / S(2,2);

			if( (_throw_mode & ThrowPan) == 0 )
				_update_mode = UpdateNone;
			break;
		}

	case UpdateDistance :
		{
			if( _distancing == false ) 
			{
				_update_mode = UpdateNone;
				break;
			}

			_distance -= dy * _scale * 10;
			if( _min_distance_is_set && _distance < _min_distance )
				_distance = _min_distance;

			// Rick Added this
			if( _max_distance_is_set && _distance > _max_distance )
				_distance = _max_distance;

			_distance_has_changed = true;
			if( _auto_scale ) 
				updateScale();

			if( (_throw_mode & ThrowDistance) == 0 )
				_update_mode = UpdateNone;
		}
		break;

	default:
		break;
	}

	if( _mbutton )
	{
		_lastx = mx;
		_lasty = my;
	}

	if( fabs(_dx) < _throwThreshold && fabs(_dy) < _throwThreshold )
		_update_mode = UpdateNone;

	if( do_update )
		update();
}
//////////////////////////////////////////////////////////////////////////
