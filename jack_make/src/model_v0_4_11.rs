// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

use serde::{Serialize, Deserialize};
use std::collections::*;

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Project Data
/// Currently only stores project name and any c++ namespaces that the codegenerator should use.
pub struct Project {
    pub name : String,
    pub namespaces : Vec<String>,
}

impl Project {
    /// Verification for the project is minimal.
    /// - No zero length strings
    ///
    pub fn verify(&self) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];

        // Ensure name is a non zero length string
        if self.name.len() == 0 {
            errors.push(ModelError::new(vec!["project.name".to_string()],
                                        vec!["project.name".to_string()],
                                        ErrorType::ZeroSizeString,
                                        "Project Name must not be empty.".to_string()));
        }

        if self.name == "Project" {
            errors.push(ModelError::new(vec!["project.name".to_string()],
                                        vec!["project.name".to_string()],
                                        ErrorType::NameSameAsType,
                                        "Project Name must not be \"Project\".".to_string()));
        }

        // Ensure namespaces vector is full of non zero string
        for (i,name) in self.namespaces.iter().enumerate() {
            if name.len() == 0 {
                errors.push(ModelError::new(vec![format!("project.namespaces.{}", i)],
                                            vec![format!("project.namespaces.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Project Namespace at index {} must not be empty.", i)));
            }
        }

        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
/// Field type describes types that can be used in messages.
/// None is never used and is the default error case.
///
pub enum FieldType {
    /// Boolean type.
    Bool,
    /// 8 bit signed Integer
    I8,
    /// 16 bit signed Integer
    I16,
    /// 32 bit signed Integer
    I32,
    /// 64 bit signed Integer
    I64,
    /// 8 bit unsigned Integer
    U8,
    /// 16 bit unsigned Integer
    U16,
    /// 32 bit unsigned Integer
    U32,
    /// 64 bit unsigned Integer
    U64,
    /// 32 bit floating point number
    F32,
    /// 64 bit floating point number
    F64,
    /// Text String
    String,
    /// Allows for the nesting of messages within messages.
    /// This *MUST* be another message in the messages list in the model.
    Custom(String),
    /// Default error case. Never used in real operations.
    None  // Error case
}

// This is the implementation of the std::default::Default trait.
// For types other than Enumerations this is automatic or requires
// a Procedural Macro decorator #[derive(Default)].
// For enumerations this must be done by us to determine which value
// is the default.
impl std::default::Default for FieldType {
    fn default() -> Self {
        Self::None
    }
}

// This is a proc macro decorator
// It gives you the ability to use procedural macros to 
// give your struct more functionality
// through code generation (internal to the rust compiler).
#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Message Fields
/// Fields for a message are defined with this struct.
pub struct Field {
    /// The field name.
    pub name            : String,
    // type is a reserved keyword in rust
    // r# stops it being a keyword
    /// Is the FieldType please look at the definition for further information.
    pub r#type          : FieldType,
    /// default is an optional field that allows for a different default other than
    /// the norm to be defined by the model.
    pub default         : Option<String>,
    /// is_array is an optional field that defines whether the field is a list of <FieldType>
    pub is_array        : Option<bool>
}

// Rust also has linting rules built into the compiler
// Sometiems you want to do things differently, 
// like we do with the case of this enumeration.
#[allow(non_camel_case_types)]
#[derive(Serialize, Deserialize, Debug, Clone, PartialEq)]
/// Protocol is the type of message that we are creating.
///
pub enum Protocol {
    /// Plan action type to allow for fields to be added to an action.
    action,
    /// Adhoc message type -> data you want to be in a message,
    /// but does not have a distinct purpose like the other message protocols.
    adhoc,
    /// Beliefset type -> main data type for data to be stored in JACK.
    /// This implementation it is a percept as known in the literature.
    beliefset,
    /// Goal pursue message when data needs ot be appended to the pursue/perform message.
    goal,
    /// Percept message
    percept,
    /// Default error type. Should not be used in general operation.
    None
}

impl std::default::Default for Protocol {
    fn default() -> Self {
        Self::None
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Structure that contains all message types in the Model.
pub struct Message {
    /// Name of message.
    pub name            : String,
    /// Protocol of the message.
    pub protocol        : Protocol,
    /// Definition of the containing fields.
    pub fields          : Vec<Field>
}

impl Message {
    /// Verficiation for Messages:
    /// 1. Checking that there are no fields with the same duplicate fields.
    /// 2. If a field has a custom type, whether the model contains the message required.
    ///
    fn verify(&self, model : &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];

        let mut field_names : BTreeMap<String, usize> = BTreeMap::new();
 
        for (i, field) in self.fields.iter().enumerate() {
            // Check if field name is contained in previous field names.
            // if found produce error.
            if field_names.contains_key(&field.name) {
                let other_index = field_names.get(&field.name).unwrap();

                // Adding both keys/indexes for errors to hopefully help the GUI.
                // *POSSIBLE ISSUE* If there is 3 or more of the same, it will only show the first and the current issue.
                // I think this is good enough.
                errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", self.name, i), format!("messages.{}.fields.{}", self.name, other_index)],
                                            vec![format!("messages.{}.fields.{}", index, i), format!("messages.{}.fields.{}", self.name, other_index)],
                                            ErrorType::MessageDuplicateFields,
                                            format!("Duplicate fields with name {} in Message {}.", field.name, self.name)));
            } else {
                // Insert field name into the map with its index.
                field_names.insert(field.name.clone(), i.clone());
            }

            // If field is of type custom:
            // Check we have a message with a name that matches
            // what is in str_type string.
            match &field.r#type {
                FieldType::Custom(str_type) => {
                    let mut found = false;

                    for msg in model.messages.iter() {
                        if &msg.name == str_type {
                            found = true;
                        }
                    }

                    if ! found {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", self.name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeNotFound,
                                                    format!("Custom field {} at index {} in Message {} cannot be found in Messages.", str_type, i, self.name)));
                    }

                    if str_type == &self.name {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", self.name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeIsCurrentMessage,
                                                    format!("Custom field {} at index {} in Message {} cannot be current Message.", str_type, i, self.name)));

                    }
                },
                _ => { /* catch all do nothing!!!!! */ }
            }
        }

        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Service data structure.
/// All fields are lookups to other parts of the model.
///
pub struct Service {
   pub name             : String,
   pub action_handlers  : Vec<String>,
   pub beliefsets       : Vec<String>,
}

/// todo: impl Service for validation

#[derive(Serialize, Deserialize, Debug, Clone, PartialEq)]
/// Used by the verification code to differentiate between Teams and Agents.
/// Teams and Agents use the same model structure.
///
pub enum AgentType {
    Team,
    Agent
}

