/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2024.
 *
 *  @file
 *  @brief StateGrid maintains a rectilinear grid that agents can traverse.
 *
 *  State grids are a matrix of values, representing states of a 2D environment that an organism
 *  can traverse.
 *
 *  @todo Functions such as Load() should throw exceptions (or equilv.), not use asserts.
 *  @todo Need to figure out a default mapping for how outputs translate to moves around a
 *    state grid.  -1 = Back up ; 0 = Turn left ; 1 = Move fast-forwards ; 2 = Turn right
 *  @todo Allow StateGridInfo to be built inside of StateGrid (change reference to pointer and
 *    possible ownership)
 */


#ifndef MABE_TOOLS_STATE_GRID_HPP
#define MABE_TOOLS_STATE_GRID_HPP

#include <map>
#include <unordered_map>
#include <string>

#include "emp/base/assert.hpp"
#include "emp/base/error.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/data/Datum.hpp"
#include "emp/io/File.hpp"
#include "emp/math/math.hpp"
#include "emp/math/Random.hpp"

namespace mabe {

  /// Full information about the states available in a state grid and meanings of each state.
  class StateGridInfo {
  protected:

    /// Information about what a particular state type means in a state grid.
    struct StateInfo {
      int state_id;        ///< Ordinal id for this state.
      char symbol;         ///< Symbol for printing this state.
      double score_change; ///< Change amount for organism score by stepping on this square.
      emp::String name;    ///< Name of this state.
      emp::String desc;    ///< Explanation of this state.

      StateInfo(int _id, char _sym, double _change,
                const emp::String & _name, const emp::String & _desc)
      : state_id(_id), symbol(_sym), score_change(_change), name(_name), desc(_desc) { ; }
      StateInfo(const StateInfo &) = default;
      StateInfo(StateInfo &&) = default;
      ~StateInfo() { ; }

      StateInfo & operator=(const StateInfo &) = default;
      StateInfo & operator=(StateInfo &&) = default;
    };

    emp::vector<StateInfo> states;           ///< All available states.  Position is key ID

    std::map<int, size_t> state_map;         ///< Map of state_id to key ID (state_id can be < 0)
    std::map<char, size_t> symbol_map;       ///< Map of symbols to associated key ID
    std::map<emp::String, size_t> name_map;  ///< Map of names to associated key ID

    size_t GetKey(int state_id) const { return emp::Find(state_map, state_id, 0); }
    size_t GetKey(char symbol) const { return emp::Find(symbol_map, symbol, 0); }
    size_t GetKey(const emp::String & name) const { return emp::Find(name_map, name, 0); }
  public:
    StateGridInfo() : states(), state_map(), symbol_map(), name_map() { ; }
    StateGridInfo(const StateGridInfo &) = default;
    StateGridInfo(StateGridInfo &&) = default;
    ~StateGridInfo() { ; }

    StateGridInfo & operator=(const StateGridInfo &) = default;
    StateGridInfo & operator=(StateGridInfo &&) = default;

    size_t GetNumStates() const { return states.size(); }

    // Convert from state ids...
    char GetSymbol(int state_id) const { return states[ GetKey(state_id) ].symbol; }
    double GetScoreChange(int state_id) const { return states[ GetKey(state_id) ].score_change; }
    const emp::String & GetName(int state_id) const { return states[ GetKey(state_id) ].name; }
    const emp::String & GetDesc(int state_id) const { return states[ GetKey(state_id) ].desc; }

    // Convert to state ids...
    int GetState(char symbol) const {
      emp_assert( states.size() > GetKey(symbol), states.size(), symbol, (int) symbol );
      return states[ GetKey(symbol) ].state_id;
    }
    int GetState(const emp::String & name) const { return states[ GetKey(name) ].state_id; }

