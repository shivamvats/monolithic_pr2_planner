#include <monolithic_pr2_planner/MotionPrimitives/ArmMotionPrimitive.h>
#include <monolithic_pr2_planner/LoggerNames.h>
#include <boost/shared_ptr.hpp>

using namespace monolithic_pr2_planner;
using namespace boost;
using namespace std;

void ArmMotionPrimitive::print() const {
    ROS_DEBUG_NAMED(MPRIM_LOG, "Arm Primitive");
    ROS_DEBUG_NAMED(MPRIM_LOG, "\tgroup: %d", getGroup());
    ROS_DEBUG_NAMED(MPRIM_LOG, "\tid: %d", getID());
    ROS_DEBUG_NAMED(MPRIM_LOG, "\tcost: %d", cost());
    printEndCoord();
    printIntermSteps();
}

bool ArmMotionPrimitive::apply(const GraphState& graph_state, 
                           GraphStatePtr& successor){
    successor.reset(new GraphState(graph_state));

    ROS_DEBUG_NAMED(MPRIM_LOG, "orig is");
    graph_state.robot_pose().printToDebug(MPRIM_LOG);
    ROS_DEBUG_NAMED(MPRIM_LOG, "successor copy is");
    successor->robot_pose().printToDebug(MPRIM_LOG);
    return successor->applyMPrim(m_end_coord);
}

void ArmMotionPrimitive::computeCost(const MotionPrimitiveParams& params){
    m_cost = 1000;
}