// This is trait implementation allows us to
// print the string form of the enumeration easily. 
// It uses the Debug decorator and the {:?} formatter to 
// output into a normal formatter.
// There may be a newer, easier way to do this.
impl std::fmt::Display for AgentType {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", format!("{:?}", self).to_lowercase())
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Team/Agent data structure.
/// All fields are lookups to other parts of the model.
/// There is nothing that differnetiates a Team from an Agent in this struct.
/// The differentiating factor is in the main model structure.
/// I.e. teams in the teams list and agents in the agents list.
///
pub struct Agent {
   pub name             : String,
   pub action_handlers  : Vec<String>,
   pub beliefsets       : Vec<String>,
   pub goals            : Vec<String>,
   pub initial_goals    : Vec<String>,
   pub plans            : Vec<String>,
   pub resources        : Vec<String>,
   pub roles            : Vec<String>,
   pub tactics          : Vec<String>,
   pub message_handlers : Vec<String>,
   pub services         : Vec<String>,
}

impl Agent {
    /// Most of the error handling that is being done is ensuring that the model is referencing the 
    /// right things in other parts of the model and that the references are not zero length strings.
    /// 1. Action handlers, Beliefsets and Message Handlers are all looking for things in the message list.
    /// 2. Goals, Initial Goals, Plans, Resources, Roles and Tactics are also looking for objects witht he same name.
    /// 3. For Teams -> Roles also have extra checks that looks for agents or plans that can complete it.
    /// 4. Agent beliefsets must also be in the goals that it has. 
    /// 5. Agents must have action_handlers for any action in a plan task/body
    /// 6. Agents must have the resources required by a goal
    fn verify(&self, agent_type : &AgentType, model : &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];

        // Action Handlers
        for (i, action) in self.action_handlers.iter().enumerate() {
            if action.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.action_handlers.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.action_handlers.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Action at index {} must not be empty.",i)));
            }

