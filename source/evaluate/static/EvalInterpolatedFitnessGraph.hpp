/*
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file  EvalInterpolatedFitnessGraph.hpp
 *  @brief MABE Evaluation module that will interpolate fitness between nodes on a graph
 */

#ifndef MABE_EVAL_INTERP_GRAPH_H
#define MABE_EVAL_INTERP_GRAPH_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/io/File.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/tools/value_utils.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/UnorderedIndexMap.hpp"
#include "emp/bits/BitVector.hpp"

namespace mabe {


  class InterpolatedFitnessGraph{
    private:
      bool initialized = false;
      std::string filename;
      std::unordered_map<std::string, size_t> name_map;
      emp::vector<std::string> name_vec;
      emp::vector<double> fitness_vec;
      emp::vector<std::map<size_t, size_t>> connection_maps;
      emp::vector<emp::UnorderedIndexMap> weighted_connection_maps;


    public:
      InterpolatedFitnessGraph() { ; }

      size_t GetIndex(const std::string& name){
        emp_assert(emp::Has(name_map, name), "IFG does not have node by name: ", name);
        return name_map[name];
      }

      void AddNode(std::string & node_name, double fitness){
        std::cout << "Adding node: " << node_name << std::endl;
        if(emp::Has(name_map, node_name)){
          emp::notify::Error("InterpolatedFitnessMap already has node: ", 
                            node_name, ". Cannot add another node of same name");
        }
        name_map[node_name] = fitness_vec.size();
        name_vec.push_back(node_name);
        fitness_vec.push_back(fitness);
        connection_maps.emplace_back();
        weighted_connection_maps.emplace_back(1);
      }
      
      void ConnectNodes(size_t idx_a, size_t idx_b, size_t num_steps, double weight_a_b = 1.0, double weight_b_a = 1.0){
        std::cout << "Connecting nodes by idx: " << idx_a << " and " << idx_b << std::endl;
        if(emp::Has(connection_maps[idx_a], idx_b)){
          emp::notify::Error("InterpolatedFitnessMap error! Node '", 
                            name_vec[idx_a], 
                            "' is already connected to node '",
                            name_vec[idx_b], 
                            "'. You cannot connect them more than once.");
        }
        if(emp::Has(connection_maps[idx_b], idx_a)){
          emp::notify::Error("InterpolatedFitnessMap error! Node '", 
                            name_vec[idx_b], 
                            "' is already connected to node '",
                            name_vec[idx_a], 
                            "'. You cannot connect them more than once.");
        }
        // We store the map offset (order in map) in our IndexMap
        size_t offset_a = connection_maps[idx_b].size();
        size_t offset_b = connection_maps[idx_a].size();
        connection_maps[idx_a][idx_b] = num_steps;
        connection_maps[idx_b][idx_a] = num_steps;
        if(weighted_connection_maps[idx_a].GetSize() <= offset_b){
          weighted_connection_maps[idx_a].Resize(offset_b + 1, 0);
        }
        if(weighted_connection_maps[idx_b].GetSize() <= offset_a){
          weighted_connection_maps[idx_b].Resize(offset_a + 1, 0);
        }
        weighted_connection_maps[idx_a].Adjust(offset_b, weight_a_b);
        weighted_connection_maps[idx_b].Adjust(offset_a, weight_b_a);
      }

      void ConnectNodes(std::string & node_a, std::string & node_b, size_t num_steps, double weight_a_b = 1.0, double weight_b_a = 1.0) {
        std::cout << "Connecting nodes by name: " << node_a << " and " << node_b << std::endl;
        if(!emp::Has(name_map, node_a)){
          emp::notify::Error("InterpolatedFitnessMap does not have node named '",
                            node_a, "'. Cannot connect"); 
        }
        if(!emp::Has(name_map, node_b)){
          emp::notify::Error("InterpolatedFitnessMap does not have node named '",
                            node_b, "'. Cannot connect"); 
        }
        size_t idx_a = GetIndex(node_a);
        size_t idx_b = GetIndex(node_b);
        ConnectNodes(idx_a, idx_b, num_steps, weight_a_b, weight_b_a);
      }

