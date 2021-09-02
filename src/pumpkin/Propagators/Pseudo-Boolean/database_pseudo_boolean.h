#ifndef DATABASE_PSEUDO_BOOLEAN_H
#define DATABASE_PSEUDO_BOOLEAN_H

#include "watch_list_pseudo_boolean.h"

namespace Pumpkin
{

class DatabasePseudoBoolean
{
public:
	DatabasePseudoBoolean(uint64_t num_variables);

	void AddPermanentConstraint(WatchedPseudoBooleanConstraint *constraint);
	void AddTemporaryConstraint(WatchedPseudoBooleanConstraint *constraint);

	WatchListPseudoBoolean watch_list_;
	std::vector<WatchedPseudoBooleanConstraint*> permanent_constraints_, temporary_constraints_;

private:

	void AddWatches(WatchedPseudoBooleanConstraint *constraint);
};

} //end Pumpkin namespace

#endif // !DATABASE_PSEUDO_BOOLEAN_H
