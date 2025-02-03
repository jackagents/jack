// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

use serde::{Serialize, Deserialize};
use std::collections::*;
use std::str::FromStr;
use std::fmt;
use std::hash::{Hash, Hasher};

fn os_newline() -> &'static str {
    #[cfg(windows)]
    let result = "\r\n";
    #[cfg(not(windows))]
    let result = "\n";
    result
}

macro_rules! writeln_os {
    ($output:expr, $($arg:tt)*) => {{
        let line = format!($($arg)*);
        $output.push_str(line.as_str());
        #[cfg(windows)]
        $output.push_str("\r\n");
        #[cfg(not(windows))]
        $output.push('\n');
    }};
}

#[derive(Serialize, Deserialize, Eq, Default, Debug, Clone)]
pub struct Identifier {
    // \note UUID was introduced in 0.6, none of these fields will exist in the
    // model so we default it and assign a random one on load. In subsequent
    // version this annotation can be removed.
    #[serde(default)]
    pub uuid:       String,
    pub name:       String,
    #[serde(default)]
    pub module:     String,
    #[serde(skip)]
    pub bumpy_case: String,
}

impl Hash for Identifier {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.uuid.hash(state);
    }
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Entity {
    #[serde(flatten)]
    pub id:         Identifier,
    pub note:       String,
    pub children:   Vec<Identifier>,

    /// Flag for it the entity will have an agent component associated with it
    #[serde(default)]
    pub agent:     bool,

    /// The list of messages to associate with the entity. These messages must
    /// have the component flag set on it.
    #[serde(default)]
    pub messages:  Vec<Identifier>,

    /// The list of services to associate with the entity
    #[serde(default)]
    pub services:  Vec<Identifier>
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct EventMessageDefaultValue {
    pub name:  String,
    pub note:  String,
    pub value: String
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct EventMessage {
    #[serde(flatten)]
    pub id:       Identifier,
    pub defaults: Vec<EventMessageDefaultValue>
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Event {
    #[serde(flatten)]
    pub id:              Identifier,
    pub note:            String,
    #[serde(default)]
    pub requires_entity: bool,
    pub message:         EventMessage
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Module {
    pub name     : String,
    pub filepath : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Project {
    pub name:          String,
    #[serde(skip)]
    pub bumpy_case:    String,
    pub namespaces:    Vec<String>,
    pub major_version: String,
    pub minor_version: String,
    pub patch_version: String,
    pub generator:     String,
    pub modules:       Vec<Module>,
    pub search_paths:  Vec<String>,
}

#[derive(Clone, PartialEq, Eq)]
pub enum EmitNamespace {
    Nil,
    Cpp,
    ROS,
}

#[derive(Serialize, Deserialize, Debug, Clone, PartialEq, Eq)]
/// Field type describes types that can be used in messages.
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
    /// X,Y
    Vec2,
    /// Default error case. Never used in real operations.
    None  // Error case
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
    pub name:       String,
    #[serde(skip)]
    pub bumpy_case: String,
    /// An optional note attached to each message field
    pub note:       String,
    // type is a reserved keyword in rust
    // r# stops it being a keyword
    /// Is the FieldType please look at the definition for further information.
    pub r#type:     FieldType,
    /// Defines whether the field represents an array of fields of 'type'
    pub is_array:   bool,
    /// Default value of the field, empty string if no default value.
    pub default:    String,
    /// Indicates if the field should be hidden from the scenario editor's event editor or not
    #[serde(default)]
    pub hidden:     bool
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct EntityPickerAttributes
{
    /// The field to bind the widget to, e.g. the picker will set this field in
    /// the message with the entity's ID that it selected.
    field: String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct ColourPickerAttributes
{
    r_field: String,
    g_field: String,
    b_field: String,
    a_field: String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct SliderF32Attributes
{
    /// The field to bind the widget to, e.g. the slider will modify this field
    /// in the message when the slider thumb is moved.
    field: String,
    min:   f32,
    max:   f32,
    /// How much to increment the value for each discrete movement of the slider
    step:  f32,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct SliderI32Attributes
{
    /// The field to bind the widget to, e.g. the slider will modify this field
    /// in the message when the slider thumb is moved.
    field: String,
    min:   i32,
    max:   i32,
    /// How much to increment the value for each discrete movement of the slider
    step:  i32,
}

// The tag and content attributes makes serde able to deserialise a JSON object
// that looks like
//
// { "widget": "<widget name>", "attributes": {<variable object payload>}}
//
// - The "widget"     is parsed by finding a matching Rust enum in 'MessageEditorWidget'
// - The "attributes" is the contents of the Rust enum's value
//
// For example a SliderF32 can be deserialised from the following JSON
//
// { "widget": "SliderF32", "attributes": {"slider_field": "percentage", "min": 0.0, "max": 100.0 }}
#[derive(Serialize, Deserialize, Debug, Clone)]
#[serde(tag = "widget", content = "attributes")]
pub enum MessageEditorWidget
{
    ColourPicker(ColourPickerAttributes),
    EntityPicker(EntityPickerAttributes),
    SliderF32(SliderF32Attributes),
    SliderI32(SliderI32Attributes),
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Structure that contains all message types in the Model.
pub struct Message {
    /// Name of message.
    #[serde(flatten)]
    pub id:        Identifier,
    /// An optional note attached to each message
    pub note:      String,
    #[serde(default)]
    pub editor:    Vec<MessageEditorWidget>,
    pub component: bool,
    /// Definition of the containing fields.
    pub fields:    Vec<Field>
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Topic
{
    pub name:    String,
    pub message: Identifier,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Service {
   #[serde(flatten)]
   pub id:              Identifier,
   pub note:            String,
   pub action_handlers: Vec<Identifier>,
   #[serde(default)]
   pub topics:          Vec<Topic>,
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
pub struct AgentGoal {
    #[serde(flatten)]
    pub id:             Identifier,
    pub startup_goal:   bool,
    pub startup_tactic: Identifier
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Team/Agent data structure.
/// All fields are lookups to other parts of the model.
/// There is nothing that differnetiates a Team from an Agent in this struct.
/// The differentiating factor is in the main model structure.
/// I.e. teams in the teams list and agents in the agents list.
///
pub struct Agent {
    #[serde(flatten)]
    pub id :              Identifier,
    #[serde(default)]
    pub note:             String,
    pub roles:            Vec<Identifier>,
    pub beliefs:          Vec<Identifier>,
    pub goals:            Vec<AgentGoal>,
    pub services:         Vec<Identifier>,
    pub plans:            Vec<Identifier>,
    pub message_handlers: Vec<Identifier>,
    pub resources:        Vec<Identifier>,
    pub action_handlers:  Vec<Identifier>,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct GoalPolicy {
    pub persistent   : bool,
    pub plan_order   : Vec<Identifier>,
    pub plan_exclude : bool,
    pub plan_loop    : bool,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct GoalPlanQuery {
    pub custom : bool,
    pub query  : String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Goals / Desires
pub struct Goal {
    #[serde(flatten)]
    pub id:             Identifier,
    #[serde(default)]
    pub note:           String,
    pub query_messages: Vec<Identifier>,
    pub heuristic:      bool,
    pub precondition:   GoalPlanQuery,
    pub dropcondition:  GoalPlanQuery,
    pub satisfied:      GoalPlanQuery,
    pub resources:      Vec<Identifier>,
    pub message:        Identifier,
    #[serde(skip)]
    pub policy:         GoalPolicy,
}

#[allow(non_camel_case_types)]
#[derive(Serialize, Deserialize, Debug, Clone, PartialEq)]
/// Tasks that a plan can contain
pub enum TaskType {
    /// Action perform an action.
    action,
    /// Sleep for N milliseconds
    sleep,
    /// Sub goal
    goal,
    /// Conditional node
    condition,
    /// Default - error state should not be used.
    None
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Mapping for task from field -> field
// TODO: I think this is wrong and needs more information on the from side.
//       Mainly, if we have 2 messages that ahve the same field name we are screwed....
pub struct Mapping {
    pub from: String,
    pub to:   String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct TaskEdge {
    // The which condition is this edge? 
    // TODO: make this an enum
    pub condition: String,
    // The source task of this edge
    pub sourceid:  String,
    // The target task of this edge
    pub targetid:  String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Task {
    /// The id for this task
    /// TODO: this could just be an index
    pub id:            String,
    /// Optional note for the task
    pub note:          String,
    /// See TaskType for more info
    pub r#type:        TaskType,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub action:        Option<Identifier>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub goal:          Option<Identifier>,
    /// Run task asynchronously
    #[serde(skip_serializing_if = "Option::is_none")]
    pub r#async:       Option<bool>,
    /// mapping of fields from message to task 
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mappings:      Option<Vec<Mapping>>,
    /// Only used in sleep tasks.
    /// Amount of time in milliseconds to sleep for
    #[serde(skip_serializing_if = "Option::is_none")]
    pub duration:      Option<u32>,
    /// Only used in conditional tasks
    /// Temporary textual description of the condition
    /// TODO: this will be replaced with BQL?
    #[serde(skip_serializing_if = "Option::is_none")]
    pub conditiontext: Option<String>,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Plan {
    #[serde(flatten)]
    pub id:             Identifier,
    pub note:           String,
    pub query_messages: Vec<Identifier>,
    pub handles:        Identifier,
    pub effects:        bool,
    pub tasks:          Vec<Task>,
    pub edges:          Vec<TaskEdge>,
    pub precondition:   GoalPlanQuery,
    pub dropcondition:  GoalPlanQuery,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// The lookup for shared beliefs and the
/// read/write permissions that are are set for the role.
pub struct MessagePerm {
    #[serde(flatten)]
    pub id:    Identifier,
    pub read:  bool,
    pub write: bool,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// Roles are what tie Agents to a Team or Subteams to a Team
pub struct Role {
    #[serde(flatten)]
    pub id:       Identifier,
    pub goals:    Vec<Identifier>,
    pub messages: Vec<MessagePerm>
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// A resource is a constrained object that can be used.
/// Currently represented as an object that can have a minimum and maximum value.
pub struct Resource {
    #[serde(flatten)]
    pub id:     Identifier,
    /// Type of resource value
    pub r#type: String,
    /// Moninum value
    pub min:    i32,
    /// Maximum value
    pub max:    i32
}

#[derive(Serialize, Deserialize, Eq, PartialEq, Debug, Clone)]
pub enum TacticPolicy {
    ExcludePlanAfterAttempt,
    Strict,
    ChooseBestPlan,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
/// A tactic is a plan filter for an individual goal.
pub struct Tactic {
    #[serde(flatten)]
    pub id:            Identifier,
    pub note:          String,
    pub goal:          Identifier,
    pub use_plan_list: bool,
    pub plan_list:     Vec<Identifier>,
    pub plan_order:    TacticPolicy,
    pub plan_loop:     i64,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Action {
    #[serde(flatten)]
    pub id:       Identifier,
    pub note:     String,
    pub request:  Identifier,
    pub reply:    Identifier,
    #[serde(default)]
    pub feedback: Identifier,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct EnumField {
    pub name:       String,
    #[serde(default)]
    pub note:       String,
    #[serde(skip)]
    pub bumpy_case: String,
    pub value:      i32,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Enum {
    #[serde(flatten)]
    pub id:     Identifier,
    #[serde(default)]
    pub note:   String,
    pub fields: Vec<EnumField>,
}

#[derive(Debug, Copy, Clone)]
pub enum ModelConcept
{
    Nil,
    Team,
    Agent,
    Message,
    Goal,
    Plan,
    Role,
    Resource,
    Tactic,
    Action,
    Service,
    Enum,
    Event,
    // \note You can't iterate an enum without an external crate, you can't get
    // the length of enums without variant_count which is in Rust experimental
    // branch, see: https://github.com/rust-lang/rust/issues/73662
    // So we do C/C++ esque pattern here to iterate over enums.
    Count,
}

#[derive(Default)]
pub struct ModelErrorMetadata
{
    pub concept:       ModelConcept,
    pub error_key:     &'static str,
    pub reserved_name: &'static str,
}

pub struct ModelDupeNameError<'a>
{
    pub concept:     ModelConcept,
    pub id:          &'a Identifier,
    /// Human readable model locations for the error.
    pub key:         String,

    /// Machine readable model location for the error.
    pub machine_key: String,
}

#[derive(Serialize, Deserialize, Debug, Default, Clone)]
pub struct Model {
    pub project:   Project,
    pub enums:     Vec<Enum>,
    pub teams:     Vec<Agent>,
    pub agents:    Vec<Agent>,
    pub tactics:   Vec<Tactic>,
    pub roles:     Vec<Role>,
    pub resources: Vec<Resource>,
    pub actions:   Vec<Action>,
    pub goals:     Vec<Goal>,
    pub plans:     Vec<Plan>,
    pub messages:  Vec<Message>,
    pub services:  Vec<Service>,
    pub entities:  Vec<Entity>,
    pub events:    Vec<Event>,
}

#[derive(Default, Clone)]
pub struct LookupID {
    model_index:   usize,
}

#[derive(Default, Clone)]
pub struct ProjectWorkspace {
    pub id_table: HashMap<Identifier, LookupID>,
    pub models:   Vec<Model>,
}
static NIL_PROJECT: Project = Project::new();
static NIL_MODEL:   Model   = Model::new();

#[derive(Copy)]
pub struct AgentIterator<'a> {
    workspace:   &'a ProjectWorkspace,
    model_index: usize,
    item_index:  usize,
}

#[derive(Copy)]
pub struct EntityIterator<'a> {
    workspace:   &'a ProjectWorkspace,
    model_index: usize,
    item_index:  usize,
}

impl<'a> AgentIterator<'a> {
    pub fn new(workspace: &'a ProjectWorkspace) -> Self {
        let result = AgentIterator { workspace: workspace, model_index: 0, item_index: 0 };
        result
    }
}

impl<'a> Clone for AgentIterator<'a> {
    fn clone(&self) -> AgentIterator<'a> {
        let result = AgentIterator {
            workspace:   self.workspace,
            model_index: self.model_index,
            item_index:  self.item_index,
        };
        result
    }
}

impl<'a> Iterator for AgentIterator<'a> {
    type Item = Agent;
    fn next(&mut self) -> Option<Self::Item> {
        let mut result = None;
        while self.model_index < self.workspace.models.len() {
            let model = &self.workspace.models[self.model_index];
            if self.item_index < model.agents.len() {
                // \todo Make this a borrow but I don't understand enough Rust
                // to do this.
                let item         = model.agents[self.item_index].clone();
                self.item_index += 1;
                result           = Some(item);
                break
            } else {
                self.model_index += 1;
                self.item_index  = 0;
            }
        }
        result
    }
}

impl<'a> EntityIterator<'a> {
    pub fn new(workspace: &'a ProjectWorkspace) -> Self {
        let result = EntityIterator { workspace: workspace, model_index: 0, item_index: 0 };
        result
    }
}

impl<'a> Clone for EntityIterator<'a> {
    fn clone(&self) -> EntityIterator<'a> {
        let result = EntityIterator {
            workspace:   self.workspace,
            model_index: self.model_index,
            item_index:  self.item_index,
        };
        result
    }
}

impl<'a> Iterator for EntityIterator<'a> {
    type Item = Entity;
    fn next(&mut self) -> Option<Self::Item> {
        let mut result = None;
        while self.model_index < self.workspace.models.len() {
            let model = &self.workspace.models[self.model_index];
            if self.item_index < model.entities.len() {
                // \todo Make this a borrow but I don't understand enough Rust
                // to do this.
                let item         = model.entities[self.item_index].clone();
                self.item_index += 1;
                result           = Some(item);
                break
            } else {
                self.model_index += 1;
                self.item_index  = 0;
            }
        }
        result
    }
}

impl ProjectWorkspace {
    pub fn add_models(&mut self, mut models: Vec<Model>) {
        let start_index = self.models.len();
        self.models.append(&mut models);

        for model_index in start_index..self.models.len() {
            let model = &self.models[model_index];
            for concept_index in 0..ModelConcept::Count as u8 {
                let concept: ModelConcept = unsafe { std::mem::transmute(concept_index as u8) };
                match concept {
                    ModelConcept::Nil      => { },
                    ModelConcept::Count    => { },
                    ModelConcept::Team     => {
                        for item in model.teams.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Agent    => {
                        for item in model.agents.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Message  => {
                        for item in model.messages.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Goal     => {
                        for item in model.goals.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Plan     => {
                        for item in model.plans.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Role     => {
                        for item in model.roles.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Resource => {
                        for item in model.resources.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Tactic   => {
                        for item in model.tactics.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Action   => {
                        for item in model.actions.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Service  => {
                        for item in model.services.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Enum     => {
                        for item in model.enums.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                    ModelConcept::Event    => {
                        for item in model.events.iter() {
                            if self.id_table.get(&item.id).is_none() {
                                let mut id = item.id.clone();
                                id.module  = model.project.name.clone();
                                self.id_table.insert(id, LookupID{model_index: model_index});
                            }
                        }
                    },
                }
            }

        }
    }

    pub fn project(&self) -> &Project {
        match self.models.is_empty() {
            true  => { &NIL_PROJECT }
            false => { &self.models[0].project },
        }
    }

    pub fn agents(&self) -> AgentIterator { AgentIterator::new(self) }

    pub fn agents_len(&self) -> usize {
        let result = self.models.iter()
                         .map(|model| model.agents.len())
                         .sum();
        result
    }

    pub fn entities(&self) -> EntityIterator { EntityIterator::new(self) }

    pub fn entities_len(&self) -> usize {
        let result = self.models.iter()
                         .map(|model| model.entities.len())
                         .sum();
        result
    }


    pub fn lookup_model_with_id(&self, id: &Identifier) -> &Model {
        let mut result: &Model = &NIL_MODEL;
        match self.id_table.get(&id) {
            Some(lookup) => {
                result = &self.models[lookup.model_index];
            },
            None => { }
        }
        result
    }

    /**************************************************************************
     * Find
     **************************************************************************/
    pub fn find_action(&self, id: &Identifier) -> bool {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.find_action(id);
        result
    }

    pub fn find_goal(&self, id: &Identifier) -> bool {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.find_goal(id);
        result
    }

    pub fn find_message(&self, id: &Identifier) -> bool {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.find_message(id);
        result
    }

    /**************************************************************************
     * Get
     **************************************************************************/
    pub fn get_role(&self, id: &Identifier) -> Result<Role, Box<dyn std::error::Error>> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_role(id);
        result
    }

    pub fn get_goal(&self, id: &Identifier) -> Result<Goal, Box<dyn std::error::Error>> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_goal(id);
        result
    }

    pub fn get_message(&self, id: &Identifier) -> Result<Message, Box<dyn std::error::Error>> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_message(id);
        result
    }

    pub fn get_action(&self, id: &Identifier) -> Result<Action, Box<dyn std::error::Error>> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_action(id);
        result
    }

    pub fn get_action_cpp_function_prototype_args(&self, action: &Action) -> String {

        let mut result = "".to_string();

        // get the request message model
        let request_model: &Model = self.lookup_model_with_id(&action.request);

        // add the request message
        if let Ok(request_msg) = request_model.get_message(&action.request) {
            if request_msg.fields.len() > 0 {
                // \todo - need to namespace the message name
                result.push_str(&format!("const {}& request", request_msg.id.bumpy_case));
            }
        }

        // add the reply message
        let reply_model: &Model = self.lookup_model_with_id(&action.reply);
        if let Ok(reply_msg) = reply_model.get_message(&action.reply) {
            if reply_msg.fields.len() > 0 {
                if result.len() > 0 {
                    result.push_str(", ");
                }
                result.push_str(&format!("{}& reply", reply_msg.id.bumpy_case));
            }
        }

        // \todo support feedback messages

        // add the action handle
        if result.len() > 0 {
            result.push_str(", ");
        }

        result.push_str(&format!("aos::jack::ActionHandle handle"));
        result
    }

    pub fn get_action_cpp_function_args(&self, action: &Action) -> String {

        let mut result = "".to_string();

        // request message
        let request_model: &Model = self.lookup_model_with_id(&action.request);

        if let Ok(request_msg) = request_model.get_message(&action.request) {
            if request_msg.fields.len() > 0 {
                result.push_str("request");
            }
        }

        // reply message
        let reply_model: &Model = self.lookup_model_with_id(&action.reply);

        if let Ok(reply_msg) = reply_model.get_message(&action.reply) {
            if reply_msg.fields.len() > 0 {
                if result.len() > 0 {
                    result.push_str(", ");
                }
                result.push_str("reply");
            }
        }

        // add the action handle
        if result.len() > 0 {
            result.push_str(", ");
        }
        result.push_str("handle");
        result
    }

    pub fn get_message_fields(&self, id: &Identifier) -> Vec<Field> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_message_fields(id);
        result
    }

    pub fn get_action_request_msg_name(&self, id: &Identifier) -> Option<Identifier> {
        let model: &Model = self.lookup_model_with_id(&id);
        let result        = model.get_action_request_msg_name(id);
        result
    }

    /**************************************************************************
     * Get actions for and entity(agent, team or service)
     **************************************************************************/
    pub fn get_actions_used_by_entity(&self, id: &Identifier) -> Vec<Action> {

        let mut result : Vec<Action> = vec![];

        // get the model that contains the entity
        let model: &Model = self.lookup_model_with_id(&id);

        // get the agent/team - if it's an agent/team
        if let Some(agent) = model.get_agent_internal(id, true /*search_agents*/, true /*search_teams*/) {

            // loop the list of action identifiers and populate the result list
            for agent_action_id in agent.action_handlers.iter() {

                // find the action
                let action_model: &Model = self.lookup_model_with_id(&agent_action_id);

                let action_result = action_model.get_action(agent_action_id);
                if action_result.is_ok() {
                    let action = action_result.unwrap();
                    result.push(action);
                } else {
                    println!("Agent Action {} not found in any modules", agent_action_id.bumpy_case);
                }
            }
        }

        // get the service - if it's a service
        if let Some(service) = model.get_service(id) {

            // loop the list of action identifiers

            for service_action in service.action_handlers.iter() {

                let action_model: &Model = self.lookup_model_with_id(&service_action);

                let action_result = action_model.get_action(service_action);
                if action_result.is_ok() {
                    let action = action_result.unwrap();
                    result.push(action)
                } else {
                    println!("Service Action {} not found in any modules", service_action.bumpy_case);
                }
            }
        }

        result
    }

    // need to support pulling all the messages out of the other modules

    pub fn get_messages_used_by_entity(&self, id: &Identifier) -> Vec<Message> {

        let mut message_set = BTreeSet::<Identifier>::new();

        let model: &Model = self.lookup_model_with_id(&id);

        // get the agent/team - if it's an agent/team
        if let Some(agent) = model.get_agent_internal(id , true /*search_agents*/, true /*search_teams*/) {
            
            // \note Check messages used in the action handler
            for action in agent.action_handlers.iter() {
                let find_action = self.get_action(action);
                if find_action.is_ok() {
                    let action = find_action.unwrap();
                    if action.reply.name.len() > 0 {
                        message_set.insert(action.reply);
                    }

                    if action.request.name.len() > 0 {
                        message_set.insert(action.request);
                    }
                }
            }

            // \note Check messages used in the goals
            for goal in agent.goals.iter() {
                let find_goal = self.get_goal(&goal.id);
                if find_goal.is_ok() {
                    let goal = find_goal.unwrap();
                    message_set.insert(goal.message);
                }
            }

            // \note Check agent beliefs
            for belief in agent.beliefs.iter() {
                message_set.insert(belief.clone());
            }
        }

        // get the service - if it's a service
        if let Some(service) = model.get_service(id) {
            let msg_list = service.get_messages_id(self);
            for it in msg_list.into_iter() {
                message_set.insert(it);
            }
        }

        // \note If any of the messages are missing we ignore this as this will
        // be validated when the agent itself is validated.
        let mut result = Vec::<Message>::new();
        for msg_id in message_set.into_iter() {
            if let Ok(msg) = self.get_message(&msg_id) {
                result.push(msg)
            }
        }

        result
    }

    pub fn emit_cpp_message_include(&self, id: &Identifier) -> String {
        let mut result = "".to_string();
        let model: &Model = self.lookup_model_with_id(&id);
        result.push_str(&format!("#include <{}/meta/messages/{}meta.h>", model.project.bumpy_case.to_lowercase(), id.bumpy_case.to_lowercase() ));
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
    /// The tactic's goal is not supported by the plan
    TacticGoalNotSupportedByPlan,
    /// The tactic's plan loop value is not supported as it specifies out of bounds
    TacticPlanLoopValueOutOfBounds,
    /// There is a default value in an event's message that does not exist in the message's schema.
    DefaultValueFieldNotFound,
    /// This is an error state. Should never been used in normal operation.
    None
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

pub trait FieldAssignmentCppCode {
    fn field_member_assignment_string_to_cpp_code(&self, prefix_code: &str) -> String;
}

pub const VERSION_MAJOR: u32 = 0;
pub const VERSION_MINOR: u32 = 6;
pub const VERSION_PATCH: u32 = 0;

pub fn convert_to_bumpy_case(value: &str) -> String {
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
    result
}


impl Identifier {
    pub fn qualified_name(&self) -> String {
        let mut result: String = Default::default();
        let namespace_divider = ".";

        if self.name.is_empty() {
            // we don't qualify an empty name
            return result;
        }

        result.reserve(self.name.len() + namespace_divider.len() + self.module.len());
        if self.module.len() > 0 {
            result.push_str(self.module.as_str());
            result.push_str(namespace_divider);
        }
        result.push_str(self.name.as_str());
        result
    }

    // same as qualified_name but using _ so that we can generate valid c++ code
    pub fn variable_name(&self) -> String {
        let mut result: String = Default::default();
        let namespace_divider = "_";
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
        self.uuid == other.uuid
    }
}

impl Project {
    // \note Declaring static structs does not work using Rust's 'default' trait
    // because it does not support const yet so we manually provide a const
    // new() constructor
    //
    // https://github.com/rust-lang/rust/issues/67792
    pub const fn new() -> Project {
        const STRING: String = String::new();
        Project {
            name:          STRING,
            bumpy_case:    STRING,
            namespaces:    vec![],
            major_version: STRING,
            minor_version: STRING,
            patch_version: STRING,
            generator:     STRING,
            modules:       vec![],
            search_paths:  vec![],
        }
    }

    /// Verification for the project is minimal.
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

    pub fn namespace_string(&self, namespace_divider: &str) -> String {
        let mut result = String::default();
        let mut count  = 0;
        for ns in self.namespaces.iter() {
            if count > 0 {
                result.push_str(namespace_divider)
            }
            count += 1;
            result.push_str(ns);
        }
        result
    }

    pub fn cpp_namespace_string(&self) -> String {
        let result = self.namespace_string("::");
        result
    }

    /**************************************************************************
     * Emit
     **************************************************************************/
    pub fn emit_cpp_header_guard_begin(&self, name: &str, suffix: &str) -> String {
        let header_decl = format!("{}_{}{}_H", self.name.replace(" ", "_").to_uppercase(), name.replace(" ", "_").to_uppercase(), suffix);
        let mut result  = String::new();
        writeln_os!(result,     "#ifndef {}", header_decl);
        result.push_str(format!("#define {}", header_decl).as_str());
        result
    }

    pub fn emit_cpp_header_guard_end(&self, name: &str, suffix: &str) -> String {
        let header_decl = format!("{}_{}{}_H", self.name.replace(" ", "_").to_uppercase(), name.replace(" ", "_").to_uppercase(), suffix);
        let result      = format!("#endif /// {}", header_decl);
        result
    }

    pub fn emit_cpp_is_automatically_generated_comment(&self) -> String {
        let mut result  = String::new();
        writeln_os!(result, "/******************************************************************************");
        writeln_os!(result, " * This file has been automatically generated by jack-make");
        result.push_str(    " ******************************************************************************/");
        result
    }

    pub fn emit_cpp_namespace_begin(&self) -> String {
        let mut result = String::new();
        if self.namespaces.len() > 0 {
            writeln_os!(result, "namespace {}", self.cpp_namespace_string());
            result.push_str(    "{");
        }
        result
    }

    pub fn emit_cpp_namespace_end(&self) -> String {
        let mut result = String::new();
        if self.namespaces.len() > 0 {
            result = format!("}} /// namespace {}", self.cpp_namespace_string());
        }
        result
    }
}

impl FieldType {
    // \todo This API does not need to return a result or potential errors, a
    // none type means no type, e.g. an empty string. This returned error is
    // viral and infects the rest of the code with error handling.
    pub fn to_cpp_type(&self, workspace: &ProjectWorkspace, emit: &EmitNamespace) -> Result<String, Box<dyn std::error::Error>> {
        match self {
            FieldType::Bool => {
                return Ok("bool".to_string());
            },
            FieldType::I8 => {
                return Ok("int8_t".to_string());
            },
            FieldType::I16 => {
                return Ok("int16_t".to_string());
            },
            FieldType::I32 => {
                return Ok("int32_t".to_string());
            },
            FieldType::I64 => {
                return Ok("int64_t".to_string());
            },
            FieldType::U8 => {
                return Ok("uint8_t".to_string());
            },
            FieldType::U16 => {
                return Ok("uint16_t".to_string());
            },
            FieldType::U32 => {
                return Ok("uint32_t".to_string());
            },
            FieldType::U64 => {
                return Ok("uint64_t".to_string());
            },
            FieldType::F32 => {
                return Ok("float".to_string());
            },
            FieldType::F64 => {
                return Ok("double".to_string());
            },
            FieldType::String => {
                return Ok("std::string".to_string());
            },
            FieldType::Custom(id) => {
                let model             = workspace.lookup_model_with_id(id);
                let namespace_divider = if *emit == EmitNamespace::Cpp { "::" } else { "_" };
                let namespace         = model.project.namespace_string(namespace_divider);
                let result: String    = match namespace.len() > 0 && *emit != EmitNamespace::Nil {
                    true =>  { namespace + "::" + &convert_to_bumpy_case(&id.name) },
                    false => { convert_to_bumpy_case(&id.name) },
                };
                return Ok(result)
            },
            FieldType::Enum(_) => {
                return Ok("int32_t".to_string());
            },
            FieldType::Vec2 => {
                return Ok("aos::jack::V2".to_string());
            },
            FieldType::None => {
                return Err("FieldType set to None, invalid.".to_string().into())
            }
        }
    }

    /// Conversion from model type to prop type for code generation.
    pub fn to_prop_type(&self) -> Result<String, Box<dyn std::error::Error>> {
        match self {
            FieldType::Bool => {
                return Ok("Bool".to_string());
            },
            FieldType::I8 => { 
                return Ok("I8".to_string());
            },
            FieldType::I16 => { 
                return Ok("I16".to_string());
            },
            FieldType::I32 => {
                return Ok("I32".to_string());
            },
            FieldType::I64 => {
                return Ok("I64".to_string());
            },
            FieldType::U8 => { 
                return Ok("U8".to_string());
            },
            FieldType::U16 => { 
                return Ok("U16".to_string());
            },
            FieldType::U32 => {
                return Ok("U32".to_string());
            },
            FieldType::U64 => {
                return Ok("U64".to_string());
            },
            FieldType::F32 => {
                return Ok("F32".to_string());
            },
            FieldType::F64 => {
                return Ok("F64".to_string());
            },
            FieldType::String => {
                return Ok("String".to_string());
            },
            FieldType::Custom(s) => {
                return Ok(s.name.clone());
            },
            FieldType::Enum(_) => {
                return Ok("I32".to_string());
            },
            FieldType::Vec2 => {
                return Ok("V2".to_string());
            },
            FieldType::None => {
                return Err("FieldType set to None, invalid.".to_string().into())
            }
        }
    }

    /// Default value in C++ for code generation.
    pub fn to_cpp_default(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        match self {
            FieldType::Bool => {
                return Ok("false".to_string());
            },
            FieldType::I8 => {
                return Ok("int8_t{0}".to_string());
            },
            FieldType::I16 => {
                return Ok("int16_t{0}".to_string());
            },
            FieldType::I32 => {
                return Ok("int32_t{0}".to_string());
            },
            FieldType::I64 => {
                return Ok("int64_t{0}".to_string());
            },
            FieldType::U8 => {
                return Ok("uint8_t{0}".to_string());
            },
            FieldType::U16 => {
                return Ok("uint16_t{0}".to_string());
            },
            FieldType::U32 => {
                return Ok("uint32_t{0}".to_string());
            },
            FieldType::U64 => {
                return Ok("uint64_t{0}".to_string());
            },
            FieldType::F32 => {
                return Ok("0.0f".to_string());
            },
            FieldType::F64 => {
                return Ok("0.0".to_string());
            },
            FieldType::String => {
                return Ok("std::string{}".to_string());
            },
            FieldType::Custom(_) => {
                let cpp_type = self.to_cpp_type(workspace, &EmitNamespace::Cpp).unwrap();
                return Ok(format!("{}()", cpp_type));
            },
            FieldType::Enum(_) => {
                return Ok("0".to_string());
            },
            FieldType::Vec2 => {
                return Ok("aos::jack::V2{0.0f,0.0f}".to_string());
            },
            FieldType::None => {
                return Err("FieldType set to None, invalid.".to_string().into())
            }
        }
    }

    /// Returns true if the FieldType is a custom type.
    pub fn is_custom(&self) -> bool {
        match self {
            FieldType::Custom(_) => return true,
            _                    => return false,
        }
    }
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

impl Field {
    /// Returns the type that this field is.
    pub fn get_type(&self) -> FieldType {
        self.r#type.clone()
    }

    pub fn is_enum(&self) -> bool {
        match self.r#type {
            FieldType::Enum(_) => return true,
            _                  => return false,
        }
    }

    pub fn is_custom(&self) -> bool {
        match self.r#type {
            FieldType::Custom(_) => return true,
            _                    => return false,
        }
    }

    pub fn is_array(&self) -> bool {
        return self.is_array;
    }

    pub fn enum_type(&self) -> String {
        match &self.r#type {
            FieldType::Enum(identifier) => {
                return convert_to_bumpy_case(&identifier.name);
            },
            _                           => return "".to_string(),
        }
    }

    pub fn to_schema_cpp_type(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        let result = self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp);
        result
    }

    pub fn to_schema_prop_type(&self) -> Result<String, Box<dyn std::error::Error>> {
        if self.is_custom() {
            return Ok("Message".to_string());
        } else {
            return self.r#type.to_prop_type();
        }
    }

    /// Returns the C++ decl string taking into account if the type is an array
    /// or not and emitting the array syntax for C++ (e.g. std::vector<T>)
    pub fn to_cpp_type(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        if self.is_array {
            return Ok(format!("std::vector<{}>", self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp)?));
        } else {
            return self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp);
        }
    }

    /// Returns the C++ decl string taking into account if the type is an array
    /// and only returning the inner type
    pub fn to_cpp_inner_type(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        return self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp);
    }

    pub fn to_cpp_json_type(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        if self.is_enum() {
            return Ok(self.enum_type());
        } else if self.is_array() {
            if self.is_custom() {
                return Ok(format!("std::vector<{}::JsonConfig>", self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp)?));
            } else {
                return Ok(format!("std::vector<{}>", self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp)?));
            }
        } else {
            if self.is_custom() {
                return Ok(format!("{}::JsonConfig", self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp)?));
            } else {
                return self.r#type.to_cpp_type(workspace, &EmitNamespace::Cpp);
            }
        }
    }

    /// Returns the field C++ property type conversion
    pub fn to_prop_type(&self) -> Result<String, Box<dyn std::error::Error>> {
        if self.is_array {
            return Ok(format!("{}[]", self.r#type.to_prop_type()?));
        } else {
            return self.r#type.to_prop_type()
        }
    }

    /// Returns the default value. Either from the defaults 
    /// codified in this executable or stored per field in the model
    // TODO: If we create defaults in the model for an array field, it must be done in the editor.
    //       unless we just want an empty list then we can just dismiss it.
    pub fn to_cpp_default(&self, workspace: &ProjectWorkspace) -> Result<String, Box<dyn std::error::Error>> {
        let mut default_value = self.default.clone();
        if default_value.len() > 0 && self.r#type == FieldType::F32 {
            match default_value.parse::<f32>() {
                Ok(_) => {
                    // e.g. 0.0 or 0
                    if !default_value.ends_with("f") {
                        let mut has_dot = false;
                        for ch in default_value.chars() {
                            if ch == '.' {
                                has_dot = true;
                                break;
                            }
                        }

                        if has_dot {
                            default_value.push_str("f");
                        } else {
                            default_value.push_str(".f");
                        }
                    }
                },
                _ => { }
            }
        }

        if self.is_array {
            if default_value.len() > 0 {
                return Ok(format!("{}{{{}}}", self.to_cpp_type(workspace)?, default_value));
            } else {
                return Ok(format!("{}{}", self.to_cpp_type(workspace)?, "{}".to_string()));
            }
        } else {
            if default_value.len() > 0 {
                return Ok(default_value);
            } else {
                return self.r#type.to_cpp_default(workspace);
            }
        }
    }

    pub fn get_owning_model<'a>(&'a self, workspace: &'a ProjectWorkspace) -> &Model {
        let mut result: &Model = &NIL_MODEL;
        match &self.r#type {
            FieldType::Custom(id) => {
                result = workspace.lookup_model_with_id(&id);
            },
            _ => { }
        }
        result
    }
}

fn emit_cpp_forward_decls_for_messages(workspace: &ProjectWorkspace, msg_list: &[Identifier]) -> String{
    let mut namespace_mapping: BTreeMap</*namespace*/ String, /*messages belonging to this namespace*/ Vec<Identifier>> = BTreeMap::new();
    for msg in msg_list {
        let model         = &workspace.lookup_model_with_id(msg);
        let cpp_namespace = model.project.cpp_namespace_string();
        let map_it        = &mut namespace_mapping.entry(cpp_namespace.clone()).or_insert(vec![]);
        map_it.push(msg.clone());
    }

    let mut result: String = String::new();
    for (namespace, msg_array) in namespace_mapping.iter() {
        if namespace.len() > 0 {
            result.push_str(namespace);
            writeln_os!(result, "{}{{", os_newline());
        }
        for msg_id in msg_array.iter() {
            let msg = &workspace.get_message(msg_id).unwrap();
            writeln_os!(result, "class {};", msg.to_cpp_type(workspace));
        }

        if namespace.len() > 0 {
            writeln_os!(result, "{}}}", os_newline());
        }
    }
    result
}

impl Service {
    /**************************************************************************
     * Emit
     **************************************************************************/
    pub fn get_messages_id(&self, workspace: &ProjectWorkspace) -> Vec<Identifier> {
        let mut set = BTreeSet::<Identifier>::new();

        // \note Check messages used in the action handler
        for action in self.action_handlers.iter() {
            let model       = workspace.lookup_model_with_id(action);
            let find_action = model.get_action(action);
            if find_action.is_ok() {
                let action = find_action.unwrap();
                if action.reply.name.len() > 0 {
                    set.insert(action.reply);
                }

                if action.request.name.len() > 0 {
                    set.insert(action.request);
                }
            }
        }

        // \note Check service messages
        for topic in self.topics.iter() {
            set.insert(topic.message.clone());
        }

        let result = set.into_iter().collect();
        result
    }

    pub fn emit_cpp_forward_decls(&self, workspace: &ProjectWorkspace) -> String {
        let msg_list = self.get_messages_id(workspace);
        let result   = emit_cpp_forward_decls_for_messages(workspace, &msg_list);
        result
    }

}

impl Message {
    /// Verification for messages:
    /// 1. Checking that there are no fields with the same duplicate fields.
    /// 2. If a field has a custom type, whether the model contains the message required.
    fn verify(&self, workspace: &ProjectWorkspace, index: &usize) -> Vec<ModelError> {
        let mut errors:      Vec<ModelError>         = vec![];
        let mut field_names: BTreeMap<String, usize> = BTreeMap::new();
        let msg_full_name                            = &self.id.qualified_name();
 
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
                FieldType::Custom(id) => {
                    let found = workspace.find_message(&self.id);
                    if !found {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", msg_full_name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeNotFound,
                                                    format!("Custom field {} at index {} in Message {} cannot be found in Messages.",
                                                            id.qualified_name(),
                                                            i,
                                                            msg_full_name)));
                    }

                    if id == &self.id {
                        errors.push(ModelError::new(vec![format!("messages.{}.fields.{}", msg_full_name, i)],
                                                    vec![format!("messages.{}.fields.{}", index, i)],
                                                    ErrorType::CustomTypeIsCurrentMessage,
                                                    format!("Custom field {} at index {} in Message {} cannot be current Message.",
                                                            id.qualified_name(),
                                                            i,
                                                            msg_full_name)));

                    }
                },
                _ => { /* catch all do nothing!!!!! */ }
            }
        }

        errors
    }

    pub fn contains_simple(&self) -> bool {
        for field in self.fields.iter() {
            if !field.is_array {
                return true;
            }
        }
        false
    }

    /// Used to determine if imports require the header for vector in beliefmeta template.
    pub fn contains_array(&self) -> bool {
        for field in self.fields.iter() {
            if field.is_array {
                return true;
            }
        }
        false
    }

    pub fn contains_enum(&self) -> bool {
        for field in self.fields.iter() {
            match field.r#type {
                FieldType::Enum(_) => return true,
                _                  => continue,
            }
        }
        false
    }

    pub fn to_cpp_type(&self, workspace: &ProjectWorkspace) -> String {
        let field_type = FieldType::Custom(self.id.clone());
        let result     = field_type.to_cpp_type(workspace, &EmitNamespace::Cpp).unwrap_or_default();
        result
    }

    pub fn namespaced_cpp_name(&self, model: &Model) -> String {
        let mut result = String::default();
        let mut count = 0;
        for ns in model.project.namespaces.iter() {
            if count > 0 {
                result.push_str("::")
            }
            count += 1;
            result.push_str(ns);
        }
        result.push_str(&self.id.bumpy_case);
        result
    }

    pub fn get_owning_model<'a>(&'a self, workspace: &'a ProjectWorkspace) -> &Model {
        let result = workspace.lookup_model_with_id(&self.id);
        result
    }

    pub fn emit_cpp_includes(&self, workspace: &ProjectWorkspace) -> String {
        let mut include_set = BTreeSet::<String>::new();
        for it in self.fields.iter() {
            match &it.r#type {
                FieldType::Custom(id) => {
                    let model = workspace.lookup_model_with_id(id);
                    include_set.insert(format!("#include <{}/meta/messages/{}meta.h>",
                                               model.project.bumpy_case.to_lowercase(),
                                               it.get_type().to_cpp_type(workspace, &EmitNamespace::Nil).unwrap().to_lowercase()));
                },
                FieldType::Enum(id) => {
                    let model = workspace.lookup_model_with_id(id);
                    include_set.insert(format!("#include <{}/meta/{}enumsmeta.h>",
                                               model.project.bumpy_case.to_lowercase(),
                                               model.project.bumpy_case.to_lowercase()));
                },
                _ => {}
            }
        }

        let mut result = String::new();
        for (index, it) in include_set.iter().enumerate() {
            if index > 0 {
                result.push_str(os_newline());
            }
            result.push_str(it.as_str());
        }
        result
    }
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
        for (i, goal) in self.goals.iter().enumerate() {
            if goal.id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, i)],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Goal at index {} must not be empty.",i)));
            }

            if !model.find_goal(&goal.id) {
                errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, goal.id.qualified_name())],
                                            vec![format!("{}s.{}.goals.{}", agent_type, index, i)],
                                            ErrorType::GoalNotFound,
                                            format!("Message {} at index {} cannot be found in messages.", goal.id.qualified_name(), i)));
            }
        }

        let mut goal_ids : Vec<Identifier> = vec![];
        for (_, val) in self.goals.iter().enumerate() {
            goal_ids.push(val.id.clone());
        }

        errors.append(&mut verify_no_duplicate_fields(&goal_ids,
                                                      &format!("{}s.{}.goals", agent_type, full_name),
                                                      &format!("{}s.{}.goals", agent_type, index),
                                                      &format!("{:?}", agent_type),
                                                      full_name,
                                                      &"goals".to_string()));

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
            for (j, agent_goal) in self.goals.iter().enumerate() {
                if &goal.id == &agent_goal.id {

                    if agent_goal.startup_tactic.name.len() > 0 {
                        if !model.find_tactic(&agent_goal.startup_tactic) {
                            errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}.startup_tactic.{}", agent_type, full_name, agent_goal.id.qualified_name(), agent_goal.startup_tactic.qualified_name())],
                                                        vec![format!("{}s.{}.goals.{}.startup_tactic", agent_type, index, i)],
                                                        ErrorType::TacticNotFound,
                                                        format!("Goal '{}' startup tactic '{}' does not exist.", agent_goal.id.qualified_name(), agent_goal.startup_tactic.qualified_name())));
                        }
                    }

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

                        errors.push(ModelError::new(vec![format!("{}s.{}.goals.{}", agent_type, full_name, agent_goal.id.qualified_name()),
                                                         format!("goals.{}.beliefsets.{}", agent_goal.id.qualified_name(), bs.qualified_name())],
                                                    vec![format!("{}s.{}.goals.{}", agent_type, index, j),
                                                         format!("goals.{}.beliefsets.{}", i, bs_index)],
                                                    ErrorType::BeliefsetInGoalNotInAgent,
                                                    format!("Goal {} with beliefset {} cannot be found in agent or agent roles.", agent_goal.id.qualified_name(), bs.qualified_name())));
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
                            if let Some(message) = task.action.as_ref() {
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
        for (j, agent_goal) in self.goals.iter().enumerate() {
            for (i, goal) in model.goals.iter().enumerate() {

                // Find the matching goal in the model.
                if &goal.id != &agent_goal.id {
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
        errors
    }

    pub fn has_desires(&self) -> bool {
        let mut result = false;
        for (_, goal) in self.goals.iter().enumerate() {
            if goal.startup_goal {
                result = true;
                break;
            }
        }
        result
    }

    pub fn desires(&self) -> Vec<Identifier> {
        let mut result: Vec<Identifier> = Vec::new();
        for (_, goal) in self.goals.iter().enumerate() {
            if goal.startup_goal {
                result.push(goal.id.clone());
            }
        }
        result
    }

    pub fn has_tactics(&self) -> bool {
        let mut result = false;
        for (_, goal) in self.goals.iter().enumerate() {
            if goal.startup_tactic.name.len() > 0 {
                result = true;
                break;
            }
        }
        result
    }

    pub fn tactics(&self) -> Vec<Identifier> {
        let mut result: Vec<Identifier> = Vec::new();
        for (_, goal) in self.goals.iter().enumerate() {
            if goal.startup_tactic.name.len() > 0 {
                result.push(goal.startup_tactic.clone());
            }
        }
        result
    }
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
                                            format!("Goal message with name '{}' cannot be found in messages.", self.message.qualified_name())));
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
            if plan.handles.module.len() == 0 && &plan.handles.name == &full_name {
                handle_cnt += 1;
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

    pub fn emit_cpp_forward_decls(&self, workspace: &ProjectWorkspace) -> String {
        let result = emit_cpp_forward_decls_for_messages(workspace, &self.query_messages);
        result
    }
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


impl Task {
    /// Return type of task.
    /// Helper funtion to make it easier for codegen.
    pub fn get_type(&self) -> TaskType {
        self.r#type.clone()
    }

    pub fn get_label(&self) -> String {
        let result: String;
        match self.r#type {
            TaskType::action => {
                result = self.action.as_ref().unwrap().variable_name();
            },
            TaskType::sleep => {
                result = "Sleep".to_string();
            },
            TaskType::goal => {
                result = self.goal.as_ref().unwrap().variable_name();
            },
            TaskType::condition => {
                result = self.conditiontext.as_ref().unwrap().to_string();
            },
            TaskType::None => {
                result = "".to_string();
            },
        }
        result
    }

    pub fn get_bumpy_case_label(&self) -> String {
        let label = self.get_label();
        let result = convert_to_bumpy_case(&label);
        result
    }

    // return the condition text
    // replace the spaces for under scores
    pub fn get_condition_text(&self) -> String {

        match &self.conditiontext {
            Some(val) => {
                convert_to_bumpy_case(val)
            },
            None => {
                String::new()
            }
        }
    }

    /// Returns true if this key is here and is true
    pub fn is_async(&self) -> bool {
        match self.r#async {
            Some(val) => {
                return val;
            },
            None => {
                return false;
            }
        }
    }

    /// Returns true if a "from" mapping is available for a "to" param
    pub fn has_mapping(&self, param_name : &str) -> bool {
        self.get_mapping(param_name).is_some()
    }

    // is this mapping a boolean value
    pub fn is_mapping_bool(&self, param_name : &str) -> bool {

        let from : Option<String> = self.get_mapping(param_name);

        if self.get_mapping(param_name).is_some() {
            let s : String = from.unwrap();
            let test = bool::from_str(&s);

            match test {
                Ok(_) => true,
                Err(_) => false
            }
        } else {
            false
        }
    }

    pub fn is_mapping_int(&self, param_name : &str) -> bool {

        let from : Option<String> = self.get_mapping(param_name);

        if self.get_mapping(param_name).is_some() {
            let s : String = from.unwrap();
            let test = s.parse::<i32>();

            match test {
                Ok(_) => true,
                Err(_) => false
            }
        } else {
            false
        }
    }

    // Returns the JACK type for a message field 
    pub fn is_literal_mapping(&self, field : Field) -> bool {

        let from : Option<String> = self.get_mapping(&field.name);
        if from.is_some() {
            let literal_str : String = from.unwrap();
            let field_type: FieldType = field.get_type();

            let test_result: bool = match field_type {
                FieldType::Bool => literal_str.parse::<bool>().is_ok(),
                FieldType::I8   => literal_str.parse::<i8>().is_ok(),
                FieldType::I16  => literal_str.parse::<i16>().is_ok(),
                FieldType::I32  => literal_str.parse::<i32>().is_ok(),
                FieldType::I64  => literal_str.parse::<i64>().is_ok(),
                FieldType::U8   => literal_str.parse::<u8>().is_ok(),
                FieldType::U16  => literal_str.parse::<u16>().is_ok(),
                FieldType::U32  => literal_str.parse::<u32>().is_ok(),
                FieldType::U64  => literal_str.parse::<u64>().is_ok(),
                FieldType::F32  => literal_str.parse::<f32>().is_ok(),
                FieldType::F64  => literal_str.parse::<f64>().is_ok(),
                FieldType::String => {
                    let first_char = literal_str.chars().next();
                    let last_char = literal_str.chars().rev().next();

                    first_char.is_some() && last_char.is_some() &&
                    first_char.unwrap() == '"' && last_char.unwrap() == '"'
                },
                FieldType::Custom(_) => false, // not supported yet (need json to message parse using serde)
                FieldType::Enum(_) => literal_str.parse::<i32>().is_ok(), // use an integer mapping for now
                FieldType::Vec2 => {
                    // do we have 2 floats seperated by a comma?
                    let mut iter = literal_str.split(',');
                    if iter.clone().count() == 2 {
                        let first_valid = iter.next().unwrap().trim().parse::<f32>().is_ok();
                        let second_valid = iter.next().unwrap().trim().parse::<f32>().is_ok();

                        first_valid && second_valid
                    } else {
                        false
                    }
                },
                FieldType::None => false
            };

            return test_result;
        }

        false
    }

    /// Returns the "from" mapping to a variable (if one is available)
    pub fn get_mapping(&self, param_name : &str) -> Option<String> {
        match &self.mappings {
            Some(mappings) => {
                for map in mappings.iter() {
                    if &map.to == param_name {
                        return Some(map.from.clone());
                    }
                }
            },
            None => { /* fall through */ }
        }

        None
    }
}

impl Plan {

    pub fn id32_to_hex16_pair_string(&self, id: &str) -> String {
        let input           = id.replace('-', "");
        let (first, second) = input.split_at(input.len() / 2);
        let result          = format!("0x{}ULL, 0x{}ULL", first, second);
        result
    }

    /// Returns a task label based on it's id
    /// TODO: hopefully the editor will output this soon
    pub fn task_number_id_label(&self, id: &str) -> String {
        let mut label: String = String::new();
        for (index, task) in self.tasks.iter().enumerate() {
            if task.id.as_str() == id {
                label.push_str(&index.to_string());
                label.push_str(&convert_to_bumpy_case(&task.get_label()))
            }
        }

        if label.len() == 0 {
            label = format!("Task ID not found in plan [plan={}, id={}]", self.id.qualified_name(), id);
        }
        return label;
    }

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
        if !model.find_goal(&self.handles) {
            errors.push(ModelError::new(vec![format!("plans.{}.handles", full_name)],
                                        vec![format!("plans.{}.handles", index)],
                                        ErrorType::GoalNotFound,
                                        format!("Plan {} handles a goal {} that cannot be found.", full_name, self.handles.qualified_name())));
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

                    match &task.action {
                        Some(msg_id) => {
                            if !model.find_action(msg_id) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::ActionNotFound,
                                                            format!("Action task {} uses action {} at index {} of tasks in Plan {} cannot be found in actions.",
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
                                                        format!("Action task {} at index {} of tasks in Plan {} does not have an action defined.", task.id, i, full_name)));
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

                    match &task.goal {
                        Some(goal_id) => {
                            if !model.find_goal(&goal_id) {
                                errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                            vec![format!("plans.{}.tasks.{}", index, i)],
                                                            ErrorType::GoalNotFound,
                                                            format!("Task {} in plan '{}' for goal '{}' is trying to sub-goal to a non-existent goal '{}'",
                                                                    i,
                                                                    full_name,
                                                                    self.handles.qualified_name(),
                                                                    goal_id.qualified_name(),
                                                                    )));
                            } else {
                                let goal = model.get_goal(&goal_id).unwrap();
                                if !goal.message.name.is_empty() && !model.find_message(&goal.message) {
                                    errors.push(ModelError::new(vec![format!("plans.{}.tasks.{}", full_name, task.id)],
                                                                vec![format!("plans.{}.tasks.{}", index, i)],
                                                                ErrorType::MessageNotFound,
                                                                format!("Task {} in plan '{}' for goal '{}' executes the sub-goal '{}' but the goal parameters (message '{}') does not exist",
                                                                        i,
                                                                        full_name,
                                                                        self.handles.qualified_name(),
                                                                        goal_id.qualified_name(),
                                                                        goal.message.qualified_name())));
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
                                            format!("Role at {} specified message that has duplicate entries of '{}' at indicies {} and {}.",
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

impl std::fmt::Display for TacticPolicy {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl std::default::Default for TacticPolicy {
    fn default() -> Self { Self::ExcludePlanAfterAttempt }
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

        for (i, plan_id) in self.plan_list.iter().enumerate() {
            if plan_id.name.len() == 0 {
                errors.push(ModelError::new(vec![format!("tactics.{}.plan_list.{}", full_name, i)],
                                            vec![format!("tactics.{}.plan_list.{}", index, i)],
                                            ErrorType::ZeroSizeString,
                                            format!("Plan at index {} must not be empty.",i)));
            }

            match model.get_plan(&plan_id) {
                Ok(plan) => {
                    if plan.handles != self.goal {
                        errors.push(ModelError::new(vec![format!("tactics.{}.plan_list.{}", full_name, plan_id.qualified_name())],
                                                    vec![format!("tactics.{}.plan_list.{}", index, i)],
                                                    ErrorType::TacticGoalNotSupportedByPlan,
                                                    format!("Tactic uses plan '{}' with a different goal '{}' than the goal '{}' supported by the tactic cannot be found in plans.",
                                                            plan_id.qualified_name(),
                                                            plan.handles.qualified_name(),
                                                            self.goal.qualified_name())));
                    }
                }
                Err(_) => {
                    errors.push(ModelError::new(vec![format!("tactics.{}.plan_list.{}", full_name, plan_id.qualified_name())],
                                                vec![format!("tactics.{}.plan_list.{}", index, i)],
                                                ErrorType::PlanNotFound,
                                                format!("Tactic '{}' references non-existent plan '{}'", full_name, plan_id.qualified_name())));
                },
            }
        }

        if self.plan_loop < -1 || self.plan_loop > i64::from(u32::MAX) {
            errors.push(ModelError::new(vec![format!("tactics.{}.plan_loop", full_name)],
                                        vec![format!("tactics.{}.plan_loop", index)],
                                        ErrorType::TacticPlanLoopValueOutOfBounds,
                                        format!("Tactic '{}' plan loop value: {}, is not an acceptable value, it must in the following range [-1, 2^32)", full_name, self.plan_loop)));
        }

        errors.append(&mut verify_no_duplicate_fields(&self.plan_list,
                                                      &format!("tactics.{}.plan_list", full_name),
                                                      &format!("tactics.{}.plan_list", index),
                                                      &"tactics".to_string(),
                                                      &full_name,
                                                      &"plans".to_string()));
        errors
    }
}

impl Action {
    fn verify(&self, model: &Model, index : &usize) -> Vec<ModelError> {
        let mut errors : Vec<ModelError> = Vec::new();
        let full_name = self.id.qualified_name();
        if self.request.name.len() > 0 {
            if let Err(_) = model.get_message(&self.request) {
                errors.push(ModelError::new(vec![format!("actions.{}.request", full_name)],
                                            vec![format!("actions.{}.request", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Action uses a request message '{}' that does not exist.", self.request.qualified_name())));
            }
        }

        if self.reply.name.len() > 0 {
            if let Err(_) = model.get_message(&self.reply) {
                errors.push(ModelError::new(vec![format!("actions.{}.reply", full_name)],
                                            vec![format!("actions.{}.reply", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Action uses a reply message '{}' that does not exist.", self.reply.qualified_name())));
            }
        }
        errors
    }
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
impl FieldAssignmentCppCode for &EventMessageDefaultValue {
    // TODO: This needs to be applied for the message editor. If a
    // message has a nested message it should be possible to override the value
    // from the parent message's definition.
    // TODO: Validator needs to check if it can parse the default value

    /// Convert a hierarchical path to a field in a JACK message and generate
    /// C++ code that assigns the value to the path specified. For example,
    /// this data structure:
    ///
    ///   struct V2     { F32 x, F32 y; };
    ///   struct Entity { V2 pos; };
    ///
    /// Given an entity declared 'monster' at position (32, 64), can be
    /// specified with the following function parameters
    ///
    ///   name:  "monster.pos.x"
    ///   value: "{32, 64}"
    ///
    /// This function will generate a string suitable for code generation inside
    /// the JACK message member class, e.g.
    ///
    ///   pos().x() = {32, 64};
    ///
    /// More generally
    ///
    ///   <prefix_code><name> = <value>
    fn field_member_assignment_string_to_cpp_code(&self, prefix_code: &str) -> String {
        if self.name.is_empty() || self.value.is_empty() {
            return "".to_string();
        }

        let mut result           = prefix_code.to_string();
        let split_result: Vec<_> = self.name.split('.').collect();
        let mut emit_count       = 0;
        for split in split_result.iter() {
            if split.is_empty() {
                continue;
            }

            let mut array_index_list: Vec<&str> = Vec::new();
            let mut first_bracket_encountered   = false;
            let mut one_before_bracket_begin    = split.len();
            let mut bracket_begin               = 0;
            for (index, ch) in split.chars().enumerate() {
                if ch == '[' {
                    bracket_begin = index + 1;
                    if !first_bracket_encountered {
                        first_bracket_encountered = true;
                        one_before_bracket_begin  = bracket_begin - 1;
                    }
                } else if ch == ']' {
                    let bracket_one_past_end = index;
                    match split.get(bracket_begin..bracket_one_past_end) {
                        Some(str_slice) => {
                            if str_slice.len() > 0 {
                                array_index_list.push(str_slice);
                            }
                        },
                        _ => {}
                    }
                }
            }

            let variable_name = split.get(0..one_before_bracket_begin).unwrap();
            if emit_count > 0 {
                result.push('.');
            }
            result.push_str(format!("{}()", variable_name).as_str());
            for array_index in array_index_list.iter() {
                result.push_str(format!("[{}]", array_index).as_str());
            }
            emit_count += 1
        }
        result.push_str(format!(" = {};", self.value).as_str());
        result
    }
}

impl Event {
    fn verify(&self, model: &Model, index: &usize) -> Vec<ModelError> {
        let mut errors:              Vec<ModelError>         = vec![];
        let mut default_field_names: BTreeMap<String, usize> = BTreeMap::new();
        let event_name                                       = &self.id.qualified_name();

        match model.get_message(&self.message.id) {
            Ok(msg) => {
                for (field_index, default_val) in self.message.defaults.iter().enumerate() {
                    // \note Check if the default value we're assigning to a field, is a field that
                    // exists in the actual message.
                    let msg_field = msg.fields.iter().find(|msg_field| msg_field.name == default_val.name);
                    match msg_field {
                        Some(_) => {
                            // \todo We need to verify the string can be serialised into a C++ type
                            // for codegen. Maybe create a transient Field and vet that all the
                            // code emitting functions can emit a C++ type without errors.
                            //
                            // Right now we will just emit the text into a C++ file and rely on a
                            // compile error when the C++ project is compiled.
                        },
                        None => {
                            errors.push(ModelError::new(vec![format!("events.{}.defaults.{}", event_name, default_val.name)],
                                                        vec![format!("events.{}.defaults.{}", index,     field_index)],
                                                        ErrorType::DefaultValueFieldNotFound,
                                                        format!("Event '{}' specifies a default value for field '{}' which does not exist in message '{}'",
                                                                event_name,
                                                                default_val.name,
                                                                self.message.id.qualified_name())));
                        },
                    }

                    if default_field_names.contains_key(&default_val.name) {
                        let other_index = default_field_names.get(&default_val.name).unwrap();
                        errors.push(ModelError::new(vec![format!("events.{}.fields.{}", event_name, field_index),
                                                         format!("events.{}.fields.{}", event_name, other_index)],
                                                    vec![format!("events.{}.fields.{}", index,      field_index),
                                                         format!("events.{}.fields.{}", event_name, other_index)],
                                                    ErrorType::MessageDuplicateFields,
                                                    format!("Event '{}' specifies duplicate a default value for field '{}' in message '{}'",
                                                            event_name,
                                                            default_val.name,
                                                            self.message.id.qualified_name())));
                    } else {
                        default_field_names.insert(default_val.name.clone(), field_index.clone());
                    }
                }
            },
            Err(_)  => {
                errors.push(ModelError::new(vec![format!("events.{}.id", event_name)],
                                            vec![format!("events.{}.id", index)],
                                            ErrorType::MessageNotFound,
                                            format!("Event uses a message '{}' that does not exist.", event_name)));
            }
        }

        errors
    }

    pub fn namespaced_cpp_name(&self, model: &Model) -> String {
        let mut result = String::default();
        let mut count = 0;
        for ns in model.project.namespaces.iter() {
            if count > 0 {
                result.push_str("::")
            }
            count += 1;
            result.push_str(ns);
        }
        result.push_str(&self.id.bumpy_case);
        result
    }

}

impl AsRef<Model> for Model {
    fn as_ref(&self) -> &Model {
        &self
    }
}

impl Default for ModelConcept {
    fn default() -> Self { ModelConcept::Nil }
}

impl Model {
    pub const fn new() -> Model {
        Model {
            project:   Project::new(),
            enums:     vec![],
            teams:     vec![],
            agents:    vec![],
            tactics:   vec![],
            roles:     vec![],
            resources: vec![],
            actions:   vec![],
            goals:     vec![],
            plans:     vec![],
            messages:  vec![],
            services:  vec![],
            entities:  vec![],
            events:    vec![],
        }
    }

    fn check_zero_length_name(&self, metadata: &ModelErrorMetadata, errors: &mut Vec<ModelError>, id: &Identifier, error_index: usize) {
        if id.name.len() == 0 {
            errors.push(ModelError::new(vec![],
                                        vec![format!("{}.{}", metadata.error_key, error_index)],
                                        ErrorType::ZeroSizeString,
                                        format!("{} name at index {} must not be empty.", metadata.reserved_name, error_index)));
        }
    }

    fn check_zero_length_uuid(&self, metadata: &ModelErrorMetadata, errors: &mut Vec<ModelError>, id: &Identifier, error_index: usize) {
        if id.name.len() == 0 {
            errors.push(ModelError::new(vec![],
                                        vec![format!("{}.{}", metadata.error_key, error_index)],
                                        ErrorType::ZeroSizeString,
                                        format!("{} '{}' at index {} does not have an UUID assigned to it.", metadata.reserved_name, id.name, error_index)));
        }
    }

    fn check_name_is_not_reserved(&self, metadata: &ModelErrorMetadata, errors: &mut Vec<ModelError>, id: &Identifier, error_index: usize) {
        if id.name == metadata.reserved_name {
            errors.push(ModelError::new(vec![format!("{}.{}", metadata.error_key, id.name)],
                                        vec![format!("{}.{}", metadata.error_key, error_index)],
                                        ErrorType::NameSameAsType,
                                        format!("{} at index {} is using reserved name '{}'.", metadata.reserved_name, error_index, metadata.reserved_name)));
        }
    }

    fn check_duplicate_name<'a>(&self, map: &mut BTreeMap<String, ModelDupeNameError<'a>>, metadata: &ModelErrorMetadata, errors: &mut Vec<ModelError>, id: &'a Identifier, error_index: usize) {
        match map.get(&id.name) {
            Some(it) => {
                errors.push(ModelError::new(vec![it.key.clone(),         format!("{}.{}", metadata.error_key, id.name)],
                                            vec![it.machine_key.clone(), format!("{}.{}", metadata.error_key, error_index)],
                                            ErrorType::DuplicateName,
                                            format!("{} '{}' has a duplicated name with {:?} '{}' (id of duplicated item is: '{}')", metadata.reserved_name, id.name, it.concept, id.name, id.uuid)));
            },
            None => {
                let error = ModelDupeNameError {
                    concept:     metadata.concept,
                    id:          id,
                    key:         format!("{}.{}", metadata.error_key, id.name),
                    machine_key: format!("{}.{}", metadata.error_key, error_index),
                };
                map.insert(id.uuid.to_string(), error);
            }
        }
    }

    fn check_duplicate_uuid<'a>(&self, map: &mut BTreeMap<String, ModelDupeNameError<'a>>, metadata: &ModelErrorMetadata, errors: &mut Vec<ModelError>, id: &'a Identifier, error_index: usize) {
        match map.get(&id.uuid) {
            Some(it) => {
                errors.push(ModelError::new(vec![it.key.clone(),         format!("{}.{}", metadata.error_key, id.uuid)],
                                            vec![it.machine_key.clone(), format!("{}.{}", metadata.error_key, error_index)],
                                            ErrorType::DuplicateName,
                                            format!("{} '{}' has a duplicated UUID with {:?} '{}' (id was: '{}')", metadata.reserved_name, id.name, it.concept, it.id.name, id.uuid)));
            },
            None => {
                let error = ModelDupeNameError {
                    concept:     metadata.concept,
                    id:          id,
                    key:         format!("{}.{}", metadata.error_key, id.uuid),
                    machine_key: format!("{}.{}", metadata.error_key, error_index),
                };
                map.insert(id.uuid.to_string(), error);
            }
        }
    }

    /// Model verification which returns an array of encountered errors if any
    pub fn verify(&self, workspace: &ProjectWorkspace) -> Vec<ModelError> {
        let mut error_metadata: [ModelErrorMetadata; ModelConcept::Count as usize] = Default::default();
        for (index, it) in error_metadata.iter_mut().enumerate() {
            let enum_val: ModelConcept = unsafe { std::mem::transmute(index as u8) };
            match enum_val {
                ModelConcept::Nil      => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "nils", reserved_name: "Nil", };
                },
                ModelConcept::Team     => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "teams", reserved_name: "Team", };
                },
                ModelConcept::Agent    => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "agents", reserved_name: "Agent", };
                },
                ModelConcept::Message  => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "messages", reserved_name: "Message", };
                },
                ModelConcept::Goal     => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "goals", reserved_name: "Goal", };
                },
                ModelConcept::Plan     => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "plans", reserved_name: "Plan", };
                },
                ModelConcept::Role     => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "roles", reserved_name: "Role", };
                },
                ModelConcept::Resource => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "resources", reserved_name: "Resource", };
                },
                ModelConcept::Tactic   => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "tactics", reserved_name: "Tactic", };
                },
                ModelConcept::Action   => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "actions", reserved_name: "Action", };
                },
                ModelConcept::Service  => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "services", reserved_name: "Service", };
                },
                ModelConcept::Enum     => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "enums", reserved_name: "Enum", };
                },
                ModelConcept::Event    => {
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "events", reserved_name: "Event", };
                },
                ModelConcept::Count    => {
                    // \note: This is filled out so that if someone
                    // mistakenly uses it, jack-make will clearly emit an
                    // errorneous value which can be caught rather than an
                    // entry string.
                    *it = ModelErrorMetadata { concept: enum_val, error_key: "counts", reserved_name: "Count", };
                },
            }
        }

        let mut duplicate_uuid_map: BTreeMap<String, ModelDupeNameError> = BTreeMap::new();
        let mut duplicate_name_map: BTreeMap<String, ModelDupeNameError> = BTreeMap::new();
        let mut errors:             Vec<ModelError>                      = self.project.verify();

        for (i, team) in self.teams.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Team as usize];
            let id       = &team.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut team.verify(&AgentType::Team, &self, &i));
        }

        for (i, agent) in self.agents.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Agent as usize];
            let id       = &agent.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut agent.verify(&AgentType::Agent, &self, &i));
        }

        for (i, msg) in self.messages.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Message as usize];
            let id       = &msg.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut msg.verify(workspace, &i));
        }

        for (i, goal) in self.goals.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Goal as usize];
            let id       = &goal.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut goal.verify(&self, &i));
        }

        for (i, plan) in self.plans.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Plan as usize];
            let id       = &plan.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut plan.verify(&self, &i));
        }

        for (i, role) in self.roles.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Role as usize];
            let id       = &role.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut role.verify(&self, &i));
        }

        for (i, resource) in self.resources.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Resource as usize];
            let id       = &resource.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut resource.verify(&self, &i));
        }

        for (i, tactic) in self.tactics.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Tactic as usize];
            let id       = &tactic.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut tactic.verify(&self, &i));
        }

        for (i, action) in self.actions.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Action as usize];
            let id       = &action.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut action.verify(&self, &i));
        }

        for (i, service) in self.services.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Service as usize];
            let id       = &service.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            // TODO: No rules to verify services yet?!?
            // errors.append(&mut service.verify(&self, &i));
        }

        for (i, enum_val) in self.enums.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Enum as usize];
            let id       = &enum_val.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut enum_val.verify(&self, &i));
        }

        for (i, event) in self.events.iter().enumerate() {
            let metadata = &error_metadata[ModelConcept::Event as usize];
            let id       = &event.id;
            self.check_zero_length_name(metadata, &mut errors, id, i);
            self.check_zero_length_uuid(metadata, &mut errors, id, i);
            self.check_name_is_not_reserved(metadata, &mut errors, id, i);
            self.check_duplicate_name(&mut duplicate_name_map, metadata, &mut errors, id, i);
            self.check_duplicate_uuid(&mut duplicate_uuid_map, metadata, &mut errors, id, i);
            errors.append(&mut event.verify(&self, &i));
        }

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

    /// Returns true if there is a service with the name supplied.
    pub fn find_service(&self, identifier : &Identifier) -> bool {
        for service in self.services.iter() {
            if &service.id == identifier {
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

    /**************************************************************************
     * Get
     **************************************************************************/
    pub fn get_message_fields(&self, id: &Identifier) -> Vec<Field> {
        let mut result: Vec<Field> = vec![];
        if id.name.len() > 0 {
            for msg in self.messages.iter() {
                if &msg.id == id {
                    result = msg.fields.clone();
                    break;
                }
            }
        }
        result
    }

    pub fn get_message(&self, id: &Identifier) -> Result<Message, Box<dyn std::error::Error>> {
        if id.name.len() > 0 {
            for msg in self.messages.iter() {
                if &msg.id == id {
                    return Ok(msg.clone());
                }
            }
        }
        Err(format!("Failed to get message '{}': Does not exist in model '{}'", id.qualified_name(), self.project.name).into())
    }

    pub fn get_goal(&self, id: &Identifier) -> Result<Goal, Box<dyn std::error::Error>> {
        if id.name.len() > 0 {
            for goal in self.goals.iter() {
                if &goal.id == id {
                    return Ok(goal.clone());
                }
            }
        }
        Err(format!("Failed to get goal '{}': Does not exist in model '{}'", id.qualified_name(), self.project.name).into())
    }

    pub fn get_plan(&self, id: &Identifier) -> Result<Plan, Box<dyn std::error::Error>> {
        if id.name.len() > 0 {
            for plan in self.plans.iter() {
                if &plan.id == id {
                    return Ok(plan.clone());
                }
            }
        }
        Err(format!("Failed to get plan '{}': Does not exist in model '{}'", id.qualified_name(), self.project.name).into())
    }

    pub fn get_role(&self, id: &Identifier) -> Result<Role, Box<dyn std::error::Error>> {
        if id.name.len() > 0 {
            for role in self.roles.iter() {
                if &role.id == id {
                    return Ok(role.clone());
                }
            }
        }
        Err(format!("Failed to get role '{}': Does not exist in model '{}'", id.qualified_name(), self.project.name).into())
    }

    pub fn get_action(&self, id: &Identifier) -> Result<Action, Box<dyn std::error::Error>> {
        if id.name.len() > 0 {
            for action in self.actions.iter() {
                if &action.id == id {
                    return Ok(action.clone());
                }
            }
        }
        Err(format!("Failed to get action '{}': Does not exist in model '{}'", id.qualified_name(), self.project.name).into())
    }

    pub fn get_action_request_msg_name(&self, id: &Identifier) -> Option<Identifier> {
        if let Ok(action) = self.get_action(id) {
            return Some(action.request.clone());
        }
        None
    }

    pub fn get_action_reply_msg_id(&self, id: &Identifier) -> Option<Identifier> {
        if let Ok(action) = self.get_action(id) {
            return Some(action.reply.clone());
        }
        None
    }

    fn get_service(&self, id: &Identifier) -> Option<&Service> {
        let mut result : Option<&Service> = None;
        for service_it in self.services.iter() {
            if &service_it.id == id {
                result = Some(service_it)
            }
        }
        result
    }

    fn get_agent_internal(&self, agent_id: &Identifier, search_agents: bool, search_teams: bool) -> Option<&Agent> {
        let mut result : Option<&Agent> = None;

        if search_agents {
            for agent_it in self.agents.iter() {
                if &agent_it.id == agent_id {
                    result = Some(agent_it)
                }
            }
        }

        if search_teams && result.is_none() {
            for team_it in self.teams.iter() {
                if &team_it.id == agent_id {
                    result = Some(team_it)
                }
            }
        }
        result
    }

    pub fn get_component_messages(&self) -> Vec<Message> {
        let mut result: Vec<Message> = vec![];
        for msg in self.messages.iter() {
            if msg.component {
                result.push(msg.clone());
            }
        }
        result
    }
}

impl std::default::Default for ErrorType {
    fn default() -> Self {
        Self::None
    }
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
                                        format!("{} at {} in {} has duplicate entries of '{}' at indicies {} and {}.", object_type, object_name, field_name, qualified_name, last_index, i)));
        } else {
            // Otherwise not found, insert the name into the names map.
            names.insert(qualified_name.clone(), i.clone());
        }
    }

    errors
}
