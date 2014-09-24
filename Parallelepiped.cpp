#include "Parallelepiped.hpp"

bool Parallelepiped::pointWithin(const Point<3,double>& p) const
{
	for(unsigned i=0;i<3;++i)
	{
		double h = dot(faces[i].first,p);
		if (!(faces[i].second.first <= h && h <= faces[i].second.second))
			return false;
	}
	return true;
}

std::ostream& operator<<(std::ostream& os,const Parallelepiped& pp)
{
	for(unsigned i=0;i<3;++i)
		os << pp.faces[i].first << " [" << pp.faces[i].second.first << ',' << pp.faces[i].second.second << "]  ";
	return os;
}


double dot(const UnitVector<3,double>& a,const Point<3,double>& b)
{
	double o=0.0;
	for(size_t i=0;i<3;++i)
		o += a[i]*b[i];
	return o;
}

Parallelepiped::Parallelepiped(const std::array<std::pair<UnitVector<3,double>,std::pair<double,double>>,3>& faces_) :
	faces(faces_)
{};

Parallelepiped::Parallelepiped(const std::array<std::pair<UnitVector<3,double>,std::pair<Point<3,double>,Point<3,double>>>,3>& faces_)
{
	for(unsigned i=0;i<3;++i)
	{
		faces[i].first  = faces_[i].first;
		faces[i].second = make_pair(
				dot(faces[i].first,faces_[i].second.first),
				dot(faces[i].first,faces_[i].second.second));
	}
}

Parallelepiped readParallelepiped(const std::string& s)
{
	array<std::pair<UnitVector<3,double>,std::pair<double,double>>,3> n;
	stringstream ss(s);
	cout << "Parallelepiped description: \"" << s<< "\"" << endl;

	for(unsigned i=0;i<3;++i)
	{
		ss >> n[i].first >> n[i].second.first >> n[i].second.second;
		cout << "  " << n[i].first << ": " << n[i].second.first << '-' << n[i].second.second << endl;
	}

	Parallelepiped pp(n);

	cout << pp << endl;
	return pp;
}