    void AddState(int id, char symbol, double mult=1.0, emp::String name="", emp::String desc="") {
      size_t key_id = states.size();
      states.emplace_back(id, symbol, mult, name, desc);
      state_map[id] = key_id;
      symbol_map[symbol] = key_id;
      name_map[name] = key_id;
    }

  };

  /// A StateGrid describes a map of grid positions to the current state of each position.
  class StateGrid {
  protected:
    size_t width;             ///< Width of the overall grid
    size_t height;            ///< Height of the overall grid
    emp::vector<int> states;  ///< Specific states at each position in the grid.
    StateGridInfo info;       ///< Information about the set of states used in this grid.
    bool is_toroidal;         /**< Decides how boundaries are handled.  
                                   If true, the grid is toroidal and agents that wander off 
                                      one side wrap to the opposite side.
                                   If false, agents are clamped to the grid. */
    std::unordered_map<std::string, emp::Datum> metadata; ///< Map of name->data for metadata
    unsigned char comment_char = '#';            ///< Character preceding a comment line
    unsigned char metadata_prefix_char = '$';    ///< Character preceding a metadata line
    unsigned char metadata_separator_char = ';'; ///< Char that splits two metadata statements
    unsigned char metadata_assign_char = '=';    ///< Char that assigns a value to a key
                                                     
    /// Parse a single line of metadata
    ///
    /// Metadata lines start with a given character ($ by deafult).
    /// Key value pairs take the from key1=val2;key2=val2... but characters can be changed
    void ParseMetadata(std::string line_str){
      line_str.erase(0, 1); // Remove prefix char
      // Remove extra separator if needed
      if(line_str[line_str.size() - 1] == metadata_separator_char){ 
        line_str.erase(line_str.size() - 1, 1);
      }
      emp::vector<std::string> pair_vec; 
      emp::slice(line_str,  pair_vec, metadata_separator_char); 
      for(std::string pair : pair_vec){
        emp::vector<std::string> parts_vec;
        emp::slice(pair, parts_vec, metadata_assign_char);
        if(parts_vec.size() != 2){
          emp_error("Error! Invalid StateGrid metadata pair! ", pair); 
        }
        std::string key = parts_vec[0];
        emp::justify(key);
        std::string value = parts_vec[1];
        emp::justify(value);
        metadata[key] = emp::Datum(value);
        std::cout << "Found metadata: " << key << " -> " << value << std::endl;
      }
    }

  public:
    StateGrid() : width(0), height(0), states(0), info(), is_toroidal(false) { ; }
    StateGrid(StateGridInfo & _i, size_t _w=1, size_t _h=1, 
        int init_val=0, bool _is_toroidal=false)
      : width(_w), height(_h), states(_w*_h,init_val), info(_i), 
        is_toroidal(_is_toroidal) { ; }
    StateGrid(StateGridInfo & _i, const emp::String & filename)
      : width(1), height(1), states(), info(_i), is_toroidal(false) { Load(filename); }
    StateGrid(const StateGrid &) = default;
    StateGrid(StateGrid && in) = default;
    ~StateGrid() { ; }

    StateGrid & operator=(const StateGrid &) = default;
    StateGrid & operator=(StateGrid &&) = default;

    size_t GetWidth() const { return width; }
    size_t GetHeight() const { return height; }
    size_t GetSize() const { return states.size(); }
    const emp::vector<int> GetStates() const { return states; }
    const StateGridInfo & GetInfo() const { return info; }
    unsigned char GetCommentChar() const { return comment_char; }
    unsigned char GetMetadataPrefixChar() const { return metadata_prefix_char; }
    unsigned char GetMetadataSepraratorChar() const { return metadata_separator_char; }
    unsigned char GetMetadataAssignChar() const { return metadata_assign_char; }
    void SetCommentChar(unsigned char c)  { comment_char = c; }
    void SetMetadataPrefixChar(unsigned char c)  { metadata_prefix_char = c; }
    void SetMetadataSepraratorChar(unsigned char c)  { metadata_separator_char = c; }
    void SetMetadataAssignChar(unsigned char c)  { metadata_assign_char = c; }
    bool HasMetadata(const std::string& s) { return metadata.find(s) != metadata.end(); }
    emp::Datum& GetMetadata(const std::string& s) { 
      emp_assert(HasMetadata(s));
      return metadata[s]; 
    }


