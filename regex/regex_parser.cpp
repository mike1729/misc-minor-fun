#include <iostream>
#include <memory>
#include <list>
#include <cstdio>

using namespace std;
class node {
public:
  char c;
  unique_ptr<node> left, right;
};

using upn = unique_ptr<node>;

inline bool in_range(char c, unsigned N) {
  return ('a'<=c && c<'a'+N) || c=='0' || c=='1';
}

bool replace(list<upn> & stack, char c, char d = 0) {
  if ( (++stack.begin() != stack.end()) && ((*++stack.begin())->c == c  || (*++stack.begin())->c == d) ){
      upn tmp(new node);
      tmp->right = move(stack.front());
      stack.pop_front();
      tmp->c = stack.front()->c;
      stack.pop_front();
      tmp->left = move(stack.front());
      stack.pop_front();
      stack.push_front(move(tmp));
      return true;
  }
  return false;
}

void free(upn &);
void end(list<upn> & stack, bool eat_line = true){
  cout << "SYNTAX ERROR" << endl;
  char c;
  if (eat_line) while ((c=getchar()) != '\n' && c!=EOF) {}
  while (!stack.empty()) {
      free(stack.front());
      stack.pop_front();
  }
}

void parse(upn & root, unsigned N) {
  char c;
  list<upn> stack;
  while ((c=getchar()) != '\n' && c!=EOF) {
      if (c == ' ' || c=='\t')
              continue;
      upn nupn(new node);
      nupn->c = c;
      nupn->left = nullptr;
      nupn->right = nullptr;
      if (in_range(c,N) || c=='(') {
              if ( !stack.empty() && (stack.front()->left != nullptr || in_range(stack.front()->c,N)) ) {
                      ungetc(c, stdin);
                      ungetc('.', stdin);
                      continue;
              }
              stack.push_front(move(nupn));
              continue;
      }
        if (c=='+' || c=='.' || c=='*')
              if (stack.empty() || !( in_range(stack.front()->c,N) || stack.front()->left!=nullptr)) {
                      end(stack);
                      return;
              }
      switch (c) {
      case '+':       replace(stack, '.', '+');
                      replace(stack, '+');
                      stack.push_front(move(nupn));
                      break;
      case '.':       replace(stack, '.');
                      stack.push_front(move(nupn));
                      break;
      case '*':       nupn->left = move(stack.front());
                      stack.pop_front();
                      stack.push_front(move(nupn));
                      break;
      case ')':       while (!stack.empty() && replace(stack, '.', '+')) {}
                      if (stack.size() <= 1) {
                              end(stack);
                              return;
                      }
                      stack.erase(++stack.begin());
                      break;
      default :       end(stack);
                      return;
      }
  }
  if (stack.empty() || stack.front()->c=='(') {
      end(stack, false);
      return;
  }
  while (replace(stack, '.', '+')) {}
  if (stack.size()>1 || (stack.size()==1 && stack.front()->c=='(')) {
      end(stack, false);
      return;
  }
  if (stack.size() == 1)
      root = move(stack.front());
}

void simplify(upn & node) {
  if (node->left != nullptr)
      simplify(node->left);
  if (node->right != nullptr)
      simplify(node->right);
  switch (node->c) {
  case '.':   if (node->left->c == '0' || node->right->c == '0') {
                      free(node->left);
                      free(node->right);
                      node->c = '0';
              } else if (node->left->c == '1') {
                      free(node->left);
                      node = move(node->right);
              } else if (node->right->c== '1') {
                      free(node->right);
                      node = move(node->left);
              }
              break;
  case '+':   if (node->left->c == '0') {
                      free(node->left);
                      node = move(node->right);
              } else if (node->right->c== '0') {
                      free(node->right);
                      node = move(node->left);
              }
              break;
  case '*':   if (node->left->c == '0' || node->left->c== '1') {
                      free(node->left);
                      node->c = '1';
              } else if (node->left->c == '*')
                      node = move(node->left);
  }
}

bool smaller_precedence(char c, char d) {
  if ( (c == '+' && (d == '.' || d == '*')) || (c == '.' && d == '*') )
      return true;
  return false;
}
void print(upn & node) {
  if (node->left != nullptr) {
      if (smaller_precedence(node->left->c, node->c))
              cout << '(';
      print(node->left);
      if (smaller_precedence(node->left->c, node->c))
              cout << ')';
  }
  if (node->c == '+')
      cout << " + ";
  else if (node->c != '.')
      cout << node->c;
  if (node->right != nullptr) {
      if (smaller_precedence(node->right->c, node->c))
              cout << '(';
      print(node->right);
      if (smaller_precedence(node->right->c, node->c))
              cout << ')';
  }
}

void free(upn & node) {
  if (node==nullptr)
      return;
  free(node->left);
  free(node->right);
  node.reset();
}

int main() {
  int N,z;
  cin >> N >> z;
  char c;
  while ( (c=getchar()) == ' ' || c == '\t') {}
  while (z--) {
      upn root;
      parse(root, N);
        if (root == nullptr)
              continue;
      simplify(root);
      print(root);
      cout << endl;
      free(root);
  }
}