      void LoadFile(const std::string & _filename){ 
        filename = _filename;
        std::cout << "Loading graph from file: " << filename << std::endl;
        emp::File file(filename);
        file.RemoveComments("#");
        file.RemoveWhitespace();
        file.RemoveEmpty();
        emp::vector<std::string> string_parts; 
        for(auto it = file.begin(); it != file.end(); it++){
          string_parts.clear();
          emp::slice(*it, string_parts, ',');
          std::cout << *it << std::endl;
          for(std::string & str : string_parts){
            std::cout << "  " << str << std::endl;
          }
          // Add a node
          // Syntax 1: add, name, fitness
          // Sytnax 2: a, name, fitness
          if(string_parts[0] == "add" || string_parts[0] == "a"){
            if(string_parts.size() != 3){
              emp::notify::Error("IFG error: 'add' line should contain exactly three values:",
                                " 'add, name, fitness'. You passed: ", *it);
            }
            std::string & node_name = string_parts[1];
            double fitness = emp::ToDouble(string_parts[2]);
            AddNode(node_name, fitness);
          }
          else if(string_parts[0] == "connect" || string_parts[0] == "c"){
            if(string_parts.size() < 4 || string_parts.size() > 6){
              emp::notify::Error("IFG error: 'connect' line should contain between ",
                                " three and five values: ",
                                "'connect, name_1, name_2, num_intermediate_nodes,",
                                " weight_a_b, weight_b_a' ",
                                "You passed: ", *it);
            }
            std::string & node_a_name = string_parts[1];
            std::string & node_b_name = string_parts[2];
            size_t num_steps = std::stoll(string_parts[3]);
            double weight_a_b = 1.0;
            double weight_b_a = 1.0;
            if(string_parts.size() >= 5) weight_a_b = emp::ToDouble(string_parts[4]);
            if(string_parts.size() >= 6) weight_b_a = emp::ToDouble(string_parts[5]);
            

            ConnectNodes(node_a_name, node_b_name, num_steps, weight_a_b, weight_b_a);
          }
        }
      }

      double GetFitness(size_t idx_a, size_t idx_b, size_t num_steps){
        double old_fitness = fitness_vec[idx_a];
        if(idx_a == idx_b) return old_fitness;
        double new_fitness = fitness_vec[idx_b];
        size_t total_steps = connection_maps[idx_a][idx_b];
        double fitness_step = (new_fitness - old_fitness) / (total_steps + 1);
        return old_fitness + num_steps * fitness_step;
      }

      double GetFitness(const emp::vector<int>& ints){
        emp_assert(ints.size() == 3, "Can only handle vectors of length 3");
        return GetFitness(ints[0], ints[1], ints[2]);
      }

      double GetFitness(const std::string& node_a, const std::string& node_b, size_t num_steps){
        size_t idx_a = GetIndex(node_a);
        size_t idx_b = GetIndex(node_b);
        return GetFitness(idx_a, idx_b, num_steps);
      }

      size_t Mutate(emp::vector<int>& genotype, emp::Random& random, double mut_prob){
        if(!random.P(mut_prob)) return 0;
        if(genotype[0] == genotype[1]){
          const double max_weight = weighted_connection_maps[genotype[0]].GetWeight();
          const double rand_index = random.GetDouble() * max_weight;
          size_t selected_option = weighted_connection_maps[genotype[0]].Index(rand_index);
          //size_t num_options = connection_maps[genotype[0]].size();
          //size_t selected_option = random.GetUInt(num_options);
          auto iterator = connection_maps[genotype[0]].begin();
          std::advance(iterator, selected_option);
          genotype[1] = iterator->first;
          genotype[2] = 1;
          size_t num_steps = connection_maps[genotype[0]][genotype[1]];
          if(static_cast<size_t>(genotype[2]) > num_steps){
            genotype[0] = genotype[1];
            genotype[2] = 0;
          }
        } 
        else {
          if(random.P(0.5)){
            genotype[2]++;
            size_t num_steps = connection_maps[genotype[0]][genotype[1]];
            if(static_cast<size_t>(genotype[2]) > num_steps){
              genotype[0] = genotype[1];
              genotype[2] = 0;
            }
          }
          else {
            genotype[2]--;
            if(genotype[2] <= 0){
              genotype[1] = genotype[0];
              genotype[2] = 0;
            }
          }

        }
        return 1;
      }

      size_t GetNumNodes(){
        return name_map.size();
      }

      const std::string & GetName(size_t idx){
        return name_vec[idx];
      }

      const std::map<size_t, size_t>& GetConnectionMap(size_t idx){
        return connection_maps[idx];
      }

      void PrintNodeDetails(size_t idx){
        std::cout << "Node details: " << idx << std::endl;
        std::cout << "  Name: " << name_vec[idx] << std::endl;
        std::cout << "  Fitness: " << fitness_vec[idx] << std::endl;
        std::cout << "  Connections: " << connection_maps[idx].size() << std::endl;
        for(auto it = connection_maps[idx].begin(); it != connection_maps[idx].end(); it++){
          std::cout << "    Node: " << it->first << ", steps: " << it->second << std::endl;
        }
      }
  };