    int & operator()(size_t x, size_t y) {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y*width+x];
    }
    int operator()(size_t x, size_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y*width+x];
    }
    int GetState(size_t x, size_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return states[y*width+x];
    }
    int GetState(size_t id) const { return states[id]; }
    StateGrid & SetState(size_t x, size_t y, int in) {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      states[y*width+x] = in;
      return *this;
    }
    char GetSymbol(size_t x, size_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetSymbol(GetState(x,y));
    }
    double GetScoreChange(size_t x, size_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetScoreChange(GetState(x,y));
    }
    const emp::String & GetName(size_t x, size_t y) const {
      emp_assert(x < width, x, width);
      emp_assert(y < height, y, height);
      return info.GetName(GetState(x,y));
    }
    bool GetIsToroidal() const { return is_toroidal; }
    void SetIsToroidal(bool b){ is_toroidal = b; }

    /// Return a BitVector indicating which positions in the state grid have a particular state.
    emp::BitVector IsState(int target_state) {
      emp::BitVector sites(states.size());
      for (size_t i = 0; i < states.size(); i++) sites[i] = (states[i] == target_state);
      return sites;
    }

    /// Setup the StateGridInfo with possible states.
    template <typename... Ts>
    void AddState(Ts &&... args) { info.AddState(std::forward<Ts>(args)...); }

    /// Load in the contents of a StateGrid using the file information provided.
    template <typename... Ts>
    StateGrid & Load(Ts &&... args) {
      std::cout << "Loading!" << std::endl;

      // Load this data from a stream or a file.
      emp::File file(std::forward<Ts>(args)...);
      file.RemoveWhitespace();
      file.RemoveEmpty();
      if(file.GetNumLines() == 0){
        emp_error("Error! StateGrid attempting to load file that is empty or missing!"); 
      }
      // Preamble (all comment and metadata lines at the top of the file) 
      size_t num_preamble_lines = 0;
      for(size_t line_idx = 0; line_idx < file.GetNumLines(); ++line_idx){
        if(file[line_idx][0] != metadata_prefix_char && file[line_idx][0] != comment_char){
          break;
        }
        num_preamble_lines++;
        // Parse metadata
        if(file[line_idx][0] == metadata_prefix_char) ParseMetadata(file[line_idx]);
      }

      // Determine the size of the new grid.
      height = file.GetNumLines() - num_preamble_lines;
      emp_assert(height > 0);
      width = file[num_preamble_lines].size();
      emp_assert(width > 0);

      // Now that we have the new size, resize the state grid.
      size_t size = width * height;
      states.resize(size);

      // Load in the specific states.
      for (size_t row = 0; row < file.GetNumLines() - num_preamble_lines; row++) {
        // Make sure all rows are the same size.
        emp_assert(file[row + num_preamble_lines].size() == width);  
        for (size_t col = 0; col < width; col++) {
          states[row*width+col] = info.GetState(file[row + num_preamble_lines][col]);
        }
      }

      return *this;
    }

    /// Print the current status of the StateGrid to an output stream.
    template <typename... Ts>
    const StateGrid & Print(std::ostream & os=std::cout) const {
      emp::String out(width*2-1, ' ');
      for (size_t i = 0; i < height; i++) {
        out[0] = info.GetSymbol( states[i*width] );
        for (size_t j = 1; j < width; j++) {
          out[j*2] = info.GetSymbol( states[i*width+j] );
        }
        os << out << std::endl;
      }
      return *this;
    }

    /// Store the current status of the StateGrid to a file.
    template <typename... Ts>
    const StateGrid & Write(Ts &&... args) const {
      emp::File file;
      emp::String out;
      for (size_t i = 0; i < height; i++) {
        out.resize(0);
        out += info.GetSymbol( states[i*width] );
        for (size_t j = 1; j < width; j++) {
          out += ' ';
          out +=info.GetSymbol( states[i*width+j] );
        }
        file.Append(out);
      }
      file.Write(std::forward<Ts>(args)...);
      return *this;
    }
  };

  /// Information about a particular agent on a state grid.
  class StateGridStatus {
  protected:
    struct State {
      size_t x;      ///< X-coordinate of this agent
      size_t y;      ///< Y-coordinate of this agent.
      int facing;    /**< 0=UL, 1=Up, 2=UR, 3=Right, 4=DR, 5=Down, 6=DL, 7=Left (+=Clockwise)
                          Most often handled as a size_t, but must be signed for rotations
                          to function correctly (i.e., for the number to go negative before
                          being put through a modulo operation) */

      State(size_t _x=0, size_t _y=0, size_t _f=1) : x(_x), y(_y), facing((int)_f) { ; }
      bool IsAt(size_t _x, size_t _y) const { return x == _x && y == _y; }
    };

    State cur_state;             ///< Position and facing currently used.
    bool track_moves;            ///< Should we record every move made by this organism?
    emp::vector<State> history;  ///< All previous positions and facings in this path.

    // --- Helper Functions ---

    /// If we are tracking moves, store the current position in the history.
    void UpdateHistory() { if (track_moves) history.push_back(cur_state); }

    /// Move explicitly in the x direction (regardless of facing).
    void MoveX(const StateGrid & grid, int steps=1) {
      emp_assert(grid.GetWidth(), grid.GetWidth());
      if(grid.GetIsToroidal()){
        cur_state.x = (size_t) emp::Mod(steps + (int) cur_state.x, (int) grid.GetWidth());
      }
      else{
        if(steps >= 0){
          cur_state.x = (size_t) ( (steps + (int) cur_state.x >= (int) grid.GetWidth()) ?
              grid.GetWidth() - 1 : steps + (int) cur_state.x );
        }
        else{
          cur_state.x = (size_t) ( ((int) cur_state.x + steps < 0) ?
              0 : (int) cur_state.x + steps );
        }
      }
    }

    /// Move explicitly in the y direction (regardless of facing).
    void MoveY(const StateGrid & grid, int steps=1) {
      emp_assert(grid.GetHeight(), grid.GetHeight());
      if(grid.GetIsToroidal()){
        cur_state.y = (size_t) emp::Mod(steps + (int) cur_state.y, (int) grid.GetHeight());
      }
      else{
        if(steps >= 0){
          cur_state.y = (size_t) ( (steps + (int) cur_state.y >= (int) grid.GetHeight()) ?
              grid.GetHeight() - 1 : steps + (int) cur_state.y );
        }
        else{
          cur_state.y = (size_t) ( (steps + (int) cur_state.y < 0) ?
              0 : steps + (int) cur_state.y );
        }
      }
    }

  public:
    StateGridStatus() : cur_state(0,0,1), track_moves(false) { ; }
    StateGridStatus(const StateGridStatus &) = default;
    StateGridStatus(StateGridStatus &&) = default;

    StateGridStatus & operator=(const StateGridStatus &) = default;
    StateGridStatus & operator=(StateGridStatus &&) = default;

    size_t GetX() const { return cur_state.x; }
    size_t GetY() const { return cur_state.y; }
    size_t GetFacing() const {
      emp_assert(cur_state.facing >= 0 && cur_state.facing < 8);
      return (size_t) cur_state.facing;
    }
    size_t GetIndex(const StateGrid& grid) const {
      return (cur_state.y * grid.GetWidth()) + cur_state.x;
    }

    bool IsAt(size_t x, size_t y) const { return cur_state.IsAt(x,y); }
    bool WasAt(size_t x, size_t y) const {
      for (const State & state : history) if (state.IsAt(x,y)) return true;
      return false;
    }

    /// Get a BitVector indicating the full history of which positions this organism has traversed.
    emp::BitVector GetVisited(const StateGrid & grid) const {
      emp::BitVector at_array(grid.GetSize());
      for (const State & state : history) {
        size_t pos = state.x + grid.GetWidth() * state.y;
        at_array.Set(pos);
      }
      return at_array;
    }

    StateGridStatus & TrackMoves(bool track=true) {
      bool prev = track_moves;
      track_moves = track;
      if (!prev && track_moves) history.push_back(cur_state);
      else history.resize(0);
      return *this;
    }

    StateGridStatus & Set(size_t _x, size_t _y, size_t _f) {
      cur_state.x = _x;
      cur_state.y = _y;
      cur_state.facing = (int) _f;
      UpdateHistory();
      return *this;
    }
    StateGridStatus & SetX(size_t _x) { cur_state.x = _x; UpdateHistory(); return *this; }
    StateGridStatus & SetY(size_t _y) { cur_state.y = _y; UpdateHistory(); return *this; }
    StateGridStatus & SetPos(size_t _x, size_t _y) {
      cur_state.x = _x;
      cur_state.y = _y;
      UpdateHistory();
      return *this;
    }
    StateGridStatus & SetFacing(size_t _f) { cur_state.facing = (int) _f; UpdateHistory(); return *this; }

    /// Move in the direction currently faced.
    void Move(const StateGrid & grid, int steps=1) {
       //std::cout << "steps = " << steps
       //          << "  facing = " << cur_state.facing
       //          << "  start = (" << cur_state.x << "," << cur_state.y << ")";
      switch (cur_state.facing) {
        case 0: MoveX(grid, -steps); MoveY(grid, -steps); break;
        case 1:                      MoveY(grid, -steps); break;
        case 2: MoveX(grid, +steps); MoveY(grid, -steps); break;
        case 3: MoveX(grid, +steps);                      break;
        case 4: MoveX(grid, +steps); MoveY(grid, +steps); break;
        case 5:                      MoveY(grid, +steps); break;
        case 6: MoveX(grid, -steps); MoveY(grid, +steps); break;
        case 7: MoveX(grid, -steps);                      break;
      }
      UpdateHistory();
       //std::cout << " end = (" << cur_state.x << "," << cur_state.y << ")"
       //          << "  facing = " << cur_state.facing
       //          << std::endl;
    }

    /// Rotate starting from current facing.
    void Rotate(int turns=1) {
      cur_state.facing = emp::Mod(cur_state.facing + turns, 8);
      UpdateHistory();
    }

    /// Move the current status to a random position and orientation.
    void Randomize(const StateGrid & grid, emp::Random & random) {
      Set(random.GetUInt(grid.GetWidth()), random.GetUInt(grid.GetHeight()), random.GetUInt(8));
    }

    /// Examine state of current position.
    int Scan(const StateGrid & grid) const{
      return grid(cur_state.x, cur_state.y);
      // @CAO: Should we be recording the scan somehow in history?
    }

    /// Set the current position in the state grid.
    void SetState(StateGrid & grid, int new_state) {
      grid.SetState(cur_state.x, cur_state.y, new_state);
    }

    /// Print the history of an organim moving around a state grid.
    void PrintHistory(StateGrid & grid, std::ostream & os=std::cout) const {
      emp_assert(history.size(), "You can only print history of a StateGrid if you track it!");
      const size_t width = grid.GetWidth();
      const size_t height = grid.GetHeight();
      emp::String out(width*2-1, ' ');
      for (size_t i = 0; i < height; i++) {
        for (size_t j = 1; j < width; j++) {
          out[j*2] = grid.GetSymbol(j,i);
          if (WasAt(j,i)) out[j*2] = '*';
        }
        os << out << std::endl;
      }
    }
  };

}

#endif
