#include "math.h"

// Standard Deviation:
//   sqrt(1/n*((x1 - avg)^2 + (x2 - avg)^2 + ... (xn - avg)^2))
double stdDev(const std::vector<double>& v, double& average)
{
	if (v.size() == 0)
		return 0;

	double total = 0;
	std::accumulate(v.begin(), v.end(), total);

	double mean = total/v.size();
	double inroot = 0;

	for (const double elem : v)
		inroot += std::pow(mean - elem, 2);
	
	average = mean;
	
	return std::sqrt(inroot/v.size());
}
