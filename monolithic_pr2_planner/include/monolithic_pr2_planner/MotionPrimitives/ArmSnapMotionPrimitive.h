#pragma once
#include <monolithic_pr2_planner/MotionPrimitives/MotionPrimitive.h>
#include <monolithic_pr2_planner/StateReps/GraphState.h>
#include <monolithic_pr2_planner/StateReps/GoalState.h>
#include <monolithic_pr2_planner/StateReps/RobotState.h>
#include <monolithic_pr2_planner/Constants.h>
#include <boost/shared_ptr.hpp>

namespace monolithic_pr2_planner {
  class ArmSnapMotionPrimitive : public MotionPrimitive {
    public:
      ArmSnapMotionPrimitive():m_tolerances(4,0)
      {
       //Quickly hardcoded. Should be read from SearchReqParam

     }
      virtual bool apply(const GraphState& graph_state,
          GraphStatePtr& successor,
          TransitionData& t_data);
      virtual void print() const;
      virtual int motion_type() const { return MPrim_Types::ARM_SNAP; };
      virtual void computeCost(const MotionPrimitiveParams& params);
      bool computeIntermSteps(const GraphState& source_state,
                        const GraphState& successor,
                        TransitionData& t_data);
      void getUpdatedGoalandTolerances(const GoalStatePtr& goal, const double xyz_tol, const double roll_tol, const double pitch_tol, const double yaw_tol)
      {
         m_goal = goal;
         m_tolerances[Tolerances::XYZ] =  xyz_tol;
         m_tolerances[Tolerances::ROLL] =  roll_tol;
         m_tolerances[Tolerances::PITCH] =  pitch_tol;
         m_tolerances[Tolerances::YAW] =  yaw_tol;
      }

      GoalStatePtr m_goal;
      std::vector<double> m_tolerances;
      RobotState m_activationCenter;
      RobotState m_activationRadius;
  };
  typedef boost::shared_ptr<ArmSnapMotionPrimitive> ArmSnapMotionPrimitivePtr;

}