            if ! model.find_message_with_protocol(action, &Protocol::action) {
                errors.push(ModelError::new(vec![format!("{}s.{}.action_handlers.{}", agent_type, self.name, action)],
                                            vec![format!("{}s.{}.action_handlers.{}", agent_type, index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Action {} at index {} cannot be found in messages.", action, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.action_handlers, &format!("{}s.{}.action_handlers", agent_type, self.name), &format!("{}s.{}.action_handlers", agent_type, index), &format!("{:?}", agent_type), &self.name, &"action_handlers".to_string()));

        // Beliefsets
        for (i, bs) in self.beliefsets.iter().enumerate() {
            if bs.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.beliefsets.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.beliefsets.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Beliefset at index {} must not be empty.",i)));
            }

            if ! model.find_message_with_protocol(bs,  &Protocol::beliefset) {
                errors.push(ModelError::new(vec![format!("{}s.{}.beliefsets.{}", agent_type, self.name, bs)],
                                            vec![format!("{}s.{}.beliefsets.{}", agent_type, index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Beliefset {} at index {} cannot be found in messages.", bs, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.beliefsets, &format!("{}s.{}.beliefsets", agent_type, self.name), &format!("{}s.{}.beliefsets", agent_type, index), &format!("{:?}", agent_type), &self.name, &"beliefsets".to_string()));

        // Message Handlers
        for (i, msg) in self.message_handlers.iter().enumerate() {
            if msg.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.message_handlers.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.message_handlers.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message at index {} must not be empty.",i)));
            }

            if ! model.find_message_with_protocol(msg, &Protocol::adhoc) {
                errors.push(ModelError::new(vec![format!("{}s.{}.message_handlers.{}", agent_type, self.name, msg)],
                                            vec![format!("{}s.{}.message_handlers.{}", agent_type, index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", msg, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.message_handlers, &format!("{}s.{}.message_handlers", agent_type, self.name), &format!("{}s.{}.message_handlers", agent_type, index), &format!("{:?}", agent_type), &self.name, &"message_handlers".to_string()));

        // goals
        for (i, goal) in self.goals.iter().enumerate() {
            if goal.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal at index {} must not be empty.",i)));
            }

            if ! model.find_goal(goal) {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, self.name, goal)],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", goal, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.goals, &format!("{}s.{}.goals", agent_type, self.name) , &format!("{}s.{}.goals", agent_type, index), &format!("{:?}", agent_type), &self.name, &"goals".to_string()));

        // initial_goals
        for (i, goal) in self.initial_goals.iter().enumerate() {
            if goal.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.initial_goals.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.initial_goals.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Initial Goal at index {} must not be empty.",i)));
            }

            if ! model.find_goal(goal) {
                errors.push(ModelError::new(vec![format!("{}s.{}.initial_goals.{}", agent_type, self.name, goal)],
                                            vec![format!("{}s.{}.initial_goals.{}", agent_type, index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Initial Goal {} at index {} cannot be found in goals.", goal, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.initial_goals, &format!("{}s.{}.initial_goals", agent_type, self.name), &format!("{}s.{}.initial_goals", agent_type, index), &format!("{:?}", agent_type), &self.name, &"initial_goals".to_string()));

        // Plans
        for (i, plan) in self.plans.iter().enumerate() {
            if plan.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.plans.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.plans.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan at index {} must not be empty.",i)));
            }

            if ! model.find_plan(plan) {
                errors.push(ModelError::new(vec![format!("{}s.{}.plans.{}", agent_type, self.name, plan)],
                                            vec![format!("{}s.{}.plans.{}", agent_type, index, i)],
                                            ErrorType::PlanNotFound,
                                            format!("Plan {} at index {} cannot be found in plans.", plan, i)));
            }
        }
 
        errors.append(&mut verify_no_duplicate_fields(&self.plans, &format!("{}s.{}.plans", agent_type, self.name), &format!("{}s.{}.plans", agent_type, index), &format!("{:?}", agent_type), &self.name, &"plans".to_string()));

        // Resources
        for (i, resource) in self.resources.iter().enumerate() {
            if resource.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.resources.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.resources.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource at index {} must not be empty.",i)));
            }

            if ! model.find_resource(resource) {
                errors.push(ModelError::new(vec![format!("{}s.{}.resources.{}", agent_type, self.name, resource)],
                                            vec![format!("{}s.{}.resources.{}", agent_type, index, i)],
                                            ErrorType::ResourceNotFound,
                                            format!("Resource {} at index {} cannot be found in resources.", resource, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.resources, &format!("{}s.{}.resources", agent_type, self.name), &format!("{}s.{}.resources", agent_type, index), &format!("{:?}", agent_type), &self.name, &"resources".to_string()));

        // Roles
        for (i, role) in self.roles.iter().enumerate() {
            if role.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Role at index {} must not be empty.",i)));
            }

            if ! model.find_role(role) {
                errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, self.name, role)],
                                            vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                            ErrorType::RoleNotFound,
                                            format!("Role {} at index {} cannot be found in roles.", role, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.roles, &format!("{}s.{}.roles", agent_type, self.name), &format!("{}s.{}.roles", agent_type, index), &format!("{:?}", agent_type), &self.name, &"roles".to_string()));

        // The service action handlers
        let mut all_action_handlers = BTreeSet::<String>::new();

        // Services
        for (i, service) in self.services.iter().enumerate() {
            if service.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.services.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.services.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Service at index {} must not be empty.",i)));
            }

            if ! model.find_service(service) {
                errors.push(ModelError::new(vec![format!("{}s.{}.services.{}", agent_type, self.name, service)],
                                            vec![format!("{}s.{}.services.{}", agent_type, index, i)],
                                            ErrorType::ServiceNotFound,
                                            format!("Service {} at index {} cannot be found in services.", service, i)));
            } else {
                // add this service's action handlers

                // find the service object
                for service_obj in model.services.iter() {
                    if &service_obj.name == service {
                        
                        // iterate the action_handlers in the service
                        for ah in service_obj.action_handlers.iter() {
                            all_action_handlers.insert(ah.clone());
                        }
                    }
                }
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.services, &format!("{}s.{}.services", agent_type, self.name), &format!("{}s.{}.services", agent_type, index), &format!("{:?}", agent_type), &self.name, &"services".to_string()));

        // Verify one atleast one agent can do each role for team
        match agent_type {
            AgentType::Team => {
                for (i, role_name) in self.roles.iter().enumerate() {
                    let mut role_cnt : u32 = 0;
                    for agent in model.agents.iter() {
                        for agnt_role in agent.roles.iter() {
                            if agnt_role == role_name {
                                role_cnt += 1;
                            } else {
                                match model.get_role(role_name) {
                                    Ok(role) => {
                                        for plan_name in agent.plans.iter() {
                                            match model.get_plan(plan_name) {
                                                Ok(plan) => {
                                                    for goal in role.goals.iter() {
                                                        if let Some(handles) = plan.handles.as_ref() {
                                                            if handles == goal {
                                                                role_cnt += 1;
                                                            }
                                                        }
                                                    }
                                                },
                                                Err(_e) => { /* Not found, that's an error for somewhere else :) */ }
                                            }
                                        }
                                    },
                                    Err(_e) => { /* Not found, that's an error for somewhere else :) */ }
                                }
                            }
                        }
                    }

                    for team in model.teams.iter() {
                        if team.name != self.name {
                            for team_role in team.roles.iter() {
                                if team_role == role_name {
                                    role_cnt += 1;
                                } else {
                                    match model.get_role(role_name) {
                                        Ok(role) => {
                                            for plan_name in team.plans.iter() {
                                                match model.get_plan(plan_name) {
                                                    Ok(plan) => {
                                                        for goal in role.goals.iter() {
                                                            if let Some(handles) = plan.handles.as_ref() {
                                                                if handles == goal {
                                                                    role_cnt += 1;
                                                                }
                                                            }
                                                        }
                                                    },
                                                    Err(_e) => { /* Not found, that's an error for somewhere else :) */ }
                                                }
                                            }
                                        },
                                        Err(_e) => { /* Not found, that's an error for somewhere else :) */ }
                                    }
                                }
                            }
                        }
                    }

                    if role_cnt == 0 {
                        errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, self.name, role_name)],
                                                    vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                                    ErrorType::TeamRoleNoAgentsOrTeams,
                                                    format!("Role {} at index {} has no Agents or Teams that support it.", role_name, i)));
                    }
                }
            },
            AgentType::Agent => {
                // Do nothing, you are an awesome agent who doesn't need to be part of a team 
            }
        }


        // Beliefsets in a goal must be in the beliefsets of the agent or its roles
        // We will use BTreeSets here to create two sets of beliefset names and look 
        // at their intersection to determine if all if goal beliefs are covered.
        // Simple :)
        let mut agent_bs : BTreeSet<String> = self.beliefsets.iter().cloned().collect();

        // Find all 
        for role in model.roles.iter() {
            for role_name in self.roles.iter() {
                if &role.name == role_name {
                    for bs in role.beliefsets.iter() {
                        agent_bs.insert(bs.name.clone());
                    }
                }
            }
        }

        for (i, goal) in model.goals.iter().enumerate() {
            let mut goal_bs  : BTreeSet<String> = BTreeSet::new();

            for (j, goal_name) in self.goals.iter().enumerate() {
                if &goal.name == goal_name {
                    for bs in goal.beliefsets.iter() {
                        goal_bs.insert(bs.clone());
                    }

                    let intersection : BTreeSet<String> = goal_bs.intersection(&agent_bs).cloned().collect();

                    if intersection.len() != goal_bs.len() {
                        for bs in intersection.symmetric_difference(&goal_bs) {
                            let mut bs_index : usize = 0; 
                            for (k, gbs) in goal.beliefsets.iter().enumerate() {
                                if bs == gbs {
                                    bs_index = k;
                                    break;
                                }
                            }

                            errors.push(ModelError::new(vec![
                                                                format!("{}s.{}.goals.{}", agent_type, self.name, goal_name),
                                                                format!("goals.{}.beliefsets.{}", goal_name, bs)
                                                        ],
                                                        vec![
                                                                format!("{}s.{}.goals.{}", agent_type, index, j),
                                                                format!("goals.{}.beliefsets.{}", i, bs_index)
                                                        ],
                                                        ErrorType::BeliefsetInGoalNotInAgent,
                                                        format!("Goal {} with beliefset {} cannot be found in agent or agent roles.", goal_name, bs)));
                        }
                    }
                }
            }
        }

        // Agents must have action_handlers for any action in a plan task/body

        // add this agent's action handlers
        for ah in self.action_handlers.iter() {
            all_action_handlers.insert(ah.clone());
        }

        for (i, plan) in model.plans.iter().enumerate() {
            let mut plan_actions : BTreeSet<String> = BTreeSet::new();

            for (j, plan_name) in self.plans.iter().enumerate() {
                if &plan.name == plan_name {
                    for task in plan.tasks.iter() {
                        if task.r#type == TaskType::action {
                            if let Some(message) = task.message.as_ref() {
                                plan_actions.insert(message.clone());
                            }
                        }
                    }

                    let intersection : BTreeSet<String> = all_action_handlers.intersection(&plan_actions).cloned().collect();

                    if intersection.len() != plan_actions.len() {
                        for action in intersection.symmetric_difference(&plan_actions) {
                            let mut act_index : usize = 0; 
                            for (k, task) in plan.tasks.iter().enumerate() {
                                if action == &task.id {
                                    act_index = k;
                                    break;
                                }
                            }

                            errors.push(ModelError::new(vec![
                                                                format!("{}s.{}.plans.{}", agent_type, self.name, plan_name),
                                                                format!("plans.{}.tasks.{}", plan_name, action)
                                                        ],
                                                        vec![
                                                                format!("{}s.{}.plans.{}", agent_type, index, j),
                                                                format!("plans.{}.tasks.{}", i, act_index)
                                                        ],
                                                        ErrorType::ActionHandlersNotInAgent,
                                                        format!("Plan {} with action {} cannot be found in {:?} {} Action Handlers.", plan_name, action, agent_type, self.name)));
                        }
                    }
                }
            }
        }

        // Agents must have the resources required by a goal
        let resources  : BTreeSet<String> = self.resources.iter().cloned().collect();

        for (i, goal) in model.goals.iter().enumerate() {
            for (j, goal_name) in self.goals.iter().enumerate() {
                if &goal.name == goal_name {
                    let goal_resources : BTreeSet<String> = goal.resources.iter().cloned().collect();

                    let intersection : BTreeSet<String> = resources.intersection(&goal_resources).cloned().collect();

                    if intersection.len() != goal_resources.len() {
                        for resource in intersection.symmetric_difference(&goal_resources) {
                            let mut res_index : usize = 0; 
                            for (k, res) in goal.resources.iter().enumerate() {
                                if resource == res {
                                    res_index = k;
                                    break;
                                }
                            }
                            errors.push(ModelError::new(vec![
                                                                format!("{}s.{}.goals.{}", agent_type, self.name, goal.name),
                                                                format!("goals.{}.resources.{}", goal.name, resource)
                                                        ],
                                                        vec![
                                                                format!("{}s.{}.goals.{}", agent_type, index, j),
                                                                format!("goals.{}.resources.{}", i, res_index)
                                                        ],
                                                        ErrorType::AgentGoalNoResource,
                                                        format!("Goal {} with resource {} cannot be found in {:?} {} Resources.", goal.name, resource, agent_type, self.name)));
                        }
                    }
                }
            }
        }

        // Tactics
        for (i, tactic) in self.tactics.iter().enumerate() {
            if tactic.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.tactics.{}", agent_type, self.name, i)],
                                            vec![format!("{}s.{}.tactics.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Tactic at index {} must not be empty.",i)));
            }

            if ! model.find_tactic(tactic) {
                errors.push(ModelError::new(vec![format!("{}s.{}.tactics.{}", agent_type, self.name, tactic)],
                                            vec![format!("{}s.{}.tactics.{}", agent_type, index, i)],
                                            ErrorType::TacticNotFound,
                                            format!("Tactic {} at index {} cannot be found in tactics.", tactic, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.tactics, &format!("{:?}s.{}.tactics", agent_type, self.name), &format!("{:?}s.{}.tactics", agent_type, index), &format!("{:?}", agent_type), &self.name, &"tactics".to_string()));

        errors
    }
}


#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Goals / Desires  
///
pub struct Goal {
    pub name            : String,
    pub precondition    : bool,
    pub dropcondition   : bool,
    pub satisfied       : bool,
    pub heuristic       : bool,
    pub beliefsets      : Vec<String>,
    pub resources       : Vec<String>,
    pub message         : String,
}

impl Goal {
    /// Validation
    /// 1. Ensure beliefsets are non zero length string, not duplicates and there is an associated beliefset message
    /// 2. Ensure there is a pursue type message that matches the message field.
    /// 3. Ensure resources are non zero length string and resource with the same name is found.
    /// 4. Ensure atleast one plan supports goal
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();

        // Beliefset Lookup
        for (i, bs) in self.beliefsets.iter().enumerate() {
            if bs.len() == 0 {
                errors.push(ModelError::new(vec![format!("goals.{}.beliefsets.{}", self.name, i)],
                                            vec![format!("goals.{}.beliefsets.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Beliefset at index {} must not be empty.",i)));
            }

            if ! model.find_message_with_protocol(bs, &Protocol::beliefset) {
                errors.push(ModelError::new(vec![format!("goals.{}.beliefsets.{}", self.name, bs)],
                                            vec![format!("goals.{}.beliefsets.{}", index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Beliefset {} at index {} cannot be found in messages.", bs, i)));
            }
        }

        // TODO: Check if goa message is not an empty string.
        if ! model.find_message_with_protocol(&self.message, &Protocol::goal) {
            errors.push(ModelError::new(vec![format!("goals.{}.message", self.name)],
                                        vec![format!("goals.{}.message", index)],
                                        ErrorType::MessageNotFound,
                                        format!("Goal message with name \"{}\" cannot be found in messages.", self.message)));
        }

        errors.append(&mut verify_no_duplicate_fields(&self.beliefsets, &format!("goals.{}.beliefsets", self.name), &format!("goals.{}.beliefsets", index), &"goals".to_string(), &self.name, &"beliefsets".to_string()));

        // Resource Lookup
        for (i, resource) in self.resources.iter().enumerate() {
            if resource.len() == 0 {
                errors.push(ModelError::new(vec![format!("goals.{}.resources.{}", self.name, i)],
                                            vec![format!("goals.{}.resources.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource at index {} must not be empty.",i)));
            }

            if ! model.find_resource(resource) {
                errors.push(ModelError::new(vec![format!("goals.{}.resources.{}", self.name, resource)],
                                            vec![format!("goals.{}.resources.{}", index, i)],
                                            ErrorType::ResourceNotFound,
                                            format!("Resource {} at index {} cannot be found in resources.", resource, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.resources, &format!("goals.{}.resources", self.name), &format!("goals.{}.resources", index), &"goals".to_string(), &self.name, &"resources".to_string()));

        // One plan supports goal
        let mut handle_cnt : u32 = 0;
        for (_i, plan) in model.plans.iter().enumerate() {
            if let Some(handles) = plan.handles.as_ref() {
                if handles == &self.name {
                    handle_cnt += 1;
                }
            }
        }

        if handle_cnt == 0 {
            errors.push(ModelError::new(vec![format!("goals.{}", self.name)],
                                        vec![format!("goals.{}", index)],
                                        ErrorType::NoPlansForGoal,
                                        format!("Goal {} has no plans that handle it.", self.name)));
        }

        errors
    }
}

#[allow(non_camel_case_types)]
#[derive(Serialize, Deserialize, Debug, Clone, PartialEq)]
/// Tasks that a plan can contain
pub enum TaskType {
    /// Action perform an action.
    action,
    /// Sleep for N milliseconds
    sleep,
    /// sub goal
    goal,
    /// This is how we can do some forms of a dag.
    /// We have a list of tasks.
    /// Tasks between this and an associated wait tasks will be executed in parallel.
    /// Tasks outside that are executed in series.
    node,
    /// this is they synch node that allows all
    wait,
    /// a conditional node
    condition,
    /// Default - error state should not be used.
    None
}

// Implementation of the default trait
impl std::default::Default for TaskType {
    fn default() -> Self {
        Self::None
    }
}

// allowing for this enumeration to be printed normally.
impl std::fmt::Display  for TaskType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}


#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Mapping for task from field -> field
// TODO: I think this is wrong and needs more information on the from side.
//       Mainly, if we have 2 messages that ahve the same field name we are screwed....
pub struct Mapping {
    to      : String,
    from    : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Plan Task
///
pub struct TaskEdge {
    // The which condition is this edge? 
    // TODO: make this an enum
    pub condition       : String,
    // The source task of this edge
    pub sourceid        : String,
    // The target task of this edge
    pub targetid        : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Plan Task
///
pub struct Task {
    /// The id for this task
    /// TODO: this could just be an index
    pub id              : String,
    /// See TaskType for more info
    pub r#type          : TaskType,
    /// Run task asynchronously
    pub nowait          : Option<bool>,
    /// Message that is appending data to the action task
    pub message         : Option<String>,
    /// mapping of fields from message to task 
    pub mappings        : Option<Vec<Mapping>>,
    /// Only used in sleep tasks.
    /// Amount of time in milliseconds to sleep for
    pub duration        : Option<u32>,
    /// Only used in conditional tasks
    /// Temporary textual description of the condition
    /// TODO: this will be replaced with BQL?
    pub conditiontext   : Option<String>,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Plans are a design for how a goal/desire should be fulfilled.
///
pub struct Plan {
    pub name            : String,
    pub precondition    : bool,
    pub dropcondition   : bool,
    pub effects         : bool,
    pub handles         : Option<String>,
    pub beliefsets      : Vec<String>,
    pub tasks           : Vec<Task>,
    pub edges           : Vec<TaskEdge>,
}

impl Plan {
    /// Validation
    /// 1. Checks beliefsets are non zero string.
    /// 2. Checks that beliefset message exists.
    /// 3. Checks that actions have a message.
    /// 4. Checks that pursue tasks have a goal and that goal has a message.
    /// 5. Checks for sleep task having a duration.
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();

        for (i, bs) in self.beliefsets.iter().enumerate() {
            if bs.len() == 0 {
                errors.push(ModelError::new(vec![format!("plans.{}.beliefsets.{}", self.name, i)],
                                            vec![format!("plans.{}.beliefsets.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Beliefset at index {} must not be empty.", i)));
            }

            if ! model.find_message_with_protocol(bs, &Protocol::beliefset) {
                errors.push(ModelError::new(vec![format!("plans.{}.beliefsets.{}", self.name, bs)],
                                            vec![format!("plans.{}.beliefsets.{}", index, i)],
                                            ErrorType::MessageNotFound, 
                                            format!("Beliefset {} at index {} cannot be found in messages.", bs, i)));
            }
        }

        // Ensure a plan handles a goal
        if let Some(handles) = self.handles.as_ref() {
            if !model.find_goal(handles) {
                errors.push(ModelError::new(vec![format!("plans.{}.handles", self.name)],
                                            vec![format!("plans.{}.handles", index)],
                                            ErrorType::GoalNotFound,
                                            format!("Plan {} handles a goal {} that cannot be found.", self.name, handles)));
            }
        } else {
                errors.push(ModelError::new(vec![format!("plans.{}.handles", self.name)],
                                            vec![format!("plans.{}.handles", index)],
                                            ErrorType::PlanMissingGoalToHandle,
                                            format!("Plan {} does not have a goal to handle.", self.name)));
        }

        for (i, task) in self.tasks.iter().enumerate() {
            if task.id.len() == 0 {
                errors.push(ModelError::new(vec![format!("plans.{}.beliefsets.{}", self.name, i)],
                                            vec![format!("plans.{}.beliefsets.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Task at index {} of Tasks in Plan {} cannot have an empty id.", i, self.name)));
            }

            match task.r#type {
                TaskType::action => {
                    // nowait
                    if task.nowait.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskNoWaitNotFound,
                                                    format!("Action task {} at index {} of Tasks in Plan {} does not have nowait field.", task.id, i, self.name)));
                    }

                    // mappings
                    if task.mappings.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskMappingsNotFound,
                                                    format!("Action task {} at index {} of Tasks in Plan {} does not have a mappings field.", task.id, i, self.name)));
                    }

                    match &task.message {
                        Some(msg_name) => {
                            if !model.find_message_with_protocol(&msg_name, &Protocol::action) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::MessageNotFound,
                                                            format!("Action task {} with message name {} at index {} of Tasks in Plan {} cannot be found in messages.", task.id, msg_name, i, self.name)));
                            }
                        },
                        None => {
                            errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                        vec![format!("plans.{}.tasks.{}", index, i)],
                                                        ErrorType::MessageNotFound,
                                                        format!("Action task {} at index {} of Tasks in Plan {} does not have a message defined.", task.id, i, self.name)));
                        }
                    }
                },
                TaskType::goal => {
                    // nowait
                    if task.nowait.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskNoWaitNotFound,
                                                    format!("Goal task {} at index {} of Tasks in Plan {} does not have nowait field.", task.id, i, self.name)));
                    }
                    // mappings
                    if task.mappings.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskMappingsNotFound,
                                                    format!("Goal task {} at index {} of Tasks in Plan {} does not have a mappings field.", task.id, i, self.name)));
                    }

                    match &task.message {
                        Some(goal_name) => {
                            if !model.find_goal(&goal_name) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::GoalNotFound,
                                                            format!("Pursue {} with Goal name {} at index {} of Tasks in Plan {} cannot be found in goals.", task.id, goal_name, i, self.name)));
                            } else {
                                let goal = model.get_goal(&goal_name).unwrap();

                                if !model.find_message_with_protocol(&goal.message, &Protocol::goal) {
                                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                                ErrorType::MessageNotFound,
                                                                format!("Pursue {} with message name {} at index {} of Tasks in Plan {} cannot be found in messages.", task.id, goal_name, i, self.name)));
                                }
                            }
                        },
                        None => {
                            errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                        vec![format!("plans.{}.tasks.{}", index, i)],
                                                        ErrorType::GoalNotFound,
                                                        format!("Pursue {} at index {} of Tasks in Plan {} does not have a goal defined.", task.id, i, self.name)));
                        }
                    }
                },
                TaskType::sleep => {
                    if task.duration.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskSleepDurationNotFound,
                                                    format!("Sleep {} at index {} of Tasks in Plan {} does not have a duration.", task.id, i, self.name)));
                    }
                },
                TaskType::node => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskForFutureUse,
                                                format!("Task {} at index {} of Tasks in Plan {} is for future use.", task.id, i, self.name)));
                },
                TaskType::wait => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskForFutureUse,
                                                format!("Task {} at index {} of Tasks in Plan {} is for future use.", task.id, i, self.name)));
                },
                TaskType::condition => {
                    //TODO: Not sure what to do here
                },
                TaskType::None => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", self.name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskIsNone,
                                                format!("Task {} at index {} of Tasks in Plan {} is None. This should not happen.", task.id, i, self.name)));
                }
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.beliefsets, &format!("plans.{}.beliefsets", self.name), &format!("plans.{}.beliefsets", index), &"plans".to_string(), &self.name, &"beliefsets".to_string()));

        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// The lookup for shared beliefs and the
