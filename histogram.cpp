#include "histogram.hpp"

double Histogram::getPercentile(double p) const
{
    long i = p*double(v.size()-1);
    return v[i];
}

double Histogram::getPercentileOf(double x) const
{
    vector<double>::const_iterator lb=lower_bound(v.begin(),v.end(),x);
    return double(lb-v.begin())/double(v.size()-1);
}
