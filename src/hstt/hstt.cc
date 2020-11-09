//
// Created by jens on 09-11-20.
//
#include "hstt.h"
#include <algorithm>

namespace simple_sat_solver::hstt {
int Hstt::FindId(std::vector<std::string> ids, std::string id) {
  for (int i = 0; i < ids.size(); ++i) {
    if (ids[i] == id)
      return i;
  }
  throw "ID not found";
}
//template <typename T>
//int Hstt::FindId(std::vector<T> list, std::string id,
//                 std::function<std::string(T)> func)  {
//  for (int i =0; i < list.size(); ++i) {
//    if (func(list[i]) == id)
//      return i;
//  }
//  throw "ID not found";
//}
}
