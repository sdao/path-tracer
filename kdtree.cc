#include "kdtree.h"

KDTree::KDTree(const std::vector<const Geom*>& o)
  : allNodes(), rootId(), objs(), lights()
{
  // Refine all geometry into primitives.
  for (const Geom* g : o) {
    g->refine(objs);
  }

  // Add all lights to the light list.
  for (const Geom* g : objs) {
    if (g->light) {
      lights.push_back(g);
    }
  }
}

KDTree::KDTree(const std::map<std::string, const Geom*>& o)
  : allNodes(), rootId(), objs(), lights()
{
  // Refine all geometry into primitives.
  for (const auto& pair : o) {
    pair.second->refine(objs);
  }

  // Add all lights to the light list.
  for (const Geom* g : objs) {
    if (g->light) {
      lights.push_back(g);
    }
  }
}

void KDTree::build() {
  rootId = 0;
  allNodes.push_back(KDNode());

  // Build kd-tree for accelerator (p. 232).
  float logOfSize = math::log2(float(objs.size()));
  int maxDepth = int(roundf(8.0f + 1.3f * floorf(logOfSize)));

  // Compute bounds for kd-tree construction (Pharr & Humphreys p. 232).
  bounds = BBox();
  std::vector<BBox> allObjBounds(objs.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    allObjBounds[i] = objs[i]->bounds();
    allObjBounds[i].expand(math::VERY_SMALL); // Avoid pathological flat bboxes.
    bounds.expand(allObjBounds[i]);
  }

  // Allocate working memory for kd-tree construction (p. 236).
  std::vector<BBoxEdge> workEdgesRaw[3];
  std::vector<BBoxEdge>::iterator workEdgesIters[3];
  for (size_t i = 0; i < 3; ++i) {
    workEdgesRaw[i] = std::vector<BBoxEdge>(2 * objs.size());
    workEdgesIters[i] = workEdgesRaw[i].begin();
  }
  std::vector<mem::ID> workObjs0(objs.size());
  std::vector<mem::ID> workObjs1(size_t(maxDepth + 1) * objs.size());

  // Initialize `objIds` for kd-tree construction (p. 232).
  std::vector<mem::ID> objIds(objs.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    objIds[i] = i;
  }

  // Start recursive construction of kd-tree (p. 233).
  buildTree(
    rootId,
    bounds,
    allObjBounds,
    objIds.begin(),
    long(objs.size()),
    maxDepth,
    workEdgesIters,
    workObjs0.begin(),
    workObjs1.begin(),
    0
  );
}

