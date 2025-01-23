#![allow(non_snake_case)]

use jack_make::model::*;

// The unit tests for the jack model validation

// Rules see model validation in the (Tools Design Document)

// #1 Valid Names

// #1.1) The Project namespace cannot be empty
// #1.2) The Agent name cannot be empty
// #1.3) The Team name cannot be empty
// #1.4) The Tactic name cannot be empty
// #1.5) The Project name cannot be empty
// #1.6) The Plan name cannot be empty
// #1.7) The Role name cannot be empty
// #1.8) The Goal name cannot be empty
// #1.9) The Service name cannot be empty
// #1.10) The Message name cannot be empty
// #1.11) The Resource name cannot be empty


// #2) Each JACK entity must have a unique name
// #2.1) Each Beliefset in a Role is unique
// #2.2) Each Plan in a Tactics is unique
// #2.3) Each Goal in a Role is unique
// #2.4) Each Action being handled in a Agent/Team is unique
// #2.5) Each Beliefset in an Agent is unique
// #2.6) Each Goal in a Agent/Team is unique (v0.4)
// #2.7) Each Starting Goal in a Agent/Team is unique
// #2.8) Each Plan in a Agent/Team is unique
// #2.9) Each Resource in a Agent/Team is unique
// #2.10) Each Role in a Agent/Team is unique
// #2.11) Each Tactic in a Agent/Team is unique
// #2.12) Each Message being handled in a Agent/Team is unique
// #2.13) Each Beliefset in a Goal is unique
// #2.14) Each Resource in a Goal is unique
// #2.15) Each Beliefset in a Plan is unique

// #3) Valid links - All Entities must have valid links
// #3.1) Actions being handled by an agent should exist in the model
// #3.2) Belief set being used by an agent should exist in the model
// #3.3) Goals used by an agent should exist in the model
// #3.4) The Starting Goals defined by an agent should exist in the model
// #3.5) Plans used by an agent should exist in the model
// #3.6) Roles used by an agent should exist in the model
// #3.7) Messages being handled by an agent should exist in the model
// #3.8) Plans in a Tactic must exist in the model
// #3.9) The Goal for a Tactics must exist in the model
// #3.10) The Tactics for an Agent must exist in the model
// #3.11) The Plans for a Tactic must exist in the model

// #4 Roles
// #4.1) At least one other agent/team uses the role of a team
// #4.2) A Role should be used at least once by an Agent or Team (warning level)

// #5 Goals
// #5.1) For each Goal there should exist at least one Plan that supports it
// #5.2) Each goal must have a message
// #5.3) Each goal must be supported by at least one plan

// #5.4) Each Beliefset specified in each Goal of an agent must be in the agent’s set of Beliefsets;
//     This includs the Beliefsets that the agent inherits from its roles.

// #6 Agents needs an action handler or service for all actions being used in all of its plans

// #7) Each beliefset message must exist

// #8) Plan task rules
// #8.1) An action task in a plan must have an action message
// #8.2) An action task in a plan must have a nowait field
// #8.3) An action task in a plan must have a mappings field
// #8.4) A goal task in a plan must have an goal message
// #8.5) A goal task in a plan must have a nowait field
// #8.6) A goal task in a plan must have a mappings field
// #8.7) A sleep task in a plan must have a duration field. 
// #8.8) A condition task in a plan must have XXX field. //TODO
// #8.9) A node task in a plan is invalid. Reserved for future use.
// #8.10) A wait task in a plan is invalid. Reserved for future use.
// #8.11) A None task in a plan is invalid. This is an error state.

// #9) Name cannot be the same as the entity type
// #9.1) The Project name cannot be "Project"
// #9.2) The Team name cannot be "Team"
// #9.3) The Agent name cannot be "Agent"
// #9.4) The Role name cannot be "Role"
// #9.5) The Plan name cannot be "Plan"
// #9.6) The Goal name cannot be "Goal"
// #9.7) The Tactic name cannot be "Tactic"
// #9.8) The Service name cannot be "Service"
// #9.9) The Message name cannot be "Message"
// #9.10) The Resource name cannot be "Resource"
// #9.11) The Message name cannot be any message protocol.

// #10)
// #10.1 A custom field in a message cannot be the encapsulation message 

// each goal must be used ??? by at least one plan? each plan mush be used at least once(?)




// Model Rule #1.5) The project name cannot be empty
// If the project name is empty an ZeroSizeString error is produced

#[test]
fn project_name_zero_name_length__1_5() {
    let pro = Project::default();
    
    let out = pro.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);

    assert!(out[0].keys[0] == "project.name");
    assert!(out[0].error_type == ErrorType::ZeroSizeString)
}

// Model Rule #1.5) The project name cannot be empty
// If the project name is *not* empty no error is produced

