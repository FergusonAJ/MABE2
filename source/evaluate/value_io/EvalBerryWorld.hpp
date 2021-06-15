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

  class EvalBerryWorld;
  class BerryWorldEvaluator {
    friend EvalBerryWorld;
    protected:
    // Variable defining the world
    size_t world_width;       // World width as measured by number of tiles/berries
    size_t world_height;      // World width as measured by number of tiles/berries
    bool is_toroidal;         // If true, walking off one side of the world will wrap 
                                // you to the opposite side (i.e., the world is a torus)
    size_t food_type_count;   // Number of different types of berries that can spawn
    double task_switch_cost;  // The penalty for an organism to eat a berry of a different type 
                                // than the previous berry consumed
    // TODO: Make a config option
    size_t max_updates;  // How many updates (eat/move/rotate) does each organism take
    emp::vector<double> food_reward_vec;  // Parsed version of food_reward_str
    emp::vector<double> food_replacement_probs_vec; // Parsed version of food_reward_str
    // Variables to keep the state of the world
    int cur_x = world_width / 2; 
    int cur_y = world_height / 2;
    int vel_x = 0;
    int vel_y = 1;
    bool has_eaten_here = false;
    bool has_eaten_before = false;
    size_t last_eaten = 0;
    double cur_fitness = 0;
    emp::vector<size_t> berry_map;
    emp::vector<size_t> clean_berry_map;
    MABE& control;
    std::string input_trait;          // Name of the trait to assign org inputs (i.e. env. stimuli)
    std::string action_trait;         // Name of the trait returned from the org to denote actions
    std::string fitness_trait;        // Name of the trait to assign fitness to

    // Eat food at the organism's feet and change fitness accordingly
    void Eat(){
      // TODO: Verify this is how MABE does it
      if(!has_eaten_here){ // Hasn't eaten here -> Can eat!
        // If the org has eaten before, and this is not the same food eaten last time -> penalty
        if(has_eaten_before && berry_map[cur_y * world_width + cur_x] == last_eaten)
          cur_fitness -= task_switch_cost;
        else // First food eaten OR same food as last time -> reward!
          cur_fitness += food_reward_vec[berry_map[cur_y * world_width + cur_x]];
        // Bookkeeping
        has_eaten_before = true;
        has_eaten_here = true;
      }
    }
    // Move organism in the current direction
    void Move(){
      int old_x = cur_x;
      int old_y = cur_y;
      cur_x += vel_x;
      cur_y += vel_y;
      if(is_toroidal){ // If world is torus, wrap!
        cur_x = cur_x % world_width;
        cur_y = cur_y % world_height;
      }
      else{ // If the world is not a torus, keep org in the bounds! 
        if(cur_x < 0) cur_x = 0;
        else if(cur_x >=  world_width) cur_x = world_width - 1;
        if(cur_y < 0) cur_y = 0;
        else if(cur_y >=  world_height) cur_y = world_height - 1;
      }
      // If org ate the food that was here and then moved, add in some new food
      if(has_eaten_here & (cur_x != old_x || cur_y != old_y)){
        has_eaten_here = false;
        double p = control.GetRandom().GetDouble();
        // Use food replacement probabilities
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
    // Rotate organism left 45 degrees 
    void TurnLeft(){
      if     (cur_x == 0  && cur_y == 1) { cur_x =  1;  cur_y = 1; }  // South     -> Southeast 
      else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = -1; } // East      -> Northeast
      else if(cur_x == 0  && cur_y == -1) { cur_x = -1; cur_y = -1; } // North     -> Northwest
      else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = 1; }  // West      -> Southwest
      else if(cur_x == 1  && cur_y == 1) { cur_x =  1;  cur_y = 0; }  // Southeast -> East
      else if(cur_x == 1  && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northeast -> North
      else if(cur_x == -1 && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southwest -> South
      else if(cur_x == -1 && cur_y == -1) { cur_x = -1; cur_y = 0; }  // Northwest -> West
    }
    // Rotate organism right 45 degrees
    void TurnRight(){
      if     (cur_x == 0  && cur_y == 1) { cur_x =  -1; cur_y = 1; }  // South     -> Southwest 
      else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = 1; }  // East      -> Southeast
      else if(cur_x == 0  && cur_y == -1) { cur_x = 1;  cur_y = -1; } // North     -> Northeast
      else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = -1; } // West      -> Northwest
      else if(cur_x == 1  && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southeast -> South
      else if(cur_x == 1  && cur_y == -1) { cur_x = 1;  cur_y = 0; }  // Northeast -> East
      else if(cur_x == -1 && cur_y == 1) { cur_x =  -1; cur_y = 0; }  // Southwest -> West
      else if(cur_x == -1 && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northwest -> North
    }

    public: 
    BerryWorldEvaluator(
       MABE& _control,
       size_t _world_width=16,
       size_t _world_height=16, 
       bool _is_toroidal = true,
       bool _max_updates = 10,
       size_t _food_types = 2,
       double _task_switch_cost = 1.4,
       const std::string & _food_reward_str = "1,1",
       const std::string & _food_replacement_probs_str = "0.5,0.5",
       const std::string & _itrait="inputs", 
       const std::string & _atrait="actions", 
       const std::string & _ftrait="fitness"
    ) : control(_control)
      , world_width(_world_width) 
      , world_height(_world_height)
      , is_toroidal(_is_toroidal)
      , max_updates(_max_updates)
      , food_type_count(_food_types)
      , task_switch_cost(_task_switch_cost)
      , input_trait(_itrait)
      , action_trait(_atrait)
      , fitness_trait(_ftrait) 
    { ; }

    // Reset the state of the simulation to prepare for a new organism
    void ResetState(){
      cur_x = world_width / 2; 
      cur_y = world_height / 2;
      vel_x = 0;
      vel_y = 1;
      has_eaten_here = false;
      has_eaten_before = false;
      last_eaten = 0;
      cur_fitness = 0;
      berry_map = clean_berry_map;
    }

    // Generate a fresh map of berries according to the food replacement probabilities
    void GenerateBerryMap(){
      clean_berry_map.resize(world_width * world_height);
      double p;
      for(size_t site_idx = 0; site_idx < clean_berry_map.size(); ++site_idx){
        p = control.GetRandom().GetDouble();
        for(size_t food_idx = 0; food_idx < food_replacement_probs_vec.size(); ++food_idx){
          if(p < food_replacement_probs_vec[food_idx]){
            clean_berry_map[site_idx] = food_idx;
            break;
          }
          else
            p -= food_replacement_probs_vec[food_idx];
        }
      }
    }

    // Get the fitness of a single organism
    double GetFitness(Organism & org){
      ResetState(); // Get a fresh slate
      // Iterate for the specified number of updates
      for(size_t update = 0; update < max_updates; ++update){
        // Calculate input to send to the organism (1 if that food is located here, 0 otherwise) 
        emp::vector<double> input_vec(food_type_count, 0);
        if(!has_eaten_here){
            size_t cur_berry = berry_map[cur_y * world_width + cur_x];
            input_vec[berry_map[cur_y * world_width + cur_x]] = 1;
        }
        // Set org's inputs and get the org's outputs (here called actions)
        org.SetVar<emp::vector<double>>(input_trait, input_vec);
        org.GenerateOutput();
        const emp::BitVector & actions = org.GetVar<emp::BitVector>(action_trait);
        if (actions[1]) Eat();
        else if(actions[0]) Move();
        else if (actions[2]) TurnLeft();
        else if (actions[3]) TurnRight();
      } 
      org.SetVar<double>(fitness_trait, cur_fitness);
      return cur_fitness;
    }
  };

  class EvalBerryWorld : public Module {
  private:
    std::string food_reward_str;          // What's the fitness reward for eating each type of berry?
                                            // Passed from config as a comma-separated string
    //emp::vector<double> food_reward_vec;  // Parsed version of food_reward_str
    std::string food_replacement_probs_str;   // Probability that a new food will be of X type.
                                                // Comma-separated string passed from config.
                                                // E.g., 0.25,0.5,0.25, each new berry has a 50% 
                                                // chance of being type 2 and a 25% chance each of 
                                                // type 1 or 3
                                                // All probabilities should add up to 1.
    //emp::vector<double> food_replacement_probs_vec; // Parsed version of food_reward_str
    mabe::Collection target_collect;  // Collection of organisms to evaluate
    BerryWorldEvaluator world;

  public:
    EvalBerryWorld(mabe::MABE & control,
           const std::string & name="EvalBerryWorld",
           const std::string & desc="Evaluate organisms' berry-foraging behavior.",
           size_t _world_width=16,
           size_t _world_height=16, 
           bool _is_toroidal = true,
           bool _max_updates = 10,
           size_t _food_types = 2,
           double _task_switch_cost = 1.4,
           const std::string & _food_reward_str = "1,1",
           const std::string & _food_replacement_probs_str = "0.5,0.5",
           const std::string & _itrait="inputs", 
           const std::string & _atrait="actions", 
           const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , world(control
        , _world_width 
        , _world_height
        , _is_toroidal
        , _max_updates
        , _food_types
        , _task_switch_cost
        , _itrait
        , _atrait
        , _ftrait)
      , target_collect(control.GetPopulation(0))
      , food_reward_str(_food_reward_str)
      , food_replacement_probs_str(_food_replacement_probs_str)
    {
      SetEvaluateMod(true);
    }
    ~EvalBerryWorld() { }

    // Setup all configuration options
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(world.world_width, "world_width", "How wide is the world?");
      LinkVar(world.world_height, "world_height", "How tall is the world?");
      LinkVar(world.is_toroidal, "is_toroidal", "Is the world a torus, or should organisms"
          " be bound by the edges?");
      LinkVar(world.max_updates, "max_updates", "Number of in-world updates each organism receives");
      LinkVar(world.food_type_count, "food_types", "Number of possible foods in the world");
      LinkVar(world.task_switch_cost, "task_switch_cost", "Penalty for eating a different food from"
          " last time");
      LinkVar(food_reward_str, "food_rewards", "Comma separated list of rewards for each food type");
      LinkVar(food_replacement_probs_str, "food_placement_probs", "Comma separated list of "
          "probabilities used for food placement");

      LinkVar(world.input_trait, "input_trait", "Which trait does the organims use as input from the"
          " world?");
      LinkVar(world.action_trait, "action_trait", "Which trait stores organisms' actions?");
      LinkVar(world.fitness_trait, "fitness_trait", "Which trait should we store fitness in?");
    }

    // Splice a comma-separated string of numbers into a c++ vector of doubles
    void SpliceStringIntoVec(const std::string & s, 
        emp::vector<double> & vec, const unsigned char sep = ','){
      vec.clear();
      std::stringstream stream;
      size_t num_chars = 0;
      // Iterate through each character in the string
      for(size_t idx = 0; idx < s.size(); idx++){
        if(s[idx] == sep){ // If we hit a separating character, pull out the latest number
          if(num_chars > 0){
            vec.push_back(std::stod(stream.str()));
            stream.str("");
            num_chars = 0;
          }
        } // TODO: What if we hit a non-numeric, non-separator character?
        else { // Otherwise, add this to the stream
          stream << s[idx];
          ++num_chars;
        }
      } 
      // If we have characters left in the stream when we hit the end of the string, convert them!
      if(num_chars > 0){ 
        vec.push_back(std::stod(stream.str()));
        stream.str("");
      }
    }

    // Post-configuration module setup
    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<double>>(world.input_trait);
      AddRequiredTrait<emp::BitVector>(world.action_trait);
      AddOwnedTrait<double>(world.fitness_trait, "BerryWorld fitness value", 0.0);
      // Convert config options passed as strings to usable vectors
      SpliceStringIntoVec(food_reward_str, world.food_reward_vec); 
      SpliceStringIntoVec(food_replacement_probs_str, world.food_replacement_probs_vec); 
      // Spit out some info to make sure conversions worked
      std::cout << "Berry world initialized with " << 
          world.food_type_count << " types of food!" << std::endl;
      std::cout << "Food rewards: " << std::endl;
      for(size_t idx = 0; idx < world.food_type_count; ++idx){
        std::cout << "    " << world.food_reward_vec[idx]  << std::endl;
      }
      std::cout << "Food replacement probabilities: " << std::endl;
      for(size_t idx = 0; idx < world.food_type_count; ++idx){
        std::cout << "    " << world.food_replacement_probs_vec[idx]  << std::endl;
      }
    
    }

    /// TODO: Create a berryworld evaluator class
    //  // We _could_ reuse the same map for each organism, and just randomize the map between gens
    //    // Unless that would cause some weird artifacts?
    //  // We get rid of the ton of local variables used here
    //// TODO: Redo the way movement is handled (or ensure this works)
    //// Use the rules of berry world to get the fitness of a particular organism
    //double GetFitness(Organism & org){
    //  // Huge list of local variables to keep the state of the world
    //  int cur_x = world_width / 2; 
    //  int cur_y = world_height / 2;
    //  int vel_x = 0;
    //  int vel_y = 1;
    //  double cur_fitness = 0;
    //  double p;
    //  bool has_eaten_here = false;
    //  bool has_eaten_before = false;
    //  size_t last_eaten = 0;
    //  // Create the world and fill it with berries according to probabilities.
    //  emp::vector<size_t> berry_map(world_width * world_height);
    //  for(size_t site_idx = 0; site_idx < berry_map.size(); ++site_idx){
    //    p = control.GetRandom().GetDouble();
    //    for(size_t food_idx = 0; food_idx < food_replacement_probs_vec.size(); ++food_idx){
    //      if(p < food_replacement_probs_vec[food_idx]){
    //        berry_map[site_idx] = food_idx;
    //        break;
    //      }
    //      else
    //        p -= food_replacement_probs_vec[food_idx];
    //    }
    //  }
    //  // Iterate for the specified number of updates
    //  for(size_t update = 0; update < max_updates; ++update){
    //    // Calculate input to send to the organism (1 if that food is located here, 0 otherwise) 
    //    emp::vector<double> input_vec(food_type_count, 0);
    //    if(!has_eaten_here){
    //        size_t cur_berry = berry_map[cur_y * world_width + cur_x];
    //        input_vec[berry_map[cur_y * world_width + cur_x]] = 1;
    //    }
    //    // Set org's inputs and get the org's outputs (here called actions)
    //    org.SetVar<emp::vector<double>>(input_trait, input_vec);
    //    org.GenerateOutput();
    //    const emp::BitVector & actions = org.GetVar<emp::BitVector>(action_trait);
    //    if (actions[1]){ // Eat
    //      // TODO: Verify this is how MABE does it
    //      if(!has_eaten_here){ // Hasn't eaten here -> Can eat!
    //        // If the org has eaten before, and this is not the same food eaten last time -> penalty
    //        if(has_eaten_before && berry_map[cur_y * world_width + cur_x] == last_eaten)
    //          cur_fitness -= task_switch_cost;
    //        else // First food eaten OR same food as last time -> reward!
    //          cur_fitness += food_reward_vec[berry_map[cur_y * world_width + cur_x]];
    //        // Bookkeeping
    //        has_eaten_before = true;
    //        has_eaten_here = true;
    //      }
    //    }
    //    else if(actions[0]){ // Move
    //      int old_x = cur_x;
    //      int old_y = cur_y;
    //      cur_x += vel_x;
    //      cur_y += vel_y;
    //      if(is_toroidal){ // If world is torus, wrap!
    //        cur_x = cur_x % world_width;
    //        cur_y = cur_y % world_height;
    //      }
    //      else{ // If the world is not a torus, keep org in the bounds! 
    //        if(cur_x < 0) cur_x = 0;
    //        else if(cur_x >=  world_width) cur_x = world_width - 1;
    //        if(cur_y < 0) cur_y = 0;
    //        else if(cur_y >=  world_height) cur_y = world_height - 1;
    //      }
    //      // If org ate the food that was here and then moved, add in some new food
    //      if(has_eaten_here & (cur_x != old_x || cur_y != old_y)){
    //        has_eaten_here = false;
    //        // Use food replacement probabilities
    //        for(size_t food_idx = 0; food_idx < food_replacement_probs_vec.size(); ++food_idx){
    //          if(p < food_replacement_probs_vec[food_idx]){
    //            berry_map[cur_y * world_width + cur_x] = food_idx;
    //            break;
    //          }
    //          else
    //            p -= food_replacement_probs_vec[food_idx];
    //        }
    //      }
    //    }
    //    else if (actions[2]){ // Turn left 45 degrees
    //      if     (cur_x == 0  && cur_y == 1) { cur_x =  1;  cur_y = 1; }  // South     -> Southeast 
    //      else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = -1; } // East      -> Northeast
    //      else if(cur_x == 0  && cur_y == -1) { cur_x = -1; cur_y = -1; } // North     -> Northwest
    //      else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = 1; }  // West      -> Southwest
    //      else if(cur_x == 1  && cur_y == 1) { cur_x =  1;  cur_y = 0; }  // Southeast -> East
    //      else if(cur_x == 1  && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northeast -> North
    //      else if(cur_x == -1 && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southwest -> South
    //      else if(cur_x == -1 && cur_y == -1) { cur_x = -1; cur_y = 0; }  // Northwest -> West
    //    }
    //    else if (actions[3]){ // Turn right 45 degrees
    //      if     (cur_x == 0  && cur_y == 1) { cur_x =  -1; cur_y = 1; }  // South     -> Southwest 
    //      else if(cur_x == 1  && cur_y == 0) { cur_x =  1;  cur_y = 1; }  // East      -> Southeast
    //      else if(cur_x == 0  && cur_y == -1) { cur_x = 1;  cur_y = -1; } // North     -> Northeast
    //      else if(cur_x == -1 && cur_y == 0) { cur_x =  -1; cur_y = -1; } // West      -> Northwest
    //      else if(cur_x == 1  && cur_y == 1) { cur_x =  0;  cur_y = 1; }  // Southeast -> South
    //      else if(cur_x == 1  && cur_y == -1) { cur_x = 1;  cur_y = 0; }  // Northeast -> East
    //      else if(cur_x == -1 && cur_y == 1) { cur_x =  -1; cur_y = 0; }  // Southwest -> West
    //      else if(cur_x == -1 && cur_y == -1) { cur_x = 0;  cur_y = -1; } // Northwest -> North
    //    }
    //  } 
    //  return cur_fitness;
    //}

    // Evaluate the collection of organisms
    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism, keeping track of best performer
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      world.GenerateBerryMap();
      for (Organism & org : alive_collect) {
        double fitness = world.GetFitness(org);
        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << world.fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalBerryWorld, "Evaluate organisms' berry-foraging behavior.");
}

#endif
