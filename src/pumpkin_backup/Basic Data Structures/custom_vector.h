#ifndef CUSTOM_VECTOR_H
#define CUSTOM_VECTOR_H

#include "boolean_literal.h"

#include <cassert>
#include <algorithm>
#include <vector>

namespace Pumpkin
{

class LiteralVector
{
public:
	//~LiteralVector(); for now
	
	const BooleanLiteral* begin() const; //should return const pointer? not sure
	const BooleanLiteral* end() const ;

	size_t Size() const;
	
	//T back();
	BooleanLiteral& operator[](int index);
	BooleanLiteral operator[](int index) const;

	size_t size_;
	BooleanLiteral literals_[0];

        bool operator==(LiteralVector o) const {bool res = this->Size() == o.Size();
        if (!res)
          return false;
        for (int i = 0; i < this->Size(); ++i) {
          if (literals_[i] != o.literals_[i])
            return false;
        }
        return true;};
};

} //end Pumpkin namespace

#endif // !CUSTOM_VECTOR_H