#ifndef KERNELS_SOFTWARE_LOGGERS_INTERNALSURFACE_HPP_
#define KERNELS_SOFTWARE_LOGGERS_INTERNALSURFACE_HPP_
#include "Logger.hpp"
#include <FullMonte/Kernels/Software/Packet.hpp>
#include <FullMonte/OutputTypes/FluenceMapBase.hpp>
#include <type_traits>

/** Accumulator type that tracks which side of the face the packet crossed to/from.
 * Per usual convention, normals point into the tetra. Crossing against a normal ( dot(d,n) < 0 ) means exiting the half-space
 * defined by the face normal/constant. For IDf < 0, the tetra face is inverted wrt the face in the array.
 *
 *            Face IDf
 * Tet IDtA -----------> Tet IDtB
 *
 * IDf > 0 -> dot(d,n) < 0 -> exiting halfspace defined by face
 * IDf < 0 -> dot(d,n) > 0 -> entering halfspace defined by face
 *
 */


struct FaceCrossingAccumulator
{
	enum Orientation { Positive, Negative };
	FaceCrossingAccumulator(){}
	FaceCrossingAccumulator(double w,Orientation orient)
	{
		if (orient == Positive)
			exitCount=w;
		else
			enterCount=w;
	}

	FaceCrossingAccumulator(const FaceCrossingAccumulator&) = default;

	double exitCount=0.0;		// Positive crossings via IDf > 0
	double enterCount=0.0;		// Negative crossings via IDf < 0

	FaceCrossingAccumulator& operator+=(const FaceCrossingAccumulator rhs)
				{
		exitCount += rhs.exitCount;
		enterCount += rhs.enterCount;
		return *this;
				}
};


/** Handles logging of internal surface crossing events.
 *
 * @tparam 	Accumulator		Must support the AccumulatorConcept.
 */

template<class Accumulator>class LoggerInternalSurface
{
public:

	typedef std::true_type is_logger;

	class ThreadWorker : public LoggerBase
	{
		typename Accumulator::ThreadWorker acc;
	public:
		typedef std::true_type is_logger;

		/// Construct from an Accumulator by getting a worker thread from the parent
		ThreadWorker(Accumulator& parent_,unsigned Nq) : acc(parent_.get_worker(Nq)){}

		/// Move constructor by simply moving the accumulator
		ThreadWorker(ThreadWorker&& wt_) : acc(std::move(wt_.acc)){}

		/// Copy constructor deleted
		ThreadWorker(const ThreadWorker& wt_) = delete;

		/// Commit results back to parent before deleting
		~ThreadWorker() { acc.commit(); }

		inline void eventNewTetra(const Packet& pkt,const Tetra& tet,const unsigned idx)
		{
			int IDf=tet.IDfs[idx];
			if ((tet.faceFlags >> (idx << 3)) & 0x1)		// check if LSB of faceFlags is set for this face
			{
				if (IDf < 0)
					acc[-IDf] += FaceCrossingAccumulator(pkt.w,FaceCrossingAccumulator::Negative);
				else
					acc[IDf]  += FaceCrossingAccumulator(pkt.w,FaceCrossingAccumulator::Positive);
			}
		}

		inline void eventClear(){ acc.clear(); }

		void eventCommit(){ acc.commit(); }
	};

	void resize(unsigned N){ acc.resize(N); }
	void qSize(unsigned Nq){ m_Nq=Nq; }

	/// Return a worker thread
	ThreadWorker get_worker() { return ThreadWorker(acc,m_Nq); }

	static std::list<OutputData*> results(const std::vector<FaceCrossingAccumulator>& values)
	{
		// convert to float
		std::vector<InternalSurface<float>> se(values.size());

		cout << "Results for LoggerInternalSurface: values.size()=" << values.size() << endl;

		for(unsigned i=0;i<values.size();++i)
			se[i] = InternalSurface<float> { float(values[i].exitCount), float(values[i].enterCount) };

		// create vector
		SpatialMapBase<InternalSurface<float>,unsigned> *smap = SpatialMapBase<InternalSurface<float>,unsigned>::newFromVector(std::move(se));
		OutputData* O = new InternalSurfaceEnergyMap(smap);
		std::list<OutputData*> L;
		L.push_back(O);
		return L;
	}

	std::list<OutputData*> results() const { return results(acc.values()); }

	inline void clear()
	{
		acc.clear();
	}

	inline void eventClear()
	{
		clear();
	}

private:
	Accumulator acc;
	unsigned m_Nq=1<<14;
};

#endif
