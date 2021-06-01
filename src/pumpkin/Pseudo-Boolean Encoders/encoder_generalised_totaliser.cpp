#include "encoder_generalised_totaliser.h"

namespace Pumpkin
{
std::vector<PairWeightLiteral> EncoderGeneralisedTotaliser::SoftLessOrEqual(std::vector<PairWeightLiteral>& input_literals, int right_hand_side, SolverState& state)
{
    runtime_assert(right_hand_side >= 0);

    //trivial check
    int64_t sum = 0; //todo check for overflows...
    for (PairWeightLiteral& pair : input_literals) { sum += pair.weight; }

    if (sum <= right_hand_side) { return std::vector<PairWeightLiteral>(); }

    //todo consider negating input literals

    std::vector<PairWeightLiteral> processed_input_literals = input_literals;
    std::sort(processed_input_literals.begin(), processed_input_literals.end(), [](const PairWeightLiteral& p1, const PairWeightLiteral& p2) { return p1.weight < p2.weight; });

    //the generalised totaliser encoding can be visualised as a binary tree
    //  the leaf nodes are the input literals
    //  each node above the leafs represents the sum of two child nodes, where each partial sum is represented by a new literal
    //  the final/root layer contains all partial sums of the input variables
    //  at the leaf layer, each node contains one literal
    //  each layer has half of the number of nodes as the previous layer

    //  the input literals are used to initialise the current layer, where each literal is one node
    Layer* current_layer = &helper1_, * next_layer = &helper2_;
    current_layer->InitialiseLayer(processed_input_literals);

    //  in each iteration, the literals of the next_layer are created and appropriate clauses are added to capture the sum.
    for (int iteration_counter = 1; iteration_counter < processed_input_literals.size(); iteration_counter *= 2)
    {
        next_layer->Clear();
        int num_nodes = current_layer->NumNodes();
        
        //neighbouring nodes of the current layer are merged and their sum is represented in their parent node that is stored in the next layer
        //  we merge the first and the second node (merge_index = 0), then the third and the fourth node (merge_index = 1), and so on
        for (int merge_index = 0; merge_index < num_nodes / 2; merge_index++)
        {
            //these are the indicies of the two nodes that will be merged in this step
            int node1_index = 2*merge_index;
            int node2_index = (2*merge_index) + 1;
            //the result of merge the two nodes will be stored in the next layer in node with index merge_index
            next_layer->AddNode();

            //create new variables and record their indicies as appropriate
            //two stage process: identify partial sums and then create the literals
            
            //  first compute the necessary partial sums
            value_to_index_map_.clear();
            partial_sums_.clear();
            for (int i = current_layer->NodeStart(node1_index); i < current_layer->NodeEnd(node1_index); i++)
            {
                partial_sums_.insert(current_layer->pairs[i].weight);
                for (int j = current_layer->NodeStart(node2_index); j < current_layer->NodeEnd(node2_index); j++)
                {
                    partial_sums_.insert(current_layer->pairs[j].weight);
                    partial_sums_.insert(current_layer->pairs[i].weight + current_layer->pairs[j].weight);
                }
            }
            //  then create the variables, one for each partial sum, and register their positions in the layer
            for (int64_t partial_sum : partial_sums_)
            {
                BooleanLiteral literal = BooleanLiteral(state.CreateNewVariable(), true);
                //record the index position...
                value_to_index_map_[partial_sum] = next_layer->pairs.size();
                //...and add the literal to the last node
                next_layer->pairs.push_back(PairWeightLiteral(literal, partial_sum));
                next_layer->node_start.back()++;
            }

            //this loop could be merged with the previous code, but I am not sure if this would make a big difference
            //now perform the merge to define the new variables / summation
            for (int i = current_layer->NodeStart(node1_index); i < current_layer->NodeEnd(node1_index); i++)
            {
                //node1[i].weight -> sum[weight]
                BooleanLiteral node1_literal = current_layer->pairs.at(i).literal;
                int64_t node1_weight = current_layer->pairs.at(i).weight;
                state.AddBinaryClause(~node1_literal, next_layer->pairs.at(value_to_index_map_[node1_weight]).literal);
                for (int j = current_layer->NodeStart(node2_index); j < current_layer->NodeEnd(node2_index); j++)
                {
                    BooleanLiteral node2_literal = current_layer->pairs.at(j).literal;
                    int64_t node2_weight = current_layer->pairs.at(j).weight;

                    //node2[i].weight -> sum[weight]
                    state.AddBinaryClause(~node2_literal, next_layer->pairs.at(value_to_index_map_[node2_weight]).literal);
                    //node1[i].weight + node2[i].weight -> sum[node1[i].weight + node2[i].weight]
                    state.AddTernaryClause(
                        ~node1_literal,
                        ~node2_literal,
                        next_layer->pairs.at(value_to_index_map_[node1_weight + node2_weight]).literal);
                }
            }
        }
        //copy over the odd-numbered node that will not merge this round
        if (num_nodes % 2 == 1)
        {
            next_layer->AddNode();
            for (int i = current_layer->NodeStart(num_nodes - 1); i < current_layer->NodeEnd(num_nodes - 1); i++)
            {
                next_layer->pairs.push_back(current_layer->pairs[i]);
                next_layer->node_start.back()++;
            }
        }        
        //the next layer has been successfully encoded, now use it as the current layer and iterate
        std::swap(current_layer, next_layer);
    }

    //current_layer now stores the final sum literals
    //  i.e., if the sum of input literals is at least k, then the first m literals that sum up to k will be set to true
    //todo explain this a bit better
    //  note that the converse does not hold in the current version

    //the first 'right_hand_side' number of literals in current_layer are not restricted by the constraint
    //the remaining literals are the ones capture violations
    //  only these will be inserted into the output_literals vector

    static std::vector<PairWeightLiteral> output_literals;
    output_literals.clear();
    for (PairWeightLiteral& p : current_layer->pairs)
    {
        if (p.weight > right_hand_side && output_literals.empty())
        {
            output_literals.push_back(PairWeightLiteral(p.literal, p.weight - right_hand_side));
        }        
        else if (p.weight > right_hand_side && !output_literals.empty())
        {
            output_literals.push_back(PairWeightLiteral(p.literal, p.weight - right_hand_side - output_literals.back().weight));
        }
    } 

    //store the result
    encoded_soft_constraints_.push_back(EncodedConstraint());
    encoded_soft_constraints_.back().input_literals = input_literals;
    encoded_soft_constraints_.back().output_literals = output_literals;
    encoded_soft_constraints_.back().right_hand_side = right_hand_side;

    return output_literals;
}

std::vector<PairWeightLiteral> EncoderGeneralisedTotaliser::HardLessOrEqual(std::vector<PairWeightLiteral>& input_literals, int right_hand_side, SolverState& state)
{
    //a lot of code is copy-pasted from the soft version; todo combine the two codes
    runtime_assert(right_hand_side >= 0);
    literals_from_last_hard_call_.clear();

    //trivial check
    int64_t sum = 0; //todo check for overflows...
    for (PairWeightLiteral& pair : input_literals) { sum += pair.weight; }

    if (sum <= right_hand_side) { return std::vector<PairWeightLiteral>(); }

    //todo consider negating input literals

    //only consider literals with weights less or equal to the right hand side
    //  literals with high weight are set to false in the solver
    std::vector<PairWeightLiteral> processed_input_literals;
    processed_input_literals.reserve(input_literals.size());
    for (PairWeightLiteral& pair : input_literals) 
    { 
        if (pair.weight <= right_hand_side) 
        { 
            processed_input_literals.push_back(pair); 
        }
        else
        {
            bool success = state.AddUnitClause(~pair.literal);
            if (!success) { std::cout << "strange, cannot add unit clause, unsat but it is not expected the formula would be unsat at this point!\n"; }
        }    
    }

    //std::sort(processed_input_literals.begin(), processed_input_literals.end(), [](const PairWeightLiteral& p1, const PairWeightLiteral& p2) { return p1.weight < p2.weight; });

    //the generalised totaliser encoding can be visualised as a binary tree
    //  the leaf nodes are the input literals
    //  each node above the leafs represents the sum of two child nodes, where each partial sum is represented by a new literal
    //  the final/root layer contains all partial sums of the input variables
    //  at the leaf layer, each node contains one literal
    //  each layer has half of the number of nodes as the previous layer

    //  the input literals are used to initialise the current layer, where each literal is one node
    Layer* current_layer = &helper1_, * next_layer = &helper2_;
    current_layer->InitialiseLayer(processed_input_literals);
    literals_from_last_hard_call_.push_back(processed_input_literals);

    //  in each iteration, the literals of the next_layer are created and appropriate clauses are added to capture the sum.
    for (int iteration_counter = 1; iteration_counter < processed_input_literals.size(); iteration_counter *= 2)
    {
        debug_last_call_.push_back(*current_layer);
        next_layer->Clear();
        int num_nodes = current_layer->NumNodes();

        //neighbouring nodes of the current layer are merged and their sum is represented in their parent node that is stored in the next layer
        //  we merge the first and the second node (merge_index = 0), then the third and the fourth node (merge_index = 1), and so on
        for (int merge_index = 0; merge_index < num_nodes / 2; merge_index++)
        {
            //these are the indicies of the two nodes that will be merged in this step
            int node1_index = 2 * merge_index;
            int node2_index = (2 * merge_index) + 1;
            //the result of merge the two nodes will be stored in the next layer in node with index merge_index
            next_layer->AddNode();

            //create new variables and record their indicies as appropriate
            //two stage process: identify partial sums and then create the literals

            //  first compute the necessary partial sums
            value_to_index_map_.clear();
            partial_sums_.clear();
            for (int i = current_layer->NodeStart(node1_index); i < current_layer->NodeEnd(node1_index); i++)
            {
                runtime_assert(current_layer->pairs[i].weight <= right_hand_side);
                partial_sums_.insert(current_layer->pairs[i].weight);
                for (int j = current_layer->NodeStart(node2_index); j < current_layer->NodeEnd(node2_index); j++)
                {
                    runtime_assert(current_layer->pairs[j].weight <= right_hand_side);
                    partial_sums_.insert(current_layer->pairs[j].weight);
                    int64_t combined_weight = current_layer->pairs[i].weight + current_layer->pairs[j].weight;
                    if (combined_weight <= right_hand_side) { partial_sums_.insert(combined_weight); }
                }
            }
            //  then create the variables, one for each partial sum, and register their positions in the layer
            for (int64_t partial_sum : partial_sums_)
            {
                BooleanLiteral literal = BooleanLiteral(state.CreateNewVariable(), true);
                //record the index position...
                value_to_index_map_[partial_sum] = next_layer->pairs.size();
                //...and add the literal to the last node
                next_layer->pairs.push_back(PairWeightLiteral(literal, partial_sum));
                next_layer->node_start.back()++;
            }

            //this loop could be merged with the previous code, but I am not sure if this would make a big difference
            //now perform the merge to define the new variables / summation
            for (int i = current_layer->NodeStart(node1_index); i < current_layer->NodeEnd(node1_index); i++)
            {
                //node1[i].weight -> sum[weight]
                BooleanLiteral node1_literal = current_layer->pairs.at(i).literal;
                int64_t node1_weight = current_layer->pairs.at(i).weight;
                runtime_assert(node1_weight <= right_hand_side);
                auto conflicting_propagator = state.AddBinaryClause(~node1_literal, next_layer->pairs.at(value_to_index_map_[node1_weight]).literal);
                runtime_assert(conflicting_propagator == NULL);
                for (int j = current_layer->NodeStart(node2_index); j < current_layer->NodeEnd(node2_index); j++)
                {
                    runtime_assert(i < j);
                    BooleanLiteral node2_literal = current_layer->pairs.at(j).literal;
                    int64_t node2_weight = current_layer->pairs.at(j).weight;
                    runtime_assert(node2_weight <= right_hand_side);

                    //node2[i].weight -> sum[weight]
                    conflicting_propagator = state.AddBinaryClause(~node2_literal, next_layer->pairs.at(value_to_index_map_[node2_weight]).literal);
                    runtime_assert(conflicting_propagator == NULL);
                    if (node1_weight + node2_weight <= right_hand_side)
                    {
                        //node1[i].weight + node2[i].weight -> sum[node1[i].weight + node2[i].weight]
                        conflicting_propagator = state.AddTernaryClause(
                            ~node1_literal,
                            ~node2_literal,
                            next_layer->pairs.at(value_to_index_map_[node1_weight + node2_weight]).literal);
                        runtime_assert(conflicting_propagator == NULL);
                    }
                    else //forbid the assignment of both literals
                    {
                        conflicting_propagator = state.AddBinaryClause(~node1_literal, ~node2_literal);
                        runtime_assert(conflicting_propagator == NULL);
                    }
                }
            }
        }
        //copy over the odd-numbered node that will not merge this round
        if (num_nodes % 2 == 1)
        {
            next_layer->AddNode();
            for (int i = current_layer->NodeStart(num_nodes - 1); i < current_layer->NodeEnd(num_nodes - 1); i++)
            {
                next_layer->pairs.push_back(current_layer->pairs[i]);
                next_layer->node_start.back()++;
            }
        }
        //the next layer has been successfully encoded, now use it as the current layer and iterate
        literals_from_last_hard_call_.push_back(next_layer->pairs);
        std::swap(current_layer, next_layer);
    }

    //current_layer now stores the final sum literals
    //  i.e., if the sum of input literals is at least k, then the first m literals that sum up to k will be set to true
    //in contrast to the soft version, the sum literals only go up to the right hand side and do not exceed it
    //todo explain this a bit better
    //  note that the converse does not hold in the current version

    //we implicitly assume the literals in the layer are sorted...
    debug_last_call_.push_back(*current_layer);

    static std::vector<PairWeightLiteral> output_literals;
    output_literals.clear();
    for (PairWeightLiteral& p : current_layer->pairs) //todo I think the whole array can simply be copied instead of this loop
    {
        runtime_assert(p.weight <= right_hand_side);
        output_literals.push_back(p);        
    }

    //store the result
    encoded_hard_constraints_.push_back(EncodedConstraint());
    encoded_hard_constraints_.back().input_literals = input_literals;
    encoded_hard_constraints_.back().output_literals = output_literals;
    encoded_hard_constraints_.back().right_hand_side = right_hand_side;

    return output_literals;
}

void EncoderGeneralisedTotaliser::Clear()
{
    encoded_hard_constraints_.clear();
    encoded_soft_constraints_.clear();
}

bool EncoderGeneralisedTotaliser::DebugCheckSatisfactionOfEncodedConstraints(BooleanAssignmentVector& solution)
{
    for (EncodedConstraint& encoded_constraint : encoded_hard_constraints_)
    {
        int num_falsified = 0;
       // bool problem_detected = false;
        for (PairWeightLiteral core_lit : encoded_constraint.input_literals)
        {
            num_falsified += (solution[core_lit.literal] * core_lit.weight);
        }
        int k = 0;
        while (k < encoded_constraint.output_literals.size())
        {
            if (num_falsified == encoded_constraint.output_literals[k].weight) 
            { 
                runtime_assert(solution[encoded_constraint.output_literals[k].literal]); 
            }
            else       
            {
                break;
            }
            k++;
        }
        runtime_assert(num_falsified <= encoded_constraint.right_hand_side);        
    }        
    return true;
}
}
