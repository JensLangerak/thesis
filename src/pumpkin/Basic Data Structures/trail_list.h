//
// Created by jens on 18-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_TRAIL_LIST_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_TRAIL_LIST_H_
#include <assert.h>
#include <cstddef>
namespace Pumpkin {
template <typename T> class TrailList {
private:
  struct Node {
    T data;
    bool valid_data;
    Node *next;
    Node *previous;

    Node(T data, Node *previous, Node *next)
        : data(data), next(next), previous(previous), valid_data(true){};
  };
public:
  struct Iterator {
  public:
    Iterator(Node *node) : node(node){};
    T GetData() const { return node->data; };
    void Next() {
      assert(node->next != nullptr);
      node = node->next;
    };
    void Previous() {
      assert(node->previous != nullptr);
      node = node->previous;
    };
    bool IsLast() const { return node->next == nullptr; };
    bool IsPastTrail() const { return node->valid_data == false;}
    bool IsFirst() const { return node->previous == nullptr; };

    T operator*() const { return GetData(); };
    Iterator& operator++() {
      Next();
      return *this;
    };
    Iterator& operator--() {
      Previous();
      return *this;
    };

    Iterator operator++(int) {
      Iterator ret = *this;
      this->operator++();
      return ret;
    };

    Iterator operator--(int) {
      Iterator ret = *this;
      this->operator--();
      return ret;
    };


    bool operator==(Iterator o) {
      return o.node == this->node;
    };

    bool operator!=(Iterator o) {
      return o.node != this->node;
    };

  private:
    Node *node;
    friend class TrailList;
  };

  TrailList();
  ~TrailList();
  T front() const {return begin_->data;};
  T back() const {return end_->previous->data;};

  Iterator begin() const {return Iterator(begin_); };
  Iterator end() const {return  Iterator(end_);};
  Iterator last() const {return Iterator(end_->previous);};

  void push_back(T data);
  void pop_back();
  void insert(Iterator place, T data);

  size_t size() const { return size_;};

private:
  struct Node;
  Node *begin_;
  Node *end_;
  size_t size_;

  Node *removed_nodes_;
};

} // namespace Pumpkin

#endif // SIMPLESATSOLVER_SRC_PUMPKIN_BASIC_DATA_STRUCTURES_TRAIL_LIST_H_
