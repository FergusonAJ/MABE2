/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2023.
 *
 *  @file  RandomNumberGenerator.hpp
 *  @brief Provides MABE script access to various functions from Empirical's RNG
 */

#ifndef MABE_RANDOM_NUMBER_GENERATOR_H
#define MABE_RANDOM_NUMBER_GENERATOR_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class RandomNumberGenerator : public Module {

  public:
    RandomNumberGenerator(mabe::MABE & control,
        const std::string & name="RandomNumberGenerator",
        const std::string & desc="Provide's access to MABE's internal RNG")
        : Module(control, name, desc)
    { ; }
    ~RandomNumberGenerator() {  ; }

    size_t GetUInt(size_t a, size_t b){
      return a + control.GetRandom().GetUInt64(b-a);
    }
    
    double GetDouble(double a, double b){
      return control.GetRandom().GetDouble(a, b);
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("GET_UINT",
          [](RandomNumberGenerator & mod, size_t a, size_t b) { 
            return mod.GetUInt(a, b);
          },
          "Return random 64-bit unsigned int within [a,b)");
      info.AddMemberFunction("GET_DOUBLE",
          [](RandomNumberGenerator & mod, size_t a, size_t b) { 
            return mod.GetDouble(a, b);
          },
          "Return the total number of possible deletion mutations");
    }

  };

  MABE_REGISTER_MODULE(RandomNumberGenerator, "Generates one-step mutants for Avida-style genomes");
}

#endif
