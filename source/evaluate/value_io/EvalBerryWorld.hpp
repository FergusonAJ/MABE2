/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalBerryWorld.hpp
 *  @brief Quick reimplementation of Berry world from the original MABE
 *    Reference: https://github.com/Hintzelab/MABE/wiki/World-Berry
 */

#ifndef MABE_EVAL_BERRY_H
#define MABE_EVAL_BERRY_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalBerryWorld : public Module {
  private:
    size_t world_width;       // World width as measured by number of tiles/berries
    size_t world_height;      // World width as measured by number of tiles/berries
    bool is_toroidal;         // If true, walking off one side of the world will wrap 
                                // you to the opposite side (i.e., the world is a torus)
    size_t food_type_count;   // Number of different types of berries that can spawn
    double task_switch_cost;  // The penalty for an organism to eat a berry of a different type 
                                // than the previous berry consumed
    // TODO: Make a config option
    size_t max_updates = 10;  // How many updates (eat/move/rotate) does each organism take
    std::string food_reward_str;          // What's the fitness reward for eating each type of berry?
                                            // Passed from config as a comma-separated string
    emp::vector<double> food_reward_vec;  // Parsed version of food_reward_str
    std::string food_replacement_probs_str;   // Probability that a new food will be of X type.
                                                // Comma-separated string passed from config.
                                                // E.g., 0.25,0.5,0.25, each new berry has a 50% 
                                                // chance of being type 2 and a 25% chance each of 
                                                // type 1 or 3
                                                // All probabilities should add up to 1.
    emp::vector<double> food_replacement_probs_vec; // Parsed version of food_reward_str
    mabe::Collection target_collect;  // Collection of organisms to evaluate
    std::string input_trait;          // Name of the trait to assign org inputs (i.e. env. stimuli)
    std::string action_trait;         // Name of the trait returned from the org to denote actions
    std::string fitness_trait;        // Name of the trait to assign fitness to

  public:
    EvalBerryWorld(mabe::MABE & control,
           const std::string & name="EvalBerryWorld",
           const std::string & desc="Evaluate organisms' berry-foraging behavior.",
           size_t _world_width=16,
           size_t _world_height=16, 
           bool _is_toroidal = true,
           size_t _food_types = 2,
           double _task_switch_cost = 1.4,
           const std::string & _food_reward_str = "1,1",
           const std::string & _food_replacement_probs_str = "0.5,0.5",
           const std::string & _itrait="inputs", 
           const std::string & _atrait="actions", 
           const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , world_width(_world_width) 
      , world_height(_world_height)
      , is_toroidal(_is_toroidal)
      , food_type_count(_food_types)
      , task_switch_cost(_task_switch_cost)
      , food_reward_str(_food_reward_str)
      , food_replacement_probs_str(_food_replacement_probs_str)
      , target_collect(control.GetPopulation(0))
      , input_trait(_itrait)
      , action_trait(_atrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalBerryWorld() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(world_width, "world_width", "How wide is the world?");
      LinkVar(world_height, "world_height", "How tall is the world?");
      LinkVar(is_toroidal, "is_toroidal", "Is the world a torus, or should organisms"
          " be bound by the edges?");
      LinkVar(food_type_count, "food_types", "Number of possible foods in the world");
      LinkVar(task_switch_cost, "task_switch_cost", "Penalty for eating a different food from"
          " last time");
      LinkVar(food_reward_str, "food_rewards", "Comma separated list of rewards for each food type");
      LinkVar(food_replacement_probs_str, "food_placement_probs", "Comma separated list of "
          "probabilities used for food placement");

      LinkVar(input_trait, "input_trait", "Which trait does the organims use as input from the"
          " world?");
      LinkVar(action_trait, "action_trait", "Which trait stores organisms' actions?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store fitness in?");
    }

    void SpliceStringIntoVec(const std::string & s, 
        emp::vector<double> & vec, const unsigned char sep = ','){
      vec.clear();
      std::stringstream stream;
      size_t num_chars = 0;
      for(size_t idx = 0; idx < s.size(); idx++){
        if(s[idx] == sep){
          if(num_chars > 0){
            vec.push_back(std::stod(stream.str()));
            stream.str("");
            num_chars = 0;
          }
        }
        else {
          stream << s[idx];
          ++num_chars;
        }
      } 
      if(num_chars > 0){
        vec.push_back(std::stod(stream.str()));
        stream.str("");
      }
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<double>>(input_trait);
      AddRequiredTrait<emp::BitVector>(action_trait);
      AddOwnedTrait<double>(fitness_trait, "BerryWorld fitness value", 0.0);

      SpliceStringIntoVec(food_reward_str, food_reward_vec); 
      SpliceStringIntoVec(food_replacement_probs_str, food_replacement_probs_vec); 

      std::cout << "Berry world initialized with " << food_type_count << " types of food!" << std::endl;
      std::cout << "Food rewards: " << std::endl;
      for(size_t idx = 0; idx < food_type_count; ++idx){
        std::cout << "    " << food_reward_vec[idx]  << std::endl;
      }
      std::cout << "Food replacement probabilities: " << std::endl;
      for(size_t idx = 0; idx < food_type_count; ++idx){
        std::cout << "    " << food_replacement_probs_vec[idx]  << std::endl;
      }
    
    }

    // TODO: Create a berryworld evaluator class
    double GetFitness(Organism & org){
      int cur_x = world_width / 2; 
      int cur_y = world_height / 2;
      int vel_x = 0;
      int vel_y = 1;
      double cur_fitness = 0;
      double p;
      bool has_eaten_here = false;
      bool has_eaten_before = false;
      size_t last_eaten = 0;
      emp::vector<size_t> berry_map(world_width * world_height);
      for(size_t site_idx = 0; site_idx < berry_map.size(); ++site_idx){
        p = control.GetRandom().GetDouble();
        for(size_t food_idx = 0; food_idx < food_replacement_probs_vec.size(); ++food_idx){
          if(p < food_replacement_probs_vec[food_idx]){
            berry_map[site_idx] = food_idx;
            break;
          }
          else
            p -= food_replacement_probs_vec[food_idx];
        }
      }
      for(size_t update = 0; update < max_updates; ++update){
        emp::vector<double> input_vec(food_type_count, 0);
        if(!has_eaten_here){
            size_t cur_berry = berry_map[cur_y * world_width + cur_x];
            input_vec[berry_map[cur_y * world_width + cur_x]] = 1;
        }
        org.SetVar<emp::vector<double>>(input_trait, input_vec);
        org.GenerateOutput();
        const emp::BitVector & actions = org.GetVar<emp::BitVector>(action_trait);
        if (actions[1]){ // Eat
          // TODO: Verify this is how MABE does it
          if(!has_eaten_here){
            has_eaten_before = true;
            has_eaten_here = true;
            if(has_eaten_before){
              if(berry_map[cur_y * world_width + cur_x] == last_eaten)
                cur_fitness -= task_switch_cost;
              else
                cur_fitness += food_reward_vec[berry_map[cur_y * world_width + cur_x]];
            }
          }
          //else
          //  cur_fitness += food_reward_vec[berry_map[cur_y * world_width + cur_x]];
        }
        else if(actions[0]){ // Move
          size_t old_x = cur_x;
          size_t old_y = cur_y;
          cur_x += vel_x;
          cur_y += vel_y;
          if(is_toroidal){
            cur_x = cur_x % world_width;
            cur_y = cur_y % world_height;
          }
          else{
            if(cur_x < 0) cur_x = world_width - 1;
            else if(cur_x >=  world_width) cur_x = 0;
            if(cur_y < 0) cur_y = world_height - 1;
            else if(cur_y >=  world_height) cur_y = 0;
          }
          if(has_eaten_here & (cur_x != old_x || cur_y != old_y)){
            has_eaten_here = false;
            for(size_t food_idx = 0; food_idx < food_replacement_probs_vec.size(); ++food_idx){
              if(p < food_replacement_probs_vec[food_idx]){
                berry_map[cur_y * world_width + cur_x] = food_idx;
                break;
              }
              else
                p -= food_replacement_probs_vec[food_idx];
            }
          }
        }
        else if (actions[2]){ // Turn left 45 degrees
          if     (cur_x == 0  && cur_y == 1) { cur_x =  1;  cur_y = 1; }  // South     -> Southeast 
          else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = -1; } // East      -> Northeast
          else if(cur_x == 0  && cur_y == -1) { cur_x = -1; cur_y = -1; } // North     -> Northwest
          else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = 1; }  // West      -> Southwest
          else if(cur_x == 1  && cur_y == 1) { cur_x =  1;  cur_y = 0; }  // Southeast -> East
          else if(cur_x == 1  && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northeast -> North
          else if(cur_x == -1 && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southwest -> South
          else if(cur_x == -1 && cur_y == -1) { cur_x = -1; cur_y = 0; }  // Northwest -> West
        }
        else if (actions[3]){ // Turn right 45 degrees
          if     (cur_x == 0  && cur_y == 1) { cur_x =  -1; cur_y = 1; }  // South     -> Southwest 
          else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = 1; }  // East      -> Southeast
          else if(cur_x == 0  && cur_y == -1) { cur_x = 1;  cur_y = -1; } // North     -> Northeast
          else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = -1; } // West      -> Northwest
          else if(cur_x == 1  && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southeast -> South
          else if(cur_x == 1  && cur_y == -1) { cur_x = 1;  cur_y = 0; }  // Northeast -> East
          else if(cur_x == -1 && cur_y == 1) { cur_x =  -1; cur_y = 0; }  // Southwest -> West
          else if(cur_x == -1 && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northwest -> North
        }
      } 
      return cur_fitness;
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {
        double fitness = GetFitness(org);
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalBerryWorld, "Evaluate organisms' berry-foraging behavior.");
}

#endif