#[test]
fn project_name_non_zero_name_length__1_5() {
    let mut pro = Project::default();
    
    pro.name = "Test".to_string();
    
    let out = pro.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn project_name_is_project__9_1() {
    let mut pro = Project::default();
    
    pro.name = "Project".to_string();
    
    let out = pro.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

// Model Rule #1.1) The project namespace cannot be empty
// test error

#[test]
fn project_name_zero_namespaces_length__1_1() {
    let mut pro = Project::default();

    pro.name = "Test".to_string();

    pro.namespaces.push("".to_string());

    let out = pro.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);

    assert!(out[0].keys[0] == "project.namespaces.0");
    assert!(out[0].error_type == ErrorType::ZeroSizeString);

}

// Model Rule #1.1) The project namespace cannot be empty
// test no error

#[test]
fn project_name_non_zero_namespaces_length__1_1() {
    let mut pro = Project::default();

    pro.name = "Test".to_string();
    pro.namespaces.push("Test".to_string());

    let out = pro.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

// Model Rule #1.2) The agent name cannot be empty
// test error

#[test]
fn agent_name_zero_name_length__1_2() {
    let mut model = Model::default();
    let mut agent = Agent::default();

    agent.name = "".to_string();

    model.project.name = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

// Model Rule #1.2) The agent name cannot be empty
// test no error

#[test]
fn agent_name_non_zero_name_length__1_2() {
    let mut model = Model::default();
    let mut agent = Agent::default();

    agent.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn agent_name_is_agent__9_3() {
    let mut model = Model::default();
    let mut agent = Agent::default();

    agent.name = "Agent".to_string();

    model.project.name = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

// Model Rule #1.3) The team name cannot be empty
// test error

#[test]
fn team_name_zero_name_length__1_3() {
    let mut model = Model::default();
    let mut team = Agent::default();

    team.name = "".to_string();

    model.project.name = "Test".to_string();

    model.teams.push(team);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

// Model Rule #1.3) The team name cannot be empty
// test no error

#[test]
fn team_name_non_zero_name_length__1_3() {
    let mut model = Model::default();
    let mut team = Agent::default();

    team.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.teams.push(team);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn team_name_is_team__9_2() {
    let mut model = Model::default();
    let mut team = Agent::default();

    team.name = "Team".to_string();

    model.project.name = "Test".to_string();

    model.teams.push(team);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

// Model Rule #2.0) Each JACK entity must have a unique name
// test error

#[test]
fn team_name_duplicate__2_0() {
    let mut model = Model::default();
    let mut team = Agent::default();

    team.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.teams.push(team.clone());
    model.teams.push(team);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::DuplicateName);
}

// Model Rule #2) Each JACK entity must have a unique name
// test error

#[test]
fn agent_name_duplicate__2_0() {
    let mut model = Model::default();
    let mut agent = Agent::default();

    agent.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.agents.push(agent.clone());
    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::DuplicateName);
}

// Model Rule #2) Each JACK entity must have a unique name
// test error

#[test]
fn team_agent_name_duplicate__2_0() {
    let mut model = Model::default();
    let mut team = Agent::default();

    team.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.teams.push(team.clone());
    model.agents.push(team);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::DuplicateName);
}

// Model Rule #3) BDI Entities being reference from another entity should exist.
// test errors

// #3.1) Actions being handled by an agent should exist in the model
// #3.2) Belief set being used by an agent should exist in the model
// #3.3) Goals used by an agent should exist in the model
// #3.4) The Starting Goals defined by an agent should exist in the model
// #3.5) Plans used by an agent should exist in the model
// #3.6) Roles used by an agent should exist in the model
// #3.7) Messages being handled by an agent should exist in the model

#[test]
fn agent_no_linked_objects__3_1_to_3_7() {
    let mut model = Model::default();
    let mut agent = Agent::default();

    agent.name = "Test".to_string();
    agent.action_handlers.push("TestAction".to_string());
    agent.beliefsets.push("TestSet".to_string());
    agent.goals.push("TestGoal".to_string());
    agent.initial_goals.push("TestGoal".to_string());
    agent.plans.push("TestPlan".to_string());
    agent.resources.push("TestResource".to_string());
    agent.roles.push("TestRole".to_string());
    agent.message_handlers.push("TestMessage".to_string());

    model.project.name = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 8);
}

// Model Rule #3) BDI Entities being reference from another entity should exist.
// test no errors

// #3.1) Actions being handled by an agent should exist in the model
// #3.2) Belief set being used by an agent should exist in the model
// #3.3) Goals used by an agent should exist in the model
// #3.4) The Starting Goals defined by an agent should exist in the model
// #3.5) Plans used by an agent should exist in the model
// #3.6) Roles used by an agent should exist in the model
// #3.7) Messages being handled by an agent should exist in the model

#[test]
fn agent_linked_objects__3_1_to_3_7() {
    let mut model   = Model::default();
    let mut agent   = Agent::default();
    let mut action  = Message::default();
    let mut bs      = Message::default();
    let mut goal    = Goal::default();
    let mut plan    = Plan::default();
    let mut plan2   = Plan::default();
    let mut task    = Task::default();
    let mut task2   = Task::default();
    let mut res     = Resource::default();
    let mut role    = Role::default();
    let mut msg     = Message::default();
    let mut goalmsg = Message::default();

    agent.name = "Test".to_string();
    agent.action_handlers.push("TestAction".to_string());
    agent.beliefsets.push("TestSet".to_string());
    agent.goals.push("TestGoal".to_string());
    agent.initial_goals.push("TestGoal".to_string());
    agent.plans.push("TestPlan".to_string());
    agent.plans.push("TestPlan2".to_string());
    agent.resources.push("TestResource".to_string());
    agent.roles.push("TestRole".to_string());
    agent.message_handlers.push("TestMessage".to_string());

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    bs.name         = "TestSet".to_string();
    bs.protocol     = Protocol::beliefset;

    goal.name       = "TestGoal".to_string();
    goal.message    = "TestGoalMsg".to_string();

    plan.name       = "TestPlan".to_string();
    task.id         = "TestGoal".to_string();
    task.r#type     = TaskType::goal;
    task.message    = Some("TestGoal".to_string());

    plan.tasks.push(task);

    plan2.name      = "TestPlan2".to_string();
    plan2.handles   = Some("TestGoal".to_string());

    task2.id      = "TestAction".to_string();
    task2.r#type    = TaskType::action;
    task2.message   = Some("TestAction".to_string());
    
    plan2.tasks.push(task2);
    
    res.name        = "TestResource".to_string();
    res.min         = 0.0;
    res.max         = 1.0;

    role.name       = "TestRole".to_string();

    msg.name        = "TestMessage".to_string();
    msg.protocol    = Protocol::adhoc;

    goalmsg.name    = "TestGoalMsg".to_string();
    goalmsg.protocol= Protocol::goal;

    model.project.name = "Test".to_string();

    model.agents.push(agent);
    model.messages.push(action);
    model.messages.push(bs);
    model.goals.push(goal);
    model.plans.push(plan);
    model.plans.push(plan2);
    model.resources.push(res);
    model.roles.push(role);
    model.messages.push(msg);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 5);
}

 // Model Rule #4.1) At least one other agent/team uses the role of a team
 // test no error

#[test]
fn team_agent_linked_roles__4_1() {
    let mut model   = Model::default();
    let mut team    = Agent::default();
    let mut agent   = Agent::default();

    let mut role    = Role::default();

    role.name       = "TestRole".to_string();

    team.name       = "TestTeam".to_string();
    agent.name      = "TestAgent".to_string();

   team.roles.push("TestRole".to_string());
   agent.roles.push("TestRole".to_string());

    model.project.name = "Test".to_string();

    model.teams.push(team);
    model.agents.push(agent);
    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

 // Model Rule #4.1) At least one other agent/team uses the role of a team
 // test no error

#[test]
fn team_team_linked_roles__4_1() {
    let mut model   = Model::default();
    let mut team    = Agent::default();
    let mut team2   = Agent::default();

    let mut role    = Role::default();

    role.name       = "TestRole".to_string();

    team.name       = "TestTeam".to_string();
    team2.name      = "TestTeam2".to_string();

    team.roles.push("TestRole".to_string());
    team2.roles.push("TestRole".to_string());

    model.project.name = "Test".to_string();

    model.teams.push(team);
    model.teams.push(team2);
    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

 // Model Rule #4.1) At least one other agent/team uses the role of a team
 // test no error

#[test]
fn team_team_agent_linked_roles__4_1() {
    let mut model   = Model::default();
    let mut team    = Agent::default();
    let mut team2   = Agent::default();
    let mut agent   = Agent::default();

    let mut role    = Role::default();

    role.name       = "TestRole".to_string();

    team.name       = "TestTeam".to_string();
    team2.name      = "TestTeam2".to_string();
    agent.name      = "TestAgent".to_string();

    team.roles.push("TestRole".to_string());
    team2.roles.push("TestRole".to_string());
    agent.roles.push("TestRole".to_string());

    model.project.name = "Test".to_string();

    model.teams.push(team);
    model.teams.push(team2);
    model.agents.push(agent);
    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

// Model Rule #4.1) At least one other agent/team uses the role of a team
// test error

#[test]
fn team_team_agent_no_linked_roles__4_1() {
    let mut model   = Model::default();
    let mut team    = Agent::default();
    let mut agent   = Agent::default();

    let mut role    = Role::default();

    role.name       = "TestRole".to_string();

    team.name       = "TestTeam".to_string();
    agent.name      = "TestAgent".to_string();

    team.roles.push("TestRole".to_string());

    model.project.name = "Test".to_string();

    model.teams.push(team);
    model.agents.push(agent);
    model.roles.push(role);

    let out = model.verify();

    println!("{:#?}", out);

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::TeamRoleNoAgentsOrTeams);
}

//

#[test]
fn plan_name_zero_name_length__1_6() {
    let mut model = Model::default();
    let mut plan = Plan::default();

    plan.name = "".to_string();

    model.project.name = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn plan_name_non_zero_name_length__1_6() {
    let mut model = Model::default();
    let mut plan = Plan::default();

    plan.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
}

#[test]
fn plan_name_is_plan__11_5() {
    let mut model = Model::default();
    let mut plan = Plan::default();

    plan.name = "Plan".to_string();

    model.project.name = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

#[test]
fn goal_name_zero_name_length__1_8() {
    let mut model = Model::default();
    let mut goal = Goal::default();

    goal.name = "".to_string();

    model.project.name = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn goal_name_non_zero_name_length__1_8() {
    let mut model = Model::default();
    let mut goal = Goal::default();

    goal.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
}

#[test]
fn goal_name_is_goal__9_6() {
    let mut model = Model::default();
    let mut goal = Goal::default();

    goal.name = "Goal".to_string();

    model.project.name = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

// #5.1) For each Goal there should exist at least one Plan that supports it
// test error - no plan for a goal

#[test]
fn goal_no_plan__5_1() {
    let mut model       = Model::default();

    let mut goal        = Goal::default();

    goal.name           =  "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}

// #5.1) For each Goal there should exist at least one Plan that supports it
// test no error

#[test]
fn goal_with_plan__5_1() {
    let mut model       = Model::default();
    let mut goal        = Goal::default();
//    let mut plan        = Plan::default();
    let mut plan2       = Plan::default();
//    let mut task        = Task::default();
    let mut goalmsg     = Message::default();

    goal.name           =  "TestGoal".to_string();
    goal.message        = "GoalMsg".to_string();

    goalmsg.name        = "GoalMsg".to_string();
    goalmsg.protocol    = Protocol::goal;

// TODO: I am pretty sure this part is wrong.
//       this should not be tested as part of this rule.
//       Having a plan handle a goal, and having a plan 
//       use a goal as a task are two separate things.
//    plan.name           = "TestPlan".to_string();
//    task.id             = "TestGoal".to_string();
//    task.message        = Some("TestGoal".to_string());
//    task.r#type         = TaskType::goal;

//    plan.tasks.push(task);

    plan2.name           = "TestPlan2".to_string();
    plan2.handles        = Some("TestGoal".to_string());
    
    model.project.name  = "Test".to_string();

    model.goals.push(goal);
//    model.plans.push(plan);
    model.plans.push(plan2);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn plan_no_goal_handle__5_1() {

    let mut model       = Model::default();

    let mut plan        = Plan::default();

    plan.name           =  "TestPlan".to_string();

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::PlanMissingGoalToHandle);
}

#[test]
fn plan_goal_handle_not_found__5_1() {

    let mut model       = Model::default();

    let mut plan        = Plan::default();

    plan.name           =  "TestPlan".to_string();
    plan.handles        = Some("TestGoal".into());

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
}

// #10.1) An action task in a plan must have an action message
// test no error

#[test]
fn plan_action_with_message__8_1() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id           = "TestAction".to_string();
    task.message        = Some("TestAction".to_string());
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.messages.push(action);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

// #10.1) An action task in a plan must have an action message
// test error - Action Message is missing

#[test]
fn plan_action_no_message__8_1() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[3].error_type == ErrorType::MessageNotFound)
}

#[test]
fn plan_action_with_nowait__8_2() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.nowait         = Some(false);
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.messages.push(action);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

#[test]
fn plan_action_no_nowait__8_2() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[1].error_type == ErrorType::TaskNoWaitNotFound)
}

#[test]
fn plan_action_with_mappings__8_3() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.mappings       = Some(vec![]);
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.messages.push(action);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

#[test]
fn plan_action_no_mappings__8_3() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.r#type         = TaskType::action;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[2].error_type == ErrorType::TaskMappingsNotFound)
}

#[test]
fn plan_goal_with_message__8_4() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut goal        = Goal::default();
    let mut goal_msg    = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.message        = Some("TestGoal".to_string());
    task.r#type         = TaskType::goal;

    plan.tasks.push(task);

    goal.name           = "TestGoal".to_string();
    goal.message        = "TestGoal".to_string();
    goal_msg.name       = "TestGoal".to_string();
    goal_msg.protocol   = Protocol::goal;

    model.project.name  = "Test".to_string();

    model.goals.push(goal);
    model.plans.push(plan);
    model.messages.push(goal_msg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 5);
}

#[test]
fn plan_goal_no_message__8_4() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut goal        = Goal::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.r#type         = TaskType::goal;
    task.message        = Some("TestGoal".to_string());
    goal.name           = "TestGoal".to_string();

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.goals.push(goal);
    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 6);
    assert!(out[5].error_type == ErrorType::MessageNotFound)
}

#[test]
fn plan_goal_with_nowait__8_5() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut goal      = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.nowait         = Some(false);
    task.r#type         = TaskType::goal;

    plan.tasks.push(task);

    goal.name     = "TestGoal".to_string();
    goal.protocol = Protocol::goal;

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.messages.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

#[test]
fn plan_goal_no_nowait__8_5() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.r#type         = TaskType::goal;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[1].error_type == ErrorType::TaskNoWaitNotFound)
}

#[test]
fn plan_goal_with_mappings__8_6() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut goal      = Message::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.mappings       = Some(vec![]);
    task.r#type         = TaskType::goal;

    plan.tasks.push(task);

    goal.name     = "TestGoal".to_string();
    goal.protocol = Protocol::goal;

    model.project.name  = "Test".to_string();

    model.plans.push(plan);
    model.messages.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

#[test]
fn plan_goal_no_mappings__8_6() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestGoal".to_string();
    task.r#type         = TaskType::goal;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[2].error_type == ErrorType::TaskMappingsNotFound)
}

