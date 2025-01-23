use serde::{Serialize, Deserialize};
use std::collections::*;

pub fn convert_to_bumpy_case(value: &String) -> String {
    let mut result: String = Default::default();
    result.reserve_exact(value.len());
    let mut prev_space_encountered = false;
    for ch in value.chars() {
        if ch == ' ' {
            prev_space_encountered = true;
        } else {
            if prev_space_encountered {
                let upper_ch = ch.to_ascii_uppercase();
                result.push(upper_ch);
            } else {
                result.push(ch);
            }
            prev_space_encountered = false;
        }
    }
    return result;
}


#[derive(Serialize, Deserialize, Eq, Default, Debug, Clone)]
pub struct Identifier {
    pub name : String,
    pub module : String,
    #[serde(skip)]
    pub bumpy_case: String,
}

impl Identifier {
    pub fn qualified_name(&self) -> String {
        let mut result: String = Default::default();
        let namespace_divider = ".";
        result.reserve(self.name.len() + namespace_divider.len() + self.module.len());
        if self.module.len() > 0 {
            result.push_str(self.module.as_str());
            result.push_str(namespace_divider);
        }
        result.push_str(self.name.as_str());
        result
    }
}

impl Ord for Identifier {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        let self_name = self.qualified_name();
        let other_name = other.qualified_name();
        let result = self_name.cmp(&other_name);
        result
    }
}

impl AsRef<Identifier> for Identifier {
    fn as_ref(&self) -> &Identifier {
        &self
    }
}

impl PartialOrd for Identifier {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}


impl PartialEq for Identifier {
    fn eq(&self, other: &Self) -> bool {
        self.qualified_name() == other.qualified_name()
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Module {
    pub name     : String,
    pub filepath : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Project Data
/// Currently only stores project name and any c++ namespaces that the codegenerator should use.
pub struct Project {
    pub name           : String,
    #[serde(skip_deserializing, skip_serializing)]
    pub bumpy_case     : String,
    pub namespaces     : Vec<String>,
    pub major_version  : Option<String>,
    pub minor_version  : Option<String>,
    pub patch_version  : Option<String>,
    pub generator      : Option<String>,
    pub modules        : Vec<Module>,
    pub search_paths   : Vec<String>,
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

#[derive(Serialize, Deserialize, Debug, Clone, PartialEq, Eq)]
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
    Custom(Identifier),
    /// Enumeration
    Enum(Identifier),
    /// Default error case. Never used in real operations.
    None  // Error case
}

impl FieldType { }

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
    pub name     : String,

    /// An optional note attached to each message field
    #[serde(skip_serializing_if = "Option::is_none")]
    pub note : Option<String>,

    // type is a reserved keyword in rust
    // r# stops it being a keyword
    /// Is the FieldType please look at the definition for further information.
    pub r#type   : FieldType,
    /// Default value of the field, empty string if no default value.
    pub default  : String,
    /// Defines whether the field represents an array of fields of 'type'
    pub is_array : bool
}

impl Field { }

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Structure that contains all message types in the Model.
pub struct Message {
    /// Name of message.
    #[serde(skip)]
    pub id     : Identifier,
    pub name   : String,

    /// An optional note attached to each message
    #[serde(skip_serializing_if = "Option::is_none")]
    pub note : Option<String>,

    /// Definition of the containing fields.
    pub fields : Vec<Field>
}

impl Message {
    /// Verficiation for Messages:
    /// 1. Checking that there are no fields with the same duplicate fields.
    /// 2. If a field has a custom type, whether the model contains the message required.
    ///
    fn verify(&self, model : &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];
        let mut field_names : BTreeMap<String, usize> = BTreeMap::new();
        let msg_full_name = &self.id.qualified_name();
 
        for (i, field) in self.fields.iter().enumerate() {
            // Check if field name is contained in previous field names.
            // if found produce error.
            if field_names.contains_key(&field.name) {
                let other_index = field_names.get(&field.name).unwrap();

                // Adding both keys/indexes for errors to hopefully help the GUI.
                // *POSSIBLE ISSUE* If there is 3 or more of the same, it will only show the first and the current issue.
                // I think this is good enough.
                errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", msg_full_name, i),
                                                 format!("messages.{}.fields.{}", msg_full_name, other_index)],
                                            vec![format!("messages.{}.fields.{}", index, i),
                                                 format!("messages.{}.fields.{}", msg_full_name, other_index)],
                                            ErrorType::MessageDuplicateFields,
                                            format!("Duplicate fields with name {} in Message {}.", field.name, msg_full_name)));
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
                        if &msg.id.qualified_name() == &str_type.qualified_name() {
                            found = true;
                        }
                    }

                    if !found {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", msg_full_name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeNotFound,
                                                    format!("Custom field {} at index {} in Message {} cannot be found in Messages.",
                                                            str_type.qualified_name(),
                                                            i,
                                                            msg_full_name)));
                    }

                    if &str_type.qualified_name() == msg_full_name {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", msg_full_name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeIsCurrentMessage,
                                                    format!("Custom field {} at index {} in Message {} cannot be current Message.",
                                                            str_type.qualified_name(),
                                                            i,
                                                            msg_full_name)));

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
   #[serde(skip)]
   pub id              : Identifier,
   pub name            : String,

   /// An optional note attached to each service
   #[serde(skip_serializing_if = "Option::is_none")]
   pub note            : Option<String>,

   pub action_handlers : Vec<Identifier>,
   pub messages        : Vec<Identifier>,
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
    #[serde(skip)]
    pub id               : Identifier,
    pub name             : String,
    pub action_handlers  : Vec<Identifier>,
    pub beliefs          : Vec<Identifier>,
    pub goals            : Vec<Identifier>,
    pub initial_goals    : Vec<Identifier>,
    pub plans            : Vec<Identifier>,
    pub resources        : Vec<Identifier>,
    pub roles            : Vec<Identifier>,
    pub tactics          : Vec<Identifier>,
    pub message_handlers : Vec<Identifier>,
    pub services         : Vec<Identifier>,
}

impl Agent {
    /// Most of the error handling that is being done is ensuring that the model is referencing the 
    /// right things in other parts of the model and that the references are not zero length strings.
    /// 1. Action handlers, Beliefsets and Message Handlers are all looking for things in the message list.
    /// 2. Goals, Initial Goals, Plans, Resources, Roles and Tactics are also looking for objects witht he same name.
    /// 3. For Teams -> Roles also have extra checks that looks for agents or plans that can complete it.
    /// 4. Agent beliefs must also be in the goals that it has. 
    /// 5. Agents must have action_handlers for any action in a plan task/body
    /// 6. Agents must have the resources required by a goal
    fn verify(&self, agent_type : &AgentType, model : &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];
        let full_name = &self.id.qualified_name();

        // Action Handlers
        for (i, action_id) in self.action_handlers.iter().enumerate() {
            if action_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.action_handlers.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.action_handlers.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Action at index {} must not be empty.",i)));
            }

