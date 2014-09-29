#pragma once
#include "Logger.hpp"
#include "AccumulationArray.hpp"
#include "fluencemap.hpp"

template<class T>class VolumeArray : public LoggerResults {
	const TetraMesh& mesh;
	vector<T> v;

public:
	VolumeArray(const VolumeArray& v_) = default;
    VolumeArray(VolumeArray&& lv_)        : mesh(lv_.mesh),v(std::move(lv_.v)){};
    VolumeArray(const TetraMesh& mesh_)    : mesh(mesh_),v(mesh.getNt()+1){};
    VolumeArray(const TetraMesh& mesh_,vector<T>&& v_) : mesh(mesh_),v(std::move(v_)){}
    VolumeArray(const TetraMesh& mesh_,const vector<T>& v_)  : mesh(mesh_),v(v_){}

    /// Returns a VolumeFluenceMap for the absorption accumulated so far*/
    /** The value of asFluence determines whether it returns total energy (false) or fluence as E/V/mu_a (true) */
    void fluenceMap(VolumeFluenceMap&,const vector<Material>&,bool asFluence=true);

    /// Returns (if available from AccumulatorT) a hit map
    void hitMap(map<unsigned,unsigned long long>& m);

    typedef typename vector<T>::const_iterator const_iterator;

    const_iterator begin() const { return v.begin(); }
    const_iterator end()   const { return v.end(); }

    virtual string getTypeString() const { return "logger.results.volume.energy"; }

    double getTotal() const {
    	double sum=0.0;
    	for(double E : v)
    		sum += E;
    	return sum;
    }

	virtual void summarize(ostream& os) const {
		os << "Volume array total energy is " << setprecision(4) << getTotal() << " (" << v.size() << " elements)" << endl;
	}
};

/*! Basic volume logger.
 *
 * Catches eventAbsorb() calls and accumulates weight in the appropriate tetra using template argument Accumulator
 *
 * Accumulator requirements:
 * 	Support operator[] returning some type T
 * 	T must support operator+= on Weight type
 * 	Copy-constructible
 * 	Constructor of type Accumulator(unsigned size,args...)
 *
 * Accumulator WorkerThread requirements:
 * 	Copy-constructible
 *
 *
 *	Examples: vector<T>& (single-thread), QueuedAccumulatorMT (thread-safe)
 */

template<class T>class LoggerVolume;
template<class T>ostream& operator<<(ostream& os,const VolumeArray<T>& lv);

template<class T>ostream& operator<<(ostream& os,const LoggerVolume<T>& lv)
{
	return os << lv.getResults();
}

template<class Accumulator>class LoggerVolume {
	Accumulator acc;
	const TetraMesh& mesh;

public:
	template<typename... Args>LoggerVolume(const TetraMesh& mesh_,Args... args) : acc(mesh_.getNt()+1,args...),mesh(mesh_){}
	LoggerVolume(LoggerVolume&& lv_) : acc(std::move(lv_.acc)),mesh(lv_.mesh){}
	LoggerVolume(const LoggerVolume& lv_) = delete;

	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread wt;
	public:
		typedef void logger_member_tag;
		WorkerThread(Accumulator& parent_) : wt(parent_.get_worker()){};
		WorkerThread(const WorkerThread& wt_) = delete;
		WorkerThread(WorkerThread&& wt_) : wt(std::move(wt_.wt)){}
		~WorkerThread(){ wt.commit(); }

	    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventAbsorb(const Packet& pkt,unsigned IDt,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventCommit(){ wt.commit(); }
	};

	typedef WorkerThread ThreadWorker;

	typedef VolumeArray<typename Accumulator::ElementType> ResultType;
	typedef true_type single_result_tag;

	WorkerThread get_worker() { return WorkerThread(acc);  };

	typedef VolumeArray<typename Accumulator::ElementType> result_type ;

	result_type getResults() const { return result_type(mesh,acc.getResults()); }
};

