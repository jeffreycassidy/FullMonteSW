

    vector<double> E(geom.mesh.getNf()+1);
    vector<double> phi_s(geom.mesh.getNf()+1);

    for(const LoggerResults * lr : results)
    {
    	if (!lr)
    		cout << "Result pointer NULL" << endl;
    	else if (const SurfaceArray<double>* p = dynamic_cast<const SurfaceArray<double>*>(lr))
    	{
    		E = p->emitted_energy();
    		for(unsigned i=0; i<=geom.mesh.getNf(); ++i)
    			phi_s[i] = E[i] / geom.mesh.getFaceArea(i);
