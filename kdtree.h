#pragma once
#include <iostream>
#include "core.h"
#include "geom.h"
#include "mem.h"

/**
 * A k-d tree used to accelerate ray-object intersections.
 */
class kdtree {
  /**
   * The number of objects in a tree leaf at which point the leaf should no
   * longer be refined. (There can be leafs larger than this, however.)
   */
  static constexpr int MAX_LEAF_OBJS = 1;
  /**
   * The estimated cost of an intersection.
   */
  static constexpr float ISECT_COST = 80.0f;
  /**
   * The estimated cost of traversing a level in the k-d tree.
   */
  static constexpr float TRAVERSAL_COST = 1.0f;
  /**
   * The estimated gain for having an empty leaf.
   */
  static constexpr float EMPTY_BONUS = 0.2f;
  /**
   * The maximum number of nodes queued to be checked.
   * Pharr and Humphreys' Physically Based Rendering says that 64 is enough in
   * practice.
   */
  static constexpr int MAX_TODO = 64;

  /**
   * A node in the k-d tree.
   */
  struct kdnode {
    mem::id below; /**< The index of the child "below" the split. */
    axis splitAxis; /**< The axis that the split plane cleaves. */
    float splitPos; /**< The point at which the split axis is cleaved. */
    std::vector<mem::id> objIds; /**< The indices of the objects at the leaf. */

    /** Constructs an empty kdnode. */
    kdnode() : below(),
               splitAxis(INVALID_AXIS),
               splitPos(0.0f),
               objIds() {}

    /**
     * Makes the kdnode into a leaf.
     *
     * @param ids   the indexes of objects in the leaf
     * @param count the number of objects in the leaf
     */
    inline void makeLeaf(const std::vector<mem::id>::iterator ids, long count) {
      for (long i = 0; i < count; ++i) {
        objIds.push_back(ids[i]);
      }
    }

    /**
     * Makes the kdnode into an interior node.
     * This will create two child nodes and add them to the node lookup table.
     * Note: any references to nodes in the tree may become invalid after
     * this function completes.
     *
     * @param ax    the axis on which to split
     * @param pos   the position at which to split along the axis
     * @param nodes the node lookup table that defines the tree structure
     */
    inline void makeInterior(axis ax, float pos, std::vector<kdnode>& nodes) {
      splitAxis = ax;
      splitPos = pos;

      below = nodes.size(); // Note: above is always (below + 1).

      // Since we change the vector that contains us in this step,
      // we no longer have safe access to our own memory. So all changes
      // must happen before modifying the vector.
      nodes.push_back(kdnode());
      nodes.push_back(kdnode());
    }

    /** Returns true if the node is a leaf, false if the node is interior. */
    inline bool isLeaf() const {
      return !below.isValid();
    }

    /** Returns the index of the child "above" the split. */
    inline mem::id aboveId() const {
      return below.offset(1);
    }

    /** Returns the index of the child "below" the split. */
    inline mem::id belowId() const {
      return below;
    }
  };

  /** Used to keep track of nodes queued to be checked. */
  struct kdtodo {
    mem::id nodeId;
    float tmin;
    float tmax;
  };

  /** Used to represent a bbox projected onto a linear axis. */
  struct bboxedge {
    mem::id objId;
    float pos;
    bool starting;

    bboxedge()
      : objId(mem::ID_INVALID), pos(0.0f), starting(false) {}

    bboxedge(mem::id o, float p,  bool start)
      : objId(o), pos(p), starting(start) {}

    bool operator<(const bboxedge &e) const {
      if (pos == e.pos) {
        return int(starting) < int(e.starting);
      } else {
        return pos < e.pos;
      }
    }
  };

  std::vector<kdnode> allNodes; /**< The node lookup table. */
  mem::id rootId; /**< The index of the root node (usually 0). */
  bbox bounds; /**< The bounds encapsulating all the objects in the tree. */

  /**
   * Recursively constructs the k-d tree.
   *
   * @param nodeId          the root node of the subtree to construct
   * @param nodeBounds      the bounds contained by this subtree
   * @param allObjBounds    the bounds of all objects in the k-d tree
   * @param nodeObjIds      the objects contained in this subtree
   * @param nodeObjCount    the number of objects contained in this subtree
   * @param depth           the number of tree levels remaining before stopping
   * @param workEdges       an array of 3 work vectors, one for each axis
   * @param workObjs0       a work vector
   * @param workObjs1       a work vector
   * @param badRefinesSoFar the number of bad refines in the current branch
   */
  void buildTree(
    mem::id nodeId,
    const bbox& nodeBounds,
    const std::vector<bbox>& allObjBounds,
    const std::vector<mem::id>::iterator nodeObjIds,
    long nodeObjCount,
    int depth,
    std::vector<bboxedge>::iterator workEdges[],
    std::vector<mem::id>::iterator workObjs0,
    std::vector<mem::id>::iterator workObjs1,
    int badRefinesSoFar
  );

protected:
  /**
   * Prints the subtree beginning at the given node.
   *
   * @param nodeId the node at the root of the subtree
   * @param os     the output stream to print to
   * @param header whitespace used to indent the printed output
   */
  void print(mem::id nodeId, std::ostream& os, std::string header = "") const;

public:
  std::vector<geom*>* objs; /**< The geometric objects in the k-d tree. */

  /**
   * Constructs an empty kdtree associated with the given objects,
   * but does not actually build out the k-d tree structure.
   * Use the kdtree::build() method to complete building.
   */
  kdtree(std::vector<geom*>* o);

  /**
   * Actually builds out the k-d tree structure.
   */
  void build();

  /**
   * Determines what object (if any) in the k-d tree a given ray intersects.
   *
   * @param r              the ray to send through the k-d tree
   * @param isectOut [out] the intersection information if the ray hit some
   *                       geometry, otherwise unmodified; the pointer must not
   *                       be null
   * @returns              the geom that was hit, or nullptr if none was hit
   */
  geom* intersect(const ray& r, intersection* isectOut = nullptr) const;

  friend std::ostream& operator<<(std::ostream& os, const kdtree& tree) {
    tree.print(tree.rootId, os);
    return os;
  }
};
