#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/NodeVisitor>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/FrameStamp>


class Expand: public osg::Drawable::UpdateCallback
{
	public:
		Expand(osg::Vec3 &a,
						osg::Vec3 &b,
						osg::Vec3 &c, bool &trigger  ):
				_a(a),
				_b(b),
				_c(c) ,
				_trigger(trigger)
		{
			_oa = _a;
			_ob = _b;
			_oc = _c;
			_v = (_a + _b + _c)/3.0;
			_v.normalize();
			_trate = (rand()%1000)/500.0;
			_rrate = (rand()%1000)/500.0;
			_axis.set( float(rand()), float(rand()), float(rand()) );
			_axis.normalize();
		}

        virtual void update(osg::NodeVisitor *nv, osg::Drawable*) 
		{
			const osg::FrameStamp *fs = nv->getFrameStamp();
			if( _trigger )
			{
				double t = fs->getReferenceTime() - _t0;
				if( t > 0.05 )
				{
					double angle = t * _rrate;
					osg::Matrix mat = osg::Matrix::rotate( angle, _axis ) * 
						          	  osg::Matrix::translate( _v * (t * _trate) );
							  
					_a = _oa * mat;
					_b = _ob * mat;
					_c = _oc * mat;
				}
			}
			else
			{
				_a = _oa;
				_b = _ob;
				_c = _oc;
				_t0 = fs->getReferenceTime();
			}
		}

	private:
		osg::Vec3 _oa, _ob, _oc;
		osg::Vec3 &_a, &_b, &_c, _v;
		osg::Vec3 _axis;
		double _trate, _rrate;
		double _t0;
		bool &_trigger;
};

class Separate : public osg::NodeVisitor
{
	public:
		Separate(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

        virtual void apply(osg::Geode& geode)
        { 
	        for( unsigned int i = 0; i < geode.getNumDrawables(); i++ )
			{
				osg::Geometry *geometry = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
				_statesets.push_back( geometry->getOrCreateStateSet());
				if( geometry !=  0L )
				{
					osg::Vec3Array *varray = dynamic_cast<osg::Vec3Array *>(geometry->getVertexArray());
					osg::Vec3Array *narray = dynamic_cast<osg::Vec3Array *>(geometry->getNormalArray());
					if( varray != 0L )
						_varrays.push_back( varray );

					if( narray != 0L )
						_narrays.push_back( narray );
				}
			}
            traverse(geode);
        }

		std::vector<osg::ref_ptr<osg::Vec3Array> > &getVArrays()  { return _varrays; }
		std::vector<osg::ref_ptr<osg::Vec3Array> > &getNArrays()  { return _narrays; }
		std::vector<osg::ref_ptr<osg::StateSet> > &getStateSetArray() { return _statesets; }
	private:
		std::vector<osg::ref_ptr<osg::Vec3Array> > _varrays; 
		std::vector<osg::ref_ptr<osg::Vec3Array> > _narrays; 
		std::vector<osg::ref_ptr<osg::StateSet> > _statesets;
};
	

osg::Node *newNode( osg::Vec3 a, osg::Vec3 b, osg::Vec3 c,
								osg::Vec3 na,  osg::Vec3 nb, osg::Vec3 nc, bool  &trigger )
{
	osg::Vec3Array *coords = new osg::Vec3Array;
	coords->push_back( a );
	coords->push_back( b );
	coords->push_back( c );

	osg::Vec3Array *normals = new osg::Vec3Array;
	normals->push_back( na );
	normals->push_back( nb );
	normals->push_back( nc );

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setVertexArray(coords);
	geometry->setNormalArray(normals);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, coords->size()));

	osg::Vec3Array::iterator ii = coords->begin();

	geometry->setUpdateCallback( new Expand(ii[0], ii[1], ii[2], trigger ));

	geometry->setUseDisplayList(false);

	osg::Geode *geode = new osg::Geode;
	geode->addDrawable( geometry.get() );

	return geode;
}


osg::Node *makeExplodingCow(bool &trigger)
{
	osg::Node *cow = osgDB::readNodeFile( "cow.osg" );

	Separate sep;
	cow->accept( sep);

	osg::Group *root = new osg::Group;

	printf( "== %d ==\n", sep.getVArrays().size());
	for( unsigned int i = 0; i < sep.getVArrays().size(); i++ )
	{
		osg::Vec3Array *v = sep.getVArrays()[i].get();	
		osg::Vec3Array *n = sep.getNArrays()[i].get();	
		osg::StateSet  *s = sep.getStateSetArray()[i].get();	
		osg::Vec3Array::iterator p;
		osg::Vec3Array::iterator pn = n->begin();;
		for( p = v->begin(); p != v->end(); p+=3, pn+= 3 )
		{
			osg::Node *nn = newNode(p[0], p[1], p[2], pn[0], pn[1], pn[2], trigger );
			nn->setStateSet( s );

			root->addChild( nn ); 
		}
	}

	return root;
}
