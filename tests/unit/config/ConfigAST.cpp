/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigAST.cpp
 *  @brief Tests for ConfigAST with various types and edge cases
 */

#include <iostream>
#include <functional>

// CATCH
#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG
#include "catch.hpp"

// MABE
#include "config/ConfigEntry.hpp"
#include "config/ConfigScope.hpp"
#include "config/ConfigAST.hpp"
#include "config/ConfigFunction.hpp"

#include "emp/base/Ptr.hpp"



using entry_ptr_t = emp::Ptr<mabe::ConfigEntry>;
using entry_vector_t = emp::vector<entry_ptr_t>;

using node_ptr_t = emp::Ptr<mabe::ASTNode>;
using node_vector_t = emp::vector<node_ptr_t>;


TEST_CASE("ASTLeaf", "[config]"){
  {
    int v = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v, "variable00", nullptr);
    entry_ptr_t ptr00 = &entry00;
    emp::Ptr<mabe::ASTNode_Leaf> leaf00_ptr = emp::NewPtr<mabe::ASTNode_Leaf>(ptr00);

    // Test getter functions
    std::string str00 = leaf00_ptr->GetName();
    REQUIRE(str00.compare("name00") == 0);

    REQUIRE(&leaf00_ptr->GetEntry() == ptr00.Raw());

    // Test boolean functions
    REQUIRE(leaf00_ptr->IsLeaf());

    // Test Process()
    REQUIRE(leaf00_ptr->Process() == ptr00);

    // Test Write()
    std::stringstream ss;
    leaf00_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("name00") == 0);

    // Test Destructor()
    leaf00_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00_ptr.id));
  }
}


TEST_CASE("ASTNode_Block", "[config]"){
  {
    emp::Ptr<mabe::ASTNode_Block> block00_ptr = emp::NewPtr<mabe::ASTNode_Block>();

    // Test getter functions
    std::string str00 = block00_ptr->GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(block00_ptr->GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(block00_ptr->IsInternal());

    // Test adding children
    int v00 = 0;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    node_ptr_t leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    block00_ptr->AddChild(leaf00);

    REQUIRE(block00_ptr->GetNumChildren() == 1);
    REQUIRE(block00_ptr->GetChild(0)->IsLeaf());
    REQUIRE(block00_ptr->GetChild(0)->Process() == leaf00->Process());

    // Test multiple children
    int v01 = 1;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);

    node_ptr_t leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    block00_ptr->AddChild(leaf01);

    REQUIRE(block00_ptr->GetNumChildren() == 2);
    REQUIRE(block00_ptr->GetChild(1)->IsLeaf());
    REQUIRE(block00_ptr->GetChild(1)->Process() == leaf01->Process());

    // Test Process()
    REQUIRE(block00_ptr->Process() == nullptr);

    // Test Write()
    std::stringstream ss;
    block00_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("name00;\nname01;\n") == 0);

    // Test Destructor
    block00_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
  }
}

double abs_value(double n) {
  if (n < 0) {
    n *= -1;
  }
  return n;
}
TEST_CASE("ASTNode_Math1", "[config]"){
  {
    emp::Ptr<mabe::ASTNode_Math1> math100_ptr = emp::NewPtr<mabe::ASTNode_Math1>("math00");

    // Test getters
    std::string str00 = math100_ptr->GetName();
    REQUIRE(str00.compare("math00") == 0);

    REQUIRE(math100_ptr->GetNumChildren() == 0);

    // Test adding children
    int v00 = -1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00); // i don't think this is getting deleted but when i try i get seg faults
    math100_ptr->AddChild(leaf00);

    REQUIRE(math100_ptr->GetNumChildren() == 1);
    REQUIRE(math100_ptr->GetChild(0)->IsLeaf());
    REQUIRE(math100_ptr->GetChild(0)->Process() == leaf00->Process());

    // Test boolean functions
    REQUIRE(math100_ptr->IsInternal());

    // Test setters
    math100_ptr->SetFun(abs_value);

    // Test Process() with one child
    entry_ptr_t result00 = math100_ptr->Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result00->AsDouble() == 1.0);

    // Test Write()
    std::stringstream ss;
    math100_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("math00name00") == 0);

    // Add multiple children
    int v01 = -2;
    mabe::ConfigEntry_Linked entry01("name01", v01, "variable01", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math100_ptr->AddChild(leaf01);

    // Test getters for multiple children
    REQUIRE(math100_ptr->GetNumChildren() == 2);
    REQUIRE(math100_ptr->GetChild(1)->IsLeaf());

    // Test Process with multiple children
    entry_ptr_t result01 = math100_ptr->Process();
    REQUIRE(emp::assert_last_fail);

    // Test Destructor()
    math100_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));

    // Delete additional pointers
    result00.Delete();
    result01.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(result00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(result01.id));
  }
}

