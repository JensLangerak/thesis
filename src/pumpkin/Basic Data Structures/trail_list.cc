//
// Created by jens on 18-10-20.
//

#include "trail_list.h"
#include "boolean_literal.h"

namespace Pumpkin {
template <typename T> TrailList<T>::TrailList() :size_(0), removed_nodes_(nullptr){
  begin_ = new Node(T(), nullptr, nullptr);
  end_ = begin_;
}
//template <typename T> T TrailList<T>::front() const { return begin_->data; }
//template <typename T> T TrailList<T>::back() const { return end_->previous->data; }
//template <typename T> class TrailList<T>::Iterator TrailList<T>::begin() const {
//  return begin_;
//}
//template <typename T> class TrailList<T>::Iterator TrailList<T>::end() const {
//  return end_;
//}
template <typename T> void TrailList<T>::push_back(T data) {
  end_->data = data;
  end_->valid_data = true;
  if (removed_nodes_ == nullptr) {
    end_->next = new Node(T(), end_, nullptr);
  } else {
    end_->next = removed_nodes_;
    removed_nodes_ = removed_nodes_->next;
    end_->next->next = nullptr;
  }
  end_ = end_->next;
  end_->valid_data = false;
  ++size_;
}
template <typename T> void TrailList<T>::pop_back() {
  end_->next = removed_nodes_;
  removed_nodes_ = end_;
  end_ = removed_nodes_->previous;
  //removed_nodes_->valid_data = false;
  end_->next = nullptr;
  end_->valid_data = false;
  --size_;
}
template <typename T>
void TrailList<T>::insert(TrailList::Iterator place, T data) {
  Node * n = new Node(data, place.node->previous, place.node);
  n->previous->next = n;
  n->next->previous = n;
  ++size_;
}
template <typename T> TrailList<T>::~TrailList() {
  Node * next = begin_;
  Node * current;
  while (next != nullptr) {
    current = next;
    next = next->next;
    delete current;
  }
  next = removed_nodes_;

  while (next != nullptr) {
    current = next;
    next = next->next;
    delete current;
  }
}
//template <typename T> class TrailList<T>::Iterator TrailList<T>::last() const {
//  return end().Previous();
//}

template class TrailList<BooleanLiteral>;

}