#include <vector>
#include <algorithm>

using namespace std;

class Histogram {
    vector<double> v;
    public:

    Histogram();
    template<class Iterator>Histogram(Iterator begin,Iterator end,unsigned sizeHint=0);

    double getMin() const { return v.front(); }
    double getMax() const { return v.back();  }

    double getMedian() const { return v[v.size()/2]; }

    pair<double,double> getRange(double pl=0.0,double pu=1.0) const { return make_pair(getPercentile(pl),getPercentile(pu)); }

    double getPercentileOf(double) const;
    double getPercentile(double) const;
};

template<class Iterator>Histogram::Histogram(Iterator begin,Iterator end,unsigned sizeHint)
{
    v.reserve(sizeHint);
    for(; begin != end; ++begin)
        v.push_back(*begin);
    sort(v.begin(),v.end());
}
