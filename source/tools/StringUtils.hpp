/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  StringUtils.hpp
 *  @brief Module that exposes various Empirical / standard library string utility 
 *  functions to the MABE config
 */

#ifndef MABE_STRING_UTILS_H
#define MABE_STRING_UTILS_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "emp/tools/string_utils.hpp"

namespace mabe {

  class StringUtils : public Module {

  public:
    StringUtils(mabe::MABE & control,
        const std::string & name="StringUtils",
        const std::string & desc="Expose string functions to MABE config")
        : Module(control, name, desc)
    { ; }
    ~StringUtils() {  ; }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("REMOVE_CHAR_AT",
          [](StringUtils & mod, const std::string& str, size_t idx_to_remove) { 
            std::string out_str = str;
            out_str.replace(idx_to_remove, 1, "");
            return out_str;
          },
          "Return the same string but with the Nth char removed");
      info.AddMemberFunction("REPLACE",
          [](StringUtils & mod, const std::string& source_str, size_t idx, size_t len, 
                const std::string& replace_str) { 
            std::string out_str = source_str;
            out_str.replace(idx, len, replace_str);
            return out_str;
          },
          "Replace the given substring with the passed string");
    }

  };

  MABE_REGISTER_MODULE(StringUtils, "Expose string functions to MABE config");
}

#endif
