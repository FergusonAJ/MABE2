/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file  AdjacencyPlacement.h
 *  @brief Population is locked to the supplied adjacency list
 *
 */

// TODO: Solidify fill mechanics (prioritize empty nodes?)

#ifndef MABE_ADJACENCY_PLACEMENT_H
#define MABE_ADJACENCY_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/io/File.hpp"

namespace mabe {

  /// Grows population to a given size, then randomly places additional births over existing orgs
  class AdjacencyPlacement : public Module {
  private:
    Collection target_collect;    ///< Collection of populations to manage
    emp::vector<emp::vector<size_t>> adj_map;
    bool bidirectional_edges = true;
    std::string adj_filename;

    void LoadFile(){ 
      if(adj_filename.size() == 0){
          emp::notify::Error("AdjacencyPlacement error: no filename given for adj list");
          return;
      }
      emp::File file(adj_filename);
      file.RemoveComments("#");
      file.RemoveEmpty();
      emp::vector<std::string> string_parts; 
      for(auto it = file.begin(); it != file.end(); it++){
        string_parts.clear();
        emp::slice(*it, string_parts, ' ');
        if(string_parts.size() < 2) continue;
        const size_t source_node = std::stoull(string_parts[0]);
        if(adj_map.size() <= source_node) adj_map.resize(source_node + 1);
        for(size_t idx = 1; idx < string_parts.size(); ++idx){
          const size_t target_node = std::stoull(string_parts[idx]);
          adj_map[source_node].push_back(target_node);
          if(bidirectional_edges){
            if(adj_map.size() <= target_node) adj_map.resize(target_node + 1);
            adj_map[target_node].push_back(source_node);
          }
        }
      }
    }

  public:
    AdjacencyPlacement(mabe::MABE & control,
        const std::string & name="AdjacencyPlacement",
        const std::string & desc="Orgs placed according to adjacency list")
      : Module(control, name, desc), target_collect(control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~AdjacencyPlacement() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
      LinkVar(bidirectional_edges, "bidirectional_edges","If 1, all edges are bidirectional");
      LinkVar(adj_filename, "adj_filename","Filename for the adjacency list");
    }

    /// Set birth and inject functions for the specified populations
    void SetupModule() override {
      for(size_t pop_id = 0; pop_id < control.GetNumPopulations(); ++pop_id){
        Population& pop = control.GetPopulation(pop_id);
        if(target_collect.HasPopulation(pop)){
          pop.SetPlaceBirthFun( 
            [this, &pop](Organism & /*org*/, OrgPosition ppos) {
              return PlaceBirth(ppos, pop);
            }
          );
          pop.SetPlaceInjectFun( 
            [this, &pop](Organism & /*org*/){
              return PlaceInject(pop);
            }
          );
          pop.SetFindAllNeighborsFun(
            [this, &pop](OrgPosition ppos){
              return FindAllNeighbors(ppos, pop);
            }
          );
        }
      }
      LoadFile();
    }

    /// Place a birth. Most be located next to parent
    OrgPosition PlaceBirth(OrgPosition ppos, Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        const size_t parent_idx = ppos.Pos();
        if(adj_map[parent_idx].size() == 0){ // No neighbors... must replace self
          return OrgPosition(target_pop, parent_idx);
        }
        size_t random_idx = control.GetRandom().GetUInt(adj_map[parent_idx].size());
        size_t offspring_idx = adj_map[parent_idx][random_idx];
        return OrgPosition(target_pop, offspring_idx);
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    /// Manually inject an organism. Pick a random position and add empty positions as needed 
    OrgPosition PlaceInject(Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        // If population not full, add new position
        if(target_collect.GetSize() < adj_map.size()) return control.PushEmpty(target_pop);
          // If population full, return a random org's position
        else{
          return OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
        }
      }
      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }
   
    emp::vector<OrgPosition> FindAllNeighbors(OrgPosition ppos, Population & target_pop) {
        const size_t parent_idx = ppos.Pos();
        emp::vector<OrgPosition> result_vec;
        for(size_t idx : adj_map[parent_idx]) result_vec.emplace_back(target_pop, idx);
        return result_vec;
    }

    size_t GetNumNodes(){
      return adj_map.size();
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("NUM_NODES",
                              [](AdjacencyPlacement & mod) { 
                                  return mod.GetNumNodes(); 
                              },
                             "Print empty vs non-empty organisms as a grid");
    }

  };

  MABE_REGISTER_MODULE(AdjacencyPlacement, "Offspring are placed next to parent on toroidal grid");
}

#endif
