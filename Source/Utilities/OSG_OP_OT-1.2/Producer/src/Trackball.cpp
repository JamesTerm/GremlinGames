/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

#include <stdio.h>
#include <math.h>
#include <Producer/Trackball>

using namespace Producer;

Trackball::Trackball( void )
{ 
    R.makeIdentity();
    O.makeIdentity();
    T.makeIdentity();
    S.makeIdentity();

    Tr.makeIdentity();
    Rr.makeIdentity();

    _scale = 1.0;
    _minimum_scale_is_set = false;

    _rscale = 1.0;
    _distance_ref = 0.0;
    _distance_has_changed = true;
    _min_distance_is_set = false;
    _computeOrientation = true;
    
    _rotation = true;
    _panning  = true;
    _distancing = true;

    _throwThreshold = 0.0;

    _pan_fov = float(deg2rad(45.0f));
    setOperationalMode( DefaultOperationalMode );
    reset();
}

void Trackball::setMinimumDistance( float dist )
{
    _min_distance = dist;
    _min_distance_is_set = true;
}

void Trackball::setDistance( float dist, bool do_update )
{
#define ALLOW_PAST_0
#ifdef ALLOW_PAST_0
    _distance = dist;
#else
    if( dist > 0.0 )
        _distance = dist;
    else
        return;
#endif

    _distance_has_changed = true;
    if( _auto_scale) 
        updateScale();

    if( do_update ) 
        update();
}

void Trackball::resetDistance(bool do_update)
{
    setDistance(_distance_ref, do_update );
}    

void Trackball::setOrientation( Orientation mode )
{
    _orientation = mode;
    switch (_orientation) 
    {
        case Y_UP:
                O.makeIdentity();
                break;

        case Z_UP:
                O = Matrix::rotate(-(float)(M_PI) / 2.0f, 1.0f, 0.0f, 0.0f);
                break;
    }
}

void Trackball::setOperationalMode( OperationalMode mode )
{
    _operational_mode = mode;
    switch( mode )
    {
    case DefaultOperationalMode :
            _buttonMap[0] = UpdateNone;
            _buttonMap[1] = UpdateRotation;
            _buttonMap[2] = UpdatePan;
            _buttonMap[3] = UpdateNone;
            _buttonMap[4] = UpdateDistance;
            _buttonMap[5] = UpdateNone;
            _buttonMap[6] = UpdateNone;
            _buttonMap[7] = UpdateNone;

        setOrientation( Z_UP );
        setRotationalMode( Spherical ); 
        setTransformOrder( TranslateRotate );
        setThrowMode( ThrowRotationPanDistance );
        setAutoScale(true);
        break;
    case InventorLike:
            _buttonMap[0] = UpdateNone;
            _buttonMap[1] = UpdateRotation;
            _buttonMap[2] = UpdatePan;
            _buttonMap[3] = UpdateDistance;
            _buttonMap[4] = UpdateNone;
            _buttonMap[5] = UpdateRotation;
            _buttonMap[6] = UpdatePan;
            _buttonMap[7] = UpdateDistance;

        setOrientation( Y_UP );
        setRotationalMode( Spherical ); 
        setTransformOrder( TranslateRotate );
        setThrowMode( ThrowRotation );
        setAutoScale(true);
        break;

    case PerformerLike:
            _buttonMap[0] = UpdateNone;
            _buttonMap[1] = UpdatePan;
            _buttonMap[2] = UpdateRotation;
            _buttonMap[3] = UpdateNone;
            _buttonMap[4] = UpdateDistance;
            _buttonMap[5] = UpdateDistance;
            _buttonMap[6] = UpdateNone; 

            // Performer trackball rotates about 'z', 
            // when bits 2 and 3 are set, but this 
            // mode is unsupported
            _buttonMap[7] = UpdateNone;

        setOrientation( Z_UP );
        setRotationalMode( FixedAxis ); 
        setTransformOrder( RotateTranslate );
        setThrowMode( ThrowRotationPanDistance );
        setAutoScale(false);

        break;
    }
}

Matrix &Trackball::getMatrix( void )
{
    return TX;
}

const Matrix &Trackball::getMatrix( void ) const
{
    return TX;
}

