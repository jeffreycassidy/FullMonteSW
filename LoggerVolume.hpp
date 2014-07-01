#include "logger.hpp"
#include "AccumulationArray.hpp"

template<class T>class VolumeArray {
	const TetraMesh& mesh;
	vector<T> v;

public:
    VolumeArray(VolumeArray&& lv_)        : mesh(lv_.mesh),v(mesh.getNt()+1){};
    VolumeArray(const TetraMesh& mesh_)    : mesh(mesh_),v(mesh.getNt()+1){};
    VolumeArray(const TetraMesh& mesh_,vector<T>&& v_) : mesh(mesh_),v(std::move(v_)){}

    /// Returns a VolumeFluenceMap for the absorption accumulated so far*/
    /** The value of asFluence determines whether it returns total energy (false) or fluence as E/V/mu_a (true) */
    void fluenceMap(VolumeFluenceMap&,const vector<Material>&,bool asFluence=true);

    /// Returns (if available from AccumulatorT) a hit map
    void hitMap(map<unsigned,unsigned long long>& m);

    typename vector<T>::const_iterator begin() const { return v.begin(); }
    typename vector<T>::const_iterator end()   const { return v.end(); }

        /// Provides a way of summarizing to an ostream
    //friend ostream& operator<<(ostream&,VolumeArray&);
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
		WorkerThread(Accumulator& parent_) : wt(parent_.get_worker()){};
		WorkerThread(const WorkerThread& wt_) = delete;
		WorkerThread(WorkerThread&& wt_) : wt(std::move(wt_.wt)){}
		~WorkerThread(){ wt.commit(); }

	    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventAbsorb(const Packet& pkt,unsigned IDt,double dw)
	    	{ wt[IDt] += dw; }
	};

	typedef WorkerThread ThreadWorker;

	WorkerThread get_worker() { return WorkerThread(acc);  };

	typedef VolumeArray<typename Accumulator::ElementType> result_type ;

	result_type getResults() const { return result_type(mesh,vector<typename Accumulator::ElementType>(acc.getResults())); }
};

