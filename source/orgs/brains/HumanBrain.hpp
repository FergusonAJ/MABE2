/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  HumanBrain.hpp
 *  @brief Brain that prompts the user for input, useful for testing. 
 *  @note Status: ALPHA
 */

#ifndef MABE_HUMAN_BRAIN_H
#define MABE_HUMAN_BRAIN_H

#include "../../core/MABE.hpp"
#include "../../core/Organism.hpp"
#include "../../core/OrganismManager.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/MVector.hpp"
#include "emp/math/MMatrix.hpp"

namespace mabe {

  class HumanBrain : public OrganismTemplate<HumanBrain> {
  protected:
    size_t input_nodes;
    size_t output_nodes;

  public:
    HumanBrain(OrganismManager<HumanBrain> & _manager)
      : OrganismTemplate<HumanBrain>(_manager)  { }
    ~HumanBrain() { ; }

    struct ManagerData : public Organism::ManagerData {
      std::string input_name = "brain_inputs";    ///< Name of trait to use as inputs to Human
      std::string output_name = "brain_outputs";  ///< Name of trait to use for Human outputs
      size_t num_inputs = 3;              ///< Number of input nodes in the Human
      size_t num_outputs = 2;             ///< Number of output nodes in the Human
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { return "human"; }

    // Not used, but must be defined
    size_t Mutate(emp::Random & random) override {
      return 0;
    }
    void Randomize(emp::Random & random) override {
      ;
    }
    void Initialize(emp::Random & random) override {
      ;
    }

    // Convert comma-separated string of numbers to a usable vector
    void SpliceStringIntoVec(const std::string & s, 
        emp::vector<size_t> & vec, const unsigned char sep = ','){
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
        vec.push_back(std::stoull(stream.str()));
        stream.str("");
      }
    }

    /// Setup this organism's configuration options.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().input_name, "input_name",
                      "Name of variable that contains input data.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain output bit sequence.");
      GetManager().LinkVar(SharedData().num_inputs, "num_inputs",
                      "Number of input values");
      GetManager().LinkVar(SharedData().num_outputs, "num_outputs",
                      "Number of output values");
    }

    /// Setup this organism type with the traits it needs to track.
    void SetupModule() override{ 
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().input_name,
                                  "Input to Human.",
                                  emp::vector<double>());
      // Setup the output trait.
      GetManager().AddOwnedTrait(SharedData().output_name,
                                  "Output from Human",
                                  emp::BitVector(SharedData().num_outputs));
    }
    
    /// Send the user current inputs and ask for outputs 
    void GenerateOutput() override {
      // Inputs
      emp::vector<double> input_vec = GetVar<emp::vector<double>>(SharedData().input_name);
      emp::BitVector output_vec(SharedData().num_outputs);
      std::cout << "Input: " << input_vec << std::endl;
      std::cout << "Please type your output as a comma separated list of non-negative integers then hit enter:" << std::endl;
      // Outputs
      bool valid_input = false;
      while(!valid_input){ // Keep looping until we get something usable (right number of values)
        emp::vector<size_t> user_input_vec; 
        std::string user_input_str;
        std::cin >> user_input_str;
        // Turn string into vector
        SpliceStringIntoVec(user_input_str, user_input_vec);
        if(SharedData().num_outputs == user_input_vec.size()){// Do we have the right number of vals?
          valid_input = true;
          for(size_t idx = 0; idx < user_input_vec.size(); ++idx){ // Convert to bits
            output_vec[idx] = user_input_vec[idx] >= 0.5 ? 1 : 0;
          }
        }
        else
          std::cout << "Invalid input!" << std::endl;
        std::cout << std::endl;
      }
      SetVar<emp::BitVector>(SharedData().output_name, output_vec);
    }
  };

  MABE_REGISTER_ORG_TYPE(HumanBrain, "Simple artificial neural network");
}

#endif
