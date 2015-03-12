#pragma once
#include "Geometry/SourceDescription.hpp"
#include "Packet.hpp"

using std::pair;

// SourceEmitter has the actual guts to emit photons; SourceDescription just tells you what it is
// avoids the need to include Random classes, AVX math, etc.

template<class RNG>class SourceEmitter : virtual public SourceDescription {
protected:
	const TetraMesh& mesh;

public:

	SourceEmitter(const TetraMesh& mesh_) : mesh(mesh_){};
	virtual ~SourceEmitter(){};
	virtual pair<Packet,unsigned> emit(RNG& rng) const=0;
};

template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh&,const SourceDescription& sd);
template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh&,const vector<SourceDescription*>& sdv);

template<class RNG>class IsotropicPointSourceEmitter : public SourceEmitter<RNG>,public IsotropicPointSourceDescription
{
	unsigned IDt;
public:
	IsotropicPointSourceEmitter(const TetraMesh& mesh_,const IsotropicPointSourceDescription& ips_)
		: SourceEmitter<RNG>(mesh_),IsotropicPointSourceDescription(ips_){

		IDt=mesh_.findEnclosingTetra(getOrigin());
	    //cout << "Enclosing tetra is " << IDt << endl;
	}

	virtual ~IsotropicPointSourceEmitter(){};
	virtual pair<Packet,unsigned> emit(RNG& rng) const;

	virtual UnitVector<3,double> getDirection(RNG& rng) const;
};

#include "LineSourceEmitter.hpp"


template<class RNG>class FaceSourceEmitter : virtual public SourceEmitter<RNG>,virtual public FaceSourceDescription {
	Packet pkt;

public:
	virtual ~FaceSourceEmitter(){};
	virtual pair<Packet,unsigned> emit(RNG& rng) const;

	pair<Point<3,double>,unsigned> getOrigin(RNG& rng) const;

	FaceSourceEmitter(const TetraMesh& m,const FaceSourceDescription& fsd,bool force_boundary=false) : SourceEmitter<RNG>(m),
			FaceSourceDescription(fsd)
	{
	    // find face in TetraMesh
	    IDf = m.getFaceID(f);

	    if(force_boundary)
	    {
	        unsigned tetraDown = m.getTetraFromFace(-IDf), tetraUp = m.getTetraFromFace(IDf);
	        if (tetraUp){
	            IDt=tetraUp;
	            if (tetraDown)
	                cerr << "Surprise: source is not on a boundary" << endl;
	            else
	                IDf = -IDf;
	        }
	        else if (tetraDown)
	            IDt=tetraDown;
	        else
	            cerr << "Surprise: face borders no tetras!" << endl;
	    }
	    else
	        IDt = m.getTetraFromFace(IDf);

	    // get normal
	    n = -m.getFace(IDf).getNormal();

	    // get anchor point (A)
	    P0 = m.getPoint(f[0]);

	    const Point<3,double> &B=m.getPoint(f[1]), &C=m.getPoint(f[2]);

	    // create shear matrix
	    M[0][0] = B[0]-P0[0];
	    M[0][1] = C[0]-P0[0];
	    M[1][0] = B[1]-P0[1];
	    M[1][1] = C[1]-P0[1];
	    M[2][0] = B[2]-P0[2];
	    M[2][1] = C[2]-P0[2];

	    pkt.setDirection(n);
	}
};


template<class RNG>class VolumeSourceEmitter : virtual public SourceEmitter<RNG>,virtual public VolumeSourceDescription {

private:
	void prepare(const TetraMesh& m)
	{
		Point<3,double> A=m.getTetraPoint(IDt,0);
		Point<3,double> B=m.getTetraPoint(IDt,1);
		Point<3,double> C=m.getTetraPoint(IDt,2);
		Point<3,double> D=m.getTetraPoint(IDt,3);

		// copy first point
		P0[0]=A[0];
		P0[1]=A[1];
		P0[2]=A[2];

		// copy AB, AC, AD
		M[0][0]=B[0]-P0[0];
		M[1][0]=B[1]-P0[1];
		M[2][0]=B[2]-P0[2];

		M[0][1]=C[0]-P0[0];
		M[1][1]=C[1]-P0[1];
		M[2][1]=C[2]-P0[2];

		M[0][2]=D[0]-P0[0];
		M[1][2]=D[1]-P0[1];
		M[2][2]=D[2]-P0[2];
	}

public:
	virtual ~VolumeSourceEmitter(){};
	virtual pair<Packet,unsigned> emit(RNG& rng) const;

	virtual pair<Point<3,double>,unsigned> getOrigin(RNG& rng) const;

	VolumeSourceEmitter(const TetraMesh& m,const VolumeSourceDescription& vsd) : SourceEmitter<RNG>(m),VolumeSourceDescription(vsd)
		{ prepare(m); }

	VolumeSourceEmitter(const TetraMesh& m,unsigned IDt,double w_=1.0) : SourceEmitter<RNG>(m),VolumeSourceDescription(IDt,w_)
		{ prepare(m); }
};


