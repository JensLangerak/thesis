#include "database_pseudo_boolean.h"

namespace Pumpkin
{

DatabasePseudoBoolean::DatabasePseudoBoolean(uint64_t num_variables):
	watch_list_(num_variables)
{
}

/*DatabasePseudoBoolean::DatabasePseudoBoolean(MaxSATformula &formula)
	:database_(formula.sat_formula_.num_variables_)
{
	for (size_t m = 0; m < formula.pseudo_boolean_constraints_.size(); m++)
	{
		PseudoBooleanConstraint &pbc = formula.pseudo_boolean_constraints_[m];
		WatchedPseudoBooleanConstraint *constraint = new WatchedPseudoBooleanConstraint(pbc.literals, pbc.coefficients, pbc.right_hand_side);
		AddPermanentConstraint(constraint); //todo make sure memory management with new is okay
	}
}*/

void DatabasePseudoBoolean::AddPermanentConstraint(WatchedPseudoBooleanConstraint * constraint)
{
	permanent_constraints_.push_back(constraint);
	AddWatches(constraint);
}

void DatabasePseudoBoolean::AddTemporaryConstraint(WatchedPseudoBooleanConstraint * constraint)
{
	temporary_constraints_.push_back(constraint);
	AddWatches(constraint);
}

void DatabasePseudoBoolean::AddWatches(WatchedPseudoBooleanConstraint * constraint)
{
	for (int i = 0; i < constraint->num_watches_; i++)
	{
		watch_list_.Add(constraint->terms_[i].literal, constraint);
	}
}

} //end Pumpkin namespace