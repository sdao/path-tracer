#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "core.h"
#include "geom.h"
#include "mem.h"

/**
 * A k-d tree used to accelerate ray-object intersections.
 * This implementation is mostly taken from Pharr and Humphreys' Physically
 * Based Rendering, second edition.
 */
class KDTree {
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
  struct KDNode {
    mem::ID below; /**< The index of the child "below" the split. */
    axis splitAxis; /**< The axis that the split plane cleaves. */
    float splitPos; /**< The point at which the split axis is cleaved. */
    std::vector<mem::ID> objIds; /**< The indices of the objects at the leaf. */

    /** Constructs an empty kdnode. */
    KDNode() : below(),
               splitAxis(INVALID_AXIS),
               splitPos(0.0f),
               objIds() {}

    /**
     * Makes the kdnode into a leaf.
     *
     * @param ids   the indexes of objects in the leaf
     * @param count the number of objects in the leaf
     */
    inline void makeLeaf(const std::vector<mem::ID>::iterator ids, long count) {
      objIds.reserve(size_t(count));
      objIds.insert(objIds.end(), ids, ids + count);
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
    inline void makeInterior(axis ax, float pos, std::vector<KDNode>& nodes) {
      splitAxis = ax;
      splitPos = pos;

      below = nodes.size(); // Note: above is always (below + 1).

      // Since we change the vector that contains us in this step,
      // we no longer have safe access to our own memory. So all changes
      // must happen before modifying the vector.
      nodes.push_back(KDNode());
      nodes.push_back(KDNode());
    }

    /** Returns true if the node is a leaf, false if the node is interior. */
    inline bool isLeaf() const {
      return !below.isValid();
    }

    /** Returns the index of the child "above" the split. */
    inline mem::ID aboveId() const {
      return below.offset(1);
    }

    /** Returns the index of the child "below" the split. */
    inline mem::ID belowId() const {
      return below;
    }
  };

  /** Used to keep track of nodes queued to be checked. */
  struct KDTodo {
    mem::ID nodeId;
    float tmin;
    float tmax;
  };

  /** Used to represent a bbox projected onto a linear axis. */
  struct BBoxEdge {
    mem::ID objId;
    float pos;
    bool starting;

    BBoxEdge()
      : objId(mem::ID_INVALID), pos(0.0f), starting(false) {}

    BBoxEdge(mem::ID o, float p,  bool start)
      : objId(o), pos(p), starting(start) {}

    bool operator<(const BBoxEdge &e) const {
      if (pos == e.pos) {
        return int(starting) < int(e.starting);
      } else {
        return pos < e.pos;
      }
    }
  };

  std::vector<KDNode> allNodes; /**< The node lookup table. */
  mem::ID rootId; /**< The index of the root node (usually 0). */
  BBox bounds; /**< The bounds encapsulating all the objects in the tree. */
  std::vector<const Geom*> objs; /**< The geometric objects in the k-d tree. */
  std::vector<const Geom*> lights; /**< The geometric objects with a light. */

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
    mem::ID nodeId,
    const BBox& nodeBounds,
    const std::vector<BBox>& allObjBounds,
    const std::vector<mem::ID>::iterator nodeObjIds,
    long nodeObjCount,
    int depth,
    std::vector<BBoxEdge>::iterator workEdges[],
    std::vector<mem::ID>::iterator workObjs0,
    std::vector<mem::ID>::iterator workObjs1,
    int badRefinesSoFar
  );

protected:
  /**
   * Prints the subtree beginning at the given node.
   *
   * @param nodeId the node at the root of the subtree
   * @param os     the output stream to print to
   * @param level  the level of indentation for the output
   */
  void print(mem::ID nodeId, std::ostream& os, int level = 0) const;

public:
  /**
   * Constructs an empty kdtree associated with the given objects,
   * but does not actually build out the k-d tree structure.
   * Use the kdtree::build() method to complete building.
   */
  KDTree(const std::vector<const Geom*>& o);

  /**
   * Constructs an empty kdtree associated with the given objects,
   * but does not actually build out the k-d tree structure.
   * Use the kdtree::build() method to complete building.
   */
  KDTree(const std::map<std::string, const Geom*>& o);

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
  const Geom* intersect(const Ray& r, Intersection* isectOut) const;

  /**
   * Determines if any object in the k-d tree intersects the given shadow ray
   * within a maximum distance.
   *
   * @param r       the shadow ray to send through the k-d tree
   * @param maxDist the maximum distance to check for intersections
   * @returns       true if any geom hit within maxDist, otherwise false
   */
  bool intersectShadow(const Ray& r, float maxDist) const;

  /**
   * Returns a reference to the list of all (refined) geometry in the k-d tree.
   */
  const std::vector<const Geom*>& allObjecs() const;

  /**
   * Returns a reference to the list of all light-source geometry in the k-d
   * tree.
   */
  const std::vector<const Geom*>& allLights() const;

  friend std::ostream& operator<<(std::ostream& os, const KDTree& tree) {
    tree.print(tree.rootId, os);
    return os;
  }
};
