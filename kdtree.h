#pragma once
#include <iostream>
#include "math.h"
#include "geom.h"
#include "mem.h"

class kdtree {
  static constexpr int MAX_LEAF_OBJS = 1;
  static constexpr float ISECT_COST = 80.0f;
  static constexpr float TRAVERSAL_COST = 1.0f;
  static constexpr float EMPTY_BONUS = 0.2f;
  static constexpr int MAX_TODO = 64; // PBR says this is enough in practice.

  struct kdnode {
    mem::id below;
    axis splitAxis;
    float splitPos;
    std::vector<mem::id> objIds;

    kdnode() : below(),
               splitAxis(INVALID_AXIS),
               splitPos(0.0f),
               objIds() {}

    inline void makeLeaf(std::vector<mem::id>::iterator ids, long count) {
      for (long i = 0; i < count; ++i) {
        objIds.push_back(ids[i]);
      }
    }

    inline void makeInterior(axis ax, float pos, std::vector<kdnode>& nodes) {
      splitAxis = ax;
      splitPos = pos;

      below = nodes.size();
      // above is always (below + 1).

      // Since we change the vector that contains us in this step,
      // we no longer have safe access to our own memory. So all changes
      // must happen before modifying the vector.
      nodes.push_back(kdnode());
      nodes.push_back(kdnode());
    }

    inline bool isLeaf() const {
      return !below.isValid();
    }
    
    inline mem::id aboveId() const {
      return below.offset(1);
    }
    
    inline mem::id belowId() const {
      return below;
    }
  };

  struct kdtodo {
    mem::id nodeId;
    float tmin;
    float tmax;
  };

  struct bboxedge {
    mem::id objId;
    float pos;
    bool starting;

    bboxedge()
      : objId(mem::ID_INVALID), pos(0.0f), starting(false) {}

    bboxedge(mem::id o, float p,  bool start)
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
  mem::id rootId;
  bbox bounds;

  void buildTree(
    mem::id nodeId,
    const bbox& nodeBounds,
    const std::vector<bbox>& allObjBounds,
    std::vector<mem::id>::iterator nodeObjIds,
    long nodeObjCount,
    int depth,
    std::vector<bboxedge>::iterator workEdges[],
    std::vector<mem::id>::iterator workObjs0,
    std::vector<mem::id>::iterator workObjs1,
    int badRefinesSoFar
  );

protected:
  void print(mem::id nodeId, std::ostream& os, std::string header = "") const;

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
