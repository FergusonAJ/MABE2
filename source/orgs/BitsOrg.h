/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  BitsOrg.h
 *  @brief An organism consisting of a series of bits.
 *  @note Status: ALPHA
 */

#ifndef MABE_BITS_ORGANISM_H
#define MABE_BITS_ORGANISM_H

#include "../core/Organism.h"

#include "tools/BitVector.h"

namespace mabe {

  class BitsOrg : public Organism {
  protected:
    emp::BitVector bits;

  public:
    BitsOrg() { ; }
    BitsOrg(const BitsOrg &) = default;
    BitsOrg(BitsOrg &&) = default;
    BitsOrg(const emp::BitVector & in) : bits(in) { ; }
    BitsOrg(size_t N) : bits(N) { ; }
    ~BitsOrg() { ; }

    /// Setup the variables that can change with different types of this organism.
    static void SetupType(OrganismType & type) {
      type.AddVar<size_t>("N", "Number of bits in organism", 1);
    }

    /// Use the default constructor for cloning.
    emp::Ptr<Organism> Clone() const override { return emp::NewPtr<BitsOrg>(*this); }

    /// Use "to_string" to convert.
    std::string ToString() override { return emp::to_string(bits); }

    /// Don't do mutations unless a mutate function has been set.
    int Mutate(emp::Random & random) override {
      if (bits.size() == 0) return 0;
      for (size_t i = 0; i < 3; i++) {
        size_t pos = random.GetUInt(bits.size());
        bits[pos] = random.P(0.5);
      }
      return 3;
    }

    /// Just use the bit sequence as the output.
    void GenerateOutput(const std::string & output_name="result", size_t=0) override {
      var_map.Set<emp::BitVector>(output_name, bits);
    }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) override {
      return emp::GetTypeID<emp::BitVector>();
    }
  };

}

#endif