#pragma once

#include "../explanation_generic.h"
#include "../../Utilities/custom_vector.h"

namespace Pumpkin
{

class ExplanationClausal : public ExplanationGeneric
{
public:
	ExplanationClausal();
	ExplanationClausal(const LiteralVector& literals);
	ExplanationClausal(const LiteralVector& literals, int ignore_position);

	BooleanLiteral operator[](int index) const;	
	size_t Size() const;

	void Initialise(const LiteralVector& literals);
	void Initialise(const LiteralVector& literals, int ignore_position);

private:

	const LiteralVector* p_literals_;
	int ignore_position_;
};

} //end Pumpkin namespace