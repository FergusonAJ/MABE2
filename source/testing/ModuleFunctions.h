/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ModuleFunctions.h
 *  @brief Test module to see if modules can expose functions in the config file
 */

#ifndef MABE_MODULE_FUNCTION_H
#define MABE_MODULE_FUNCTION_H

#include "../core/MABE.h"
#include "../core/Module.h"


namespace mabe {

  class ModuleFunction : public Module {
  private:
    std::string print_value;
  public:
    ModuleFunction(mabe::MABE & control,
           const std::string & name="ModuleFunction",
           const std::string & desc="Test module to see if modules can expose functions in the config",
           const std::string & _pvalue="This is a test!")
      : Module(control, name, desc), print_value(_pvalue)
    {
    }
    ~ModuleFunction() { }

    void SetupConfig() override {
      LinkVar(print_value, "print_value", "Value to print when function is called.");
      std::function<int()> fire_func =
        [this]() {
          std::cout << print_value << std::endl;
          return 0;
        };
      GetScope().AddFunction("fire", fire_func,  
        "Prints the modules 'print_value' to the console.");
    }

    void SetupModule() override {
    }

  };

  MABE_REGISTER_MODULE(ModuleFunction, 
      "Test module to see if modules can expose functions in the config");
}

#endif
