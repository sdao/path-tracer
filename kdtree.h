#pragma once
#include <iostream>
#include "math.h"
#include "geom.h"

class kdtree {
  typedef size_t id;
  typedef std::vector<id>::iterator iditer;
  static constexpr id ID_INVALID = std::numeric_limits<id>::max();

  static constexpr int MAX_LEAF_OBJS = 1;
  static constexpr float ISECT_COST = 80.0f;
  static constexpr float TRAVERSAL_COST = 1.0f;
  static constexpr float EMPTY_BONUS = 0.2f;
  static constexpr int MAX_TODO = 64; // PBR says this is enough in practice.

  struct kdnode {
    id above;
    id below;
    axis splitAxis;
    float splitPos;
    std::vector<id> objIds;

    kdnode() : above(ID_INVALID),
               below(ID_INVALID),
               splitAxis(INVALID_AXIS),
               splitPos(0.0f),
               objIds() {}

    inline void makeLeaf(iditer ids, long count) {
      for (long i = 0; i < count; ++i) {
        objIds.push_back(ids[i]);
      }
    }

    inline void makeInterior(axis ax, float pos, std::vector<kdnode>& nodes) {
      splitAxis = ax;
      splitPos = pos;

      below = nodes.size();
      above = nodes.size() + 1;

      // Since we change the vector that contains us in this step,
      // we no longer have safe access to our own memory. So all changes
      // must happen before modifying the vector.
      nodes.push_back(kdnode());
      nodes.push_back(kdnode());
    }

    inline bool isLeaf() const {
      return (above == ID_INVALID && below == ID_INVALID);
    }
  };

  struct kdtodo {
    id nodeId;
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

  std::vector<kdnode> allNodes;
  id rootId;
  bbox bounds;

  void buildTree(
    id nodeId,
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

protected:
  void print(id nodeId, std::ostream& os, std::string header = "") const;

public:
  std::vector<geom*>* objs;

  kdtree(std::vector<geom*>* o);
  void build();
  geom* intersect(const ray& r, intersection* isectOut = nullptr) const;

  friend std::ostream& operator<<(std::ostream& os, const kdtree& tree) {
    tree.print(tree.rootId, os);
    return os;
  }
};
