%typemap(out) std::array<float,3> {
	stringstream ss;
	std::array<float,3> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str());
}

%typemap(out) Point<3,double> {
	stringstream ss;
	Point<3,double> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str());
}

%typemap(in) std::array<float,3> const& (std::array<float,3> p){
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = &p;
}

%typemap(in) Point<3,double> const& (Point<3,double> p) {
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = &p;
}

%typemap(in) Point<3,double> {
	Point<3,double> p;
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}