            if !model.find_action(action_id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.action_handlers.{}", agent_type, full_name, action_id.qualified_name())],
                                            vec![format!("{}s.{}.action_handlers.{}", agent_type, index, i)],
                                            ErrorType::ActionNotFound,
                                            format!("Action {} at index {} cannot be found in messages.", action_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.action_handlers,
                                                      &format!("{}s.{}.action_handlers", agent_type, full_name),
                                                      &format!("{}s.{}.action_handlers", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"action_handlers".to_string()));

        // Beliefs
        for (i, bs_id) in self.beliefs.iter().enumerate() {
            if bs_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.beliefs.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.beliefs.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Belief at index {} must not be empty.",i)));
            }

            if !model.find_message(bs_id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.beliefs.{}", agent_type, full_name, bs_id.qualified_name())],
                                            vec![format!("{}s.{}.beliefs.{}", agent_type, index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Belief {} at index {} cannot be found in messages.", bs_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.beliefs,
                                                      &format!("{}s.{}.beliefs", agent_type, full_name),
                                                      &format!("{}s.{}.beliefs", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      &full_name,
                                                      &"beliefsets".to_string()));

        // Message Handlers
        for (i, msg_id) in self.message_handlers.iter().enumerate() {
            if msg_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.message_handlers.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.message_handlers.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message at index {} must not be empty.",i)));
            }

            if !model.find_message(msg_id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.message_handlers.{}", agent_type, full_name, msg_id.qualified_name())],
                                            vec![format!("{}s.{}.message_handlers.{}", agent_type, index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", msg_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.message_handlers,
                                                      &format!("{}s.{}.message_handlers", agent_type, full_name),
                                                      &format!("{}s.{}.message_handlers", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      &full_name,
                                                      &"message_handlers".to_string()));

        // goals
        for (i, goal_id) in self.goals.iter().enumerate() {
            if goal_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal at index {} must not be empty.",i)));
            }

            if !model.find_goal(goal_id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, goal_id.qualified_name())],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", goal_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.goals,
                                                      &format!("{}s.{}.goals", agent_type, full_name),
                                                      &format!("{}s.{}.goals", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"goals".to_string()));

        // initial_goals
        for (i, goal) in self.initial_goals.iter().enumerate() {
            if goal.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.initial_goals.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.initial_goals.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Initial Goal at index {} must not be empty.",i)));
            }

            if !model.find_goal(goal) {
                errors.push(ModelError::new(vec![format!("{}s.{}.initial_goals.{}", agent_type, full_name, goal.qualified_name())],
                                            vec![format!("{}s.{}.initial_goals.{}", agent_type, index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Initial Goal {} at index {} cannot be found in goals.", goal.qualified_name(), i)));
            }
        }
        errors.append(&mut verify_no_duplicate_fields(&self.initial_goals,
                                                      &format!("{}s.{}.initial_goals", agent_type, full_name),
                                                      &format!("{}s.{}.initial_goals", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"initial_goals".to_string()));

        // Plans
        for (i, plan) in self.plans.iter().enumerate() {
            if plan.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.plans.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.plans.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan at index {} must not be empty.",i)));
            }

            if !model.find_plan(plan) {
                errors.push(ModelError::new(vec![format!("{}s.{}.plans.{}", agent_type, full_name, plan.qualified_name())],
                                            vec![format!("{}s.{}.plans.{}", agent_type, index, i)],
                                            ErrorType::PlanNotFound,
                                            format!("Plan {} at index {} cannot be found in plans.", plan.qualified_name(), i)));
            }
        }
        errors.append(&mut verify_no_duplicate_fields(&self.plans,
                                                      &format!("{}s.{}.plans", agent_type, full_name),
                                                      &format!("{}s.{}.plans", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"plans".to_string()));

        // Resources
        for (i, resource) in self.resources.iter().enumerate() {
            if resource.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.resources.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.resources.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource at index {} must not be empty.",i)));
            }

            if !model.find_resource(resource) {
                errors.push(ModelError::new(vec![format!("{}s.{}.resources.{}", agent_type, full_name, resource.qualified_name())],
                                            vec![format!("{}s.{}.resources.{}", agent_type, index, i)],
                                            ErrorType::ResourceNotFound,
                                            format!("Resource {} at index {} cannot be found in resources.", resource.qualified_name(), i)));
            }
        }
        errors.append(&mut verify_no_duplicate_fields(&self.resources,
                                                      &format!("{}s.{}.resources", agent_type, full_name),
                                                      &format!("{}s.{}.resources", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"resources".to_string()));

        // Roles
        for (i, role) in self.roles.iter().enumerate() {
            if role.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Role at index {} must not be empty.",i)));
            }

            if !model.find_role(role) {
                errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, full_name, role.qualified_name())],
                                            vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                            ErrorType::RoleNotFound,
                                            format!("Role {} at index {} cannot be found in roles.", role.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.roles,
                                                      &format!("{}s.{}.roles", agent_type, full_name),
                                                      &format!("{}s.{}.roles", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"roles".to_string()));

        // The service action handlers
        let mut all_action_handlers = BTreeSet::<String>::new();

        // Services
        for (i, service_id) in self.services.iter().enumerate() {
            if service_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.services.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.services.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Service at index {} must not be empty.",i)));
            }

            if let Some(service) = model.get_service(service_id) {
                // Add this service's action handlers
                for handler in service.action_handlers.iter() {
                    all_action_handlers.insert(handler.qualified_name().clone());
                }
            } else {
                errors.push(ModelError::new(vec![format!("{}s.{}.services.{}", agent_type, full_name, service_id.qualified_name())],
                                            vec![format!("{}s.{}.services.{}", agent_type, index, i)],
                                            ErrorType::ServiceNotFound,
                                            format!("Service {} at index {} cannot be found in services.", service_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.services,
                                                      &format!("{}s.{}.services", agent_type, full_name),
                                                      &format!("{}s.{}.services", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"services".to_string()));

        // Verify one atleast one agent can do each role for the team
        // \todo This check is wrong, we need to check if the team can handle the role. If they
        // can't handle the role, they need to check there is atleast one agent that can handle the
        // role instead (e.g. allow the team to delegate to it as long as they're in the same
        // team).
        //
        // This check assumes that teams always delegate the role.
        if *agent_type == AgentType::Team {
            for (i, role_id) in self.roles.iter().enumerate() {
                let mut role_cnt : u32 = 0;

                // \note Search the model for agents that support this role
                for agent in model.agents.iter() {
                    for agent_role in agent.roles.iter() {

                        // \note Found an agent that supports the role
                        if agent_role.qualified_name() == role_id.qualified_name() {
                            role_cnt += 1;
                            continue;
                        }

                        let get_role_result = model.get_role(role_id);
                        if get_role_result.is_err() {
                            continue; // Missing role is logged in a prior check
                        }

                        // \note Check if the agent has a plan that supports the role
                        // \todo This looks incorrect? We should check that the agent indeed has
                        // a goal to support the role as well as checking that it has the role.
                        let role = get_role_result.unwrap();
                        for plan_name in agent.plans.iter() {
                            let get_plan_result = model.get_plan(plan_name);
                            if get_plan_result.is_err() {
                                continue; // Missing plan is logged in a prior check
                            }

                            let plan = get_plan_result.unwrap();
                            for goal in role.goals.iter() {
                                if let Some(handles) = plan.handles.as_ref() {
                                    if handles.qualified_name() == goal.qualified_name() {
                                        role_cnt += 1;
                                    }
                                }
                            }
                        }
                    }
                }

                for team in model.teams.iter() {
                    if team.id == self.id {
                        continue;
                    }

                    for team_role in team.roles.iter() {
                        if team_role.qualified_name() == role_id.qualified_name() {
                            role_cnt += 1;
                            continue;
                        }

                        let get_role_result = model.get_role(role_id);
                        if get_role_result.is_err() {
                            continue; // Missing role is logged in a prior check
                        }

                        // \todo Same problem as above
                        let role = get_role_result.unwrap();
                        for plan_name in team.plans.iter() {
                            let get_plan_result = model.get_plan(plan_name);
                            if get_plan_result.is_err() {
                                continue; // Missing plan is logged in a prior check
                            }

                            let plan = get_plan_result.unwrap();
                            for goal in role.goals.iter() {
                                if let Some(handles) = plan.handles.as_ref() {
                                    if handles.qualified_name() == goal.qualified_name() {
                                        role_cnt += 1;
                                    }
                                }
                            }
                        }
                    }

                    if role_cnt == 0 {
                        errors.push(ModelError::new(vec![format!("{}s.{}.roles.{}", agent_type, full_name, role_id.qualified_name())],
                                                    vec![format!("{}s.{}.roles.{}", agent_type, index, i)],
                                                    ErrorType::TeamRoleNoAgentsOrTeams,
                                                    format!("Role {} at index {} has no Agents or Teams that support it.", role_id.qualified_name(), i)));
                    }
                }
            }
        }

        // Beliefsets in a goal must be in the beliefsets of the agent or its roles
        // We will use BTreeSets here to create two sets of beliefset names and look 
        // at their intersection to determine if all if goal beliefs are covered.
        // Simple :)
        let mut agent_bs : BTreeSet<Identifier> = Default::default();
        for belief_id in self.beliefs.iter() {
            agent_bs.insert(belief_id.clone());
        }

        for role in model.roles.iter() {
            for role_id in self.roles.iter() {
                if role_id == &role.id {
                    for msg in role.messages.iter() {
                        agent_bs.insert(msg.id.clone());
                    }
                }
            }
        }

        for (i, goal) in model.goals.iter().enumerate() {
            let mut goal_bs  : BTreeSet<Identifier> = BTreeSet::new();
            for (j, goal_id) in self.goals.iter().enumerate() {
                if &goal.id == goal_id {
                    for msg_id in goal.query_messages.iter() {
                        goal_bs.insert(msg_id.clone());
                    }

                    let intersection: BTreeSet<Identifier> = goal_bs.intersection(&agent_bs).cloned().collect();
                    if intersection.len() == goal_bs.len() {
                        continue;
                    }

                    for bs in intersection.symmetric_difference(&goal_bs) {
                        let mut bs_index : usize = 0;
                        for (k, gbs) in goal.query_messages.iter().enumerate() {
                            if bs == gbs {
                                bs_index = k;
                                break;
                            }
                        }

                        errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, goal_id.qualified_name()),
                                                         format!("goals.{}.beliefsets.{}", goal_id.qualified_name(), bs.qualified_name())],
                                                    vec![format!("{}s.{}.goals.{}", agent_type, index, j),
                                                         format!("goals.{}.beliefsets.{}", i, bs_index)],
                                                    ErrorType::BeliefsetInGoalNotInAgent,
                                                    format!("Goal {} with beliefset {} cannot be found in agent or agent roles.", goal_id.qualified_name(), bs.qualified_name())));
                    }
                }
            }
        }

        // Agents must have action_handlers for any action in a plan task/body

        // add this agent's action handlers
        for handler in self.action_handlers.iter() {
            all_action_handlers.insert(handler.qualified_name());
        }

        for (i, plan) in model.plans.iter().enumerate() {
            let mut plan_actions : BTreeSet<String> = BTreeSet::new();

            for (j, plan_id) in self.plans.iter().enumerate() {
                if &plan.id == plan_id {
                    for task in plan.tasks.iter() {
                        if task.r#type == TaskType::action {
                            if let Some(message) = task.message.as_ref() {
                                plan_actions.insert(message.qualified_name());
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

                            errors.push(ModelError::new(vec![format!("{}s.{}.plans.{}", agent_type, full_name, plan_id.qualified_name()),
                                                             format!("plans.{}.tasks.{}", plan_id.qualified_name(), action)],
                                                        vec![format!("{}s.{}.plans.{}", agent_type, index, j),
                                                             format!("plans.{}.tasks.{}", i, act_index)],
                                                        ErrorType::ActionHandlersNotInAgent,
                                                        format!("Plan {} with action {} cannot be found in {:?} {} Action Handlers.", plan_id.qualified_name(), action, agent_type, full_name)));
                        }
                    }
                }
            }
        }

        // \note For each goal in the agent, ensure that resources declared in the goal are present
        // in the agent.

        // First collect all the resources available in the agent
        let mut resources : BTreeSet<String> = Default::default();
        for resource_id in self.resources.iter() {
            resources.insert(resource_id.qualified_name());
        }

        // For each goal in the agent, match it to the concrete goal in the model for doing
        // resource checking against.
        for (j, goal_id) in self.goals.iter().enumerate() {
            for (i, goal) in model.goals.iter().enumerate() {

                // Find the matching goal in the model.
                if &goal.id != goal_id {
                    continue;
                }

                // Get the list of resources required in the goal and do a set intersection against
                // the resources available in the agent.
                let mut goal_resources : BTreeSet<String> = Default::default();
                for resource_id in goal.resources.iter() {
                    goal_resources.insert(resource_id.qualified_name());
                }

                let intersection   : BTreeSet<String> = resources.intersection(&goal_resources).cloned().collect();
                if intersection.len() == goal_resources.len() {
                    continue;
                }

                // Report the resources that are missing from the agent, e.g. the agent is not able
                // to complete the goal because it does not have the necessary resources.
                for resource in intersection.symmetric_difference(&goal_resources) {
                    let mut res_index : usize = 0;
                    for (k, res) in goal.resources.iter().enumerate() {
                        if resource == &res.qualified_name() {
                            res_index = k;
                            break;
                        }
                    }
                    errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, goal.id.qualified_name()),
                                                     format!("goals.{}.resources.{}", goal.id.qualified_name(), resource)],
                                                vec![format!("{}s.{}.goals.{}", agent_type, index, j),
                                                     format!("goals.{}.resources.{}", i, res_index)],
                                                ErrorType::AgentGoalNoResource,
                                                format!("Goal {} with resource {} cannot be found in {:?} {} Resources.", goal.id.qualified_name(), resource, agent_type, full_name)));
                }
            }
        }

        // Tactics
        for (i, tactic_id) in self.tactics.iter().enumerate() {
            if tactic_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.tactics.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.tactics.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Tactic at index {} must not be empty.",i)));
            }

            if !model.find_tactic(tactic_id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.tactics.{}", agent_type, full_name, tactic_id.qualified_name())],
                                            vec![format!("{}s.{}.tactics.{}", agent_type, index, i)],
                                            ErrorType::TacticNotFound,
                                            format!("Tactic {} at index {} cannot be found in tactics.", tactic_id.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.tactics,
                                                      &format!("{:?}s.{}.tactics", agent_type, full_name),
                                                      &format!("{:?}s.{}.tactics", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"tactics".to_string()));
        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct GoalPlanQuery {
    pub custom : bool,
    pub query  : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Goals / Desires
pub struct Goal {
    #[serde(skip)]
    pub id             : Identifier,
    pub name           : String,
    pub query_messages : Vec<Identifier>,
    pub heuristic      : bool,
    pub precondition   : GoalPlanQuery,
    pub dropcondition  : GoalPlanQuery,
    pub satisfied      : GoalPlanQuery,
    pub resources      : Vec<Identifier>,
    #[serde(default)]
    pub message        : Identifier,
}

impl Goal {
    /// Validation
    /// 1. Ensure beliefsets are non zero length string, not duplicates and there is an associated beliefset message
    /// 2. Ensure there is a pursue type message that matches the message field.
    /// 3. Ensure resources are non zero length string and resource with the same name is found.
    /// 4. Ensure atleast one plan supports goal
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = self.id.qualified_name();

        // Beliefset Lookup
        for (i, msg_id) in self.query_messages.iter().enumerate() {
            if msg_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("goals.{}.query_messages.{}", full_name, i)],
                                            vec![format!("goals.{}.query_messages.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message at index {} must not be empty.",i)));
            }

            if !model.find_message(msg_id) {
                errors.push(ModelError::new(vec![format!("goals.{}.query_messages.{}", full_name, msg_id.qualified_name())],
                                            vec![format!("goals.{}.query_messages.{}", index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", msg_id.qualified_name(), i)));
            }
        }

        if self.message.name.len() > 0 {
            if !model.find_message(&self.message) {
                errors.push(ModelError::new(vec![format!("goals.{}.message", full_name)],
                                            vec![format!("goals.{}.message", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Goal message with name \"{}\" cannot be found in messages.", self.message.qualified_name())));
            }
        }
        errors.append(&mut verify_no_duplicate_fields(&self.query_messages,
                                                      &format!("goals.{}.query_messages", full_name),
                                                      &format!("goals.{}.query_messages", index),
                                                      &"goals".to_string(),
                                                      &full_name,
                                                      &"query_messages".to_string()));

        // Resource Lookup
        for (i, resource_id) in self.resources.iter().enumerate() {
            if resource_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("goals.{}.resources.{}", full_name, i)],
                                            vec![format!("goals.{}.resources.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource at index {} must not be empty.",i)));
            }

            if !model.find_resource(resource_id) {
                errors.push(ModelError::new(vec![format!("goals.{}.resources.{}", full_name, resource_id.qualified_name())],
                                            vec![format!("goals.{}.resources.{}", index, i)],
                                            ErrorType::ResourceNotFound,
                                            format!("Resource {} at index {} cannot be found in resources.", resource_id.qualified_name(), i)));
            }
        }
        errors.append(&mut verify_no_duplicate_fields(&self.resources,
                                                      &format!("goals.{}.resources", full_name),
                                                      &format!("goals.{}.resources", index),
                                                      &"goals".to_string(),
                                                      &full_name,
                                                      &"resources".to_string()));

        // Ensure that there is atleast one plan that handles the goal in the model
        let mut handle_cnt : u32 = 0;
        for (_i, plan) in model.plans.iter().enumerate() {
            if let Some(handles) = plan.handles.as_ref() {
                if handles.module.len() == 0 && &handles.name == &full_name {
                    handle_cnt += 1;
                }
            }
        }

        if handle_cnt == 0 {
            errors.push(ModelError::new(vec![format!("goals.{}", full_name)],
                                        vec![format!("goals.{}", index)],
                                        ErrorType::NoPlansForGoal,
                                        format!("Goal {} has no plans that handle it.", full_name)));
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
    to   : String,
    from : String,
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
    #[serde(skip_serializing_if = "Option::is_none")]
    pub r#async         : Option<bool>,
    /// Message that is appending data to the action task
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message         : Option<Identifier>,
    /// mapping of fields from message to task 
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mappings        : Option<Vec<Mapping>>,
    /// Only used in sleep tasks.
    /// Amount of time in milliseconds to sleep for
    #[serde(skip_serializing_if = "Option::is_none")]
    pub duration        : Option<u32>,
    /// Only used in conditional tasks
    /// Temporary textual description of the condition
    /// TODO: this will be replaced with BQL?
    #[serde(skip_serializing_if = "Option::is_none")]
    pub conditiontext   : Option<String>,

    // optional note for the task
    #[serde(skip_serializing_if = "Option::is_none")]
    pub note : Option<String>,
}

impl Task { }

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Plans are a design for how a goal/desire should be fulfilled.
///
pub struct Plan {
    #[serde(skip)]
    pub id             : Identifier,
    pub name           : String,
    pub precondition   : GoalPlanQuery,
    pub dropcondition  : GoalPlanQuery,
    pub effects        : bool,
    pub handles        : Option<Identifier>, // \todo Why is this optional?
    pub query_messages : Vec<Identifier>,
    pub tasks          : Vec<Task>,
    pub edges          : Vec<TaskEdge>,
}

impl Plan {
    /// Validation
    /// 1. Checks query messages are non zero string.
    /// 2. Checks that query message exists.
    /// 3. Checks that actions have a message.
    /// 4. Checks that pursue tasks have a goal and that goal has a message.
    /// 5. Checks for sleep task having a duration.
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = &self.id.qualified_name();

        for (i, query_msg_id) in self.query_messages.iter().enumerate() {
            if query_msg_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("plans.{}.query_messages.{}", full_name, i)],
                                            vec![format!("plans.{}.query_messages.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Query message at index {} must not be empty.", i)));
            }

            if !model.find_message(query_msg_id) {
                errors.push(ModelError::new(vec![format!("plans.{}.query_messages.{}", full_name, query_msg_id.qualified_name())],
                                            vec![format!("plans.{}.query_messages.{}", index, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Query message {} at index {} cannot be found in messages.", query_msg_id.qualified_name(), i)));
            }
        }

        // Ensure a plan handles a goal
        if let Some(handles) = self.handles.as_ref() {
            if !model.find_goal(handles) {
                errors.push(ModelError::new(vec![format!("plans.{}.handles", full_name)],
                                            vec![format!("plans.{}.handles", index)],
                                            ErrorType::GoalNotFound,
                                            format!("Plan {} handles a goal {} that cannot be found.", full_name, handles.qualified_name())));
            }
        } else {
                errors.push(ModelError::new(vec![format!("plans.{}.handles", full_name)],
                                            vec![format!("plans.{}.handles", index)],
                                            ErrorType::PlanMissingGoalToHandle,
                                            format!("Plan {} does not have a goal to handle.", full_name)));
        }

        for (i, task) in self.tasks.iter().enumerate() {
            if task.id.len() == 0 {
                errors.push(ModelError::new(vec![format!("plans.{}.query_messages.{}", full_name, i)],
                                            vec![format!("plans.{}.query_messages.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Task at index {} of Tasks in Plan {} cannot have an empty id.", i, full_name)));
            }

            match task.r#type {
                TaskType::action => {
                    // mappings
                    if task.mappings.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskMappingsNotFound,
                                                    format!("Action task {} at index {} of Tasks in Plan {} does not have a mappings field.", task.id, i, full_name)));
                    }

                    match &task.message {
                        Some(msg_id) => {
                            if !model.find_action(msg_id) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::MessageNotFound,
                                                            format!("Action task {} with message name {} at index {} of Tasks in Plan {} cannot be found in actions.",
                                                                    task.id,
                                                                    msg_id.qualified_name(),
                                                                    i,
                                                                    full_name)));
                            }
                        },
                        None => {
                            errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                        vec![format!("plans.{}.tasks.{}", index, i)],
                                                        ErrorType::MessageNotFound,
                                                        format!("Action task {} at index {} of Tasks in Plan {} does not have an action defined.", task.id, i, full_name)));
                        }
                    }
                },
                TaskType::goal => {
                    // r#async
                    if task.r#async.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskAsyncNotFound,
                                                    format!("Goal task {} at index {} of Tasks in Plan {} does not have r#async field.", task.id, i, full_name)));
                    }
                    // mappings
                    if task.mappings.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskMappingsNotFound,
                                                    format!("Goal task {} at index {} of Tasks in Plan {} does not have a mappings field.", task.id, i, full_name)));
                    }

                    match &task.message {
                        Some(goal_id) => {
                            if !model.find_goal(&goal_id) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::GoalNotFound,
                                                            format!("Pursue {} with Goal name {} at index {} of Tasks in Plan {} cannot be found in goals.",
                                                                    task.id,
                                                                    goal_id.qualified_name(),
                                                                    i,
                                                                    full_name)));
                            } else {
                                let goal = model.get_goal(&goal_id).unwrap();

                                if !model.find_message(&goal.message) {
                                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                                ErrorType::MessageNotFound,
                                                                format!("Pursue {} with message name {} at index {} of Tasks in Plan {} cannot be found in messages.", task.id, goal_id.qualified_name(), i, full_name)));
                                }
                            }
                        },
                        None => {
                            errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                        vec![format!("plans.{}.tasks.{}", index, i)],
                                                        ErrorType::GoalNotFound,
                                                        format!("Pursue {} at index {} of Tasks in Plan {} does not have a goal defined.", task.id, i, full_name)));
                        }
                    }
                },
                TaskType::sleep => {
                    if task.duration.is_none() {
                        errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                    vec![format!("plans.{}.tasks.{}", index, i)],
                                                    ErrorType::TaskSleepDurationNotFound,
                                                    format!("Sleep {} at index {} of Tasks in Plan {} does not have a duration.", task.id, i, full_name)));
                    }
                },
                TaskType::node => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskForFutureUse,
                                                format!("Task {} at index {} of Tasks in Plan {} is for future use.", task.id, i, full_name)));
                },
                TaskType::wait => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskForFutureUse,
                                                format!("Task {} at index {} of Tasks in Plan {} is for future use.", task.id, i, full_name)));
                },
                TaskType::condition => {
                    //TODO: Not sure what to do here
                },
                TaskType::None => {
                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                ErrorType::TaskIsNone,
                                                format!("Task {} at index {} of Tasks in Plan {} is None. This should not happen.", task.id, i, full_name)));
                }
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.query_messages,
                                                      &format!("plans.{}.beliefsets", full_name),
                                                      &format!("plans.{}.beliefsets", index),
                                                      &"plans".to_string(),
                                                      full_name,
                                                      &"beliefsets".to_string()));

        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// The lookup for shared beliefs and the
