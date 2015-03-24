#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <array>
#include <memory>
#include <cstdio>

using namespace std;
class Automaton;
using Automatons = array<Automaton, 2>;
class State;
using state_sptr = shared_ptr<State>;
class comparator;
using Automatons_ends = array< set<state_sptr,comparator>, 2 >;

//alphabet size is a global variable in order to dynamicaly control size of State class
unsigned short alphabet_size;

class State {
public:
  int number;
  unique_ptr< vector<state_sptr> > epsi;
  unique_ptr< state_sptr[] > delta;

  State(): delta(new state_sptr[alphabet_size]),
         epsi(new vector<state_sptr>) {}
};

class Automaton {
public:
  state_sptr begin, end;

  Automaton() = default;
  Automaton(state_sptr begin, state_sptr end): begin(begin), end(end) {}
};

class comparator{
public:
  bool operator()(state_sptr const & p, state_sptr const & q) {
      return p->number < q->number;
  }
};
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                parse                                                                *
 *---------------------------------------------------------------------------------------------------------------------*/
Automatons parse() {
  Automatons automatons;

  for (int i=0; i<2; ++i) {
      unsigned int number = 0;
      stack<Automaton> stack;
      char c;
      cin >> c;
      do {
              Automaton A, B;
              state_sptr begin = make_shared<State>(), end = make_shared<State>();
              begin->number = number++;
              end->number = number++;
              if (97 <= c && c < 97 + alphabet_size) {
                      begin->delta[c-97] = end;
              }
              else switch (c) {
                   case '0':  break; // empty one
                   case '1':  begin->epsi->push_back(end);
                              break;
                   case '+':  B = stack.top();
                              stack.pop();
                              A = stack.top();
                              stack.pop();
                              begin->epsi->push_back(A.begin);
                              begin->epsi->push_back(B.begin);
                              A.end->epsi->push_back(end);
                              B.end->epsi->push_back(end);
                              break;
                   case '.':  B = stack.top();
                              stack.pop();
                              A = stack.top();
                              stack.pop();
                              begin = A.begin;
                              number--;
                              A.end->epsi->push_back(B.begin);
                              end = B.end;
                              number--;
                              break;
                   case '*':  A = stack.top();
                              stack.pop();
                              begin->epsi->push_back(A.begin);
                              end = begin;
                              number--;
                              A.end->epsi->push_back(begin);
              }
              stack.push(Automaton(begin, end));
      } while ((c=getchar())!=' ' && c!='\n');
      automatons[i] = stack.top();
  }
  return automatons;
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                epsi_closure                                                         *
 *---------------------------------------------------------------------------------------------------------------------*/
void epsi_closure(set<state_sptr,comparator> & destination) {
  set<state_sptr,comparator> visited;
  queue<state_sptr> queue;

  for (auto & bstate : destination) {
      visited.insert(bstate);
      queue.push(bstate);
  }
  while (!queue.empty()) {
      state_sptr current(queue.front());
      queue.pop();
      for (auto & state : *current->epsi)
              if (visited.insert(state).second) {
                      destination.insert(state);
                      queue.push(state);
              }
  }
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                clean_up_time                                                        *
 *---------------------------------------------------------------------------------------------------------------------*/
void clean_up_time(state_sptr & state, set<state_sptr,comparator> & visited) {
  visited.insert(state);
  for (short a=0; a<alphabet_size; ++a)
      if (state->delta[a] != nullptr && visited.insert(state->delta[a]).second)
              clean_up_time(state->delta[a], visited);
  for (auto & child: *state->epsi)
      if (child != nullptr && visited.insert(child).second)
              clean_up_time(child, visited);
  state.reset();
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                epsi_NFA_to_DFA                                                      *
 *---------------------------------------------------------------------------------------------------------------------*/
set<state_sptr,comparator> epsi_NFA_to_DFA(Automaton & automaton, int counter) {
  queue< set<state_sptr,comparator> > queue;
  map< set<state_sptr,comparator>, state_sptr > rename_map;
  set<state_sptr,comparator> current, destination;
  state_sptr begin_dfa = make_shared<State>();
  int number = 2*counter;
  begin_dfa->number = number;
  number += counter;
  set<state_sptr,comparator> ends;

  // generate new begin state, remember it and push it on the queue
  current.insert(automaton.begin);
  epsi_closure(current);
  rename_map[current] = begin_dfa;
  if (current.find(automaton.end) != current.end())
      ends.insert(begin_dfa);
  queue.push(current);
  while (!queue.empty()) {
      current = queue.front();
      queue.pop();
        // generate states equal to delta(current, a)
      for (int a=0; a<alphabet_size; ++a) {
              destination.clear();
              // generate (possibly new) state as set of states
              for (auto & state : current)
                      if (state->delta[a] != nullptr)
                              destination.insert(state->delta[a]);
              if (destination.empty())
                      continue;
              epsi_closure(destination);
              // prepare state which will be assigned as delta(current, a)
              state_sptr dst_dfa;
              if (rename_map[destination] == nullptr) {
                      // create new state
                      dst_dfa = make_shared<State>();
                      dst_dfa->number = number;
                      number += counter;
                      // assign it to set of states
                      rename_map[destination] = dst_dfa;
                      // mark it as final state if necessary
                      if (destination.find(automaton.end) != destination.end())
                              ends.insert(dst_dfa);
                      // remember new state to process it
                      queue.push(destination);
              } else
                      dst_dfa = rename_map[destination];

              // finally define delta(current, a)
              rename_map[current]->delta[a] = dst_dfa;
      }
  }
  // remove old automaton and change old automaton to new one
  set<state_sptr,comparator> visited;
  clean_up_time(automaton.begin, visited);
  automaton.begin = begin_dfa;
  automaton.end = nullptr;
  //return set of end states of new automaton
  return ends;
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                find-union                                                           *
 *---------------------------------------------------------------------------------------------------------------------*/
map<state_sptr, state_sptr> representants;
map<state_sptr, unsigned int> heights;
void add_to_fu(state_sptr & state) {
  representants[state] = state;
  heights[state] = 0;
}

state_sptr ufind(state_sptr & state) {
  if (representants[state] == nullptr)
      add_to_fu(state);
  if (state == representants[state])
      return state;
  return representants[state] = ufind(representants[state]);
}

void funion(state_sptr & state0, state_sptr & state1) {
  state_sptr repre0 = ufind(state0);
  state_sptr repre1 = ufind(state1);
  if (heights[repre0] == heights[repre1]) {
      representants[repre0] = repre1;
      heights[repre1]++;
  } else if (heights[repre0] < heights[repre1])
      representants[repre0] = repre1;
  else
      representants[repre1] = repre0;
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                are_isomorpic                                                        *
 *---------------------------------------------------------------------------------------------------------------------*/
bool are_isomorphic(Automatons & automatons, Automatons_ends & ends) {
  add_to_fu(automatons[0].begin);
  add_to_fu(automatons[1].begin);
  state_sptr trash[2] = {make_shared<State>(), make_shared<State>()};
  trash[0]->number = 1;
  trash[1]->number = -1;
  using sspair = pair<state_sptr, state_sptr>;
  stack<sspair> stack;
  funion(automatons[0].begin, automatons[1].begin);
  stack.push( sspair(automatons[0].begin, automatons[1].begin) );
  while (!stack.empty()) {
      state_sptr first  = stack.top().first;
      state_sptr second = stack.top().second;
      stack.pop();
      //check if first, second are both final states or both are not
      int f_ind = (first->number > 0 ? 0 : 1);
      int s_ind = (second->number > 0 ? 0 : 1);
      if (ends[f_ind].find(first) == ends[f_ind].end() && ends[s_ind].find(second) != ends[s_ind].end()) return false;
      if (ends[f_ind].find(first) != ends[f_ind].end() && ends[s_ind].find(second) == ends[s_ind].end()) return false;
      //end of checking
      for (int a = 0; a < alphabet_size; ++a) {
              state_sptr nxt_first = ufind(trash[(first->number > 0 ? 0 : 1)]);
              state_sptr nxt_second = ufind(trash[(second->number > 0 ? 0 : 1)]);
              if (first->delta[a] != nullptr)   nxt_first  = ufind(first->delta[a]);
              if (second->delta[a] != nullptr)  nxt_second = ufind(second->delta[a]);
              if (nxt_first != nxt_second) {
                      stack.push(sspair(nxt_first, nxt_second));
                      funion(nxt_first, nxt_second);
              }
      }
  }
  return true;
}
/*---------------------------------------------------------------------------------------------------------------------*
 *                                                       main                                                          *
 *---------------------------------------------------------------------------------------------------------------------*/
int main () {
  cin >> alphabet_size;
  int z;
  cin >> z;

  while (z--) {
      Automatons automatons( parse() );
      Automatons_ends ends{ epsi_NFA_to_DFA(automatons[0], 1),
                                               epsi_NFA_to_DFA(automatons[1], -1) };
      if (are_isomorphic(automatons, ends))
              cout << "TAK" << endl;
      else
              cout << "NIE" << endl;
      //clean up
      set<state_sptr, comparator> visited;
      clean_up_time(automatons[0].begin, visited);
      visited.clear();
      clean_up_time(automatons[1].begin, visited);
      representants.clear();
      heights.clear();
  }
}