void Trackball::setMatrix( const Matrix mat )
{
    Matrix _DD;
    if( _orientation == Y_UP )
        _DD = Matrix::translate( 0, 0, _distance );
    else if( _orientation == Z_UP )
        _DD = Matrix::translate( 0, -_distance, 0 );

    if( _computeOrientation )
    {
        Matrix _Oinv;
        _Oinv.invert(O);
        R = mat * _Oinv * _DD;
    }
    else
        R = mat * _DD;

    T.makeIdentity();

    if( _transform_order == TranslateRotate )
    {
        Producer::Matrix t;
        t.invert(R);
        T(3,0) = -t(3,0);
        T(3,1) = -t(3,1);
        T(3,2) = -t(3,2);
    }
    else if( _transform_order == RotateTranslate )
    {
        T(3,0) = R(3,0);
        T(3,1) = R(3,1);
        T(3,2) = R(3,2);
    }
    R(3,0) = R(3,1) = R(3,2) = 0.0;
}

void Trackball::restart(float x, float y)
{
    _lastx = x;
    _lasty = y;

}

void Trackball::setReference( void )
{
    Tr = T;
    Rr = R;
    _distance_ref = _distance;
    T.makeIdentity();
    R.makeIdentity();
    reset();
}

void Trackball::reset( bool do_update )
{
    T = Tr;
    R = Rr;
    _distance = _distance_ref;
    _distance_has_changed = true;
    _mbutton = 0;
    _update_mode = UpdateNone;
    _dx = _dy = 0.0f;
    restart( 0.0, 0.0);
    if( _auto_scale) 
        updateScale();
    if( do_update ) 
        update();
}

void Trackball::update(void)
{
    Matrix D;
    if( _orientation == Y_UP )
        D = Matrix::translate( 0, 0, -_distance );
    else if( _orientation == Z_UP )
        D = Matrix::translate( 0, _distance, 0 );


    if( _computeOrientation )
    {
        if( _transform_order == TranslateRotate )
            TX = T * (S * (R * (D  * O ))); 
        else if( _transform_order == RotateTranslate )
            TX = R * (T * (D  * O )); 
    }
    else
    {
        if( _transform_order == TranslateRotate )
            TX = T * (S * (R * D)); 
        else if( _transform_order == RotateTranslate )
            TX = R * (S * (T * D)); 
    }
}

void Trackball::input( float mx, float my, unsigned int mbutton, bool do_update )
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

    if( _mbutton )
    {
        _update_mode = getMappedButtonState( _mbutton );
        _dx = _lastx - mx;
        _dy = _lasty - my;
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
                float mag = sqrt(sqr(_dy)+sqr(_dx));
                if( mag != 0.0 )
                {
                    if( _orientation == Y_UP )
                        R *= Matrix::rotate( 2.0 * asin(mag),(float)_dy, (float)-_dx, 0.0);
                    else if( _orientation == Z_UP )
                        R *= Matrix::rotate( 2.0 * asin(mag),(float)_dy, 0.0, (float)-_dx);
                }
            }
            else if( _rotational_mode == Spherical )
            {
                float ry = -(_lastx - _dx) * deg2rad(90);
                float rx =  (_lasty - _dy) * deg2rad(90);
                Vec3 a( cos(rx) * -sin(ry), sin(rx), cos(rx) * cos(ry) ); 
                ry = -_lastx * deg2rad(90);
                rx =  _lasty * deg2rad(90);
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
            pan.set ( _dx * _scale, _dy * _scale, 0.0);

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
#ifdef ALLOW_PAST_0
            _distance -= _dy * _scale * 10;
            if( _min_distance_is_set && _distance < _min_distance )
                _distance = _min_distance;
#else
            float dd = _dy * _scale * 10;
            if( _distance - dd <= 0.0 ) 
                break;
            _distance -= dd;
#endif
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

void Trackball::translate( float x, float y, float z, bool do_update )
{
    T *= Matrix::translate( x, y, z);
    if( do_update ) 
        update();
}

void Trackball::rotate( float rad, float x, float y, float z, bool do_update)
{

    if( _orientation == Y_UP )
        R *= Matrix::rotate( rad, x, y, z );
    else if( _orientation == Z_UP )
        R *= Matrix::rotate( rad, x, z, y );
    if( do_update ) 
        update();
}

void Trackball::scale( float sx, float sy, float sz, bool do_update )
{
    S *= Matrix::scale(sx, sy,sz);
    if( do_update ) 
        update();
}

void Trackball::setScale( float scale )
{
    if( !_minimum_scale_is_set || scale >= _minimum_scale )
    _scale = scale;
}

void Trackball::setMinimumScale( float minScale )
{
    _minimum_scale_is_set = true;
    _minimum_scale = minScale;
}

void Trackball::updateScale()
{
    _scale = _pan_fov * 0.5 * fabs(_distance);
    if( _scale < 1.0 ) _scale = 1.0;
}