#[test]
fn plan_task_node__8_9() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestNode".to_string();
    task.r#type         = TaskType::node;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[1].error_type == ErrorType::TaskForFutureUse)
}

#[test]
fn plan_task_wait__8_10() {
    let mut model       = Model::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();

    plan.name           = "TestPlan".to_string();
    task.id             = "TestNode".to_string();
    task.r#type         = TaskType::wait;

    plan.tasks.push(task);

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[1].error_type == ErrorType::TaskForFutureUse)
}

// #6 Agents needs an action handler or service for all actions being used in all it's plans
// test error

// not working with services

#[test]
fn agent_plan_action_no_handler__6_0() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();

    agent.name          = "TestAgent".to_string();

    agent.plans.push("TestPlan".to_string());

    plan.name           = "TestPlan".to_string();
    task.id             = "TestAction".to_string();
    task.r#type         = TaskType::action;
    task.message        = Some("TestAction".to_string());

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.plans.push(plan);
    model.messages.push(action);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::ActionHandlersNotInAgent)
}

// #6 Agents needs an action handler or service for all actions being used in all it's plans
// test no error

// not working with services

#[test]
fn agent_plan_action_with_handler__6_0() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();

    agent.name          = "TestAgent".to_string();

    agent.plans.push("TestPlan".to_string());
    agent.action_handlers.push("TestAction".to_string());

    plan.name           = "TestPlan".to_string();
    task.id           = "TestAction".to_string();
    task.r#type         = TaskType::action;
    task.message        = Some("TestAction".to_string());

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.plans.push(plan);
    model.messages.push(action);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

