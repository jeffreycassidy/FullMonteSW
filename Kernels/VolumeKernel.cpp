
    	else if (const VolumeArray<double>* p = dynamic_cast<const VolumeArray<double>*>(lr))
    	{
    		E_v = p->absorbed_energy();

    		for(unsigned i=0; i<=geom.mesh.getNt(); ++i)
    		{
    			double v = geom.mesh.getTetraVolume(i);
    			double mu_a = geom.mats[geom.mesh.getMaterial(i)].getMuA();

   				phi_v[i] = E_v[i] > 0.0 ? E_v[i]/v/mu_a : 0.0;
    		}