/// read/write permissions that are are set for the role.
pub struct MessagePerm {
    #[serde(skip)]
    pub id    : Identifier,
    pub name  : String,
    pub module : String,
    pub read  : bool,
    pub write : bool,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Roles are what tie Agents to a Team or Subteams to a Team 
///
pub struct Role {
    #[serde(skip)]
    pub id       : Identifier,
    pub name     : String,
    pub goals    : Vec<Identifier>,
    pub messages : Vec<MessagePerm>
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
        let mut errors : Vec<ModelError>         = Vec::new();
        let mut names  : BTreeMap<String, usize> = BTreeMap::new();
        let role_full_name = &self.id.qualified_name();

        for (i, msg) in self.messages.iter().enumerate() {
            let human_key_prefix   = format!("roles.{}.messages", role_full_name);
            let machine_key_prefix = format!("roles.{}.messages", index);
            let msg_full_name      = msg.id.qualified_name();

            // \note Validate the message
            if msg.id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}.{}", human_key_prefix, i)],
                                            vec![format!("{}.{}", machine_key_prefix, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message at index {} must not be empty.",i)));
            }

            if !model.find_message(&msg.id) {
                errors.push(ModelError::new(vec![format!("{}.{}", human_key_prefix, msg_full_name)],
                                            vec![format!("{}.{}", machine_key_prefix, i)],
                                            ErrorType::MessageNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", msg_full_name, i)));
            }

            // \note Add the message to a set for duplicate check
            if names.contains_key(&msg_full_name) {
                let last_index = names.get(&msg_full_name).unwrap();
                errors.push(ModelError::new(vec![format!("{}.{}", human_key_prefix,   msg_full_name)],
                                            vec![format!("{}.{}", machine_key_prefix, last_index),
                                                 format!("{}.{}", machine_key_prefix, i)],
                                            ErrorType::DuplicateName,
                                            format!("Role at {} specified message that has duplicate entries of \"{}\" at indicies {} and {}.",
                                                    role_full_name,
                                                    msg_full_name,
                                                    last_index,
                                                    i)));
            } else {
                names.insert(msg_full_name, i);
            }
        }

        for (i, goal_id) in self.goals.iter().enumerate() {
            let human_key_prefix   = format!("roles.{}.goals", role_full_name);
            let machine_key_prefix = format!("roles.{}.goals", index);
            let full_name     = goal_id.qualified_name();

            if goal_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}.{}", human_key_prefix, i)],
                                            vec![format!("{}.{}", machine_key_prefix, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal at index {} must not be empty.", i)));
            }

            if !model.find_goal(goal_id) {
                errors.push(ModelError::new(vec![format!("{}.{}", human_key_prefix, full_name)],
                                            vec![format!("{}.{}", machine_key_prefix, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", full_name, i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.goals,
                                                      &format!("roles.{}.goals", role_full_name),
                                                      &format!("roles.{}.goals", index),
                                                      &"roles".to_string(),
                                                      &role_full_name,
                                                      &"goals".to_string()));
        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// A resource is a constrained object that can be used.
/// Currently represented as an object that can have a minimum and maximum value.
///
pub struct Resource {
    #[serde(skip)]
    pub id             : Identifier,
    /// Name of resource
    pub name           : String,
    /// Type of resource value
    pub r#type         : String,
    /// Moninum value
    pub min            : i32,
    /// Maximum value
    pub max            : i32
}

impl Resource {
    /// There is only one rule that must be considered for Resources -- max > min.
    /// As a resource is a supply that can be used.
    ///
    fn verify(&self, _model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = self.id.qualified_name();

        if self.min > self.max {
            errors.push(ModelError::new(vec![format!("resources.{}", full_name)],
                                        vec![format!("resources.{}", index)],
                                        ErrorType::ResourceMinMaxSwapped,
                                        format!("Resouce has its min and max values swapped.")));
        }

        if self.min == self.max {
            errors.push(ModelError::new(vec![format!("resources.{}", full_name)],
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
    #[serde(skip)]
    pub id             : Identifier,
    /// Name of tactic.
    pub name           : String,
    /// Goal the tactic supports.
    pub goal           : Identifier,
    /// Plans that the tactic allows.
    pub plans          : Vec<Identifier>,
}

impl Tactic {
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = &self.id.qualified_name();
        if self.goal.name.len() == 0 {
            errors.push(ModelError::new(vec![format!("tactics.{}.goal", full_name)],
                                        vec![format!("tactics.{}.goal", index)],
                                        ErrorType::ZeroSizeString,
                                        format!("Goal handle at {} must not be empty.", index)));
        }

        if !model.find_goal(&self.goal) {
            errors.push(ModelError::new(vec![format!("tactics.{}.goal", full_name)],
                                        vec![format!("tactics.{}.goal", index)],
                                        ErrorType::GoalNotFound,
                                        format!("Goal {} that tactic handles cannot be found in goals.", self.goal.qualified_name())));
        }

        for (i, plan) in self.plans.iter().enumerate() {
            if plan.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("tactics.{}.plans.{}", full_name, i)],
                                            vec![format!("tactics.{}.plans.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan at index {} must not be empty.",i)));
            }

            if !model.find_plan(&plan) {
                errors.push(ModelError::new(vec![format!("tactics.{}.plans.{}", full_name, plan.qualified_name())],
                                            vec![format!("tactics.{}.plans.{}", index, i)],
                                            ErrorType::PlanNotFound,
                                            format!("Plan {} at index {} cannot be found in plans.", plan.qualified_name(), i)));
            }
        }

        errors.append(&mut verify_no_duplicate_fields(&self.plans,
                                                      &format!("tactics.{}.plans", full_name),
                                                      &format!("tactics.{}.plans", index),
                                                      &"tactics".to_string(),
                                                      &full_name,
                                                      &"plans".to_string()));
        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Action {
    #[serde(skip)]
    pub id  : Identifier,
    pub name: String,

    // optional note for the action
    #[serde(skip_serializing_if = "Option::is_none")]
    pub note : Option<String>,

    /// Request message
    #[serde(default)]
    pub req : Identifier,
    /// Reply message
    #[serde(default)]
    pub rpy : Identifier,
}

impl Action {
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = self.id.qualified_name();
        if self.req.name.len() > 0 {
            if let Err(_) = model.get_message(&self.req) {
                errors.push(ModelError::new(vec![format!("actions.{}.req", full_name)],
                                            vec![format!("actions.{}.req", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Action uses a request message \"{}\" that does not exist.", self.req.qualified_name())));
            }
        }

        if self.rpy.name.len() > 0 {
            if let Err(_) = model.get_message(&self.rpy) {
                errors.push(ModelError::new(vec![format!("actions.{}.rpy", full_name)],
                                            vec![format!("actions.{}.rpy", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Action uses a reply message \"{}\" that does not exist.", self.rpy.qualified_name())));
            }
        }
        errors
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct EnumField {
    pub name  : String,
    pub value : i32,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Enum {
    #[serde(skip)]
    pub id     : Identifier,
    pub name   : String,
    pub fields : Vec<EnumField>,
}

impl Enum {
    /// Verficiation for Messages:
    /// 1. Checking that there are no fields with the same duplicate fields.
    /// 2. If a field has a custom type, whether the model contains the message required.
    ///
    fn verify(&self, _ : &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = vec![];
        let mut field_names  : BTreeMap<String /*field name*/,  usize /*field index*/> = BTreeMap::new();
        let mut field_values : BTreeMap<i32    /*field value*/, usize /*index*/>       = BTreeMap::new();
        let full_name = &self.id.qualified_name();
        for (i, field) in self.fields.iter().enumerate() {

            // Check if field name has been duplicated
            if field_names.contains_key(&field.name) {
                let other_index = field_names.get(&field.name).unwrap();
                errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", full_name, i), format!("messages.{}.fields.{}", full_name, other_index)],
                                            vec![format!("messages.{}.fields.{}", index, i),     format!("messages.{}.fields.{}", full_name, other_index)],
                                            ErrorType::MessageDuplicateFields,
                                            format!("Duplicate fields with name {} in Message {}.", field.name, full_name)));
            } else {
                field_names.insert(field.name.clone(), i);
            }

            // Check if field value has been duplicated
            if field_values.contains_key(&field.value) {
                let other_index = field_values.get(&field.value).unwrap();
                errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", full_name, i), format!("messages.{}.fields.{}", full_name, other_index)],
                                            vec![format!("messages.{}.fields.{}", index, i),     format!("messages.{}.fields.{}", full_name, other_index)],
                                            ErrorType::MessageDuplicateFields,
                                            format!("Duplicate value with name {} in Message {}.", field.name, full_name)));
            } else {
                field_values.insert(field.value, i);
            }
        }

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
    pub project   : Project,
    pub enums     : Vec<Enum>,
    pub teams     : Vec<Agent>,
    pub agents    : Vec<Agent>,
    pub services  : Vec<Service>,
    pub messages  : Vec<Message>,
    pub goals     : Vec<Goal>,
    pub plans     : Vec<Plan>,
    pub roles     : Vec<Role>,
    pub resources : Vec<Resource>,
    pub tactics   : Vec<Tactic>,
    pub actions   : Vec<Action>,
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
            if team.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("teams.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Team name at index {} must not be empty.",i)));
            }

            // Check if name is "Team" -> This leads to code generation errors
            if team.id.name == "Team" {
                errors.push(ModelError::new(vec![format!("teams.{}", team.id.name)],
                                            vec![format!("teams.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Team name at index {} must not be \"Team\".",i)));
            }

            // Check for duplicate team name
            if names.contains_key(&team.id.name) {
                errors.push(ModelError::new(vec![names.get(&team.id.name).unwrap().0.clone(), format!("teams.{}", team.id.name)],
                                            vec![names.get(&team.id.name).unwrap().1.clone(), format!("teams.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Team at index {} must have a unique name.",i)));
            } else {
                names.insert(team.id.name.clone(), (format!("teams.{}", team.id.name), format!("teams.{}", i)));
            }

            // Add errors from Team/Agent specific rules
            errors.append(&mut team.verify(&AgentType::Team, &self, &i));
        }

        for (i,agent) in self.agents.iter().enumerate() {
            
            // Check for zero length agent name
            if agent.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("agents.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Agent name at index {} must not be empty.",i)));
            }

            // Check if name is "Agent" -> This leads to code generation errors
            if agent.id.name == "Agent" {
                errors.push(ModelError::new(vec![format!("agents.{}", agent.id.name)],
                                            vec![format!("agents.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Agent name at index {} must not be \"Agent\".",i)));
            }

            // Check for duplicate agent name
            if names.contains_key(&agent.id.name) {
                errors.push(ModelError::new(vec![names.get(&agent.id.name).unwrap().0.clone(), format!("agents.{}", agent.id.name)],
                                            vec![names.get(&agent.id.name).unwrap().1.clone(), format!("agents.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Agent at index {} must have a unique name.",i)));
            } else {
                names.insert(agent.id.name.clone(), (format!("agents.{}", agent.id.name), format!("agents.{}", i)));
            }

            // Add errors from Team/Agent specific rules
            errors.append(&mut agent.verify(&AgentType::Agent, &self, &i));
        }

        for (i, msg) in self.messages.iter().enumerate() {
            // Check for zero length message name
            if msg.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("messages.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Message name at index {} must not be empty.",i)));
            }

            // Check if name is "Message" -> This leads to code generation errors
            if msg.id.name == "Message" {
                errors.push(ModelError::new(vec![format!("messages.{}", msg.id.name)],
                                            vec![format!("messages.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Message name  not be \"Message\".")));
            }

            // Check for duplicate message name
            // NOTE: We're introduced a percept message for the 0.4 model to allow for reply messages from an action
            // These message protols will be removed for model 0.5
            // for now this duplicate name test needs to be removed

            // if names.contains_key(&msg.id.name) {
            //     errors.push(ModelError::new(vec![ names.get(&msg.id.name).unwrap().0.clone(), format!("messages.{}", msg.id.name)],
            //                                 vec![ names.get(&msg.id.name).unwrap().1.clone(), format!("messages.{}", i)],
            //                                 ErrorType::DuplicateName,
            //                                 format!("Message at index {} must have a unique name.",i)));
            // } else {
            //     names.insert(msg.id.name.clone());
            // }

            // still add the name, duplicate or not
            names.insert(msg.id.name.clone(),  (format!("messages.{}", msg.id.name), format!("messages.{}", i)));

            // Add errors from message specific rules
            errors.append(&mut msg.verify(&self, &i));
        }

        for (i, goal) in self.goals.iter().enumerate() {

            // Check for zero length goal name
            if goal.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("goals.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal name at index {} must not be empty.", i)));
            }

            // Check for duplicate goal name
            if names.contains_key(&goal.id.name) {
                errors.push(ModelError::new(vec![names.get(&goal.id.name).unwrap().0.clone(), format!("goals.{}", goal.id.name)],
                                            vec![names.get(&goal.id.name).unwrap().1.clone(), format!("goals.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Goal name {} must have a unique name.", goal.id.name)));
            } else {
                names.insert(goal.id.name.clone(), (format!("goals.{}", goal.id.name), format!("goals.{}", i)));
            }

            // Check if name is "Goal" -> This leads to code generation errors
            if goal.id.name == "Goal" {
                errors.push(ModelError::new(vec![format!("goals.{}", goal.id.name)],
                                            vec![format!("goals.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Goal name must not be \"Goal\".")));
            }

            // Add errors from goal specific rules
            errors.append(&mut goal.verify(&self, &i));
        }

        for (i, plan) in self.plans.iter().enumerate() {
            // Check for zero length plan name
            if plan.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("plans.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan name at index {} must not be empty.",i)));
            }

            // Check for duplicate plan name
            if names.contains_key(&plan.id.name) {
                errors.push(ModelError::new(vec![names.get(&plan.id.name).unwrap().0.clone(), format!("plans.{}", plan.id.name)],
                                            vec![names.get(&plan.id.name).unwrap().1.clone(), format!("plans.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Plan with name {} must have a unique name.", plan.id.name)));
            } else {
                names.insert(plan.id.name.clone(), (format!("plans.{}", plan.id.name), format!("plans.{}", i)));
            }

            // Check if name is "Plan" -> This leads to code generation errors
            if plan.id.name == "Plan" {
                errors.push(ModelError::new(vec![format!("plans.{}", plan.id.name)],
                                            vec![format!("plans.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Plan name must not be \"Plan\".")));
            }

            // Add errors from plan specific rules
            errors.append(&mut plan.verify(&self, &i));
        }

        for (i, role) in self.roles.iter().enumerate() {
            // Check for zero length role name
            if role.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("roles.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Role name at index {} must not be empty.",i)));
            }

            // Check for duplicate role name
            if names.contains_key(&role.id.name) {
                errors.push(ModelError::new(vec![names.get(&role.id.name).unwrap().0.clone(), format!("roles.{}", role.id.name)],
                                            vec![names.get(&role.id.name).unwrap().1.clone(), format!("roles.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Role with name {} must have a unique name.", role.id.name)));
            } else {
                names.insert(role.id.name.clone(), (format!("roles.{}", role.id.name), format!("roles.{}", i)));
            }

            // Check if name is "Role" -> This leads to code generation errors
            if role.id.name == "Role" {
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
            if resource.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("resources.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Resource name at index {} must not be empty.",i)));
            }