// #6 Agents needs an action handler or service for all actions being used in all it's plans
// test no error
// The action handler is in a service

#[test]
fn agent_plan_action_with_handler_in_service__6_0() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut plan        = Plan::default();
    let mut task        = Task::default();
    let mut action      = Message::default();
    let mut service     = Service::default();

    agent.name          = "TestAgent".to_string();

    agent.plans.push("TestPlan".to_string());
    agent.services.push("TestService".to_string());

    service.name        = "TestService".to_string();
    service.action_handlers.push("TestAction".to_string());

    plan.name           = "TestPlan".to_string();
    task.id           = "TestAction".to_string();
    task.r#type         = TaskType::action;
    task.message        = Some("TestAction".to_string());

    plan.tasks.push(task);

    action.name     = "TestAction".to_string();
    action.protocol = Protocol::action;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.plans.push(plan);
    model.messages.push(action);
    model.services.push(service);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
}

// # Rule 5.3 - see Model Validation Rules
// Every beliefset associated with a goal is also in any agents using that goal
// Test of the valid case

#[test]
fn goal_beliefsets_in_agent_beliefsets__5_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut goal        = Goal::default();
    let mut beliefset   = Message::default();
    let mut goalmsg     = Message::default();

    agent.name          = "TestAgent".to_string();
    agent.beliefsets.push("TestBelief".to_string());
    agent.goals.push("TestGoal".to_string());

    goal.name           = "TestGoal".to_string();
    goal.beliefsets.push("TestBelief".to_string());
    goal.message        = "GoalMsg".to_string();

    goalmsg.name        = "GoalMsg".to_string();
    goalmsg.protocol    = Protocol::goal;

    beliefset.name      = "TestBelief".to_string();
    beliefset.protocol  = Protocol::beliefset;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.goals.push(goal);
    model.messages.push(beliefset);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NoPlansForGoal);
}