void KDTree::buildTree(
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
) {
  // Initialize leaf node at `node` if termination criteria met (p. 233).
  // ====================================================================
  if (nodeObjCount <= MAX_LEAF_OBJS || depth == 0) {
    mem::ref(allNodes, nodeId).makeLeaf(nodeObjIds, nodeObjCount);
    return;
  }

  // Intialize interior node and continue recursion (full index on p. 234).
  // ======================================================================

  // Choose split axis position for interior node (p. 236).
  // ------------------------------------------------------
  axis bestAxis = INVALID_AXIS;
  long bestOffset = -1;
  float bestCost = std::numeric_limits<float>::max();
  float oldCost = ISECT_COST * float(nodeObjCount);
  float totalSA = nodeBounds.surfaceArea();
  float invTotalSA = 1.0f / totalSA;
  Vec d = nodeBounds.upper - nodeBounds.lower;

  // Choose which axis to split along (p. 236).
  axis ax = nodeBounds.maximumExtent();

  int retries = 0;

retrySplit:
  // Initialize edges for axis (p. 236).
  for (long i = 0; i < nodeObjCount; ++i) {
    const mem::ID j = nodeObjIds[i];
    const BBox& jBounds = mem::refConst(allObjBounds, j);
    workEdges[ax][2 * i]     = BBoxEdge(j, jBounds.lower[ax], true);
    workEdges[ax][2 * i + 1] = BBoxEdge(j, jBounds.upper[ax], false);
  }
  std::sort(
    workEdges[ax],
    workEdges[ax] + 2 * nodeObjCount
  );

  // Compute cost of all splits for axis to find best (p. 237).
  int nBelow = 0; // Objects below the current split.
  int nAbove = int(nodeObjCount); // Objects above the current split.
  for (long i = 0; i < 2 * nodeObjCount; ++i) {
    if (!workEdges[ax][i].starting) {
      // Passed the end of an object; remove it from the "above" count.
      --nAbove;
    }

    float edgePos = workEdges[ax][i].pos;
    if (edgePos > nodeBounds.lower[ax] && edgePos < nodeBounds.upper[ax]) {
      // Compute cost for split at `i`th edge (p. 238).
      axis otherAx0 = math::axisFromInt((ax + 1) % 3);
      axis otherAx1 = math::axisFromInt((ax + 2) % 3);
      float belowSA = 2.0f * (d[otherAx0] * d[otherAx1]
        + (edgePos - nodeBounds.lower[ax]) * (d[otherAx0] + d[otherAx1]));
      float aboveSA = 2.0f * (d[otherAx0] * d[otherAx1]
        + (nodeBounds.upper[ax] - edgePos) * (d[otherAx0] + d[otherAx1]));
      float probBelow = belowSA * invTotalSA;
      float probAbove = aboveSA * invTotalSA;
      float eb = (nAbove == 0 || nBelow == 0) ? EMPTY_BONUS : 0.0f;
      float cost = TRAVERSAL_COST
        + ISECT_COST
        * (1.0f - eb) * (probBelow * nBelow + probAbove * nAbove);

      // Update best split if this is lowest cost so far (p. 238).
      if (cost < bestCost) {
        bestCost = cost;
        bestAxis = ax;
        bestOffset = i;
      }
    }

    if (workEdges[ax][i].starting) {
      // Passed the start of an object; add it to the "below" count.
      ++nBelow;
    }
  }

  // Create leaf if no good splits were found (p. 239).
  // --------------------------------------------------
  if (bestAxis == INVALID_AXIS && retries < 2) {
    ++retries;
    ax = math::axisFromInt((ax + 1) % 3);
    goto retrySplit;
  }
  if (bestCost > oldCost) {
    ++badRefinesSoFar;
  }
  if ((bestCost > 4.0f * oldCost && nodeObjCount < 16)
      || bestAxis == INVALID_AXIS || badRefinesSoFar == 3) {
    mem::ref(allNodes, nodeId).makeLeaf(nodeObjIds, nodeObjCount);
    return;
  }

  // Classify primitives with respect to split (p. 239).
  // ---------------------------------------------------
  long n0 = 0;
  long n1 = 0;
  for (long i = 0; i < bestOffset; ++i) {
    if (workEdges[bestAxis][i].starting) {
      workObjs0[n0++] = workEdges[bestAxis][i].objId;
    }
  }
  for (long i = bestOffset + 1; i < 2 * nodeObjCount; ++i) {
    if (!workEdges[bestAxis][i].starting) {
      workObjs1[n1++] = workEdges[bestAxis][i].objId;
    }
  }

  // Recursively initialize children nodes (p. 240).
  // -----------------------------------------------
  float splitPos = workEdges[bestAxis][bestOffset].pos;
  BBox bounds0 = nodeBounds;
  BBox bounds1 = nodeBounds;
  bounds0.upper[bestAxis] = bounds1.lower[bestAxis] = splitPos;

  mem::ref(allNodes, nodeId).makeInterior(bestAxis, splitPos, allNodes);

  buildTree(
    mem::ref(allNodes, nodeId).belowId(),
    bounds0,
    allObjBounds,
    workObjs0,
    n0,
    depth - 1,
    workEdges,
    workObjs0,
    workObjs1 + long(nodeObjCount),
    badRefinesSoFar
  );

  buildTree(
    mem::ref(allNodes, nodeId).aboveId(),
    bounds1,
    allObjBounds,
    workObjs1,
    n1,
    depth - 1,
    workEdges,
    workObjs0,
    workObjs1 + long(nodeObjCount),
    badRefinesSoFar
  );
}

const Geom* KDTree::intersect(
  const Ray& r,
  Intersection* isectOut
) const {
  // Compute initial parametric range of ray inside kd-tree extent (p. 240).
  float tmin, tmax;
  if (!bounds.intersect(r, &tmin, &tmax)) {
    return nullptr;
  }

  // Prepare to traverse kd-tree for ray (p. 241).
  Vec invDir(
    1.0f / r.direction.x(),
    1.0f / r.direction.y(),
    1.0f / r.direction.z()
  );
  KDTodo todo[MAX_TODO];
  int todoPos = 0;

  // Traverse kd-tree nodes in order for ray (p. 242).
  mem::ID nodeId = rootId;

  Intersection winnerIsect; // By default, distance is set to max float value.
  const Geom* winnerObj = nullptr;
  while (nodeId.isValid()) {
    const KDNode& node = mem::refConst(allNodes, nodeId);

    // Bail out if we found a hit closer than the curent node (p. 242).
    if (winnerIsect.distance < tmin) break;

    if (!node.isLeaf()) {
      // Process kd-tree interior node (p. 242).
      // ---------------------------------------

      // Compute parametric distance along ray to split plane.
      axis ax = node.splitAxis;
      float tplane = (node.splitPos - r.origin[ax]) * invDir[ax];

      // Get node children pointers for ray.
      mem::ID firstChild;
      mem::ID secondChild;
      bool belowFirst = (r.origin[ax] < node.splitPos) ||
                        ((r.origin[ax] == node.splitPos)
                         && r.direction[ax] <= 0);
      if (belowFirst) {
        firstChild = node.belowId();
        secondChild = node.aboveId();
      } else {
        firstChild = node.aboveId();
        secondChild = node.belowId();
      }

      // Advance to next child node, possibly enqueue other child (p. 244).
      if (tplane > tmax || tplane <= 0) {
        nodeId = firstChild;
      } else if (tplane < tmin) {
        nodeId = secondChild;
      } else {
        // Enqueue secondChild in todo list (p. 244).
        todo[todoPos].nodeId = secondChild;
        todo[todoPos].tmin = tplane;
        todo[todoPos].tmax = tmax;
        ++todoPos;

        nodeId = firstChild;
        tmax = tplane;
      }
    } else  {
      // Check for intersections inside leaf node (p. 244).
      for (mem::ID objId : node.objIds) {
        const Geom* obj = mem::refConst(objs, objId);

        // Check one primitive inside leaf node (p. 244).
        Intersection isect;
        bool didHit = obj->intersect(r, &isect);

        if (didHit && isect.distance < winnerIsect.distance) {
          winnerIsect = isect;
          winnerObj = obj;
        }
      }

      // Grab next node to process from todo list (p. 245).
      if (todoPos > 0) {
        --todoPos;
        nodeId = todo[todoPos].nodeId;
        tmin = todo[todoPos].tmin;
        tmax = todo[todoPos].tmax;
      } else {
        break;
      }
    }
  }

  if (winnerObj) {
    *isectOut = winnerIsect;
    return winnerObj;
  }

  return nullptr;
}

