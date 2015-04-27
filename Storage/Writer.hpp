/*
 * Writer.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_WRITER_HPP_
#define STORAGE_WRITER_HPP_

#ifndef SWIG

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <FullMonte/Geometry/Material.hpp>

#endif

class Writer {
public:
	Writer(){}
	virtual void write(const TetraMesh&) const=0;
	virtual void write(const std::vector<SourceDescription*>&) const=0;
	virtual void write(const std::vector<Material>&) const=0;

	virtual ~Writer(){}

#ifndef SWIG

	class writer_exception : public std::exception {
		const char* msg_=nullptr;
	public:
		writer_exception(const char* e) : msg_(e){}
		virtual const char* what() const noexcept { return msg_; }
	};

	class write_exception : public writer_exception {
	public:
		write_exception(const char* e) : writer_exception(e){}
	};

	class open_for_write_exception : public writer_exception {
	public:
		open_for_write_exception(const char* e) : writer_exception(e){}
	};
#endif

};


#endif /* STORAGE_WRITER_HPP_ */
