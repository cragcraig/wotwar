////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// STL A* Search implementation
// (C)2001 Justin Heyes-Jones
//
// Finding a path on a simple grid maze
// This shows how to do shortest path finding using A*
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Greatly modified by Craig Harrison for use in the 'WoT War' project
// 2007-2008 http://wotwar.googlecode.com.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "player.h"
#include "mapsearchnode.h"
#include "stlastar.h"
#include "sizes.h"

// Global data

MASK* map;

// map helper functions
void MapSearchNode::setCurMap(MASK* m) {
	map = m;
}

int MapSearchNode::GetMap( int x, int y )
{
	if(x < 0 || x >= map->w || y < 0 || y >= map->h) return MASK::BLOCK;
	return map->m[x*map->h+y];
}

bool MapSearchNode::IsSameState( MapSearchNode &rhs )
{
	// same state in a maze search is simply when (x,y) are the same
	if((x == rhs.x) && (y == rhs.y)) return true;
	else return false;
}

// Here's the heuristic function that estimates the distance from a Node
// to the Goal.

float MapSearchNode::GoalDistanceEstimate( MapSearchNode &nodeGoal )
{
	return sqrt(((float)x - (float)nodeGoal.x)*((float)x - (float)nodeGoal.x) + ((float)y - (float)nodeGoal.y)*((float)y - (float)nodeGoal.y)) + (GetMap( x, y ) >= MASK::BAD ? NEAR_STATIONARY_COST : 0);
	// returns distance to destination and adds 3 if the passibility is bad
}

bool MapSearchNode::IsGoal( MapSearchNode &nodeGoal )
{
	if( (x == nodeGoal.x) && (y == nodeGoal.y) ) return true;
	return false;
}

// This generates the successors to the given Node. It uses a helper function called
// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
// is done for each node internally, so here you just set the state information that
// is specific to the application
bool MapSearchNode::GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node )
{
	int parent_x = -1;
	int parent_y = -1;

	if (parent_node) {
		parent_x = parent_node->x;
		parent_y = parent_node->y;
	}


	MapSearchNode NewNode;

	// push each possible move except allowing the search to go backwards

	if (GetMap( x-1, y ) && !((parent_x == x-1) && (parent_y == y)))  {
		NewNode = MapSearchNode( x-1, y, 1);
		astarsearch->AddSuccessor( NewNode );
	}

	if (GetMap( x, y-1 ) && !((parent_x == x) && (parent_y == y-1))) {
		NewNode = MapSearchNode( x, y-1, 2);
		astarsearch->AddSuccessor( NewNode );
	}

	if (GetMap( x+1, y ) && !((parent_x == x+1) && (parent_y == y))) {
		NewNode = MapSearchNode( x+1, y, 3);
		astarsearch->AddSuccessor( NewNode );
	}


	if (GetMap( x, y+1 ) && !((parent_x == x) && (parent_y == y+1))) {
		NewNode = MapSearchNode( x, y+1, 4);
		astarsearch->AddSuccessor( NewNode );
	}

	// diagonals

	if (GetMap( x-1, y-1 ) && (GetMap( x-1, y ) || GetMap( x, y-1 )) && !((parent_x == x-1) && (parent_y == y-1))) {
		NewNode = MapSearchNode( x-1, y-1, 5);
		astarsearch->AddSuccessor( NewNode );
	}

	if (GetMap( x-1, y+1 ) && (GetMap( x-1, y ) || GetMap( x, y+1 )) && !((parent_x == x-1) && (parent_y == y+1))) {
		NewNode = MapSearchNode( x-1, y+1, 6);
		astarsearch->AddSuccessor( NewNode );
	}

	if (GetMap( x+1, y+1 ) && (GetMap( x+1, y ) || GetMap( x, y+1 )) && !((parent_x == x+1) && (parent_y == y+1))) {
		NewNode = MapSearchNode( x+1, y+1, 7);
		astarsearch->AddSuccessor( NewNode );
	}


	if (GetMap( x+1, y-1 ) && (GetMap( x+1, y ) || GetMap( x, y-1 )) && !((parent_x == x+1) && (parent_y == y-1))) {
		NewNode = MapSearchNode( x+1, y-1, 8);
		astarsearch->AddSuccessor( NewNode );
	}

	return true;
}

// given this node, what does it cost to move to successor. In the case
// of our map the answer is the map terrain value at this node since that is
// conceptually where we're moving

float MapSearchNode::GetCost( MapSearchNode &successor )
{
	return (float) GetMap( x, y );
}


// Main
TASK* MapSearchNode::findPath(int x, int y, int fx, int fy, MASK* m, int * error, bool skip_last)
{
	// set map
	map = m;

	// Create an instance of the search class
	AStarSearch<MapSearchNode> astarsearch(PATHFIND_MAX_SEARCH_NODES);
	TASK* r = NULL;
	TASK* cur = NULL;
	// Create a start state
	MapSearchNode nodeStart;
	nodeStart.x = x / map->areasize; // convert world coordinates to current map-mask coordinates
	nodeStart.y = y / map->areasize;

	// Define the goal state
	MapSearchNode nodeEnd;
	nodeEnd.x = fx / map->areasize;
	nodeEnd.y = fy / map->areasize;

	// Set Start and goal states
	astarsearch.SetStartAndGoalStates(nodeStart, nodeEnd);

	unsigned int SearchState, pathId, lastDir;

	do {
		SearchState = astarsearch.SearchStep();
	} while (SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING);

	if (SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED) {
		// found goal

			MapSearchNode *node;
			astarsearch.GetSolutionStart(); // throw start away
			MapSearchNode *nextnode = astarsearch.GetSolutionNext();

			// loop though creating tasks (skipping start and end locations)
			while ((node = nextnode)) {
				nextnode = astarsearch.GetSolutionNext(); // get next node
				if (skip_last && !nextnode) break; // throw away last node if skip_last flag is set
				// stuff to do for nodes
				if (!r) { // first one
					r = new TASK(TASK::MOVE, PLAYER::ANY_PLAYER, (int)((node->x + 0.5)*map->areasize), (int)((node->y + 0.5)*map->areasize), false);
					cur = r;
					pathId = r->pId;
					lastDir = -1;
				}
				else if (lastDir != node->dir) { // after first, make sure they all have the same pathId
					cur->pNext = new TASK(TASK::MOVE, PLAYER::ANY_PLAYER, (int)((node->x + 0.5)*map->areasize), (int)((node->y + 0.5)*map->areasize), false);
					cur = cur->pNext;
					cur->pId = pathId;
					lastDir = node->dir;
				}
				cur->pathfind_level = map->level;
				cur->is_pathfind = true;
			}

			// Once you're done with the solution you can free the nodes up
			astarsearch.FreeSolutionNodes();
			*error = 0;
	}
	else {
		// search failed to find goal or ran out of memory
		r = NULL;
		*error = 1;
	}

	astarsearch.EnsureMemoryFreed();

	return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
