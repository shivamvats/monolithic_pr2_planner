#pragma once
#include <bfs3d/BFS_3D.h>
#include <monolithic_pr2_planner/StateReps/DiscObjectState.h>
#include <monolithic_pr2_planner/StateReps/GraphState.h>
#include <monolithic_pr2_planner/OccupancyGridUser.h>
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace monolithic_pr2_planner {
    /*! \brief Manages heuristic computation used by the SBPL planner. Currently
     * implements a 3D breadth first search for the end effector.
     */
    class Heuristic : public OccupancyGridUser {
        public:
            Heuristic();
            int getGoalHeuristic(GraphStatePtr state);
            // TODO cheating for now. should accept a graph state
            void setGoal(DiscObjectState& state);
            void loadObstaclesFromOccupGrid();
            void visualize();
        private:
            std::unique_ptr<sbpl_arm_planner::BFS_3D> m_bfs;
    };
    typedef boost::shared_ptr<Heuristic> HeuristicPtr;
}
