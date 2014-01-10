#include <vtkPoints.h>
#include <vtkPolyData.h>

// typically used as a functor in for_each
class VTKPointInsertNext
{
    vtkPoints* p;
    public:
    VTKPointInsertNext(vtkPoints* p_) : p(p_){}

    void operator()(Point<3,double> p_){ p->InsertNextPoint(p_[0],p_[1],p_[2]); }
};

// NOTE: Class automatically subtracts 1 from indices to match VTK conventions
// typically used as a functor in for_each
class VTKCellInsertNext
{
    vtkCellArray *c;
    vtkFloatArray   *s;
    public:
    VTKCellInsertNext(vtkCellArray *c_,vtkFloatArray *s_=NULL) : c(c_),s(s_){}
    void operator()(FaceByPointID f) const { vtkIdType t[3] = {f[0]-1, f[1]-1, f[2]-1}; c->InsertNextCell(3,t); }
    void operator()(pair<FaceByPointID,double> p) const {
        FaceByPointID f;
        vtkIdType t[3] = { p.first[0]-1, p.first[1]-1, p.first[2]-1 };
        c->InsertNextCell(3,t);
        if (s)
            s->InsertNextTuple1(p.second);
    }
};
