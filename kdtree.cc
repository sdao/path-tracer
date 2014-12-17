#include "kdtree.h"

kdtree::kdtree() : root(), objs() {}

void kdtree::build() {
  root = std::make_shared<kdnode>();

  // Build kd-tree for accelerator (p. 232).
  int maxDepth = int(roundf(8.0f + 1.3f * floorf(math::log2(objs.size()))));

  // Compute bounds for kd-tree construction (Pharr & Humphreys p. 232).
  bounds = bbox();
  std::vector<bbox> allObjBounds(objs.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    allObjBounds[i] = objs[i]->bounds();
    allObjBounds[i].expand(0.1f); // Avoid pathological "flat" bboxes.
    bounds.expand(allObjBounds[i]);
  }

  // Allocate working memory for kd-tree construction (p. 236).
  std::vector<bboxedge> workEdgesRaw[3];
  std::vector<bboxedge>::iterator workEdges[3];
  for (size_t i = 0; i < 3; ++i) {
    workEdgesRaw[i] = std::vector<bboxedge>(2 * objs.size());
    workEdges[i] = workEdgesRaw[i].begin();
  }
  std::vector<id> workObjs0(objs.size());
  std::vector<id> workObjs1(size_t(maxDepth + 1) * objs.size());

  // Initialize `objIds` for kd-tree construction (p. 232).
  std::vector<id> objIds(objs.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    objIds[i] = i;
  }

  // Start recursive construction of kd-tree (p. 233).
  buildTree(
    root,
    bounds,
    allObjBounds,
    objIds.begin(),
    long(objs.size()),
    maxDepth,
    workEdges,
    workObjs0.begin(),
    workObjs1.begin(),
    0
  );
}

