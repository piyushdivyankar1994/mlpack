/**
 * @file sort_policy_test.cpp
 * @author Ryan Curtin
 *
 * Tests for each of the implementations of the SortPolicy class.
 */
#include <mlpack/core.hpp>
#include <mlpack/core/tree/binary_space_tree.hpp>

// Classes to test.
#include <mlpack/methods/neighbor_search/sort_policies/nearest_neighbor_sort.hpp>
#include <mlpack/methods/neighbor_search/sort_policies/furthest_neighbor_sort.hpp>

#include <boost/test/unit_test.hpp>
#include "test_tools.hpp"

using namespace mlpack;
using namespace mlpack::neighbor;
using namespace mlpack::bound;
using namespace mlpack::tree;
using namespace mlpack::metric;

BOOST_AUTO_TEST_SUITE(SortPolicyTest);

// Tests for NearestNeighborSort

/**
 * Ensure the best distance for nearest neighbors is 0.
 */
BOOST_AUTO_TEST_CASE(NnsBestDistance)
{
  BOOST_REQUIRE(NearestNeighborSort::BestDistance() == 0);
}

/**
 * Ensure the worst distance for nearest neighbors is DBL_MAX.
 */
BOOST_AUTO_TEST_CASE(NnsWorstDistance)
{
  BOOST_REQUIRE(NearestNeighborSort::WorstDistance() == DBL_MAX);
}

/**
 * Make sure the comparison works for values strictly less than the reference.
 */
BOOST_AUTO_TEST_CASE(NnsIsBetterStrict)
{
  BOOST_REQUIRE(NearestNeighborSort::IsBetter(5.0, 6.0) == true);
}

/**
 * Warn in case the comparison is not strict.
 */
BOOST_AUTO_TEST_CASE(NnsIsBetterNotStrict)
{
  BOOST_WARN(NearestNeighborSort::IsBetter(6.0, 6.0) == true);
}

/**
 * A simple test case of where to insert when all the values in the list are
 * DBL_MAX.
 */
BOOST_AUTO_TEST_CASE(NnsSortDistanceAllDblMax)
{
  arma::vec list(5);
  list.fill(DBL_MAX);
  arma::Col<size_t> indices(5);
  indices.fill(0);

  // Should be inserted at the head of the list.
  BOOST_REQUIRE(NearestNeighborSort::SortDistance(list, indices, 5.0) == 0);
}

/**
 * Another test case, where we are just putting the new value in the middle of
 * the list.
 */
BOOST_AUTO_TEST_CASE(NnsSortDistance2)
{
  arma::vec list(3);
  list[0] = 0.66;
  list[1] = 0.89;
  list[2] = 1.14;
  arma::Col<size_t> indices(3);
  indices.fill(0);

  // Run a couple possibilities through.
  BOOST_REQUIRE(NearestNeighborSort::SortDistance(list, indices, 0.61) == 0);
  BOOST_REQUIRE(NearestNeighborSort::SortDistance(list, indices, 0.76) == 1);
  BOOST_REQUIRE(NearestNeighborSort::SortDistance(list, indices, 0.99) == 2);
  BOOST_REQUIRE(NearestNeighborSort::SortDistance(list, indices, 1.22) ==
      (size_t() - 1));
}

/**
 * Very simple sanity check to ensure that bounds are working alright.  We will
 * use a one-dimensional bound for simplicity.
 */