// # Rule 5.3 - see Model Validation Rules
// Every beliefset associated with a goal is also in any agents using that goal
// Test of the valid case

#[test]
fn goal_beliefsets_not_in_agent_beliefsets__5_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut goal        = Goal::default();
    let mut beliefset   = Message::default();
    let mut goalmsg     = Message::default();

    agent.name          = "TestAgent".to_string();
    agent.goals.push("TestGoal".to_string());

    goal.name           = "TestGoal".to_string();
    goal.beliefsets.push("TestBelief".to_string());
    goal.message        = "GoalMsg".to_string();

    goalmsg.name        = "GoalMsg".to_string();
    goalmsg.protocol    = Protocol::goal;

    beliefset.name      = "TestBelief".to_string();
    beliefset.protocol  = Protocol::beliefset;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.goals.push(goal);
    model.messages.push(beliefset);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::BeliefsetInGoalNotInAgent);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}

// Model Rule #5.3) Each Beliefset specified in each Goal of an agent must be in the agent’s set of Beliefsets;
//     This includs the Beliefsets that the agent inherits from its roles.
// test no error

#[test]
fn goal_beliefsets_in_agent_role_beliefsets__5_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut goal        = Goal::default();
    let mut rbs         = BeliefsetPerm::default();
    let mut role        = Role::default();
    let mut beliefset   = Message::default();
    let mut goalmsg     = Message::default();

    agent.name          = "TestAgent".to_string();
    agent.roles.push("TestRole".to_string());
    agent.goals.push("TestGoal".to_string());

    role.name           = "TestRole".to_string();
    rbs.name            = "TestBelief".to_string();
    role.beliefsets.push(rbs);

    goal.name           = "TestGoal".to_string();
    goal.beliefsets.push("TestBelief".to_string());
    goal.message        = "GoalMsg".to_string();
    
    goalmsg.name        = "GoalMsg".to_string();
    goalmsg.protocol    = Protocol::goal;

    beliefset.name      = "TestBelief".to_string();
    beliefset.protocol  = Protocol::beliefset;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.roles.push(role);
    model.goals.push(goal);
    model.messages.push(beliefset);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NoPlansForGoal);
}

// Model Rule #5.3) Each Beliefset specified in each Goal of an agent must be in the agent’s set of Beliefsets;
//     This includs the Beliefsets that the agent inherits from its roles.
// test error

#[test]
fn goal_beliefsets_not_in_agent_role_beliefsets__5_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut goal        = Goal::default();
    let mut role        = Role::default();
    let mut beliefset   = Message::default();
    let mut goalmsg     = Message::default();

    agent.name          = "TestAgent".to_string();
    agent.goals.push("TestGoal".to_string());
    agent.roles.push("TestRole".to_string());

    role.name           = "TestRole".to_string();

    goal.name           = "TestGoal".to_string();
    goal.beliefsets.push("TestBelief".to_string());
    goal.message        = "GoalMsg".to_string();
    
    goalmsg.name        = "GoalMsg".to_string();
    goalmsg.protocol    = Protocol::goal;

    beliefset.name      = "TestBelief".to_string();
    beliefset.protocol  = Protocol::beliefset;

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.goals.push(goal);
    model.roles.push(role);
    model.messages.push(beliefset);
    model.messages.push(goalmsg);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::BeliefsetInGoalNotInAgent);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}

// #2.4) Each Action being handled in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_action_handlers__2_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.action_handlers.push("THING0".to_string());
    agent.action_handlers.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.4) Each Action being handled in a Agent/Team is unique
// test no error

#[test]
fn agent_different_action_handlers__2_4() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.action_handlers.push("THING0".to_string());
    agent.action_handlers.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
}

// #2.5) Each Beliefset in an Agent is unique
// test error

#[test]
fn agent_duplicate_beliefsets__2_5() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.beliefsets.push("THING0".to_string());
    agent.beliefsets.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.5) Each Beliefset in an Agent is unique
// test no error

