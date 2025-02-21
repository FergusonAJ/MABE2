/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file  Eval2DGrid.hpp
 *  @brief MABE Evaluation module for narrow forking gradients
 */

#ifndef MABE_EVAL_2D_GRID_H
#define MABE_EVAL_2D_GRID_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "emp/io/File.hpp"

namespace mabe {

  class Eval2DGrid : public Module {
  private:
    size_t N = 10;
    size_t num_rows = 5;
    size_t num_cols = 5;
    std::string map_filename;
    emp::vector<emp::vector<double>> val_map;
    bool exponentiate = false;
    double exponent_base = 1.1;

    std::string ints_trait = "ints";
    std::string fitness_trait = "fitness";

  public:
    Eval2DGrid(mabe::MABE & control,
           const std::string & name="Eval2DGrid",
           const std::string & desc="Module to evaluate organisms on a discrete 2D grid")
      : Module(control, name, desc){
      SetEvaluateMod(true);
    }
    ~Eval2DGrid() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                              [](Eval2DGrid & mod, Collection list) { 
                                return mod.Evaluate(list); 
                             },
                             "Evaluate all organisms in collection on the discrete 2D grid.");
    }
    
    void SetupGrid(){
      val_map.clear();
      emp::File map_file(map_filename);
      for(int row_idx = 0; row_idx < num_rows; row_idx++){
        val_map.push_back(map_file.ExtractRowAs<double>(","));
      }
      for(size_t row_idx = 0; row_idx < num_rows; row_idx++){
        for(size_t col_idx = 0; col_idx < num_cols; col_idx++){
          std::cout << "[" << val_map[row_idx][col_idx] << "] ";
        }
        std::cout << std::endl;
      }
    }

    void SetupConfig() override {
      LinkVar(N, "N", "Number of ints required in input");
      LinkVar(ints_trait, "ints_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(num_rows, "num_rows", "Number of rows in the map");
      LinkVar(num_cols, "num_cols", "Number of columns in the map");
      LinkVar(map_filename, "map_filename", "Where is the map located?");
      LinkVar(exponentiate, "exponentiate", "Do we exponentiate fitness?");
      LinkVar(exponent_base, "exponent_base", 
          "If we exponentiate fitness, what base should we use?");

    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<int>>(ints_trait);
      AddOwnedTrait<double>(fitness_trait, "2D Grid fitness value", 0.0);
      SetupGrid();
    }

    double GetScore(Organism & org) {
      org.GenerateOutput();
      const auto & ints = org.GetTrait<emp::vector<int>>(ints_trait);
      if (ints.size() != N) {
        emp::notify::Error("Org returns ", ints.size(), " ints, but ",
                           N, " ints needed for Eval2DGrid.",
                           "\nOrg: ", org.ToString());
      }
      double fitness = 0;
      if(ints[0] < 0 || ints[0] >= num_rows || ints[1] < 0 || ints[1] >= num_cols){
        fitness = -100;
      }
      else{
        fitness = val_map[ints[1]][ints[0]];
      }
      if(exponentiate) fitness = std::pow(exponent_base, fitness);
      return fitness;
    }

    double Evaluate(const Collection & orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        const double fitness = GetScore(org);
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

  MABE_REGISTER_MODULE(Eval2DGrid, "Evaluate organisms on a discrete 2D grid.");
}

#endif