template<class RNG>class PencilBeamSourceEmitter : public SourceEmitter<RNG>,public PencilBeamSourceDescription {
	Packet pkt;
	unsigned IDt;

public:
	PencilBeamSourceEmitter(const TetraMesh& mesh_) : SourceEmitter<RNG>(mesh_)
	{
	    Point<3,double> Q;
	    double t,t_min=numeric_limits<double>::infinity();

	    unsigned i=1;

	    // Iterate through all points
	    // TODO: Really inefficient. Should Delaunay triangulate and walk the resulting graph
	    for(TetraMesh::face_id_const_iterator it=mesh_.faceIDBegin(); it != mesh_.faceIDEnd(); ++it,++i)
	    {
	        Point<3,double> tmp[3] = { mesh_.getPoint((*it)[0]), mesh_.getPoint((*it)[1]), mesh_.getPoint((*it)[2]) };

	        if (PointInTriangle(getOrigin(),getDirection(),tmp,Q,t)){
	            if (t < t_min)
	            {
	                t_min=t;
	                IDf=i;
	                if (dot(mesh_.getFace(IDf).getNormal(),getDirection()) < 0)
	                    IDf *= -1;
	                IDt=mesh_.getTetraIDFromFaceID(IDf);
	            }
	        }
	    }
	}
	virtual ~PencilBeamSourceEmitter<RNG>(){};
	virtual pair<Packet,unsigned> emit(RNG& rng) const;
};


template<class RNG>class SourceMultiEmitter : virtual public SourceEmitter<RNG>,virtual public SourceMultiDescription {
	boost::random::discrete_distribution<unsigned> source_dist;
	vector<SourceEmitter<RNG>*> emitters;

public:

	template<class ConstIterator>SourceMultiEmitter(const TetraMesh& mesh_,ConstIterator begin,ConstIterator end) :
	SourceEmitter<RNG>(mesh_),
	SourceMultiDescription(begin,end),
	source_dist(
			boost::make_transform_iterator(begin,std::mem_fn(&SourceMultiDescription::getPower)),
			boost::make_transform_iterator(end,std::mem_fn(&SourceMultiDescription::getPower)))
	{
		for(; begin != end; ++begin)
			emitters.push_back(SourceEmitterFactory<RNG>(mesh_,**begin));
	}

	pair<Packet,unsigned> emit(RNG& rng) const
	{
	    return emitters[source_dist(rng)]->emit(rng);
	}
};

//    bool prepare(const TetraMesh&);
//    virtual pair<Packet,unsigned> emit(RNG_Type& rng) const
//        {
//            Packet pkt;
//            pkt.setDirection(rng.draw_m128f3_uvect());
//            pkt.p = to_m128f(getOrigin(rng).first);
//            return make_pair(pkt,IDt);
//        }

template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh& mesh,const SourceDescription& sd)
{
	if (const IsotropicPointSourceDescription* ipsd=dynamic_cast<const IsotropicPointSourceDescription*>(&sd))
		return new IsotropicPointSourceEmitter<RNG>(mesh,*ipsd);
	else if (const VolumeSourceDescription* vsd=dynamic_cast<const VolumeSourceDescription*>(&sd))
		return new VolumeSourceEmitter<RNG>(mesh,*vsd);
	else if (const FaceSourceDescription* fsd=dynamic_cast<const FaceSourceDescription*>(&sd))
		return new FaceSourceEmitter<RNG>(mesh,*fsd);
	else if (const LineSourceDescription* lsd=dynamic_cast<const LineSourceDescription*>(&sd))
		return new LineSourceEmitter<RNG>(mesh,*lsd);
	else if (const BallSourceDescription* bsd=dynamic_cast<const BallSourceDescription*>(&sd))
	{
		vector<unsigned> Ts = mesh.tetras_close_to(bsd->getCentre(),bsd->getRadius());
		vector<double> w(Ts.size(),0.0);
		vector<SourceEmitter<RNG>*> s;
		double wsum=0.0;

				s.reserve(Ts.size());

		for(unsigned i=0;i<Ts.size();++i)
		{
			w[i] = mesh.getTetraVolume(Ts[i]);
			wsum += w[i];
		}

		for(unsigned i=0;i<Ts.size();++i)
		{
			w[i] = w[i]*bsd->getPower()/wsum;
			s.push_back(new VolumeSourceEmitter<RNG>(mesh,Ts[i],w[i]));
			s.back()->setPower(w[i]);
		}
		return new SourceMultiEmitter<RNG>(mesh,s.begin(),s.end());
	}
	else
	{
		cerr << "ERROR: Unknown source type!" << endl;
		return NULL;
	}
}