bool KDTree::intersectShadow(const Ray& r, float maxDist) const {
  // Compute initial parametric range of ray inside kd-tree extent (p. 240).
  float tmin, tmax;
  if (!bounds.intersect(r, &tmin, &tmax)) {
    return false;
  }

  // Prepare to traverse kd-tree for ray (p. 241).
  Vec invDir(
    1.0f / r.direction.x(),
    1.0f / r.direction.y(),
    1.0f / r.direction.z()
  );
  KDTodo todo[MAX_TODO];
  int todoPos = 0;

  // Traverse kd-tree nodes in order for ray (p. 242).
  mem::ID nodeId = rootId;

  while (nodeId.isValid()) {
    const KDNode& node = mem::refConst(allNodes, nodeId);

    if (node.isLeaf()) {
      // Check for shadow ray intersections inside leaf node.
      for (mem::ID objId : node.objIds) {
        const Geom* obj = mem::refConst(objs, objId);

        if (obj->intersectShadow(r, maxDist)) {
          return true;
        }
      }

      // Grab next node to process from todo list (p. 245).
      if (todoPos > 0) {
        --todoPos;
        nodeId = todo[todoPos].nodeId;
        tmin = todo[todoPos].tmin;
        tmax = todo[todoPos].tmax;
      } else {
        break;
      }
    } else {
      // Process kd-tree interior node (p. 242).
      // ---------------------------------------

      // Compute parametric distance along ray to split plane.
      axis ax = node.splitAxis;
      float tplane = (node.splitPos - r.origin[ax]) * invDir[ax];

      // Get node children pointers for ray.
      mem::ID firstChild;
      mem::ID secondChild;
      bool belowFirst = (r.origin[ax] < node.splitPos) ||
                        ((r.origin[ax] == node.splitPos)
                         && r.direction[ax] <= 0);
      if (belowFirst) {
        firstChild = node.belowId();
        secondChild = node.aboveId();
      } else {
        firstChild = node.aboveId();
        secondChild = node.belowId();
      }

      // Advance to next child node, possibly enqueue other child (p. 244).
      if (tplane > tmax || tplane <= 0) {
        nodeId = firstChild;
      } else if (tplane < tmin) {
        nodeId = secondChild;
      } else {
        // Enqueue secondChild in todo list (p. 244).
        todo[todoPos].nodeId = secondChild;
        todo[todoPos].tmin = tplane;
        todo[todoPos].tmax = tmax;
        ++todoPos;

        nodeId = firstChild;
        tmax = tplane;
      }
    }
  }

  return false;
}

void KDTree::print(mem::ID nodeId, std::ostream& os, std::string header) const {
  const KDNode& node = mem::refConst<KDNode>(allNodes, nodeId);

  if (!node.isLeaf()) {
    os << header << "interior ";
  } else {
    os << header <<
    "leaf: ";
    if (node.objIds.size() > 0) {
      for (auto& i : node.objIds) {
        os << i.val << " ";
      }
    } else {
      os << "[empty] ";
    }
  }

  os << "(" << node.splitPos;
  if (node.splitAxis == X_AXIS) {
    os << "x";
  } else if (node.splitAxis == Y_AXIS) {
    os << "y";
  } else if (node.splitAxis == Z_AXIS) {
    os << "z";
  } else {
    os << "?";
  }

  os << ") {\n";
  if (node.belowId().isValid()) {
    print(node.belowId(), os, header + "  ");
  } else {
    os << header << "  [none below]";
  }

  os << "\n";
  if (node.aboveId().isValid()) {
    print(node.aboveId(), os, header + "  ");
  } else {
    os << header << "  [none above]";
  }

  os << "\n" << header << "}";
}

const std::vector<const Geom*>& KDTree::allObjecs() const {
  return objs;
}

const std::vector<const Geom*>& KDTree::allLights() const {
  return lights;
}
