#ifndef LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_H
#define LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_H

#include "boolean_literal.h"
#include "vector_object_indexed.h"
#include "small_helper_structures.h"

#include <vector>
#include <string>

namespace Pumpkin
{

class SolverState;
class LearnedPseudoBooleanConstraintIterator;

//TODO: I need to make sure the overflows do not happen
//take a look into the arguments and so forth, it's not consistent in the current version, e.g., the parameter might be 64 bit but we only accept adding a 32bit to it

//BUG: some literals might already be set, and this can effect the right hand side
//need to take this into account
//should I do this in the end of resolution, or immediately at the start, does it make a difference?
//I think I resolved this - at the moment during conflict analysis this is directly taken into account, so no literal should be fixed at root I think

class LearnedPseudoBooleanConstraint
{
public:
	LearnedPseudoBooleanConstraint(size_t num_variables);
	LearnedPseudoBooleanConstraint(const LearnedPseudoBooleanConstraint&);
	
	void RoundToOne(BooleanLiteral, SolverState &state); //seems out of place, perhaps should be in another part

	void AddTerm(const Term&);
	void AddToRightHandSide(uint32_t val);
	void RemoveFromRightHandSide(uint32_t val);
	void MultiplyByFraction(uint64_t numerator, uint64_t denominator); //multiplies each coefficient and the right hand side by the input fraction. Special care is taken to avoid overflow. The assumption is that the result of the multiplication will be an integer. 

	bool IsVariablePresent(BooleanVariable);
	bool IsClause() const;

	void RemoveZeroCoefficients();
	bool CheckZeroCoefficients();	

	LearnedPseudoBooleanConstraintIterator GetIterator();

	/*std::vector<BooleanLiteral> GetRootAssignments() {
		assert(1 == 2);
		return std::vector<BooleanLiteral>();
	}//return GetPropagationsFromDecisionLevel(0).propagation_literals; }
	
	std::vector<BooleanLiteral> GetCurrentPropagations(SolverState &state) { 
		assert(1 == 2);
		return std::vector<BooleanLiteral>();
		//return GetPropagationsFromDecisionLevel(state.GetCurrentDecisionLevel());
	}*/

	//problem: root assignments are needed to detect unit clauses
	//but otherwise why do we need propagation...
	//need to simply ask the propagator to do it, give it a constraint and say propagate
	//does conflict analysis guarantee that one falsified literal will be set to one? I think so, right? ...
	//this method should be in the propagator, not here...but I am hacking it here for now, needs to changed later
	PairUnsatFlagPropagationLiterals GetPropagationsFromDecisionLevel(int decision_level, SolverState &state); //computes the propagations that (should have) took place at the given decision level

	size_t ComputeLBD(const SolverState & state) const;

	uint64_t GetCoefficient(BooleanLiteral); //returns the coefficient. Note that it is not a constant method since it might internally change the coefficient due to saturation
	std::vector<BooleanLiteral> GetLiterals() const;
	//BooleanLiteral GetJthLiteral(size_t index) const; //note that the literal might have a zero coefficient. Maybe an iterator interface would make more sense to remove the zero coefficient on the fly? todo
	int64_t GetRightHandSide() const;
	bool Empty() const;
	size_t Size() const;

	void Grow();
	void Clear();	

	bool CheckOverflow();

	void Print();

	std::string ToString();
//private:
	//should implicitly saturate?
	void Saturate(); //probably will remove this method in the future, I think it is not necessary with the on-the-fly saturation
	void SaturateCoefficient(BooleanLiteral literal);
	void WeakenLiteral(BooleanLiteral literal);
	uint64_t DivideRoundUp(uint64_t n, uint64_t divisor);

	bool CheckClear();
	bool CheckCorrectness(BooleanLiteral literal) const;

	void RemoveZeroCoefficientVariableFromDataStructures(BooleanVariable zero_variable); //removes the term associated with the literal. Only used to remove literals that have a zero coefficient from the variable_location vector. Remember that this changes the internal data structures, so all iterators are no longer valid.

	std::vector<BooleanLiteral> present_literals_; //keep tracks of all literals that were used in this constraint. Note that some of these literals might have zero coefficients due to cancellations. Used to make Clear() more efficient. note: I am not sure, but I think we can't just remove the zero-coefficient literals from this vector, since I think the algorithms needs to know which literals were used...not sure, something to think about.
	VectorObjectIndexed<BooleanVariable, int64_t> variable_location_; //a direct hashmap f: variable -> index, indicating the position of the literal in 'present_literals_' or -1 if it is not present. Note that the index is the variable, not the literal
	VectorObjectIndexed<BooleanLiteral, int64_t> map_literal_coefficient64_; //a direct hashmap f: literal -> coefficient64
	int64_t right_hand_side64_;

	friend LearnedPseudoBooleanConstraintIterator;
};

} //end Pumpkin namespace

#endif // !LEARNED_PSEUDO_BOOLEAN_CONSTRAINT_H