#[test]
fn agent_different_beliefsets__2_5() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.beliefsets.push("THING0".to_string());
    agent.beliefsets.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
}

// #2.6) Each Goal in a Agent/Team is unique (v0.4)
// test error

#[test]
fn agent_duplicate_goals__2_6() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.goals.push("THING0".to_string());
    agent.goals.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.6) Each Goal in a Agent/Team is unique (v0.4)
// test no error

#[test]
fn agent_different_goals__2_6() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.goals.push("THING0".to_string());
    agent.goals.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
}

// #2.7) Each Starting Goal in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_initial_goals__2_7() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.initial_goals.push("THING0".to_string());
    agent.initial_goals.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.7) Each Starting Goal in a Agent/Team is unique
// test no error

#[test]
fn agent_different_initial_goals__2_7() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.initial_goals.push("THING0".to_string());
    agent.initial_goals.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
}

// #2.8) Each Plan in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_plans__2_8() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.plans.push("THING0".to_string());
    agent.plans.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::PlanNotFound);
    assert!(out[1].error_type == ErrorType::PlanNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.8) Each Plan in a Agent/Team is unique
// test no error

#[test]
fn agent_different_plans__2_8() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.plans.push("THING0".to_string());
    agent.plans.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::PlanNotFound);
    assert!(out[1].error_type == ErrorType::PlanNotFound);
}

// #2.9) Each Resource in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_resources__2_9() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.resources.push("THING0".to_string());
    agent.resources.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::ResourceNotFound);
    assert!(out[1].error_type == ErrorType::ResourceNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.9) Each Resource in a Agent/Team is unique
// test no error

#[test]
fn agent_different_resources__2_9() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.resources.push("THING0".to_string());
    agent.resources.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::ResourceNotFound);
    assert!(out[1].error_type == ErrorType::ResourceNotFound);
}

// #2.10) Each Role in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_roles__2_10() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.roles.push("THING0".to_string());
    agent.roles.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::RoleNotFound);
    assert!(out[1].error_type == ErrorType::RoleNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.10) Each Role in a Agent/Team is unique
// test no error

#[test]
fn agent_different_roles__2_10() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.roles.push("THING0".to_string());
    agent.roles.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::RoleNotFound);
    assert!(out[1].error_type == ErrorType::RoleNotFound);
}

// #2.11) Each Tactic in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_tactics__2_11() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.tactics.push("THING0".to_string());
    agent.tactics.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::TacticNotFound);
    assert!(out[1].error_type == ErrorType::TacticNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.11) Each Tactic in a Agent/Team is unique
// test no error

#[test]
fn agent_different_tactics__2_11() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.tactics.push("THING0".to_string());
    agent.tactics.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::TacticNotFound);
    assert!(out[1].error_type == ErrorType::TacticNotFound);
}

// #2.12) Each Message being handled in a Agent/Team is unique
// test error

#[test]
fn agent_duplicate_message_handlers__2_12() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.message_handlers.push("THING0".to_string());
    agent.message_handlers.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.12) Each Message being handled in a Agent/Team is unique
// test no error

#[test]
fn agent_different_message_handlers__2_12() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();

    agent.name          = "TestAgent".to_string();
    agent.message_handlers.push("THING0".to_string());
    agent.message_handlers.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.agents.push(agent);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
}

// #2.13) Each Beliefset in a Goal is unique
// test error

#[test]
fn goal_duplicate_beliefsets__2_13() {
    let mut model       = Model::default();
    let mut goal       = Goal::default();

    goal.name          = "Testgoal".to_string();
    goal.beliefsets.push("THING0".to_string());
    goal.beliefsets.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 5);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::MessageNotFound);
    assert!(out[3].error_type == ErrorType::DuplicateName);
    assert!(out[4].error_type == ErrorType::NoPlansForGoal);
}

// #2.13) Each Beliefset in a Goal is unique
// test no error

#[test]
fn goal_different_beliefsets__2_13() {
    let mut model       = Model::default();
    let mut goal       = Goal::default();

    goal.name          = "Testgoal".to_string();
    goal.beliefsets.push("THING0".to_string());
    goal.beliefsets.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::MessageNotFound);
    assert!(out[3].error_type == ErrorType::NoPlansForGoal);
  //  assert!(out[4].error_type == ErrorType::NoPlansCanPursueGoal);
}

// #2.14) Each Resource in a Goal is unique
// test error

#[test]
fn goal_duplicate_resources__2_14() {
    let mut model       = Model::default();
    let mut goal       = Goal::default();

    goal.name          = "Testgoal".to_string();
    goal.resources.push("THING0".to_string());
    goal.resources.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 5);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::ResourceNotFound);
    assert!(out[2].error_type == ErrorType::ResourceNotFound);
    assert!(out[3].error_type == ErrorType::DuplicateName);
    assert!(out[4].error_type == ErrorType::NoPlansForGoal);
}

// #2.14) Each Resource in a Goal is unique
// test no error

#[test]
fn goal_different_resources__2_14() {
    let mut model       = Model::default();
    let mut goal       = Goal::default();

    goal.name          = "Testgoal".to_string();
    goal.resources.push("THING0".to_string());
    goal.resources.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::ResourceNotFound);
    assert!(out[2].error_type == ErrorType::ResourceNotFound);
    assert!(out[3].error_type == ErrorType::NoPlansForGoal);
}

// #2.15) Each Beliefset in a Plan is unique
// test error

#[test]
fn plan_duplicate_beliefsets__2_15() {
    let mut model       = Model::default();
    let mut plan       = Plan::default();

    plan.name          = "Testplan".to_string();
    plan.beliefsets.push("THING0".to_string());
    plan.beliefsets.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[3].error_type == ErrorType::DuplicateName);
}

