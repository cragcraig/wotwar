#include <stdio.h>
#include <time.h>
#include "mask.h"
#include "task.h"

template<class UserState> class AStarSearch; // forward declaration

using namespace std;

class MapSearchNode
{
public:
	int x;	 // the (x,y) positions of the node
	int y;
	int dir;

	MapSearchNode() { x = y = 0; }
	MapSearchNode( unsigned int px, unsigned int py, int direction = -1 ) { x=px; y=py; dir=direction; }

	float GoalDistanceEstimate( MapSearchNode &nodeGoal );
	bool IsGoal( MapSearchNode &nodeGoal );
	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node );
	float GetCost( MapSearchNode &successor );
	bool IsSameState( MapSearchNode &rhs );

	// static pathfinding functions
	static void setCurMap(MASK* m); // set the map mask used for searching
	static int GetMap( int x, int y ); // get map terrain value with bounds checking
	static TASK* findPath(int x, int y, int fx, int fy, MASK* m, int * error, bool skip_last=true); // find a path
};
