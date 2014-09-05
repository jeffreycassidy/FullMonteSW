#include "SourceDescription.hpp"

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


template<class RNG>class IsotropicPointSourceEmitter : public SourceEmitter<RNG>,public IsotropicPointSourceDescription
{
public:
	IsotropicPointSourceEmitter(const TetraMesh& mesh_,const IsotropicPointSourceDescription& ips_)
		: SourceEmitter<RNG>(mesh_),IsotropicPointSourceDescription(ips_){}

	virtual ~IsotropicPointSourceEmitter();
	virtual pair<Packet,unsigned> emit(RNG& rng);
};


template<class RNG>class FaceSourceEmitter : public SourceEmitter<RNG>,public FaceSourceDescription {
public:
	FaceSourceEmitter();
	virtual ~FaceSourceEmitter(){};
	virtual pair<Packet,unsigned> emit(RNG& rng);

};

template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh&,const SourceDescription& sd);
template<class RNG>SourceEmitter<RNG>* SourceEmitterFactory(const TetraMesh&,const vector<SourceDescription*>& sdv);

template<class RNG>class SourceMultiEmitter : virtual public SourceEmitter<RNG>,virtual public SourceMultiDescription {
	boost::random::discrete_distribution<unsigned> source_dist;
	vector<SourceEmitter<RNG>*> emitters;

public:

	template<class ConstIterator>SourceMultiEmitter(const TetraMesh& mesh_,ConstIterator begin,ConstIterator end) :
	SourceEmitter<RNG>(mesh_),
	source_dist(boost::make_transform_iterator(begin,std::mem_fn(&SourceMultiDescription::getPower)),
			boost::make_transform_iterator(end,std::mem_fn(&SourceMultiDescription::getPower)))
	{
		for(; begin != end; ++begin)
			emitters.push_back(SourceEmitterFactory<RNG>(*begin));
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
	{
		cout << "OOPS! Isotropic point sources aren't fully implemented yet!" << endl;
		return NULL;
	}
	else if (const VolumeSourceDescription* vsd=dynamic_cast<const VolumeSourceDescription*>(&sd))
	{
		cout << "OOPS! Volume sources aren't fully implemented yet!" << endl;
		return NULL;
	}
	else if (const FaceSourceDescription* fsd=dynamic_cast<const FaceSourceDescription*>(&sd))
	{
		cout << "OOPS! Face sources aren't fully implemented yet!" << endl;
		return NULL;
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
		return SourceEmitterFactory<RNG>(*sdv.front());
	else
		return new SourceMultiEmitter<RNG>(mesh,sdv.begin(),sdv.end());
}


