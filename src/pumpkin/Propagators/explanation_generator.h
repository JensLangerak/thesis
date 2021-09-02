#pragma once

#include <iostream>

namespace Pumpkin
{

template<typename ExplanationType>
class ExplanationGenerator
{
public:
	//todo - could add a method to preallocate a lot of explanations
	ExplanationGenerator():num_used_explanations_(0) {}
	~ExplanationGenerator() { for (ExplanationType* p : allocated_explanations_) { delete p; p = 0; } }

	ExplanationType* GetAnExplanationInstance()
	{
		//allocate new explanations if all current ones are already in use
		if (num_used_explanations_ == allocated_explanations_.size()) 
		{ 
			//std::cout << "c allocating more: " << allocated_explanations_.size() << "\n";  
			allocated_explanations_.push_back(new ExplanationType()); 
		}
		//return the next unused exlanation
		ExplanationType* explanation = allocated_explanations_[num_used_explanations_];
		num_used_explanations_++;
		return explanation;
	}

	void Reset()
	{
		num_used_explanations_ = 0;
	}

private:
	//each time the propagator is asked to explain, it reuses a previous explanation if possible or allocates a new one if it does not have any available explanation instances left
	//note that we need to allocate new instances with operator new and stores pointers rather than providing pointers to instances in the vector, since the vector class may reallocate its memory upon resizing/pushing 
	size_t num_used_explanations_;
	std::vector<ExplanationType*> allocated_explanations_;
};

}//end namespace Pumpkin