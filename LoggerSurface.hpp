#include "logger.hpp"

// fluence needs mesh, materials, region ID, absorbed energy

/*class LoggerSurface {
    const TetraMesh& mesh;

    protected:
    vector<FluenceCountType> counts;

    public:
    LoggerSurface(const TetraMesh& mesh_) : mesh(mesh_),counts(mesh_.getNf()+1){};
    
    // get results as a map; optional arg per_area specifies to return fluence (per-area) or total energy per patch
    void resultMap(map<FaceByPointID,double>& m,bool per_area=true);
    void fluenceMap(SurfaceFluenceMap&);

    void hitMap(map<unsigned,unsigned long long>& m);
};*/

template<class T>class SurfaceArray {
	vector<T> s;
	const TetraMesh& mesh;

public:
    SurfaceArray(SurfaceArray&& ls_)        : mesh(ls_.mesh),s(std::move(ls_.s)){};
    SurfaceArray(const TetraMesh& mesh_)    : mesh(mesh_),s(mesh_.getNt()+1){};

        /// Returns a VolumeFluenceMap for the absorption accumulated so far*/
        /** The value of asFluence determines whether it returns total energy (false) or fluence as E/V/mu_a (true) */
    void fluenceMap(SurfaceFluenceMap&,bool asFluence=true);

        /// Returns (if available from AccumulatorT) a hit map
    void hitMap(map<unsigned,unsigned long long>& m);

    void resultMap(map<FaceByPointID,double>& m,bool per_area=true);

        /// Provides a way of summarizing to an ostream
    //friend ostream& operator<<(ostream&,VolumeArray&);
};


template<class Accumulator>class LoggerSurface : public LoggerNull {
	Accumulator acc;
public:

	/// Copy constructor
	//LoggerSurface(LoggerSurface& acc_) : acc(acc_){}
	template<typename... Args>LoggerSurface(const TetraMesh& mesh_,Args... args) : acc(mesh_.getNf()+1,args...){}
	LoggerSurface(LoggerSurface&& ls_) : acc(std::move(acc)){}
	LoggerSurface(const LoggerSurface& ls_) : acc(ls_.acc){}

	/// Record the exit event
	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread acc;
	public:
		WorkerThread(Accumulator& parent_) : acc(parent_.get_worker()){}
		WorkerThread(const WorkerThread& wt_) : acc(wt_.acc){}
		WorkerThread(WorkerThread&& wt_) : acc(std::move(wt_.acc)){}
		inline void eventExit(const Ray3,int IDf,double w){ acc[abs(IDf)] += w; }
	};

	LoggerSurface& operator+=(const WorkerThread&){}

	WorkerThread get_worker() { return WorkerThread(acc); }
};

template<class T>ostream& operator<<(ostream& os,const LoggerSurface<T>& ls)
{
	return os << "Hello from surface logger!" << endl;
}