double add_fun(double n, double m) {
  return n + m;
}
TEST_CASE("ASTNode_Math2", "[config]"){
  {
    emp::Ptr<mabe::ASTNode_Math2> math200_ptr = emp::NewPtr<mabe::ASTNode_Math2>("math00");

    // Test getters
    std::string str00 = math200_ptr->GetName();
    REQUIRE(str00.compare("math00") == 0);

    REQUIRE(math200_ptr->GetNumChildren() == 0);

    // Test boolean functions
    REQUIRE(math200_ptr->IsInternal());

    // Test adding children
    int v00 = 1;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    math200_ptr->AddChild(leaf00);

    REQUIRE(math200_ptr->GetNumChildren() == 1);
    REQUIRE(math200_ptr->GetChild(0)->IsLeaf());
    REQUIRE(math200_ptr->GetChild(0)->Process() == leaf00->Process());

    // Add second child
    int v01 = 2;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    math200_ptr->AddChild(leaf01);

    REQUIRE(math200_ptr->GetNumChildren() == 2);
    REQUIRE(math200_ptr->GetChild(1)->IsLeaf());
    REQUIRE(math200_ptr->GetChild(1)->Process() == leaf01->Process());

    // Set function
    math200_ptr->SetFun(add_fun);

    // Test Process()
    emp::assert_clear();
    entry_ptr_t result00 = math200_ptr->Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result00->AsDouble() == 3.0);

    // Test Write()
    std::stringstream ss;
    math200_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("name00 math00 name01") == 0);

    // Add third child
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    math200_ptr->AddChild(leaf02);

    REQUIRE(math200_ptr->GetNumChildren() == 3);
    REQUIRE(math200_ptr->GetChild(2)->IsLeaf());
    REQUIRE(math200_ptr->GetChild(2)->Process() == leaf02->Process());

    // Test Process() with three children
    emp::assert_clear();
    entry_ptr_t result01 = math200_ptr->Process();
    REQUIRE(emp::assert_last_fail);

    // Test Destructor
    math200_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));

    // Delete additional pointers
    result00.Delete();
    result01.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(result00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(result01.id));

  }
}

TEST_CASE("ASTNode_Assign", "[config]"){
  {
    std::string v00 = "variable";
    mabe::ConfigEntry_Linked<std::string> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> lhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);

    int v01 = 1;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> rhs = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);

    emp::Ptr<mabe::ASTNode_Assign> assign00_ptr = emp::NewPtr<mabe::ASTNode_Assign>(lhs, rhs);

    // Test getters
    std::string str00 = assign00_ptr->GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(assign00_ptr->GetNumChildren() == 2);

    // Test boolean functions
    REQUIRE(assign00_ptr->IsInternal());

    // Test Process()
    emp::assert_clear(); // do this other places too!
    entry_ptr_t result00 = assign00_ptr->Process();
    REQUIRE(!emp::assert_last_fail);
    REQUIRE(result00->AsDouble() == 1.0);

    // Add third child
    int v02 = 2;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);

    emp::Ptr<mabe::ASTNode_Leaf> leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    assign00_ptr->AddChild(leaf02);

    REQUIRE(assign00_ptr->GetNumChildren() == 3);
    REQUIRE(assign00_ptr->GetChild(2)->IsLeaf());
    REQUIRE(assign00_ptr->GetChild(2)->Process() == leaf02->Process());

    // Test Process() with too many children
    entry_ptr_t result01 = assign00_ptr->Process();
    REQUIRE(emp::assert_last_fail);

    // Test Write()
    std::stringstream ss;
    assign00_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("name00 = name01") == 0);

    // Test Destructor
    assign00_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(lhs.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(rhs.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));
  }
}

