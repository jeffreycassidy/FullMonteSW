#include <string>
#include <vector>
#include <sstream>

template<class T>class Sequence {
	std::string str;
	std::vector<T> v;

public:
	Sequence(){}
	Sequence(std::string str_){ parse(str); }

	void parse(const std::string& str_){ str=str_; std::stringstream ss(str); ss>>*this; }

	std::vector<T> as_vector() const { return v; }

	template<class U>friend std::istream& operator>>(std::istream& is,Sequence<U>& seq);
};

template<class T>std::istream& operator>>(std::istream& is,Sequence<T>& seq)
{
	T i,r_start;
	bool isRange=false,expectMore=false;

	seq.v.clear();

	do
	{
		is >> i;
		if (is.fail())
			break;

		if (is.peek()=='-')
		{
			is.ignore(1);
			if(isRange)			// if we're already in a range, this is a parse error
				break;
			else
			{					// otherwise start the range
				isRange=true;
				r_start=i;
			}
			expectMore=true;
		}
		else if (is.eof() || is.peek()==',')
		{
			expectMore=true;
			if (isRange)
				for(unsigned j=r_start;j<=i;++j)
					seq.v.push_back(j);
			else
				seq.v.push_back(i);
			isRange=false;
			if ((expectMore = (is.peek()==',')))
				is.ignore(1);
		}
	}
	while(!is.eof());

	if (expectMore)
		std::cerr << "Premature end of input" << std::endl;
	else
		is.clear(std::ios::eofbit);

	return is;
}
