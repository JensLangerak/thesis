#include "explanation_clausal_binary.h"
#include "../../Utilities/runtime_assert.h"

namespace Pumpkin
{

ExplanationClausalBinary::ExplanationClausalBinary() {}

ExplanationClausalBinary::ExplanationClausalBinary(BooleanLiteral literal1)
{
    Initialise(literal1);
}

ExplanationClausalBinary::ExplanationClausalBinary(BooleanLiteral literal1, BooleanLiteral literal2)
{
    Initialise(literal1, literal2);
}

void ExplanationClausalBinary::Initialise(BooleanLiteral lit)
{
    runtime_assert(!lit.IsUndefined());

    literal1 = lit;
    literal2 = BooleanLiteral(); //explicitly setting it to the undefined literal. It will be ignored by the class
}

void ExplanationClausalBinary::Initialise(BooleanLiteral lit1, BooleanLiteral lit2)
{
    runtime_assert(!lit1.IsUndefined());
    runtime_assert(!lit2.IsUndefined());

    literal1 = lit1;
    literal2 = lit2;
}

BooleanLiteral ExplanationClausalBinary::operator[](int index) const
{
    runtime_assert(index >= 0 && index < 2);

    if (index == 0)      { return literal1; }
    else if (index == 1) { return literal2; }
}

size_t ExplanationClausalBinary::Size() const
{
    return 1 + size_t(!literal2.IsUndefined());
}

}//end namespace Pumpkin