TEST_CASE("ASTNode_Call", "[config]"){
  {
    int children_processed = 0;
    bool function_called;

    std::function<double(const entry_vector_t&)> setup = [&children_processed, &function_called](const entry_vector_t& entries) {
      std::cout << "in function" << std::endl;
      for (entry_ptr_t entry : entries) {
        children_processed++;
      }
      function_called = true;
      return 0;
    };

    // Create ConfigFunction
    mabe::ConfigFunction entry_func("func00", "desc00", nullptr);
    entry_func.SetFunction(setup);
    node_ptr_t funcs00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry_func);

    // Create vector of arguments
    node_vector_t args00;

    int v00 = 2;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    node_ptr_t leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    args00.push_back(leaf00);
    int v01 = 3;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    node_ptr_t leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    args00.push_back(leaf01);
    int v02 = 4;
    mabe::ConfigEntry_Linked<int> entry02("name02", v02, "variable02", nullptr);
    node_ptr_t leaf02 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry02);
    args00.push_back(leaf02);

    // Create ASTNode_Call
    emp::Ptr<mabe::ASTNode_Call> call00_ptr = emp::NewPtr<mabe::ASTNode_Call>(funcs00, args00);

    // Test getters
    std::string str00 = call00_ptr->GetName();
    REQUIRE(str00.compare("") == 0);

    REQUIRE(call00_ptr->GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    REQUIRE(call00_ptr->IsInternal());

    // Test Process()
    entry_ptr_t result = call00_ptr->Process();
    REQUIRE(children_processed == args00.size());
    REQUIRE(function_called == true);

    // Test Write()
    std::stringstream ss;
    call00_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("func00(name00, name01, name02)") == 0);

    // Test Destructor
    call00_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(funcs00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf02.id));

    // Delete additional pointers
    result.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(result.id));
  }
}

TEST_CASE("ASTNode_Event", "[config]"){
  {
    // Create action
    std::string v = "action00";
    mabe::ConfigEntry_Var<std::string> entry("action00", v, "desc00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> action00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry);

    // Create vector of arguments
    node_vector_t args00;

    int v00 = 2;
    mabe::ConfigEntry_Linked<int> entry00("name00", v00, "variable00", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf00 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry00);
    args00.push_back(leaf00);
    int v01 = 3;
    mabe::ConfigEntry_Linked<int> entry01("name01", v01, "variable01", nullptr);
    emp::Ptr<mabe::ASTNode_Leaf> leaf01 = emp::NewPtr<mabe::ASTNode_Leaf>(&entry01);
    args00.push_back(leaf01);

    // Create setup function

    int children_processed = 0;
    std::string action_result;

    std::function<void(node_ptr_t, const entry_vector_t &)> setup = [&children_processed, &action_result](node_ptr_t node, const entry_vector_t & vector) {
      std::stringstream ss;
      node->Write(ss, "");
      action_result = ss.str();

      for (emp::Ptr<mabe::ConfigEntry> child : vector) {
        children_processed++;
      }
    };

    emp::Ptr<mabe::ASTNode_Event> event00_ptr = emp::NewPtr<mabe::ASTNode_Event>("event00", action00, args00, setup);

    // Test getters
    std::string str00 = event00_ptr->GetName();
    REQUIRE(str00.compare("event00") == 0);

    REQUIRE(event00_ptr->GetNumChildren() == args00.size() + 1);

    // Test boolean functions
    REQUIRE(event00_ptr->IsInternal());

    // Test Process()
    event00_ptr->Process();
    REQUIRE(action_result.compare("action00") == 0);
    REQUIRE(children_processed == args00.size());

    // Test Write()
    std::stringstream ss;
    event00_ptr->Write(ss, "");
    REQUIRE(ss.str().compare("@event00(name00, name01) action00") == 0);

    // Test Destructor
    event00_ptr.Delete();
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf00.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(leaf01.id));
    REQUIRE(emp::BasePtr<void>::Tracker().IsDeleted(action00.id));
  }
}