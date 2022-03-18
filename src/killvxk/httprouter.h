#pragma once
#include <map>
#include <functional>
#include <vector>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>

namespace killvxk
{
  // placeholder
  struct string_view {
    const char *data;
    unsigned int length;

    std::string_view sv() const { return { data, length }; }
  };

  inline std::ostream &operator<<(std::ostream &os, string_view &s) {
    os << std::string(s.data, s.length);
    return os;
  }

  template <typename UserData, typename Result>
  class HttpRouter {
  private:
    using Handler = std::function<Result(UserData, std::vector<string_view> &&)>;
    std::vector<Handler> handlers;

    struct Node {
      using Ptr = std::unique_ptr<Node>;
      Node( std::string n, int16_t h ) : name{ std::move( n ) }, handler{ h } {}
      std::string name;
      std::map<std::string, Ptr> children{};
      int16_t handler;
    };

    typename Node::Ptr tree = std::make_unique<Node>( "GET", -1 );
    std::string compiled_tree;

    void add(std::vector<std::string> route, int16_t handler) {
      Node* parent = tree.get();
      for (std::string node : route) {
        if (parent->children.find(node) == parent->children.end()) {
          parent->children[node] = std::make_unique<Node>( node, handler );
        }
        parent = parent->children[node].get();
      }
    }

    unsigned short compile_tree(Node *n) {
      unsigned short nodeLength = 6 + n->name.length();
      for (auto&& c : n->children) {
        nodeLength += compile_tree(c.second.get());
      }

      unsigned short nodeNameLength = n->name.length();

      std::string compiledNode;
      compiledNode.append((char *) &nodeLength, sizeof(nodeLength));
      compiledNode.append((char *) &nodeNameLength, sizeof(nodeNameLength));
      compiledNode.append((char *) &n->handler, sizeof(n->handler));
      compiledNode.append(n->name.data(), n->name.length());

      compiled_tree = compiledNode + compiled_tree;
      return nodeLength;
    }

    inline const char *find_node(const char *parent_node, const char *name,
        int name_length, std::vector<string_view>& params) {
      unsigned short nodeLength = *(unsigned short *) &parent_node[0];
      unsigned short nodeNameLength = *(unsigned short *) &parent_node[2];

      //std::cout << "Finding node: <" << std::string(name, name_length) << ">" << std::endl;

      const char *stoppp = parent_node + nodeLength;
      for (const char *candidate = parent_node + 6 + nodeNameLength; candidate < stoppp; ) {

        nodeLength = *(unsigned short *) &candidate[0];
        nodeNameLength = *(unsigned short *) &candidate[2];

        // whildcard, parameter, equal
        if (nodeNameLength == 0) {
          return candidate;
        } else if (candidate[6] == ':') {
          // parameter

          // todo: push this pointer on the stack of args!
          params.push_back(string_view({name, static_cast<unsigned int>(name_length)}));

          return candidate;
        } else if (nodeNameLength == name_length && !memcmp(candidate + 6, name, name_length)) {
          return candidate;
        }

        candidate = candidate + nodeLength;
      }

      return nullptr;
    }

    // returns next slash from start or end
    inline const char *getNextSegment(const char *start, const char *end) {
      const char *stop = (const char *) memchr(start, '/', end - start);
      return stop ? stop : end;
    }

    // should take method also!
    inline int lookup(const char *url, int length, std::vector<string_view>& params) {
      // all urls start with /
      url++;
      length--;

      const char *treeStart = (char *) compiled_tree.data();

      const char *stop, *start = url, *end_ptr = url + length;
      do {
        stop = getNextSegment(start, end_ptr);

        //std::cout << "Matching(" << std::string(start, stop - start) << ")" << std::endl;

        if(nullptr == (treeStart = find_node(treeStart, start, stop - start, params))) {
          return -1;
        }

        start = stop + 1;
      } while (stop != end_ptr);

      return *(short *) &treeStart[4];
    }

    void compile() {
      compiled_tree.clear();
      compile_tree(tree.get());
    }

  public:
    HttpRouter() = default;

    void add(const char *pattern, Handler&& handler) {

      // step over any initial slash
      if (pattern[0] == '/') {
        pattern++;
      }

      std::vector<std::string> nodes;
      //nodes.push_back(method);

      const char *stop, *start = pattern, *end_ptr = pattern + strlen(pattern);
      do {
        stop = getNextSegment(start, end_ptr);

        //std::cout << "Segment(" << std::string(start, stop - start) << ")" << std::endl;

        nodes.push_back(std::string(start, stop - start));

        start = stop + 1;
      } while (stop != end_ptr);


      // if pattern starts with / then move 1+ and run inline slash parser

      add(nodes, handlers.size());
      handlers.push_back(handler);

      compile();
    }

    std::optional<Result> route(const char *url, unsigned int url_length, UserData userData) {
      std::vector<string_view> params;
      auto idx = lookup(url, url_length, params);
      if ( idx < 0 ) return std::nullopt;
      return handlers[idx](userData, std::move(params));
    }
  };
}