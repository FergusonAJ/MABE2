/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file  StringUtils.hpp
 *  @brief MABE helper modules
 */

#ifndef MABE_STRING_UTILS_H
#define MABE_STRING_UTILS_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class StringUtils : public Module {

  public:
    StringUtils(mabe::MABE & control,
           const std::string & name="StringUtils",
           const std::string & desc="Helper modules for strings")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~StringUtils() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("CONVERT_TO_BITSTRING",
                              [](StringUtils & mod, size_t idx, size_t num_bits) { 
                                emp::BitVector bit_vec(num_bits);
                                bit_vec.SetUInt64(0, idx);
                                return bit_vec.ToBinaryString();
                              },
                             "Convert int to bitstring of a given length");
    }

    void SetupConfig() override {
    }

    void SetupModule() override {
    }

  };

  MABE_REGISTER_MODULE(StringUtils, "Helper module for strings");
}

#endif