template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh& mesh,const vector<SourceDescription*>& sdv)
{
	if (sdv.size()==0)
	{
		cerr << "ERROR: No sources specified in emitter factory" << endl;
		return NULL;
	}
	else if (sdv.size()==1)
		return SourceEmitterFactory<RNG>(mesh,*sdv.front());
	else
		return new SourceMultiEmitter<RNG>(mesh,sdv.begin(),sdv.end());
}



template<class RNG>pair<Packet,unsigned> PencilBeamSourceEmitter<RNG>::emit(RNG& rng) const
{
    return make_pair(pkt,IDt);
}



template<class RNG>pair<Point<3,double>,unsigned> FaceSourceEmitter<RNG>::getOrigin(RNG& rng) const
{
    double s=rng.draw_float_u01(),t=rng.draw_float_u01();
    if (s+t > 1)
        s=1-s,t=1-t;

    double P[4] = {
        P0[0] + M[0][0]*s + M[0][1]*t,
        P0[1] + M[1][0]*s + M[1][1]*t,
        P0[2] + M[2][0]*s + M[2][1]*t,
        0
    };
    return make_pair(Point<3,double>(P),IDt);
}

template<class RNG>pair<Packet,unsigned> FaceSourceEmitter<RNG>::emit(RNG& rng) const
{
    Packet p=pkt;
    p.setPosition(getOrigin(rng).first);
    return make_pair(p,IDt);
}

template<class RNG>pair<Packet,unsigned> IsotropicPointSourceEmitter<RNG>::emit(RNG& rng) const
{
    Packet pkt;

    pkt.setPosition(getOrigin());
    pkt.setDirection(rng.draw_m128f3_uvect());

    return make_pair(pkt,IDt);
}

//TODO: Avoid conversion to and from UnitVector<3,double> type

template<class RNG>UnitVector<3,double> IsotropicPointSourceEmitter<RNG>::getDirection(RNG& rng) const
{
	float f[4];
	double d[3];
	__m128 uv = rng.draw_m128f3_uvect();
	_mm_store_ps(f,uv);
	copy(f,f+3,d);
	return UnitVector<3,double>(d,true);

}

template<class RNG>pair<Point<3,double>,unsigned> VolumeSourceEmitter<RNG>::getOrigin(RNG& rng) const
{
    double p[3],src[3];
    double rnd[3] = { rng.draw_float_u01(), rng.draw_float_u01(), rng.draw_float_u01() };
    if (rnd[0]+rnd[1] > 1.0)
    {
        rnd[0] = 1.0-rnd[0];
        rnd[1] = 1.0-rnd[1];
    }

    if (rnd[0]+rnd[1]+rnd[2] > 1.0)
    {
        if(rnd[1]+rnd[2] < 1.0)
        {
            p[0] = 1-rnd[1]-rnd[2];
            p[1] = rnd[1];
            p[2] = rnd[0]+rnd[1]+rnd[2] - 1;
        }
        else {
            p[0] = rnd[0];
            p[1] = 1-rnd[2];
            p[2] = 1-rnd[0]-rnd[1];
        }
    }
    else {
        p[0]=rnd[0];
        p[1]=rnd[1];
        p[2]=rnd[2];
    }

    assert(p[0] >= 0.0 && p[1] >= 0.0 && p[2] >= 0.0 && p[0]+p[1]+p[2] <= 1.0);

    src[0] = P0[0] + M[0][0]*p[0] + M[0][1]*p[1] + M[0][2]*p[2];
    src[1] = P0[1] + M[1][0]*p[0] + M[1][1]*p[1] + M[1][2]*p[2];
    src[2] = P0[2] + M[2][0]*p[0] + M[2][1]*p[1] + M[2][2]*p[2];

    return make_pair(Point<3,double>(src),IDt);
}

template<class RNG>pair<Packet,unsigned> VolumeSourceEmitter<RNG>::emit(RNG& rng) const
{
	Packet p;
	p.setPosition(getOrigin(rng).first);
    p.setDirection(rng.draw_m128f3_uvect());
	return make_pair(p,IDt);
}

