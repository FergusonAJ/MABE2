/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2024.
 *
 *  @file  data_collect.hpp
 *  @brief Functions to collect data from containers.
 *
 *  A collection of mechanisms to aggregate data from arbitrary objects in arbitrary containers.
 * 
 *  Each build function must know the data type it is working with (DATA_T), the type of container
 *  it should expect (CONTAIN_T), and be provided a function that will take a container element and
 *  return the appropriate value of type DATA_T.
 * 
 *  BuildCollectFun(emp::String action, FUN_T get_fun) will return a the correct parse function.
 *
 *  Options are an index value for a supplied container or:
 *    "unique" || "richness"
 *    "mode" || "dom" || "dominant"
 *    "min"
 *    "max"
 *    "min_id"
 *    "max_id"
 *    "ave" || "mean"
 *    "median"
 *    "variance"
 *    "stddev"
 *    "sum" || "total"
 *    "entropy"
 */

#ifndef EMP_DATA_COLLECT_H
#define EMP_DATA_COLLECT_H

#include <functional>

#include "emp/datastructs/vector_utils.hpp"
#include "emp/tools/String.hpp"

#include "../Emplode/Symbol.hpp"

namespace mabe {
  namespace DataCollect {
    using Symbol_Var = emplode::Symbol_Var;

    // Return the value at a specified index.
    template <typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Index(const CONTAIN_T & container, FUN_T get_fun, const size_t index, FUN_2_T valid_fun) {
      if (container.size() <= index) return std::string{"nan"};
      if (valid_fun(container.At(index))) return get_fun( container.At(index) );
      return std::string{"nan"};
    }