BOOST_AUTO_TEST_CASE(NnsNodeToNodeDistance)
{
  // Well, there's no easy way to make HRectBounds the way we want, so we have
  // to make them and then expand the region to include new points.
  arma::mat dataset("1");
  typedef KDTree<EuclideanDistance, EmptyStatistic, arma::mat> TreeType;
  TreeType nodeOne(dataset);
  arma::vec utility(1);
  utility[0] = 0;

  nodeOne.Bound() = HRectBound<EuclideanDistance>(1);
  nodeOne.Bound() |= utility;
  utility[0] = 1;
  nodeOne.Bound() |= utility;

  TreeType nodeTwo(dataset);
  nodeTwo.Bound() = HRectBound<EuclideanDistance>(1);

  utility[0] = 5;
  nodeTwo.Bound() |= utility;
  utility[0] = 6;
  nodeTwo.Bound() |= utility;

  // This should use the L2 distance.
  BOOST_REQUIRE_CLOSE(NearestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 4.0, 1e-5);

  // And another just to be sure, from the other side.
  nodeTwo.Bound().Clear();
  utility[0] = -2;
  nodeTwo.Bound() |= utility;
  utility[0] = -1;
  nodeTwo.Bound() |= utility;

  // Again, the distance is the L2 distance.
  BOOST_REQUIRE_CLOSE(NearestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 1.0, 1e-5);

  // Now, when the bounds overlap.
  nodeTwo.Bound().Clear();
  utility[0] = -0.5;
  nodeTwo.Bound() |= utility;
  utility[0] = 0.5;
  nodeTwo.Bound() |= utility;

  BOOST_REQUIRE_SMALL(NearestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 1e-5);
}

/**
 * Another very simple sanity check for the point-to-node case, again in one
 * dimension.
 */
BOOST_AUTO_TEST_CASE(NnsPointToNodeDistance)
{
  // Well, there's no easy way to make HRectBounds the way we want, so we have
  // to make them and then expand the region to include new points.
  arma::vec utility(1);
  utility[0] = 0;

  arma::mat dataset("1");
  typedef KDTree<EuclideanDistance, EmptyStatistic, arma::mat> TreeType;
  TreeType node(dataset);
  node.Bound() = HRectBound<EuclideanDistance>(1);
  node.Bound() |= utility;
  utility[0] = 1;
  node.Bound() |= utility;

  arma::vec point(1);
  point[0] = -0.5;

  // The distance is the L2 distance.
  BOOST_REQUIRE_CLOSE(NearestNeighborSort::BestPointToNodeDistance(point,
      &node), 0.5, 1e-5);

  // Now from the other side of the bound.
  point[0] = 1.5;

  BOOST_REQUIRE_CLOSE(NearestNeighborSort::BestPointToNodeDistance(point,
      &node), 0.5, 1e-5);

  // And now when the point is inside the bound.
  point[0] = 0.5;

  BOOST_REQUIRE_SMALL(NearestNeighborSort::BestPointToNodeDistance(point,
      &node), 1e-5);
}

// Tests for FurthestNeighborSort

/**
 * Ensure the best distance for furthest neighbors is DBL_MAX.
 */
BOOST_AUTO_TEST_CASE(FnsBestDistance)
{
  BOOST_REQUIRE(FurthestNeighborSort::BestDistance() == DBL_MAX);
}

/**
 * Ensure the worst distance for furthest neighbors is 0.
 */
BOOST_AUTO_TEST_CASE(FnsWorstDistance)
{
  BOOST_REQUIRE(FurthestNeighborSort::WorstDistance() == 0);
}

/**
 * Make sure the comparison works for values strictly less than the reference.
 */
BOOST_AUTO_TEST_CASE(FnsIsBetterStrict)
{
  BOOST_REQUIRE(FurthestNeighborSort::IsBetter(5.0, 4.0) == true);
}

/**
 * Warn in case the comparison is not strict.
 */
BOOST_AUTO_TEST_CASE(FnsIsBetterNotStrict)
{
  BOOST_WARN(FurthestNeighborSort::IsBetter(6.0, 6.0) == true);
}

/**
 * A simple test case of where to insert when all the values in the list are
 * 0.
 */
BOOST_AUTO_TEST_CASE(FnsSortDistanceAllZero)
{
  arma::vec list(5);
  list.fill(0);
  arma::Col<size_t> indices(5);
  indices.fill(0);

  // Should be inserted at the head of the list.
  BOOST_REQUIRE(FurthestNeighborSort::SortDistance(list, indices, 5.0) == 0);
}

