/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  Spatial1DPlacement.h
 *  @brief Population is locked to a one-dimensional grid (wrapping is configurable). Births are placed in an neighboring cell.
 *
 */

#ifndef MABE_SPATIAL_1D_PLACEMENT_H
#define MABE_SPATIAL_1D_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Grows population to a given size, then randomly places additional births over existing orgs
  class Spatial1DPlacement : public Module {
  private:
    Collection target_collect;    ///< Collection of populations to manage
    size_t grid_width = 64;       ///< Size of the one-dimensional population 
    bool does_wrap = false;       ///< If true, leftmost and rightmost cells are adjacent 

  public:
    Spatial1DPlacement(mabe::MABE & control,
        const std::string & name="Spatial1DPlacement",
        const std::string & desc="Orgs can only interact with neighboring cells in a one-dimensional grid")
      : Module(control, name, desc), target_collect(control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~Spatial1DPlacement() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
      LinkVar(grid_width, "grid_width", "Max size of the population");
      LinkVar(does_wrap, "does_wrap",
          "If false, the ends are separate. If true, leftmost and rightmost cells are adjacent");
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
    }

    /// Place a birth. Most be located next to parent
    OrgPosition PlaceBirth(OrgPosition ppos, Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        const size_t parent_idx = ppos.Pos();
        const size_t direction = control.GetRandom().GetUInt(2);
        size_t offspring_idx = parent_idx;
        switch(direction){
          case 0: // Left
            if(parent_idx > 0) offspring_idx = parent_idx - 1;
            else offspring_idx = does_wrap ? grid_width - 1 : parent_idx + 1;
            break;
           case 1: // Right
            if(parent_idx < grid_width - 1) offspring_idx = parent_idx + 1;
            else offspring_idx = does_wrap ? 0 : parent_idx - 1;
            break;
        }
        if(offspring_idx >= target_pop.GetSize()){
          for(size_t i = target_pop.GetSize(); i < offspring_idx; ++i){
            auto pop_iterator = control.PushEmpty(target_pop);
          }
          return control.PushEmpty(target_pop);
        }
        else return OrgPosition(target_pop, offspring_idx);
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    /// Manually inject an organism. Pick a random position and add empty positions as needed 
    OrgPosition PlaceInject(Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        // If population not full, add new position
        if(target_collect.GetSize() < grid_width) return control.PushEmpty(target_pop);
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
        // Left
        if(parent_idx > 0) result_vec.emplace_back(target_pop, parent_idx - 1);
        else if (does_wrap) result_vec.emplace_back(target_pop, grid_width - 1);
        // Right
        if(parent_idx < grid_width - 1) result_vec.emplace_back(target_pop, parent_idx + 1);
        else if (does_wrap) result_vec.emplace_back(target_pop, 0);
        return result_vec;
    }

    double PrintGrid(Collection& list){
      for(size_t idx = 0; idx < grid_width; ++idx){
        if(idx >= list.GetSize()){
          std::cout << ".";
        } 
        else{
          Organism& org = list[idx];
          if(org.IsEmpty()) std::cout << "o";
          else std::cout << "X";
        }
      }
      std::cout << std::endl;
      return 0;
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("PRINT",
                              [](Spatial1DPlacement & mod, Collection list) { 
                                  return mod.PrintGrid(list); 
                              },
                             "Print empty vs non-empty organisms as a grid");
    }

  };

  MABE_REGISTER_MODULE(Spatial1DPlacement, "Offspring are placed next to parent on toroidal grid");
}

#endif
