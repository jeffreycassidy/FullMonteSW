/*
 * PathScorer.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#include "PathScorer.hpp"

#include <FullMonteSW/OutputTypes/PacketPositionTraceSet.hpp>

PathScorer::PathScorer()
{

}

PathScorer::~PathScorer()
{

}

// lock to prevent conflicting access from commit() methods - shouldn't happen but just in case...
void PathScorer::clear()
{
	std::unique_lock<std::mutex> L(m_mutex);
	m_traces.clear();
}

// lock to accept a commit from a thread
void PathScorer::commit(PathScorer::Logger& L)
{
	std::vector<PacketPositionTrace::Step> T(L.m_trace);

	{
		std::unique_lock<std::mutex> L(m_mutex);
		PacketPositionTrace* tr = new PacketPositionTrace(std::move(T));
		m_traces.emplace_back(tr);
	}

	L.clear();
}

std::list<OutputData*> PathScorer::results() const
{
	OutputData* o = new PacketPositionTraceSet(m_traces);
	return std::list<OutputData*>{o};
}


PathScorer::Logger::Logger(std::size_t reserveSize) :
		m_reserveSize(reserveSize)
{
	m_trace.reserve(m_reserveSize);
}

PathScorer::Logger::~Logger()
{
}


void PathScorer::Logger::launch(__m128 pos)
{
	array<float,4> p;
	_mm_store_ps(p.data(),pos);
	m_length=0.0f;
	m_trace.emplace_back( Step{array<float,3>{{p[0],p[1],p[2]}}, 1.0f, 0.0f, 0.0f} );
}

void PathScorer::Logger::propagate(Point3 posvf,float w,float t)
{
	__m128 delta = posvf - m_lastPos;

	__m128 l2 = _mm_dp_ps(delta,delta,0x77);
	__m128 l  = _mm_sqrt_ss(l2);

	m_length += _mm_cvtss_f32(l);

	float f[4];
	_mm_store_ps(f,posvf);

	m_trace.emplace_back(Step{array<float,3>{f[0],f[1],f[2]},w,m_length,0.0f});
	m_lastPos = posvf;
}

void PathScorer::Logger::terminate(PathScorer& S)
{
	S.commit(*this);
	clear();
}


void PathScorer::Logger::clear()
{
	m_trace.clear();
	m_length=0.0f;
}

PathScorer::Logger PathScorer::get_logger()
{
	return Logger(m_reserveSize);
}