void kdtree::buildTree(
  std::shared_ptr<kdnode> node,
  const bbox& nodeBounds,
  const std::vector<bbox>& allObjBounds,
  iditer nodeObjIds,
  long nodeObjCount,
  int depth,
  std::vector<bboxedge>::iterator workEdges[],
  iditer workObjs0,
  iditer workObjs1,
  int badRefinesSoFar
) {
  // Initialize leaf node at `node` if termination criteria met (p. 233).
  // ====================================================================
  if (nodeObjCount <= MAX_LEAF_OBJS || depth == 0) {
    node->makeLeaf(nodeObjIds, nodeObjCount);
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
  vec d = nodeBounds.max - nodeBounds.min;

  // Choose which axis to split along (p. 236).
  axis ax = nodeBounds.maximumExtent();

  int retries = 0;

retrySplit:
  // Initialize edges for axis (p. 236).
  for (long i = 0; i < nodeObjCount; ++i) {
    id j = nodeObjIds[i];
    const bbox& jBounds = allObjBounds[j];
    workEdges[ax][2 * i]     = bboxedge(j, jBounds.min[ax], true);
    workEdges[ax][2 * i + 1] = bboxedge(j, jBounds.max[ax], false);
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
    if (edgePos > nodeBounds.min[ax] && edgePos < nodeBounds.max[ax]) {
      // Compute cost for split at `i`th edge (p. 238).
      axis otherAx0 = math::axisFromInt((ax + 1) % 3);
      axis otherAx1 = math::axisFromInt((ax + 2) % 3);
      float belowSA = 2.0f * (d[otherAx0] * d[otherAx1]
        + (edgePos - nodeBounds.min[ax]) * (d[otherAx0] + d[otherAx1]));
      float aboveSA = 2.0f * (d[otherAx0] * d[otherAx1]
        + (nodeBounds.max[ax] - edgePos) * (d[otherAx0] + d[otherAx1]));
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
    node->makeLeaf(nodeObjIds, nodeObjCount);
    return;
  }

  // Classify primitives with respect to split (p. 239).
  // ---------------------------------------------------
  long n0 = 0;
  long n1 = 0;
  for (long i = 0; i < bestOffset; ++i) {
    if (workEdges[bestAxis][i].starting) {
      workObjs0[n0++] = id(workEdges[bestAxis][i].objId);
    }
  }
  for (long i = bestOffset + 1; i < 2 * nodeObjCount; ++i) {
    if (!workEdges[bestAxis][i].starting) {
      workObjs1[n1++] = id(workEdges[bestAxis][i].objId);
    }
  }

  // Recursively initialize children nodes (p. 240).
  // -----------------------------------------------
  float splitPos = workEdges[bestAxis][bestOffset].pos;
  bbox bounds0 = nodeBounds;
  bbox bounds1 = nodeBounds;
  bounds0.max[bestAxis] = bounds1.min[bestAxis] = splitPos;

  node->makeInterior(bestAxis, splitPos);

  buildTree(
    node->below,
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
    node->above,
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

geomptr kdtree::intersect(
  const ray& r,
  intersection* isect_out
) const {
  // Compute initial parametric range of ray inside kd-tree extent (p. 240).
  float tmin, tmax;
  if (!bounds.intersect(r, &tmin, &tmax)) {
    return nullptr;
  }

  // Prepare to traverse kd-tree for ray (p. 241).
  vec invDir(
    1.0f / r.direction.x,
    1.0f / r.direction.y,
    1.0f / r.direction.z
  );
  #define MAX_TODO 64 // Pharr & Humphreys says this is enough in practice.
  kdtodo todo[MAX_TODO];
  int todoPos = 0;

  // Traverse kd-tree nodes in order for ray (p. 242).
  std::shared_ptr<kdnode> node = root;
  intersection winnerIsect;
  geomptr winnerObj = nullptr;
  while (node) {
    // Bail out if we found a hit closer than the curent node (p. 242).
    if (winnerIsect.distance < tmin) break;

    if (!node->isLeaf()) {
      // Process kd-tree interior node (p. 242).
      // ---------------------------------------

      // Compute parametric distance along ray to split plane.
      axis ax = node->splitAxis;
      float tplane = (node->splitPos - r.origin[ax]) * invDir[ax];

      // Get node children pointers for ray.
      std::shared_ptr<kdnode> firstChild;
      std::shared_ptr<kdnode> secondChild;
      bool belowFirst = (r.origin[ax] < node->splitPos) ||
                        (math::unsafeEquals(r.origin[ax], node->splitPos)
                         && r.direction[ax] <= 0);
      if (belowFirst) {
        firstChild = node->below;
        secondChild = node->above;
      } else {
        firstChild = node->above;
        secondChild = node->below;
      }

      // Advance to next child node, possibly enqueue other child (p. 244).
      if (tplane > tmax || tplane <= 0) {
        node = firstChild;
      } else if (tplane < tmin) {
        node = secondChild;
      } else {
        // Enqueue secondChild in todo list (p. 244).
        todo[todoPos].node = secondChild;
        todo[todoPos].tmin = tplane;
        todo[todoPos].tmax = tmax;
        ++todoPos;

        node = firstChild;
        tmax = tplane;
      }
    } else  {
      // Check for intersections inside leaf node (p. 244).
      for (id objId : node->objIds) {
        const geomptr obj = objs[objId];

        // Check one primitive inside leaf node (p. 244).
        const intersection isect = obj->intersect(r);

        if (isect.hit()
          && (!winnerIsect.hit() || isect.distance < winnerIsect.distance)) {
          winnerIsect = isect;
          winnerObj = obj;
        }
      }

      // Grab next node to process from todo list (p. 245).
      if (todoPos > 0) {
        --todoPos;
        node = todo[todoPos].node;
        tmin = todo[todoPos].tmin;
        tmax = todo[todoPos].tmax;
      } else {
        break;
      }
    }
  }

  if (isect_out) {
    *isect_out = winnerIsect;
  }

  return winnerObj;
}
