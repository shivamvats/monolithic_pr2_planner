#include <monolithic_pr2_planner/StateReps/RobotState.h>
#include <monolithic_pr2_planner/LoggerNames.h>
#include <monolithic_pr2_planner/Constants.h>
#include <monolithic_pr2_planner/Visualizer.h>
#include <pr2_collision_checker/pr2_collision_space.h>
#include <kdl/frames.hpp>
#include <vector>

using namespace monolithic_pr2_planner;
using namespace boost;


bool RobotState::operator==(const RobotState& other){
    return (m_base_state == other.m_base_state &&
            m_right_arm == other.m_right_arm &&
            m_left_arm == other.m_left_arm);
}

bool RobotState::operator!=(const RobotState& other){
    return !(*this == other);
}

RobotState::RobotState(ContBaseState base_state, RightContArmState r_arm, 
                     LeftContArmState l_arm):
    m_base_state(base_state), 
    m_right_arm(r_arm), 
    m_left_arm(l_arm),
    m_obj_state(r_arm.getObjectStateRelBody()){
}

ContBaseState RobotState::getContBaseState(){
    return ContBaseState(m_base_state);    
}

void RobotState::printToDebug(char* log_level) const {
    ContBaseState base_state = m_base_state.getContBaseState();
    ROS_DEBUG_NAMED(log_level, "\tbase: %f %f %f %f", 
                   base_state.x(),
                   base_state.y(),
                   base_state.z(),
                   base_state.theta());
    std::vector<double> l_arm, r_arm;
    m_right_arm.getAngles(&r_arm);
    m_left_arm.getAngles(&l_arm);
    ROS_DEBUG_NAMED(log_level, "\tleft arm: %f %f %f %f %f %f %f",
                    l_arm[Joints::SHOULDER_PAN],
                    l_arm[Joints::SHOULDER_LIFT],
                    l_arm[Joints::UPPER_ARM_ROLL],
                    l_arm[Joints::ELBOW_FLEX],
                    l_arm[Joints::FOREARM_ROLL],
                    l_arm[Joints::WRIST_FLEX],
                    l_arm[Joints::WRIST_ROLL]);
    ROS_DEBUG_NAMED(log_level, "\tright arm: %f %f %f %f %f %f %f", 
                    r_arm[Joints::SHOULDER_PAN],
                    r_arm[Joints::SHOULDER_LIFT],
                    r_arm[Joints::UPPER_ARM_ROLL],
                    r_arm[Joints::ELBOW_FLEX],
                    r_arm[Joints::FOREARM_ROLL],
                    r_arm[Joints::WRIST_FLEX],
                    r_arm[Joints::WRIST_ROLL]);
}

void RobotState::printToInfo(char* log_level) const {
    ContBaseState base_state = m_base_state.getContBaseState();
    ROS_INFO_NAMED(log_level, "\tbase: %f %f %f %f", 
                   base_state.x(),
                   base_state.y(),
                   base_state.z(),
                   base_state.theta());
    std::vector<double> l_arm, r_arm;
    m_right_arm.getAngles(&r_arm);
    m_left_arm.getAngles(&l_arm);
    ROS_INFO_NAMED(log_level, "\tleft arm: %f %f %f %f %f %f %f",
                    l_arm[0],
                    l_arm[1],
                    l_arm[2],
                    l_arm[3],
                    l_arm[4],
                    l_arm[5],
                    l_arm[6]);
    ROS_INFO_NAMED(log_level, "\tright arm: %f %f %f %f %f %f %f", 
                    r_arm[0],
                    r_arm[1],
                    r_arm[2],
                    r_arm[3],
                    r_arm[4],
                    r_arm[5],
                    r_arm[6]);
}

//void RobotState::setPViz(boost::shared_ptr<PViz> pviz){
//    m_pviz = pviz;
//}

void RobotState::visualize(){
    std::vector<double> l_arm, r_arm;
    m_left_arm.getAngles(&l_arm);
    m_right_arm.getAngles(&r_arm);
    BodyPose body_pose = m_base_state.getBodyPose();
    Visualizer::pviz->visualizeRobot(r_arm, l_arm, body_pose, 150, 
                                    std::string("planner"), 0);
}


// this isn't a static function because we need seed angles.
// this is a bit weird at the moment, but we use the arm angles as seed angles
// disc_obj_state is in body frame
bool RobotState::computeRobotPose(const DiscObjectState& disc_obj_state,
                                 const RobotState& seed_robot_pose,
                                 RobotPosePtr& new_robot_pose){
    ContObjectState obj_state = disc_obj_state.getContObjectState();

    KDL::Frame obj_frame;
    obj_frame.p.x(obj_state.x());
    obj_frame.p.y(obj_state.y());
    obj_frame.p.z(obj_state.z());
    obj_frame.M = KDL::Rotation::RPY(obj_state.roll(), 
                                     obj_state.pitch(),
                                     obj_state.yaw());
    KDL::Frame obj_to_wrist_offset = seed_robot_pose.right_arm().getObjectOffset();

    // TODO: move this into cont arm
    // TODO: add in the left arm computation
    KDL::Frame wrist_frame = obj_frame * obj_to_wrist_offset;

    SBPLArmModelPtr arm_model = seed_robot_pose.m_right_arm.getArmModel();
    vector<double> seed(7,0), r_angles(7,0);
    seed_robot_pose.right_arm().getAngles(&seed);

    bool ik_success = arm_model->computeFastIK(wrist_frame, seed, r_angles);
    if (!ik_success){
        if (!arm_model->computeIK(wrist_frame, seed, r_angles)){
            ROS_DEBUG_NAMED(KIN_LOG, "Both IK failed!");
            return false;
        }
    }

    new_robot_pose = make_shared<RobotState>(seed_robot_pose.base_state(),
                                            RightContArmState(r_angles),
                                            seed_robot_pose.left_arm());

    return true;
}

// TODO only does base interpolation right now
//bool RobotPose::interpolateRobotPose(const RobotPose& start, const RobotPose& end,
//                                     int num_steps, vector<RobotPose>* interp_steps){
//    vector<ContBaseState> base_states;
//    if (start.m_base_state != end.m_base_state){
//        ContBaseState::interpolate(start.m_base_state, end.m_base_state, 
//                                   num_steps, &base_states);
//    }
//
//    for (auto& base_state : base_states){
//        RobotPose robot_state(base_state, start.getContRightArm(), 
//                              start.getContLeftArm());
//        interp_steps->push_back(robot_state);
//    }
//    return true;
//}

ContObjectState RobotState::getObjectStateRelMap() const {
    // This is an adaptation of computeContinuousObjectPose from the old
    // planner.  TODO: make this arm agnostic?
    std::vector<double> r_angles;
    m_right_arm.getAngles(&r_angles);
    SBPLArmModelPtr arm_model = m_right_arm.getArmModel();

    // don't remember what 10 is for. ask ben.
    KDL::Frame to_wrist;
    arm_model->computeFK(r_angles, m_base_state.getBodyPose(), 10, &to_wrist);
    KDL::Frame f = to_wrist * m_right_arm.getObjectOffset().Inverse();

    double wr,wp,wy;
    f.M.GetRPY(wr,wp,wy);

    return ContObjectState(f.p.x(), f.p.y(), f.p.z(), wr, wp, wy);
}


DiscObjectState RobotState::getObjectStateRelBody() const {
    return m_obj_state;
}