// #2.15) Each Beliefset in a Plan is unique
// test no error

#[test]
fn plan_different_beliefsets__2_15() {
    let mut model       = Model::default();
    let mut plan       = Plan::default();

    plan.name          = "Testplan".to_string();
    plan.beliefsets.push("THING0".to_string());
    plan.beliefsets.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
}

// #2.3) Each Goal in a Role is unique
// test error

#[test]
fn role_duplicate_goals__2_3() {
    let mut model       = Model::default();
    let mut role       = Role::default();

    role.name          = "TestRole".to_string();
    role.goals.push("THING0".to_string());
    role.goals.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// #2.3) Each Goal in a Role is unique
// test no error

#[test]
fn role_different_goals__2_3() {
    let mut model       = Model::default();
    let mut role       = Role::default();

    role.name          = "TestRole".to_string();
    role.goals.push("THING0".to_string());
    role.goals.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
}

// Model Rule #2.1) Each Beliefset in a Role is unique
// test error

#[test]
fn role_duplicate_beliefsets__2_1() {
    let mut model       = Model::default();
    let mut role        = Role::default();
    let mut bs0         = BeliefsetPerm::default();
    bs0.name            = "THING0".to_string();

    role.name           = "TestRole".to_string();

    role.beliefsets.push(bs0.clone());
    role.beliefsets.push(bs0);

    model.project.name  = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::DuplicateName);
}

// Model Rule #2.1) Each Beliefset in a Role is unique
// test no error

#[test]
fn role_different_beliefsets__2_1() {
    let mut model       = Model::default();
    let mut role       = Role::default();
    let mut bs0         = BeliefsetPerm::default();
    let mut bs1         = BeliefsetPerm::default();
    bs0.name            = "THING0".to_string();
    bs1.name            = "THING1".to_string();

    role.name           = "TestRole".to_string();

    role.beliefsets.push(bs0);
    role.beliefsets.push(bs1);

    model.project.name  = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
}

// Model Rule #2.2) Each Plan in a Tactics is unique
// test error

#[test]
fn tactic_duplicate_plans__2_2() {
    let mut model       = Model::default();
    let mut tactic       = Tactic::default();

    tactic.name          = "TestTactic".to_string();
    tactic.goal          = "TestGoal".to_string();
    tactic.plans.push("THING0".to_string());
    tactic.plans.push("THING0".to_string());

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::PlanNotFound);
    assert!(out[2].error_type == ErrorType::PlanNotFound);
    assert!(out[3].error_type == ErrorType::DuplicateName);
}

// Model Rule #3.11) The Plans for a Tactic must exist in the model
// test error

#[test]
fn tactic_different_plans__3_11() {
    let mut model       = Model::default();
    let mut tactic       = Tactic::default();

    tactic.name          = "TestTactic".to_string();
    tactic.goal          = "TestGoal".to_string();
    tactic.plans.push("THING0".to_string());
    tactic.plans.push("THING1".to_string());

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
    assert!(out[1].error_type == ErrorType::PlanNotFound);
    assert!(out[2].error_type == ErrorType::PlanNotFound);
}

// Model Rule #1.4) The Tactic name cannot be empty
// test error

#[test]
fn tactics_name_zero_length_string__1_4() {
    let mut model       = Model::default();
    let     tactic      = Tactic::default();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

// Model Rule #1.4) The Tactic name cannot be empty
// test no error

#[test]
fn tactics_name_non_zero_length_string__1_4() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();

    model.project.name  = "Test".to_string();

    tactic.name          = "TestTactic".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
}

// Model Rule #3.9) The Goal for a Tacics must exist in the model

// deplicate unit test
// There is no point in testing the name length of the goal it's a link

#[test]
fn tactics_goal_zero_length_string__3_9() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();

    tactic.name          = "TestTactic".to_string();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
    assert!(out[1].error_type == ErrorType::GoalNotFound);
}

// Mode Rule #3.9) The Goal for a Tacics must exist in the model
// test error

#[test]
fn tactics_not_goal_found__3_9() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::GoalNotFound);
}

// Mode Rule #3.9) The Goal for a Tacics must exist in the model
// test no error

#[test]
fn tactics_goal_found__3_9() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();
    let mut goal        = Goal::default();

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();
    goal.name           = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);
    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}

// Model Rule #3.8) Plans in a Tactic must exist in the model
// Test error

#[test]
fn tactics_plans_not_found__3_8() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();
    let mut goal        = Goal::default();

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();
    tactic.plans.push("TestPlan".to_string());

    goal.name           = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);
    model.goals.push(goal);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
    assert!(out[2].error_type == ErrorType::PlanNotFound);
}

// Model Rule #3.8) Plans in a Tactic must exist in the model
// Test no error

#[test]
fn tactics_plans_found__3_8() {
    let mut model       = Model::default();
    let mut tactic      = Tactic::default();
    let mut goal        = Goal::default();
    let mut plan        = Plan::default();

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();
    tactic.plans.push("TestPlan".to_string());

    plan.name           = "TestPlan".to_string();

    goal.name           = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.tactics.push(tactic);
    model.goals.push(goal);
    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}


// Model Rule #3.10) The Tactics for an Agent must exist in the model
// test no error