/**
 * Another test case, where we are just putting the new value in the middle of
 * the list.
 */
BOOST_AUTO_TEST_CASE(FnsSortDistance2)
{
  arma::vec list(3);
  list[0] = 1.14;
  list[1] = 0.89;
  list[2] = 0.66;
  arma::Col<size_t> indices(3);
  indices.fill(0);

  // Run a couple possibilities through.
  BOOST_REQUIRE(FurthestNeighborSort::SortDistance(list, indices, 1.22) == 0);
  BOOST_REQUIRE(FurthestNeighborSort::SortDistance(list, indices, 0.93) == 1);
  BOOST_REQUIRE(FurthestNeighborSort::SortDistance(list, indices, 0.68) == 2);
  BOOST_REQUIRE(FurthestNeighborSort::SortDistance(list, indices, 0.62) ==
      (size_t() - 1));
}

/**
 * Very simple sanity check to ensure that bounds are working alright.  We will
 * use a one-dimensional bound for simplicity.
 */
BOOST_AUTO_TEST_CASE(FnsNodeToNodeDistance)
{
  // Well, there's no easy way to make HRectBounds the way we want, so we have
  // to make them and then expand the region to include new points.
  arma::vec utility(1);
  utility[0] = 0;

  arma::mat dataset("1");
  typedef KDTree<EuclideanDistance, EmptyStatistic, arma::mat> TreeType;
  TreeType nodeOne(dataset);
  nodeOne.Bound() = HRectBound<EuclideanDistance>(1);
  nodeOne.Bound() |= utility;
  utility[0] = 1;
  nodeOne.Bound() |= utility;

  TreeType nodeTwo(dataset);
  nodeTwo.Bound() = HRectBound<EuclideanDistance>(1);
  utility[0] = 5;
  nodeTwo.Bound() |= utility;
  utility[0] = 6;
  nodeTwo.Bound() |= utility;

  // This should use the L2 distance.
  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 6.0, 1e-5);

  // And another just to be sure, from the other side.
  nodeTwo.Bound().Clear();
  utility[0] = -2;
  nodeTwo.Bound() |= utility;
  utility[0] = -1;
  nodeTwo.Bound() |= utility;

  // Again, the distance is the L2 distance.
  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 3.0, 1e-5);

  // Now, when the bounds overlap.
  nodeTwo.Bound().Clear();
  utility[0] = -0.5;
  nodeTwo.Bound() |= utility;
  utility[0] = 0.5;
  nodeTwo.Bound() |= utility;

  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestNodeToNodeDistance(&nodeOne,
      &nodeTwo), 1.5, 1e-5);
}

/**
 * Another very simple sanity check for the point-to-node case, again in one
 * dimension.
 */
BOOST_AUTO_TEST_CASE(FnsPointToNodeDistance)
{
  // Well, there's no easy way to make HRectBounds the way we want, so we have
  // to make them and then expand the region to include new points.
  arma::vec utility(1);
  utility[0] = 0;

  arma::mat dataset("1");
  typedef KDTree<EuclideanDistance, EmptyStatistic, arma::mat> TreeType;
  TreeType node(dataset);
  node.Bound() = HRectBound<EuclideanDistance>(1);
  node.Bound() |= utility;
  utility[0] = 1;
  node.Bound() |= utility;

  arma::vec point(1);
  point[0] = -0.5;

  // The distance is the L2 distance.
  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestPointToNodeDistance(point,
      &node), 1.5, 1e-5);

  // Now from the other side of the bound.
  point[0] = 1.5;

  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestPointToNodeDistance(point,
      &node), 1.5, 1e-5);

  // And now when the point is inside the bound.
  point[0] = 0.5;

  BOOST_REQUIRE_CLOSE(FurthestNeighborSort::BestPointToNodeDistance(point,
      &node), 0.5, 1e-5);
}

BOOST_AUTO_TEST_SUITE_END();
