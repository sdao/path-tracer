#pragma once
#include <iostream>
#include "math.h"
#include "geom.h"

#define MAX_LEAF_OBJS 1
#define ISECT_COST 80.0f
#define TRAVERSAL_COST 1.0f
#define EMPTY_BONUS 0.2f

class kdtree {
  typedef size_t id;
  typedef std::vector<id>::iterator iditer;

  struct kdnode {
    kdnode* above;
    kdnode* below;
    axis splitAxis;
    float splitPos;

    std::vector<id> objIds;

    kdnode() : above(nullptr), below(nullptr), objIds() {}

    ~kdnode() {
      if (above) {
        delete above;
      }
      if (below) {
        delete below;
      }
    }

    inline void makeLeaf(iditer ids, long count) {
      for (long i = 0; i < count; ++i) {
        objIds.push_back(ids[i]);
      }
    }

    inline void makeInterior(axis ax, float pos) {
      splitAxis = ax;
      splitPos = pos;
      below = new kdnode();
      above = new kdnode();
    }

    inline bool isLeaf() const {
      return (!above && !below);
    }

    void print(std::ostream& os, std::string header = "") const {
      if (!isLeaf()) {
        os << header << "interior ";
      } else {
        os << header <<
        "leaf: ";
        if (objIds.size() > 0) {
          for (auto& i : objIds) {
            os << i << " ";
          }
        } else {
          os << "[empty] ";
        }
      }

      os << "(" << splitPos;
      if (splitAxis == X_AXIS)
        os << "x";
        else if (splitAxis == Y_AXIS)
        os << "y";
        else if (splitAxis == Z_AXIS)
        os << "z";
        else
          os << "?";

      os << ") {\n";
      if (below) {
        below->print(os, header + "  ");
      } else {
        os << header << "  [none below]";
      }

      os << "\n";
      if (above) {
        above->print(os, header + "  ");
      } else {
        os << header << "  [none above]";
      }

      os << "\n" << header << "}";
    }

    friend std::ostream& operator<<(std::ostream& os, const kdnode& n) {
      n.print(os);
      return os;
    }
  };

  struct kdtodo {
    const kdnode* node;
    float tmin;
    float tmax;
  };

  struct bboxedge {
    id objId;
    float pos;
    bool starting;

    bboxedge()
      : objId(std::numeric_limits<id>::max()), pos(0.0f), starting(false) {}

    bboxedge(id o, float p,  bool start)
      : objId(o), pos(p), starting(start) {}

    bool operator<(const bboxedge &e) const {
      if (math::unsafeEquals(pos, e.pos)) {
        return int(starting) < int(e.starting);
      } else {
        return pos < e.pos;
      }
    }
  };

  kdnode* root;
  bbox bounds;

  void buildTree(
    kdnode* node,
    const bbox& nodeBounds,
    const std::vector<bbox>& allObjBounds,
    iditer nodeObjIds,
    long nodeObjCount,
    int depth,
    std::vector<bboxedge>::iterator workEdges[],
    iditer workObjs0,
    iditer workObjs1,
    int badRefinesSoFar
  );

public:
  std::vector<geomptr> objs;

  kdtree();
  ~kdtree();
  void build();
  geomptr intersect(const ray& r, intersection* isect_out = nullptr) const;

  friend std::ostream& operator<<(std::ostream& os, const kdtree& tree) {
    os << *(tree.root);
    return os;
  }
};
