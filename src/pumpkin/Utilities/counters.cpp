#include "counters.h"

#include <iostream>

namespace Pumpkin
{

void Counters::PrintStats() const
{
	std::cout << "Num conflicts: " << conflicts << std::endl;
	std::cout << "Num units learned: " << unit_clauses_learned << std::endl;
	std::cout << "Num small LBD clauses learned: " << unit_clauses_learned << std::endl;
	std::cout << "Num restarts: " << num_restarts << std::endl;
	std::cout << "Avg restart length: " << double(conflicts) / num_restarts << std::endl;
	std::cout << "Blocked restarts: " << blocked_restarts << std::endl;
}

} //end Pumpkin namespace