    // Count up the number of distinct values.
    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Unique(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      std::unordered_set<DATA_T> vals;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          vals.insert( get_fun(entry) );
        }
      }
      return vals.size();
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Mode(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      std::unordered_map<DATA_T, size_t> vals;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          vals[ get_fun(entry) ]++;
        }
      }
      DATA_T mode_val{};
      size_t mode_count = 0;

      for (auto [cur_val, cur_count] : vals) {
        if (cur_count > mode_count) {
          mode_count = cur_count;
          mode_val = cur_val;
        }
      }
      if(mode_count == 0) return std::string {"nan"};
      return mode_val;
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Min(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      DATA_T min{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        min = std::numeric_limits<DATA_T>::max();
      }
      else if constexpr (std::is_same_v<std::string, DATA_T>) {
        min = std::string('~',22);   // '~' is ascii char 126 (last printable one.)
      }
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          count++;
          const DATA_T cur_val = get_fun(entry);
          if (cur_val < min) min = cur_val;
        }
      }
      if(count == 0) return std::string {"nan"};
      return min;
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Max(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      DATA_T max{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        max = std::numeric_limits<DATA_T>::lowest();
      }
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          count++;
          const DATA_T cur_val = get_fun(entry);
          if (cur_val > max) max = cur_val;
        }
      }
      if(count == 0) return std::string {"nan"};
      return max;
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var MinID(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      DATA_T min_val{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        min_val = std::numeric_limits<DATA_T>::max();
      }
      else if constexpr (std::is_same_v<std::string, DATA_T> || std::is_same_v<emp::String, DATA_T>) {
        min_val = std::string('~',22);   // '~' is ascii char 126 (last printable one.)
      }
      size_t id = 0;
      size_t min_id = 0;
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          count++;
          const DATA_T cur_val = get_fun(entry);
          if (cur_val < min_val) { min_val = cur_val; min_id = id; }
        }
        ++id;
      }
      if(count == 0) return std::string {"nan"};
      return min_id;
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var MaxID(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      DATA_T max_val{};
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        max_val = std::numeric_limits<DATA_T>::lowest();
      }
      size_t id = 0;
      size_t max_id = 0;
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          count++;
          const DATA_T cur_val = get_fun(entry);
          if (cur_val > max_val) { max_val = cur_val; max_id = id; }
        }
        ++id;
      }
      if(count == 0) return std::string {"nan"};
      return max_id;
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Mean(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        size_t count = 0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            total += (double) get_fun(entry);
            count++;
          }
        }
        if(count == 0) return std::string {"nan"};
        return total / count;
      }
      return emp::String{"nan"};
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Median(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      emp::vector<DATA_T> values(container.size());
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          values[count++] = get_fun(entry);
        }
      }
      if(count == 0) return std::string {"nan"};
      emp::Sort(values);
      return values[count/2];
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Variance(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        double N = 0.0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            total += (double) get_fun(entry);
            N += 1;
          }
        }
        if(N == 0) return std::string {"nan"};
        double mean = total / N;
        double var_total = 0.0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            double cur_val = mean - (double) get_fun(entry);
            var_total += cur_val * cur_val;
          }
        }

        return var_total / (N-1);
      }
      return emp::String{"nan"};
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var StandardDeviation(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        double N = 0.0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            total += (double) get_fun(entry);
          }
        }
        if(N == 0) return std::string {"nan"};
        double mean = total / N;
        double var_total = 0.0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            double cur_val = mean - (double) get_fun(entry);
            var_total += cur_val * cur_val;
          }
        }

        return sqrt(var_total / (N-1));
      }
      return emp::String{"nan"};
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Sum(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      if constexpr (std::is_arithmetic_v<DATA_T>) {
        double total = 0.0;
        for (const auto & entry : container) {
          if(valid_fun(entry)){
            total += (double) get_fun(entry);
          }
        }
        return total;
      }
      return emp::String{"nan"};
    }

    template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
    Symbol_Var Entropy(const CONTAIN_T & container, FUN_T get_fun, FUN_2_T valid_fun) {
      std::map<DATA_T, size_t> vals;
      size_t count = 0;
      for (const auto & entry : container) {
        if(valid_fun(entry)){
          count += 1;
          vals[ get_fun(entry) ]++;
        }
      }
      if(count == 0) return std::string {"nan"};
      const size_t N = container.size();
      double entropy = 0.0;
      for (auto [entry, count] : vals) {
        double p = ((double) count) / (double) N;
        entropy -= p * log2(p);
      }
      return entropy;
    }
  } // End namespace DataCollect

  template <typename DATA_T, typename CONTAIN_T, typename FUN_T, typename FUN_2_T>
  std::function<emplode::Symbol_Var(const CONTAIN_T &)>
  BuildCollectFun(std::string action, FUN_T get_fun, FUN_2_T valid_fun) {
    // ### DEFAULT
    // If no trait function is specified, assume that we should use the first index.
    if (action == "") action = "0";

    // Return the index if a simple number was provided.
    if (emp::is_digits(action)) {
      size_t index = emp::from_string<size_t>(action);
      return [get_fun,index,valid_fun](const CONTAIN_T & container) {
        return DataCollect::Index<CONTAIN_T>(container, get_fun, index, valid_fun);
      };
    }

    // Return the number of distinct values found in this trait.
    else if (action == "unique" || action == "richness") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Unique<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the most common value found for this trait.
    else if (action == "mode" || action == "dom" || action == "dominant") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Mode<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the lowest trait value.
    else if (action == "min") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Min<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the highest trait value.
    else if (action == "max") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Max<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the lowest trait value.
    else if (action == "min_id") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::MinID<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the highest trait value.
    else if (action == "max_id") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::MaxID<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the average trait value.
    else if (action == "ave" || action == "mean") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Mean<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the middle-most trait value.
    else if (action == "median") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Median<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the standard deviation of all trait values.
    else if (action == "variance") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Variance<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the standard deviation of all trait values.
    else if (action == "stddev") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::StandardDeviation<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the total of all trait values.
    else if (action == "sum" || action == "total") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Sum<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    // Return the entropy of values for this trait.
    else if (action == "entropy") {
      return [get_fun, valid_fun](const CONTAIN_T & container) {
        return DataCollect::Entropy<DATA_T, CONTAIN_T>(container, get_fun, valid_fun);
      };
    }

    return std::function<emplode::Symbol_Var(const CONTAIN_T &)>();
  }

}

#endif
