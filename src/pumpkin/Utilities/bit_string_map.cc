//
// Created by jens on 01-03-21.
//

#include "bit_string_map.h"
#include <cassert>

namespace Pumpkin {

BitStringMap::BitStringMap(int num_entries) : values_(num_entries), count_(0) {}
void BitStringMap::Increment(int key_id) {
  if (values_[key_id].size() > 0 && values_[key_id].back() == count_)
    return;
  values_[key_id].push_back(count_); }
void BitStringMap::Grow() { values_.push_back(std::vector<int>()); }
std::vector<int> BitStringMap::GetKeyValue(int key_id) const {
  return values_[key_id];
}
bool BitStringMap::IsKeyPresent(int key_id) const {
  return key_id < values_.size();
}
int BitStringMap::Size() const { return values_.size(); }
bool BitStringMap::Empty() const { return Size() == 0; }
void BitStringMap::IncreaseId() {
  count_++;
  assert(count_ >= 0);
}
int BitStringMap::HammingDistance(std::vector<int> &l , std::vector<int> &r) {
  int distance = 0;

  int i = 0;
  int j = 0;
  while(i < l.size() && j <r.size()) {
    int current_l = l[i];
    int current_r = r[j];
    if (current_l == current_r) {
      i++;
      j++;
    } else {
      distance++;
      if (current_l < current_r) {
        i++;
      }else {
        j++;
      }
    }


  }
  if (i < l.size())
    distance += l.size() - i;
  if (j < r.size())
    distance += r.size() -j;
  return distance;

}
} // namespace Pumpkin