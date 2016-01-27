#include "newgeom.hpp"
#include <limits>
#include <sstream>

//SourceDescription* parse_string(const string& s)
//{
//	stringstream ss(s);
//
//	char tmp[s.size()];
//	double w;
//
//	ss.get(tmp,s.size(),':');
//	tmp[ss.gcount()]='\0';
//
//	ss.ignore(1,':');
//	ss >> w;
//
//	ss.ignore(1,':');
//
//	SourceDescription* src=NULL;
//	string type(tmp);
//
//	if (type=="tet")
//	{
//		unsigned IDt;
//		ss >> IDt;
//		if (!ss.fail() && ss.eof())
//			src = new VolumeSourceDescription(IDt,w);
//	}
//	else if(type=="line")
//	{
//		Point<3,double> a,b;
//		ss >> a >> b;
//		if (!ss.fail() && ss.eof())
//			src = new LineSourceDescription(a,b,w);
//	}
//	else if(type=="point")
//	{
//		Point<3,double> p;
//		ss >> p;
//		if (!ss.fail() && ss.eof())
//			src = new IsotropicPointSourceDescription(p,w);
//	}
//	else if (type=="ball")
//	{
//		Point<3,double> p;
//		double r;
//
//		ss >> p >> r;
//		if (!ss.fail() && ss.eof())
//			src = new BallSourceDescription(p,r,w);
//	}
//	else if(type=="pencil")
//	{
//		Point<3,double> p;
//		UnitVector<3,double> dir;
//		ss >> p >> dir;
//		if (!ss.fail() && ss.eof())
//			src = new PencilBeamSourceDescription(p,dir,w);
//	}
//	else
//		cerr << "Invalid source type in string: \"" << s << '"' << endl;
//
//	if (!src)
//		cerr << "Source of type " << type << " failed to parse: \"" << s << '"' << endl;
//
//	return src;
//}
