#include "logger.hpp"
#include "AccumulationArray.hpp"

/** Holds quantities accumulated over a surface, using sequential IDs ranging [0,N).
 * @tparam T	Type to be accumulated; must support operator[](unsigned), operator+=(double) and operator+=(T)
 */

template<class T>class SurfaceArray {
	const TetraMesh& mesh;
	vector<T> s;

public:
    SurfaceArray(SurfaceArray&& ls_)        : mesh(ls_.mesh),s(std::move(ls_.s)){};
    SurfaceArray(const TetraMesh& mesh_)    : mesh(mesh_),s(mesh_.getNt()+1){};
    SurfaceArray(const TetraMesh& mesh_,vector<T>&& s_) : mesh(mesh_),s(std::move(s_)){};
    SurfaceArray(const TetraMesh& mesh_,const vector<T>& s_) : mesh(mesh_),s(s_){};

    /// Returns a VolumeFluenceMap for the absorption accumulated so far
    /** The value of asFluence determines whether it returns total energy (false) or fluence as E/V/mu_a (true) */
    void fluenceMap(SurfaceFluenceMap&,bool asFluence=true);

    /// Returns a hit map TODO: Improve this comment and check how the function works
    void hitMap(map<unsigned,unsigned long long>& m);

    void resultMap(map<FaceByPointID,double>& m,bool per_area=true);

    typename vector<T>::const_iterator begin() const { return s.begin(); }
    typename vector<T>::const_iterator end()   const { return s.end(); }

    // Provides a way of summarizing to an ostream
    friend ostream& operator<<(ostream& os,const SurfaceArray& sa){
    	double sumE=0;
    	unsigned i=0;
    	for(auto it=sa.begin(); it != sa.end(); ++it)
    	{
    		sumE += *it;
    		++i;
    	}
    	return os << "Surface array total energy is " << setprecision(4) << sumE << " (" << i << " elements)" << endl;
    }
};

template<class T>ostream& operator<<(ostream& os,const SurfaceArray<T>&ls);
template<>ostream& operator<<(ostream& os,const SurfaceArray<double>& ls);

/** Handles logging of surface exit events.
 *
 * @tparam 	Accumulator		Must support the AccumulatorConcept.
 */

template<class Accumulator>class LoggerSurface {
	Accumulator acc;
	const TetraMesh& mesh;
public:

	typedef vector<typename Accumulator::ElementType> results_type;

	/** Construct and associate with a tetrahedral mesh.
	 * @param mesh_		Associated mesh, used to get the number of
	 * @param args...	Arguments to be passed through to the constructor for the underlying Accumulator type
	 */

	template<typename... Args>LoggerSurface(const TetraMesh& mesh_,Args... args) : acc(mesh_.getNf()+1,args...),mesh(mesh_){}
	LoggerSurface(LoggerSurface&& ls_) : acc(std::move(ls_.acc)),mesh(ls_.mesh){}

	/// Copy constructor deleted - no need for it
	LoggerSurface(const LoggerSurface& ls_) = delete;

	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread acc;
	public:
		/// Construct from an Accumulator by getting a worker thread from the parent
		WorkerThread(Accumulator& parent_) : acc(parent_.get_worker()){}

		/// Move constructor by simply moving the accumulator
		WorkerThread(WorkerThread&& wt_) : acc(std::move(wt_.acc)){}

		/// Copy constructor deleted
		WorkerThread(const WorkerThread& wt_) = delete;

		/// Commit results back to parent before deleting
		~WorkerThread() { acc.commit(); }

		/// Record exit event by accumulating weight to the appropriate surface entry
		inline void eventExit(const Ray3,int IDf,double w){ acc[abs(IDf)] += w; }
	};

	typedef WorkerThread ThreadWorker;

	/// Merge a worker thread's partial results
	LoggerSurface& operator+=(const WorkerThread& wt_){ wt_.commit(); return *this; }

	/// Return a worker thread
	WorkerThread get_worker() { return WorkerThread(acc); }

	typedef SurfaceArray<typename Accumulator::ElementType> result_type;

	result_type getResults() const { return result_type(mesh,acc.getResults()); }
};
