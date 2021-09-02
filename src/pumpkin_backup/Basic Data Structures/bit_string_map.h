//
// Created by jens on 01-03-21.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_BIT_STRING_MAP_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_BIT_STRING_MAP_H_
#include <vector>

namespace Pumpkin {


class BitStringMap {
public:
  explicit BitStringMap(int num_entries); //create a heap with keys [0, ..., num_entries-1] and values all set to zero. Internally 'reserve' entry spaces will be allocated.
  //no need for now ~KeyValueHeap();

  void Increment(int key_id); //increments the value of the element of 'key_id' by increment. O(logn) worst case, but average case might be better.
  void Grow(); //increases the Size of the heap by one. The key will have zero assigned to its value.

  std::vector<int> GetKeyValue(int key_id) const;
  bool IsKeyPresent(int key_id) const;
  int Size() const;  //returns the Size of the heap. O(1)
  bool Empty() const; //return if the heap is empty, i.e. Size() == 0.
  void IncreaseId();

  int HammingDistance(std::vector<int> & l, std::vector<int> & r);

private:
  std::vector<std::vector<int>> values_; //contains the values stored as a heap
  int count_;

};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_BIT_STRING_MAP_H_