            // Check for duplicate resource name
            if names.contains_key(&resource.id.name) {
                errors.push(ModelError::new(vec![names.get(&resource.id.name).unwrap().0.clone(), format!("resources.{}", resource.id.name)],
                                            vec![names.get(&resource.id.name).unwrap().1.clone(), format!("resources.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Resource with name {} must have a unique name.", resource.id.name)));
            } else {
                names.insert(resource.id.name.clone(), (format!("resources.{}", resource.id.name), format!("resources.{}", i)));
            }

            // Check if name is "Resource" -> This leads to code generation errors
            if resource.id.name == "Resource" {
                errors.push(ModelError::new(vec![format!("resources.{}", resource.id.name)],
                                            vec![format!("resources.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Resource name must not be \"Resource\".")));
            }

            // Add errors from resource specific rules
            errors.append(&mut resource.verify(&self, &i));
        }

        for (i, tactic) in self.tactics.iter().enumerate() {
            // Check for zero length tactic name
            if tactic.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("tactics.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Tactic name at index {} must not be empty.",i)));
            }

            // Check for duplicate tactic name
            if names.contains_key(&tactic.id.name) {
                errors.push(ModelError::new(vec![names.get(&tactic.id.name).unwrap().0.clone(), format!("tactics.{}", tactic.id.name)],
                                            vec![names.get(&tactic.id.name).unwrap().1.clone(), format!("tactics.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Tactic name {} must have a unique name.", tactic.id.name)));
            } else {
                names.insert(tactic.id.name.clone(),  (format!("tactics.{}", tactic.id.name), format!("tactics.{}", i)));
            }

            // Check if name is "Tactic" -> This leads to code generation errors
            if tactic.id.name == "Tactic" {
                errors.push(ModelError::new(vec![format!("tactics.{}", tactic.id.name)],
                                            vec![format!("tactics.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Tactic name must not be \"Tactic\".")));
            }

            // Add errors from tactic specific rules
            errors.append(&mut tactic.verify(&self, &i));
        }

        for (i, action) in self.actions.iter().enumerate() {
            // Check for zero length action name
            if action.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("actions.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Action name at index {} must not be empty.",i)));
            }

            // Check for duplicate service name
            if names.contains_key(&action.id.name) {
                errors.push(ModelError::new(vec![names.get(&action.id.name).unwrap().0.clone(), format!("actions.{}", action.id.name)],
                                            vec![names.get(&action.id.name).unwrap().1.clone(), format!("actions.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Action with name {} must have a unique name.", action.id.name)));
            } else {
                names.insert(action.id.name.clone(),  (format!("actions.{}", action.id.name), format!("actions.{}", i)));
            }

            // Check if name is "Action" -> This leads to code generation errors
            if action.id.name == "Action" {
                errors.push(ModelError::new(vec![format!("actions.{}", action.id.name)],
                                            vec![format!("actions.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Action name must not be \"Action\".")));
            }

            // Add errors from action specific rules
            errors.append(&mut action.verify(&self, &i));
        }

        for (i, service) in self.services.iter().enumerate() {
            // Check for zero length service name
            if service.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("services.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Service name at index {} must not be empty.",i)));
            }

            // Check for duplicate service name
            if names.contains_key(&service.id.name) {
                errors.push(ModelError::new(vec![names.get(&service.id.name).unwrap().0.clone(), format!("services.{}", service.id.name)],
                                            vec![names.get(&service.id.name).unwrap().1.clone(), format!("services.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Service with name {} must have a unique name.", service.id.name)));
            } else {
                names.insert(service.id.name.clone(),  (format!("services.{}", service.id.name), format!("services.{}", i)));
            }

            // Check if name is "Service" -> This leads to code generation errors
            if service.id.name == "Service" {
                errors.push(ModelError::new(vec![format!("services.{}", service.id.name)],
                                            vec![format!("services.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Service name must not be \"Service\".")));
            }

            // Add errors from service specific rules
            // TODO: No rules to verify services yet?!?
            //errors.append(&mut service.verify(&self, &i));
        }

        for (i, enum_val) in self.enums.iter().enumerate() {
            // Check for zero length enum name
            if enum_val.id.name.len() == 0 {
                errors.push(ModelError::new(vec![],
                                            vec![format!("enums.{}", i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Service name at index {} must not be empty.",i)));
            }

            // Check for duplicate enum name
            if names.contains_key(&enum_val.id.name) {
                errors.push(ModelError::new(vec![names.get(&enum_val.id.name).unwrap().0.clone(), format!("enums.{}", enum_val.id.name)],
                                            vec![names.get(&enum_val.id.name).unwrap().1.clone(), format!("enums.{}", i)],
                                            ErrorType::DuplicateName,
                                            format!("Service with name {} must have a unique name.", enum_val.id.name)));
            } else {
                names.insert(enum_val.id.name.clone(),  (format!("enums.{}", enum_val.id.name), format!("enums.{}", i)));
            }

            // Check if name is "Enum" -> This leads to code generation errors
            if enum_val.id.name == "Enum" {
                errors.push(ModelError::new(vec![format!("enums.{}", enum_val.id.name)],
                                            vec![format!("enums.{}", i)],
                                            ErrorType::NameSameAsType,
                                            format!("Service name must not be \"Service\".")));
            }

            errors.append(&mut enum_val.verify(&self, &i));
        }

        // return errors
        errors
    }

    /// Returns true if there is a message with the given name
    pub fn find_message(&self, identifier : &Identifier) -> bool {
        for msg in self.messages.iter() {
            if &msg.id == identifier {
                return true;
            }
        }
        false
    }

    /// Returns true if there is a action with the name supplied.
    pub fn find_action(&self, identifier : &Identifier) -> bool {
        for action in self.actions.iter() {
            if &action.id == identifier {
                return true;
            }
        }
        false
    }

    /// Returns true if there is a goal with the name supplied.
    pub fn find_goal(&self, identifier : &Identifier) -> bool {
        for goal in self.goals.iter() {
            if &goal.id == identifier {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a plan with the name supplied.
    pub fn find_plan(&self, identifier : &Identifier) -> bool {
        for plan in self.plans.iter() {
            if &plan.id == identifier {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a resource with the name supplied.
    pub fn find_resource(&self, identifier : &Identifier) -> bool {
        for resource in self.resources.iter() {
            if &resource.id == identifier {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a role with the name supplied.
    pub fn find_role(&self, identifier : &Identifier) -> bool {
        for role in self.roles.iter() {
            if &role.id == identifier {
                return true;
            }
        }

        false
    }

    /// Returns true if there is a tactic with the name supplied.
    pub fn find_tactic(&self, identifier : &Identifier) -> bool {
        if identifier.name.len() > 0 {
            for tactic in self.tactics.iter() {
                if &tactic.id == identifier {
                    return true;
                }
            }
        }
        false
    }

    pub fn get_message(&self, identifier : &Identifier) -> Result<Message, Box<dyn std::error::Error>> {
        if identifier.name.len() > 0 {
            for msg in self.messages.iter() {
                if &msg.id == identifier {
                    return Ok(msg.clone());
                }
            }
        }
        Err(format!("Could not find Message {}", identifier.qualified_name()).into())
    }

    /// Returns a result::Ok(Goal) if there is a goal with the name supplied.
    /// If no goal is found, an result::Err(String) is returned.
    pub fn get_goal(&self, identifier : &Identifier) -> Result<Goal, Box<dyn std::error::Error>> {
        if identifier.name.len() > 0 {
            for goal in self.goals.iter() {
                if &goal.id == identifier {
                    return Ok(goal.clone());
                }
            }
        }
        Err(format!("Could not find Goal with name {}", identifier.qualified_name()).into())
    }

    /// Returns a result::Ok(Plan) if there is a plan with the name supplied.
    /// If no plan is found, an result::Err(String) is returned.
    pub fn get_plan(&self, identifier : &Identifier) -> Result<Plan, Box<dyn std::error::Error>> {
        if identifier.name.len() > 0 {
            for plan in self.plans.iter() {
                if &plan.id == identifier {
                    return Ok(plan.clone());
                }
            }
        }
        Err(format!("Could not find plan with name {}", identifier.qualified_name()).into())
    }

    /// Returns a result::Ok(Role) if there is a role with the name supplied.
    /// If no role is found, an result::Err(String) is returned.
    pub fn get_role(&self, identifier : &Identifier) -> Result<Role, Box<dyn std::error::Error>> {
        if identifier.name.len() > 0 {
            for role in self.roles.iter() {
                if &role.id == identifier {
                    return Ok(role.clone());
                }
            }
        }
        Err(format!("Could not find Role with name {}", identifier.qualified_name()).into())
    }

    fn get_service(&self, identifier: &Identifier) -> Option<&Service> {
        let mut result : Option<&Service> = None;
        for service_it in self.services.iter() {
            if &service_it.id == identifier {
                result = Some(service_it)
            }
        }
        result
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
    /// There is an action referenced in the model that cannot be found in the action list.
    ActionNotFound,
    /// There are two fields with the same name in a message.
    MessageDuplicateFields,
    /// Sleep tasks require the duration field to be complete.
    TaskSleepDurationNotFound,
    /// Action or Goal task are missing r#async field
    TaskAsyncNotFound,
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
pub fn verify_no_duplicate_fields(input : &Vec<Identifier>,
                                  field_id : &String,
                                  machine_id : &String,
                                  object_type : &String,
                                  object_name : &String,
                                  field_name : &String) -> Vec<ModelError> {

    // Using a BTreeMap (think C++ std::map) to store the name in a list 
    // with its position in the list as the value.
    let mut names : BTreeMap<String, usize> = BTreeMap::new();
    let mut errors : Vec<ModelError> = Vec::new();

    // Creates an iterator that is enumerated.
    // i is the position in the list (the enumerated value)
    // name is the object in the list (in this case a string)
    for (i, identifier) in input.iter().enumerate() {

        let qualified_name = identifier.qualified_name();
        // looks or the name in the map
        // if found create an error
        if names.contains_key(&qualified_name) {
            let last_index = names.get(&qualified_name).unwrap();

            // Push back ModelError struct in the the errors vec
            // format! is a macro that creates a string that can have data pushed into it.
            // similar in function to printf
            errors.push(ModelError::new(vec![format!("{}.{}", field_id, qualified_name)],
                                        vec![format!("{}.{}", machine_id, i),
                                             format!("{}.{}", machine_id, last_index)],
                                        ErrorType::DuplicateName,
                                        format!("{} at {} in {} has duplicate entries of \"{}\" at indicies {} and {}.", object_type, object_name, field_name, qualified_name, last_index, i)));
        } else {
            // Otherwise not found, insert the name into the names map.
            names.insert(qualified_name.clone(), i.clone());
        }
    }

    errors
}