  class EvalInterpolatedFitnessGraph : public Module {
  private:
    std::string graph_filename;
    std::string genotype_trait;
    std::string fitness_trait;

    double mut_prob;

    InterpolatedFitnessGraph graph;
    emp::BitVector visited_nodes;

  public:
    EvalInterpolatedFitnessGraph(mabe::MABE & control,
           const std::string & name="EvalInterpolatedFitnessGraph",
           const std::string & desc="Module to interpolate fitness based on connected graph nodes",
           const std::string & _gtrait="genotype", const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , genotype_trait(_gtrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalInterpolatedFitnessGraph() { }

    // Setup member functions associated with this class.
      static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalInterpolatedFitnessGraph & mod, Collection list) { 
                                return mod.Evaluate(list); 
                              },
                             "Use interpolated fitness for all orgs in an OrgList.");
      info.AddMemberFunction("GET_FITNESS",
                             [](EvalInterpolatedFitnessGraph & mod, 
                                  size_t a, size_t b, size_t num_steps) { 
                                return mod.Evaluate(a, b, num_steps); 
                              },
                             "Calculate fitness at a given step"); 
      info.AddMemberFunction("GET_FITNESS_STR",
                             [](EvalInterpolatedFitnessGraph & mod, 
                                  const std::string& a, const std::string& b, size_t num_steps) { 
                                return mod.Evaluate(a, b, num_steps); 
                              },
                             "Calculate fitness at a given step"); 
      info.AddMemberFunction("DETECT_DISCOVERIES",
                             [](EvalInterpolatedFitnessGraph & mod, Collection list) { 
                                return mod.DetectDiscoveries(list); 
                              },
                             "Check to see if any new nodes were reached in this OrgList.");
    }

    void SetupConfig() override {
      LinkVar(graph_filename, "graph_filename", "Name of the .graph file to load.");
      LinkVar(genotype_trait, "genotype_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(mut_prob, "mut_prob", "Probability of a mutation");
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<int>>(genotype_trait);
      AddOwnedTrait<double>(fitness_trait, "Interpolated fitness value", 0.0);

      graph.LoadFile(graph_filename);
      visited_nodes.resize(graph.GetNumNodes());
      
      ActionMap& action_map = control.GetActionMap(0);
      std::function<size_t(emp::vector<int>&, emp::Random&)> mutate_func = 
          [this](emp::vector<int>& genotype, emp::Random& rand){ 
            return graph.Mutate(genotype, rand, mut_prob);
          };
      action_map.AddFunc<size_t, emp::vector<int>&, emp::Random&>("Mutate", mutate_func);
    }

    size_t DetectDiscoveries(const Collection & orgs){
      std::unordered_map<size_t, bool> discovered_sites;
      size_t discovery_count = 0;
      for(size_t org_idx = 0; org_idx < orgs.size(); ++org_idx){
        const Organism& org = orgs[org_idx];
        const auto & genotype = org.GetTrait<emp::vector<int>>(genotype_trait);
        if(genotype[0] == genotype[1]){
          if(!visited_nodes[genotype[0]]){
            discovered_sites[genotype[0]] = true;
            std::cout << "Node " << genotype[0] << " (" << graph.GetName(genotype[0])  
                      << ") discovered at site " << org_idx << std::endl;
            discovery_count++;
          }
        }
      }  
      for(auto it = discovered_sites.begin(); it != discovered_sites.end(); it++){
        visited_nodes[it->first] = 1;
      }
      return discovery_count;
    }

    double Evaluate(size_t idx_a, size_t idx_b, size_t steps){
      return graph.GetFitness(idx_a, idx_b, steps);
    }
    
    double Evaluate(const std::string& node_a, const std::string& node_b, size_t steps){
      return graph.GetFitness(node_a, node_b, steps);
    }

    double Evaluate(const Collection & orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.GenerateOutput();
        const auto & genotype = org.GetTrait<emp::vector<int>>(genotype_trait);
        double fitness = graph.GetFitness(genotype);
        org.SetTrait<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }
      return max_fitness;
    }

    // If a population is provided to Evaluate, first convert it to a Collection.
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    // If a string is provided to Evaluate, convert it to a Collection.
    double Evaluate(const std::string & in) { return Evaluate( control.ToCollection(in) ); }
  };

  MABE_REGISTER_MODULE(EvalInterpolatedFitnessGraph, "Evaluate bitstrings on an NK fitness lanscape.");
}

#endif
