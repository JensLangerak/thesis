//
// Created by jens on 01-06-21.
//

#include "max_sat_bench_mark.h"
#include <algorithm>
namespace simple_sat_solver::max_sat {
Pumpkin::ProblemSpecification MaxSatBenchMark::GetProblem() {
  Pumpkin::ProblemSpecification problem =Pumpkin::ProblemSpecification::ReadMaxSATFormula(problem_file_full_path_);
//  ClusterWeights(problem, 5);
  for (int i =0; i < problem.weighted_literals_.size(); ++i) {
    problem.weighted_literals_[i].weight = 1;
  }
  return problem;
}

struct WeightDiffIndex{
  int index;
  int diff;
  WeightDiffIndex(int index, int diff) : index(index), diff(diff) {};
  bool operator<(const WeightDiffIndex& rhs) const
  {
    return diff > rhs.diff;
  }
};
void MaxSatBenchMark::ClusterWeights(
    Pumpkin::ProblemSpecification & problem, int clusters) {
  auto lits = problem.weighted_literals_;
  assert(clusters > 0);
  std::vector<int> weights;
  for (auto l : lits) {
    weights.push_back(l.weight);
  }
  std::sort(weights.begin(), weights.end());
  std::vector<WeightDiffIndex> diffs;
  for (int i =1; i < weights.size(); ++i) {
    int d = weights[i] - weights[i-1];
    diffs.push_back(WeightDiffIndex(i, d));
  }
  std::sort(diffs.begin(), diffs.end());

  std::vector<int> boundaries_indices;
  for (int i = 0; i < clusters -1; ++i) {
    WeightDiffIndex diff = diffs[i];
    if (diffs[i].diff == 0)
      break;
    boundaries_indices.push_back(diff.index);
  }
  std::sort(boundaries_indices.begin(), boundaries_indices.end());

  std::vector<int> cluster_values;
  for (int i =0; i <= boundaries_indices.size(); ++i) {
    int start_index = 0;
    if (i > 0)
      start_index = boundaries_indices[i-1];
    int end_index= weights.size();
    if (i < boundaries_indices.size())
      end_index = boundaries_indices[i];
    int sum = 0;
    for (int j = start_index; j < end_index; ++j) {
      sum += weights[j];
    }
    cluster_values.push_back(sum / (end_index - start_index));
  }
  std::vector<int> boundary_weights;
  for (int i = 0; i < boundaries_indices.size(); ++i) {
    boundary_weights.push_back(weights[boundaries_indices[i]]);
  }

  int scale = cluster_values[0];
  for (int i = 0; i < cluster_values.size(); ++i) {
    cluster_values[i]/=scale;
  }


  for (int i = 0; i < problem.weighted_literals_.size(); ++i) {
    int cluster=0;
    for (int j =0; j < boundary_weights.size(); ++j) {
      if (boundary_weights[j] < problem.weighted_literals_[i].weight) {
        cluster = j + 1;
      } else {
        cluster = j;
        break;
      }
    }
    problem.weighted_literals_[i].weight = cluster_values[cluster];

  }


}
}