#[test]
fn tactics_agent_found__3_10() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut tactic      = Tactic::default();
    let mut goal        = Goal::default();
    let mut plan        = Plan::default();

    agent.name          = "TestAgent".to_string();
    agent.tactics.push("TestTactic".to_string());

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();
    tactic.plans.push("TestPlan".to_string());

    plan.name           = "TestPlan".to_string();

    goal.name           = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.tactics.push(tactic);
    model.goals.push(goal);
    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::MessageNotFound);
    assert!(out[1].error_type == ErrorType::NoPlansForGoal);
}

// Model Rule #3.10) The Tactics for an Agent must exist in the model
// test error

#[test]
fn tactics_agent_not_found__3_10() {
    let mut model       = Model::default();
    let mut agent       = Agent::default();
    let mut tactic      = Tactic::default();
    let mut goal        = Goal::default();
    let mut plan        = Plan::default();

    agent.name          = "TestAgent".to_string();
    agent.tactics.push("TestTactic1".to_string());

    tactic.name         = "TestTactic".to_string();
    tactic.goal         = "TestGoal".to_string();
    tactic.plans.push("TestPlan".to_string());

    plan.name           = "TestPlan".to_string();

    goal.name           = "TestGoal".to_string();

    model.project.name  = "Test".to_string();

    model.agents.push(agent);
    model.tactics.push(tactic);
    model.goals.push(goal);
    model.plans.push(plan);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 4);
    assert!(out[0].error_type == ErrorType::TacticNotFound);
    assert!(out[1].error_type == ErrorType::MessageNotFound);
    assert!(out[2].error_type == ErrorType::NoPlansForGoal);
}

#[test]
fn service_name_zero_name_length__1_9() {
    let mut model = Model::default();
    let mut service = Service::default();

    service.name = "".to_string();

    model.project.name = "Test".to_string();

    model.services.push(service);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn service_name_non_zero_name_length__1_9() {
    let mut model = Model::default();
    let mut service = Service::default();

    service.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.services.push(service);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn service_name_is_service__9_8() {
    let mut model = Model::default();
    let mut service = Service::default();

    service.name = "Service".to_string();

    model.project.name = "Test".to_string();

    model.services.push(service);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

#[test]
fn message_name_zero_name_length__1_10() {
    let mut model = Model::default();
    let mut message = Message::default();

    message.name = "".to_string();

    model.project.name = "Test".to_string();

    model.messages.push(message);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn message_name_non_zero_name_length__1_10() {
    let mut model = Model::default();
    let mut message = Message::default();

    message.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.messages.push(message);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn message_name_is_message__9_9() {
    let mut model = Model::default();
    let mut message = Message::default();

    message.name = "Message".to_string();

    model.project.name = "Test".to_string();

    model.messages.push(message);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

#[test]
fn message_field_is_current_message__10_1() {
    let mut model = Model::default();
    let mut message = Message::default();

    message.name = "Msg".to_string();

    let mut field = Field::default();
    field.r#type = FieldType::Custom("Msg".to_string()); 

    message.fields.push(field);

    model.project.name = "Test".to_string();

    model.messages.push(message);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::CustomTypeIsCurrentMessage);
}



#[test]
fn message_name_is_protocol__9_11() {
    let protos : Vec<Protocol> = vec![Protocol::action, Protocol::adhoc, Protocol::beliefset, Protocol::goal, Protocol:: percept];
    for proto in protos.iter() {
        let mut model = Model::default();
        let mut message = Message::default();

        message.name = format!("{:?}", proto);

        model.project.name = "Test".to_string();

        model.messages.push(message);

        let out = model.verify();

        for o in out.iter() {
            println!("{:?}", o);
        }

        assert!(out.len() == 1);
        assert!(out[0].error_type == ErrorType::NameSameAsMessageType);
    }
}

#[test]
fn role_name_zero_name_length__1_7() {
    let mut model = Model::default();
    let mut role = Role::default();

    role.name = "".to_string();

    model.project.name = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn role_name_non_zero_name_length__1_7() {
    let mut model = Model::default();
    let mut role = Role::default();

    role.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 0);
}

#[test]
fn role_name_is_role__9_4() {
    let mut model = Model::default();
    let mut role = Role::default();

    role.name = "Role".to_string();

    model.project.name = "Test".to_string();

    model.roles.push(role);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

#[test]
fn resource_name_zero_name_length__1_11() {
    let mut model = Model::default();
    let mut resource = Resource::default();

    resource.name = "".to_string();

    model.project.name = "Test".to_string();

    model.resources.push(resource);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn resource_name_non_zero_name_length__1_11() {
    let mut model = Model::default();
    let mut resource = Resource::default();

    resource.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.resources.push(resource);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 1);
}

#[test]
fn resource_name_is_resource__9_10() {
    let mut model = Model::default();
    let mut resource = Resource::default();

    resource.name = "Resource".to_string();

    model.project.name = "Test".to_string();

    model.resources.push(resource);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}

#[test]
fn tactic_name_zero_name_length__1_4() {
    let mut model = Model::default();
    let mut tactic = Tactic::default();

    tactic.name = "".to_string();

    model.project.name = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::ZeroSizeString);
}

#[test]
fn tactic_name_non_zero_name_length__1_4() {
    let mut model = Model::default();
    let mut tactic = Tactic::default();

    tactic.name = "Test".to_string();

    model.project.name = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 2);
}

#[test]
fn tactic_name_is_tactic__9_7() {
    let mut model = Model::default();
    let mut tactic = Tactic::default();

    tactic.name = "Tactic".to_string();

    model.project.name = "Test".to_string();

    model.tactics.push(tactic);

    let out = model.verify();

    for o in out.iter() {
        println!("{:?}", o);
    }

    assert!(out.len() == 3);
    assert!(out[0].error_type == ErrorType::NameSameAsType);
}


