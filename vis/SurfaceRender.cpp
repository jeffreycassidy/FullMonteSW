    // histogram
    vector<double> hist;

    if (vm.count("vtkfile"))
    {
        // histogram not supported here
        vtkDataSetReader *reader = vtkDataSetReader::New();
        reader->SetFileName(globalopts::vtkfn.c_str());
        reader->Update();
        surfMapper->SetInputConnection(reader->GetOutputPort());
        surfMapper->SetScalarModeToUseCellData();
    }
    else if (vm.count("binary") || (vm.count("input") && vm.count("mesh")))
    {
        map<FaceByPointID,double> surf;

        vtkPoints *points = vtkPoints::New();
        vtkCellArray *celldata = vtkCellArray::New();
        vtkFloatArray *scalars = vtkFloatArray::New();
        vtkPolyData *surface = vtkPolyData::New();

        if (vm.count("binary"))
        {
            vector<Point<3,double> > P;
            readFileBin(globalopts::binfile,P,surf);
            unsigned i=0;
            for(vector<Point<3,double> >::const_iterator it=P.begin(); it != P.end(); ++it,++i)
                points->InsertPoint(i,(*it)[0],(*it)[1],(*it)[2]);
        }
