#include "cumulative_moving_average.h"

namespace Pumpkin
{

CumulativeMovingAverage::CumulativeMovingAverage()
	:sum_(0), num_terms(0)
{
}

void CumulativeMovingAverage::AddTerm(int new_term)
{
	sum_ += (long long) new_term;
	num_terms++;
}

double CumulativeMovingAverage::GetCurrentValue() const
{
	return double(((long double) sum_) / num_terms);
}

} //end Pumpkin namespace