%typemap(out) std::array<float,3> {
	stringstream ss;
	std::array<float,3> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str(),nullptr);
}

%typemap(in) std::array<float,3>
{
	std::array<float,3> p;
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}

%typemap(out) std::array<unsigned,3> {
	stringstream ss;
	std::array<unsigned,3> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str(),nullptr);
}

%typemap(in) std::array<unsigned,3>
{
	std::array<unsigned,3> p;
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}

%typemap(out) Point<3,double> {
	stringstream ss;
	Point<3,double> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str(),nullptr);
}


%typemap(typecheck) std::array<float,3>
{
	std::string str(Tcl_GetString($input));
	stringstream ss(str);
	$1 = 1;
	if (!ss.good())
		$1 = 0;
	float f;
	for(unsigned i=0;i<3;++i)
	{
		ss >> f;
		if (ss.fail())
		{
			$1=0;
			break;
		} 
	}
}

%typemap(in) std::array<double,3> {
	std::array<double,3> p;
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}

%typemap(in) Point<3,double> {
	Point<3,double> p;
	stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}