/// read/write permissions that are are set for the role.
///
pub struct BeliefsetPerm {
    pub name            : String,
    pub read            : bool,
    pub write           : bool,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Roles are what tie Agents to a Team or Subteams to a Team 
///
pub struct Role {
    pub name            : String,
    pub goals           : Vec<String>,
    pub beliefsets      : Vec<BeliefsetPerm>
}

impl Role {
    /// Validation:
    /// 1. Ensure beliefset.name is not zero length.
    /// 2. Ensure there is a beliefset message.
    /// 3. Ensure that beliefsets are not duplicated.
    /// 4. Ensure goals are non zero in length.
    /// 5. Ensure there are goals that are referenced.
    /// 6. Ensure there are no duplicate fields.
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();

        for (i, bs) in self.beliefsets.iter().enumerate() {
            if bs.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("roles.{}.beliefsets.{}", self.name, i)],
                                            vec![format!("roles.{}.beliefsets.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Beliefset at index {} must not be empty.",i)));
            }

            if ! model.find_message_with_protocol(&bs.name, &Protocol::beliefset) {
                errors.push(ModelError::new(vec![format!("roles.{}.beliefsets.{}", self.name, bs.name)],
                                            vec![format!("roles.{}.beliefsets.{}", index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Beliefset {} at index {} cannot be found in messages.", bs.name, i)));
            }
        }

        let mut names : BTreeMap<String, usize> = BTreeMap::new();

        for (i, bs) in self.beliefsets.iter().enumerate() {
            if names.contains_key(&bs.name) {
                let last_index = names.get(&bs.name).unwrap();

                errors.push(ModelError::new(vec![format!("roles.{}.beliefsets.{}", self.name, bs.name)],
                                            vec![
                                                    format!("roles.{}.beliefsets.{}", index, last_index),
                                                    format!("roles.{}.beliefsets.{}", index, i)
                                            ],
                                            ErrorType::DuplicateName,
                                            format!("Role at {} in beliefsets has duplicate entries of \"{}\" at indicies {} and {}.", self.name, bs.name, last_index, i)));
            } else {
                names.insert(bs.name.clone(), i.clone());
            }
        }

        for (i, goal) in self.goals.iter().enumerate() {
            if goal.len() == 0 {
                errors.push(ModelError::new(vec![format!("roles.{}.goals.{}", self.name, i)],
                                            vec![format!("roles.{}.goals.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("goal at index {} must not be empty.",i)));
            }

            if ! model.find_goal(goal) {
                errors.push(ModelError::new(vec![format!("roles.{}.goals.{}", self.name, goal)],
                                            vec![format!("roles.{}.goals.{}", index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", goal, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.goals, &format!("roles.{}.goals", self.name), &format!("roles.{}.goals", index), &"roles".to_string(), &self.name, &"goals".to_string()));

        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// A resource is a constrained object that can be used.
/// Currently represented as an object that can have a minimum and maximum value.
///
pub struct Resource {
    /// Name of resource
    pub name            : String,
    /// Type of resource value
    pub r#type          : String,
    /// Moninum value
    pub min             : f32,
    /// Maximum value
    pub max             : f32
}

impl Resource {
    /// There is only one rule that must be considered for Resources -- max > min.
    /// As a resource is a supply that can be used.
    ///
    fn verify(&self, _model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();

        if self.min > self.max {
            errors.push(ModelError::new(vec![format!("resources.{}", self.name)],
                                        vec![format!("resources.{}", index)],
                                        ErrorType::ResourceMinMaxSwapped,
                                        format!("Resouce has its min and max values swapped.")));
        }

        if self.min == self.max {
            errors.push(ModelError::new(vec![format!("resources.{}", self.name)],
                                        vec![format!("resources.{}", index)],
                                        ErrorType::ResourceMinMaxEqual,
                                        format!("Resouce has its min and max values that are equal.")));
        }

        errors
    }
}


#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// A tactic is a plan filter for an individual goal.
///
pub struct Tactic {
    /// Name of tactic.
    pub name            : String,
    /// Goal the tactic supports.
    pub goal            : String,
    /// Plans that the tactic allows.
    pub plans           : Vec<String>,
}

impl Tactic {
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();

        if self.goal.len() == 0 {
            errors.push(ModelError::new(vec![format!("tactics.{}.goal", self.name)],
                                        vec![format!("tactics.{}.goal", index)],
                                        ErrorType::ZeroSizeString,
                                        format!("Goal handle at must not be empty.")));
        }

        if ! model.find_goal(&self.goal) {
            errors.push(ModelError::new(vec![format!("tactics.{}.goal", self.name)],
                                        vec![format!("tactics.{}.goal", index)],
                                        ErrorType::GoalNotFound,
                                        format!("Goal {} that tactic handles cannot be found in goals.", self.goal)));
        }


        for (i, plan) in self.plans.iter().enumerate() {
            if plan.len() == 0 {
                errors.push(ModelError::new(vec![format!("tactics.{}.plans.{}", self.name, i)],
                                            vec![format!("tactics.{}.plans.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan at index {} must not be empty.",i)));
            }

            if ! model.find_plan(&plan) {
                errors.push(ModelError::new(vec![format!("tactics.{}.plans.{}", self.name, plan)],
                                            vec![format!("tactics.{}.plans.{}", index, i)],
                                            ErrorType::PlanNotFound,
                                            format!("Plan {} at index {} cannot be found in plans.", plan, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.plans, &format!("tactics.{}.plans", self.name), &format!("tactics.{}.plans", index), &"tactics".to_string(), &self.name, &"plans".to_string()));

        errors
    }
}


#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// The JACK Model Structure.
/// This structure is used as the main object for model files to be pulled into
/// jack-make and jack-validator.
///
/// Code generation and Validation uses this as the main data store for objects.
///
/// 
pub struct Model {
    pub project         : Project,
    pub teams           : Vec<Agent>,
    pub agents          : Vec<Agent>,
    pub services        : Vec<Service>,
    pub messages        : Vec<Message>,
    pub goals           : Vec<Goal>,
    pub plans           : Vec<Plan>,
    pub roles           : Vec<Role>,
    pub resources       : Vec<Resource>,
    pub tactics         : Vec<Tactic>,
}

impl Model {
    /// Main validation function.
    /// Outputs a list of ModelErrors.
    /// All other verify functions that are unused in other parts of the model are called
    /// in this function.
    pub fn verify(&self) -> Vec<ModelError> {
        
        // Names is a map. The key is what was used for the set.
        // The value now contains a tuple (could be a struct) 
        // which contains the named index and the numerical index.
        let mut names : BTreeMap<String,(String, String)> = BTreeMap::new();

        let mut errors : Vec<ModelError> = self.project.verify();

        for (i,team) in self.teams.iter().enumerate() {
            // Check for zero length team name
            if team.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("teams.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Team name at index {} must not be empty.",i)));
            }

            // Check if name is "Team" -> This leads to code generation errors
            if team.name == "Team" {
                errors.push(ModelError::new(vec![format!("teams.{}", team.name)],
                                            vec![format!("teams.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Team name at index {} must not be \"Team\".",i)));
            }

            // Check for duplicate team name
            if names.contains_key(&team.name) {
                errors.push(ModelError::new(vec![names.get(&team.name).unwrap().0.clone(), format!("teams.{}", team.name)],
                                            vec![names.get(&team.name).unwrap().1.clone(), format!("teams.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Team at index {} must have a unique name.",i)));
            } else {
                names.insert(team.name.clone(), (format!("teams.{}", team.name), format!("teams.{}", i)));
            }

            // Add errors from Team/Agent specific rules
            errors.append(&mut team.verify(&AgentType::Team, &self, &i));
        }

        for (i,agent) in self.agents.iter().enumerate() {
            
            // Check for zero length agent name
            if agent.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("agents.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Agent name at index {} must not be empty.",i)));
            }

            // Check if name is "Agent" -> This leads to code generation errors
            if agent.name == "Agent" {
                errors.push(ModelError::new(vec![format!("agents.{}", agent.name)],
                                            vec![format!("agents.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Agent name at index {} must not be \"Agent\".",i)));
            }

            // Check for duplicate agent name
            if names.contains_key(&agent.name) {
                errors.push(ModelError::new(vec![names.get(&agent.name).unwrap().0.clone(), format!("agents.{}", agent.name)],
                                            vec![names.get(&agent.name).unwrap().1.clone(), format!("agents.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Agent at index {} must have a unique name.",i)));
            } else {
                names.insert(agent.name.clone(), (format!("agents.{}", agent.name), format!("agents.{}", i)));
            }

            // Add errors from Team/Agent specific rules
            errors.append(&mut agent.verify(&AgentType::Agent, &self, &i));
        }

        for (i, msg) in self.messages.iter().enumerate() {
            // Check for zero length message name
            if msg.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("messages.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message name at index {} must not be empty.",i)));
            }

            // Check if name is "Message" -> This leads to code generation errors
            if msg.name == "Message" {
                errors.push(ModelError::new(vec![format!("messages.{}", msg.name)],
                                            vec![format!("messages.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Message name  not be \"Message\".")));
            }

            // Adding this just incase it could also cause code generation issues
            // TODO: Check with james
            let protos : Vec<Protocol> = vec![Protocol::action, Protocol::adhoc, Protocol::beliefset, Protocol::goal, Protocol:: percept];
            for proto in protos.iter() {
                if msg.name == format!("{:?}", proto) {
                    errors.push(ModelError::new(vec![format!("messages.{}", msg.name)],
                                                vec![format!("messages.{}", i)],
                                                ErrorType::NameSameAsMessageType,
                                                format!("Message name must not be \"{:?}\".", proto)));

                }
            }

            // Check for duplicate message name
            // NOTE: We're introduced a percept message for the 0.4 model to allow for reply messages from an action
            // These message protols will be removed for model 0.5
            // for now this duplicate name test needs to be removed

            // if names.contains_key(&msg.name) {
            //     errors.push(ModelError::new(vec![ names.get(&msg.name).unwrap().0.clone(), format!("messages.{}", msg.name)],
            //                                 vec![ names.get(&msg.name).unwrap().1.clone(), format!("messages.{}", i)],
            //                                 ErrorType::DuplicateName,
            //                                 format!("Message at index {} must have a unique name.",i)));
            // } else {
            //     names.insert(msg.name.clone());
            // }

            // still add the name, duplicate or not
            names.insert(msg.name.clone(),  (format!("messages.{}", msg.name), format!("messages.{}", i)));

            // Add errors from message specific rules
            errors.append(&mut msg.verify(&self, &i));
        }

        for (i, goal) in self.goals.iter().enumerate() {

            // Check for zero length goal name
            if goal.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("goals.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal name at index {} must not be empty.", i)));
            }

            // Check for duplicate goal name
            if names.contains_key(&goal.name) {
                errors.push(ModelError::new(vec![names.get(&goal.name).unwrap().0.clone(), format!("goals.{}", goal.name)],
                                            vec![names.get(&goal.name).unwrap().1.clone(), format!("goals.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Goal name {} must have a unique name.", goal.name)));
            } else {
                names.insert(goal.name.clone(), (format!("goals.{}", goal.name), format!("goals.{}", i)));
            }

            // Check if name is "Goal" -> This leads to code generation errors
            if goal.name == "Goal" {
                errors.push(ModelError::new(vec![format!("goals.{}", goal.name)],
                                            vec![format!("goals.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Goal name must not be \"Goal\".")));
            }

            // Add errors from goal specific rules
            errors.append(&mut goal.verify(&self, &i));
        }

        for (i, plan) in self.plans.iter().enumerate() {
 
            // Check for zero length plan name
            if plan.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("plans.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan name at index {} must not be empty.",i)));
            }

            // Check for duplicate plan name
            if names.contains_key(&plan.name) {
                errors.push(ModelError::new(vec![names.get(&plan.name).unwrap().0.clone(), format!("plans.{}", plan.name)],
                                            vec![names.get(&plan.name).unwrap().1.clone(), format!("plans.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Plan with name {} must have a unique name.", plan.name)));
            } else {
                names.insert(plan.name.clone(), (format!("plans.{}", plan.name), format!("plans.{}", i)));
            }

            // Check if name is "Plan" -> This leads to code generation errors
            if plan.name == "Plan" {
                errors.push(ModelError::new(vec![format!("plans.{}", plan.name)],
                                            vec![format!("plans.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Plan name must not be \"Plan\".")));
            }

            // Add errors from plan specific rules
            errors.append(&mut plan.verify(&self, &i));
        }

        for (i, role) in self.roles.iter().enumerate() {
 
            // Check for zero length role name
            if role.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("roles.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Role name at index {} must not be empty.",i)));
            }

            // Check for duplicate role name
            if names.contains_key(&role.name) {
                errors.push(ModelError::new(vec![names.get(&role.name).unwrap().0.clone(), format!("roles.{}", role.name)],
                                            vec![names.get(&role.name).unwrap().1.clone(), format!("roles.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Role with name {} must have a unique name.", role.name)));
            } else {
                names.insert(role.name.clone(), (format!("roles.{}", role.name), format!("roles.{}", i)));
            }

            // Check if name is "Role" -> This leads to code generation errors
            if role.name == "Role" {
                errors.push(ModelError::new(vec![format!("roles.{}", i)],
                                            vec![format!("roles.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Role name must not be \"Role\".")));
            }

            // Add errors from role specific rules
            errors.append(&mut role.verify(&self, &i));
        }

        for (i, resource) in self.resources.iter().enumerate() {
            // Check for zero length resource name
            if resource.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("resources.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource name at index {} must not be empty.",i)));
            }

            // Check for duplicate resource name
            if names.contains_key(&resource.name) {
                errors.push(ModelError::new(vec![names.get(&resource.name).unwrap().0.clone(), format!("resources.{}", resource.name)],
                                            vec![names.get(&resource.name).unwrap().1.clone(), format!("resources.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Resource with name {} must have a unique name.", resource.name)));
            } else {
                names.insert(resource.name.clone(), (format!("resources.{}", resource.name), format!("resources.{}", i)));
            }

            // Check if name is "Resource" -> This leads to code generation errors
            if resource.name == "Resource" {
                errors.push(ModelError::new(vec![format!("resources.{}", resource.name)],
                                            vec![format!("resources.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Resource name must not be \"Resource\".")));
            }

            // Add errors from resource specific rules
            errors.append(&mut resource.verify(&self, &i));
        }

        for (i, tactic) in self.tactics.iter().enumerate() {
            // Check for zero length tactic name
            if tactic.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("tactics.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Tactic name at index {} must not be empty.",i)));
            }

            // Check for duplicate tactic name
            if names.contains_key(&tactic.name) {
                errors.push(ModelError::new(vec![names.get(&tactic.name).unwrap().0.clone(), format!("tactics.{}", tactic.name)],
                                            vec![names.get(&tactic.name).unwrap().1.clone(), format!("tactics.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Tactic name {} must have a unique name.", tactic.name)));
            } else {
                names.insert(tactic.name.clone(),  (format!("tactics.{}", tactic.name), format!("tactics.{}", i)));
            }

            // Check if name is "Tactic" -> This leads to code generation errors
            if tactic.name == "Tactic" {
                errors.push(ModelError::new(vec![format!("tactics.{}", tactic.name)],
                                            vec![format!("tactics.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Tactic name must not be \"Tactic\".")));
            }

            // Add errors from tactic specific rules
            errors.append(&mut tactic.verify(&self, &i));
        }

        for (i, service) in self.services.iter().enumerate() {
            // Check for zero length service name
            if service.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("services.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Service name at index {} must not be empty.",i)));
            }

            // Check for duplicate service name
            if names.contains_key(&service.name) {
                errors.push(ModelError::new(vec![names.get(&service.name).unwrap().0.clone(), format!("services.{}", service.name)],
                                            vec![names.get(&service.name).unwrap().1.clone(), format!("services.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Service with name {} must have a unique name.", service.name)));
            } else {
                names.insert(service.name.clone(),  (format!("services.{}", service.name), format!("services.{}", i)));
            }

            // Check if name is "Tactic" -> This leads to code generation errors
            if service.name == "Service" {
                errors.push(ModelError::new(vec![format!("services.{}", service.name)],
                                            vec![format!("services.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Service name must not be \"Service\".")));
            }

            // Add errors from service specific rules
            // TODO: No rules to verify services yet?!?
            //errors.append(&mut service.verify(&self, &i));
        }

        // return errors
        errors
    }

    /// Returns true if there is a message with the name and protocol supplied.
    pub fn find_message_with_protocol(&self, name : &str, protocol : &Protocol) -> bool {
        for msg in self.messages.iter() {
            if &msg.name == name && &msg.protocol == protocol {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a goal with the name supplied.
    pub fn find_goal(&self, name : &str) -> bool {
        for goal in self.goals.iter() {
            if &goal.name == name {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a plan with the name supplied.
    pub fn find_plan(&self, name : &str) -> bool {
        for plan in self.plans.iter() {
            if &plan.name == name {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a resource with the name supplied.
    pub fn find_resource(&self, name : &str) -> bool {
        for resource in self.resources.iter() {
            if &resource.name == name {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a role with the name supplied.
    pub fn find_role(&self, name : &str) -> bool {
        for role in self.roles.iter() {
            if &role.name == name {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a service with the name supplied.
    pub fn find_service(&self, name : &str) -> bool {
        for service in self.services.iter() {
            if &service.name == name {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a tactic with the name supplied.
    pub fn find_tactic(&self, name : &str) -> bool {
        for tactic in self.tactics.iter() {
            if &tactic.name == name {
                return true;
            }
        }

        false
    }

    /// Returns a result::Ok(Goal) if there is a goal with the name supplied.
    /// If no goal is found, an result::Err(String) is returned.
    pub fn get_goal(&self, name : &str) -> Result<Goal, Box<dyn std::error::Error>> {
        for goal in self.goals.iter() {
            if &goal.name == name {
                return Ok(goal.clone());
            }
        }

        Err(format!("Could not find Goal with name {}", name).into())
    }

    /// Returns a result::Ok(Plan) if there is a plan with the name supplied.
    /// If no plan is found, an result::Err(String) is returned.
    pub fn get_plan(&self, name : &str) -> Result<Plan, Box<dyn std::error::Error>> {
        for plan in self.plans.iter() {
            if &plan.name == name {
                return Ok(plan.clone());
            }
        }

        Err(format!("Could not find plan with name {}", name).into())
    }

    /// Returns a result::Ok(Role) if there is a role with the name supplied.
    /// If no role is found, an result::Err(String) is returned.
    pub fn get_role(&self, name : &str) -> Result<Role, Box<dyn std::error::Error>> {
        for role in self.roles.iter() {
            if &role.name == name {
                return Ok(role.clone());
            }
        }

        Err(format!("Could not find Role with name {}", name).into())
    }
}

#[derive(Serialize, Deserialize, Eq, PartialEq, Debug, Clone)]
/// This is an enumeration of all the possible ModelErrors that could be found in the JACK Model.
pub enum ErrorType {
    /// Serde cannot deserialise the model from input.
    /// There is an issue with the JSON being presented to jack-make / jack-validator.
    JsonDeserialisationFailure,
    /// Serde cannot serialise the model to output.
    /// There is an issue with the JSON being created by jack-make / jack-validator.
    JsonSerialisationFailure,
    /// There is a string for a field that has no content.
    ZeroSizeString,
    /// There is a message referenced in the model that cannot be found in the messages list.
    MessageNotFound,
    /// There are two fields with the same name in a message.
    MessageDuplicateFields,
    /// Sleep tasks require the duration field to be complete.
    TaskSleepDurationNotFound,
    /// Action or Goal task are missing nowait field
    TaskNoWaitNotFound,
    /// Action or Goal task are missing mappings field
    TaskMappingsNotFound,
    /// The task is missing a field in the model file.
    TaskMissingField,
    /// Task type has not been implemented and will be later.
    TaskForFutureUse,
    /// TaskType::None is an error condition for the model.
    TaskIsNone,
    /// There is a goal referenced in the model that cannot be found in the goals list.
    GoalNotFound,
    /// There is a plan referenced in the model that cannot be found in the plans list.
    PlanNotFound,
    /// The plan does not handle a goal.
    PlanMissingGoalToHandle,
    /// There is a resource referenced in the model that cannot be found in the resources list.
    ResourceNotFound,
    /// The resource minimum and maximum numbers are in the wrong fields.
    ResourceMinMaxSwapped,
    /// The resource minimum and maximum numbers are the same value.
    ResourceMinMaxEqual,
    /// There is a role referenced in the model that cannot be found in the roles list.
    RoleNotFound,
    /// There is a tactic referenced in the model that cannot be found in the tactics list.
    TacticNotFound,
    /// There are two objects in the model with the same name.
    DuplicateName,
    /// The goal has no plans that support it.
    NoPlansForGoal,
    /// The goal has no plans that can pursue it.
    NoPlansCanPursueGoal,
    /// The goal has a beliefset that is not in the agent that has that goal.
    BeliefsetInGoalNotInAgent,
    /// The agent does not handle an action that is needs to support for a plan.
    ActionHandlersNotInAgent,
    /// A role in a team is not also in any agent sor other teams.
    TeamRoleNoAgentsOrTeams,
    /// Agents must have the resources required by a goal
    AgentGoalNoResource,
    /// A beliefset that is nested under another beliefset could not be found.
    CustomTypeNotFound,
    /// The Service must exist in the model
    ServiceNotFound,
    /// The name of the model object is the same as the type.
    NameSameAsType,
    /// The name of the message is the same as the type of the message.
    NameSameAsMessageType,
    /// The custom type for a field cannot be the message that holds it. 
    CustomTypeIsCurrentMessage,
    /// This is an error state. Should never been used in normal operation.
    None
}

impl std::default::Default for ErrorType {
    fn default() -> Self {
        Self::None
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Structure for returning errors in JSON to the modellig tool or printed out to console. 
pub struct ModelError {
    /// Human readable model locations for the error.
    pub keys            : Vec<String>,
    /// Machine readable model location for the error.
    pub machine_keys    : Vec<String>,
    /// Machine readable error enumeration.
    pub error_type      : ErrorType,
    /// Human readable error string.
    pub error_string    : String,
}

impl ModelError {
    /// Create an new error
    /// keys is the human readable addresses of fields where there are issues in the model.
    /// machine_keys is the machine readable address as above.
    /// error_type is the machine readable error code.
    /// error_string is the human readable error.
    pub fn new(keys : Vec<String>, machine_keys : Vec<String>, error_type : ErrorType, error_string : String) -> Self {
        Self { keys, machine_keys, error_type, error_string }
    }
}

/// This function is used to ensure duplicates are not found in any of the lists that reference other objects in the model.
/// It requires input information about the parent object ids, the field that is being looked at, the type and the name.
/// returns a list of ModelErrors.
pub fn verify_no_duplicate_fields(input : &Vec<String>, field_id : &String, machine_id : &String, object_type : &String, object_name : &String, field_name : &String) -> Vec<ModelError> {

    // Using a BTreeMap (think C++ std::map) to store the name in a list 
    // with its position in the list as the value.
    let mut names : BTreeMap<String, usize> = BTreeMap::new();

    let mut errors : Vec<ModelError> = Vec::new();

    // Creates an iterator that is enumerated.
    // i is the position in the list (the enumerated value)
    // name is the object in the list (in this case a string)
    for (i, name) in input.iter().enumerate() {

        // looks or the name in the map
        // if found create an error
        if names.contains_key(name) {
            // 
            let last_index = names.get(name).unwrap();

            // Push back ModelError struct in the the errors vec
            // format! is a macro that creates a string that can have data pushed into it.
            // similar in function to printf
            errors.push(ModelError::new(vec![format!("{}.{}", field_id, name)],
                                        vec![
                                                format!("{}.{}", machine_id, i),
                                                format!("{}.{}", machine_id, last_index)
                                        ],
                                        ErrorType::DuplicateName,
                                        format!("{} at {} in {} has duplicate entries of \"{}\" at indicies {} and {}.", object_type, object_name, field_name, name, last_index, i)));
        } else {
            // Otherwise not found, insert the name into the names map.
            names.insert(name.clone(), i.clone());
        }
    }

    errors
}

