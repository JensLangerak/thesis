#ifndef VECTOR_OBJECT_INDEXED_H_
#define VECTOR_OBJECT_INDEXED_H

#include <vector>

namespace Pumpkin
{

//a class merely for convenience
//used to have standard vectors that are indexed directly by literals or variables
template <class ObjectType, class DataType>
class VectorObjectIndexed
{
public:
	VectorObjectIndexed(size_t num_entries = 0, DataType initial_values = DataType()) :data_(num_entries, initial_values) {}
	void push_back(const DataType &val) { data_.push_back(val); }

	const DataType& operator[](const ObjectType &obj) const
	{
		return data_[obj.ToPositiveInteger()];
	}

	DataType& operator[](const ObjectType &obj)
	{
		return data_[obj.ToPositiveInteger()];
	}

	typename std::vector<DataType>::const_iterator begin() const { return data_.begin(); }
	typename std::vector<DataType>::const_iterator end() const { return data_.end(); }
	
private:
	std::vector<DataType> data_;
};

} //end Pumpkin namespace

#endif // !VECTOR_OBJECT_INDEXED_H_