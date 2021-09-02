#ifndef EXPLANATION_CLAUSE_H
#define EXPLANATION_CLAUSE_H

#include "../explanation_generic.h"
#include "../../Basic Data Structures/custom_vector.h"

namespace Pumpkin
{

class ExplanationClausal : public ExplanationGeneric
{
public:
	ExplanationClausal():p_literals_(0),ignore_position_(-1) {}; //todo possibly can remove this constructor?
	ExplanationClausal(const LiteralVector &literals);
	ExplanationClausal(const LiteralVector &literals, int ignore_position);

	void Clear();
	BooleanLiteral operator[](int index) const;
	ExplanationClausal & operator=(const ExplanationClausal &); //should be removed eventually, after I remove the legacy conflictanalysis code

	size_t Size() const;

private:
	const LiteralVector *p_literals_;
	int ignore_position_;
};

} //end Pumpkin namespace

#endif // !EXPLANATION_CLAUSE_H