//use argparse::{ArgumentParser, StoreTrue, StoreFalse, Store};
use serde::Serialize;

use std::fs;
use std::collections::{BTreeMap};
use std::path::{Path, PathBuf};
use std::error::Error;

use super::model_v0_4_11;
use super::model_v0_5_0;
use super::model_v0_5_1;
use super::model_v0_5_2;
use super::model_v0_5_3;
use super::model_v0_6_0;
 
use uuid::Uuid;

// \note Determine model file version
pub struct ModelVersion {
    pub major: u32,
    pub minor: u32,
    pub patch: u32,
}

pub fn migrate_0_4_11_to_0_5_0(model_version: &mut ModelVersion, model_path: &str, json: &mut serde_json::Value, validate: bool) -> Result<(), Box<dyn Error>>
{
    if model_version.major == 0 && model_version.minor == 4 {
        return Err(format!(r#"Can't migrate model file '{}' incorrect version: {}"#, model_path, model_version.minor).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Migrating model version to v0.5.0 @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    // \note Load the model under the old data scheme
    let old_model = match serde_json::from_value::<model_v0_4_11::Model>(json.clone()) {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure: {}"#, model_path, error).into()),
    };

    // \note Convert the model to next version
    if validate {
        let error_list = old_model.verify();
        for error in error_list.iter() {
            // Using the debug trait attached to the ModelError struct we can print easily
            // Can use other traits (format trait?) to print this out if we desite it to be nice
            println!("{:?}", error); // Print out the errors
        }

        if error_list.len() > 0 {
            return Err(format!("{} model errors found.", error_list.len()).into());
        }
    }

    // \note Project
    let mut model: model_v0_5_0::Model = Default::default();
    {
        model.project.name           = old_model.project.name;
        model.project.bumpy_case     = model_v0_5_0::convert_to_bumpy_case(&model.project.name);
        model.project.namespaces     = old_model.project.namespaces;
        model.project.major_version  = Some("0".to_string());
        model.project.minor_version  = Some("5".to_string());
        model.project.patch_version  = Some("0".to_string());
        model.project.generator      = Some("jack-edit".to_string());
    }

    // \note Direct migrate any compatible data structures
    {
        // \note We do this easily by serialising to JSON and deserialising it back into the
        // new model.
        let old_resources_json = serde_json::to_value(old_model.resources).unwrap();
        model.resources = serde_json::from_value::<Vec<model_v0_5_0::Resource>>(old_resources_json).unwrap();

        let old_tactics_json = serde_json::to_value(old_model.tactics).unwrap();
        model.tactics = serde_json::from_value::<Vec<model_v0_5_0::Tactic>>(old_tactics_json).unwrap();
    }

    let clone_string_array_to_identifiers = |array: &Vec<String>| -> Vec<model_v0_5_0::Identifier> {
        let mut result: Vec<model_v0_5_0::Identifier> = Default::default();
        result.reserve_exact(array.len());
        for item in array.iter() {
            result.push(model_v0_5_0::Identifier{
                module: "".to_string(),
                name: item.to_string(),
                bumpy_case: model_v0_5_0::convert_to_bumpy_case(&item),
            });
        }
        result
    };

    // \note Convert old data structures to new data structures
    {
        println!("Migrating {} teams ...", old_model.teams.len());
        model.teams.reserve_exact(old_model.teams.len());
        for old_team in old_model.teams.iter() {
            let team = model_v0_5_0::Agent {
                id:               model_v0_5_0::Identifier { module: "".to_string(), name: old_team.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_team.name) },
                name:             old_team.name.clone(),
                action_handlers:  clone_string_array_to_identifiers(&old_team.action_handlers),
                beliefs:          clone_string_array_to_identifiers(&old_team.beliefsets),
                goals:            clone_string_array_to_identifiers(&old_team.goals),
                initial_goals:    clone_string_array_to_identifiers(&old_team.initial_goals),
                plans:            clone_string_array_to_identifiers(&old_team.plans),
                resources:        clone_string_array_to_identifiers(&old_team.resources),
                roles:            clone_string_array_to_identifiers(&old_team.roles),
                tactics:          clone_string_array_to_identifiers(&old_team.tactics),
                message_handlers: clone_string_array_to_identifiers(&old_team.message_handlers),
                services:         clone_string_array_to_identifiers(&old_team.services),
            };
            model.teams.push(team);
        }

        println!("Migrating {} agents ...", old_model.agents.len());
        model.agents.reserve_exact(old_model.agents.len());
        for old_agent in old_model.agents.iter() {
            let agent = model_v0_5_0::Agent {
                id:               model_v0_5_0::Identifier { module: "".to_string(), name: old_agent.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_agent.name) },
                name:             old_agent.name.clone(),
                action_handlers:  clone_string_array_to_identifiers(&old_agent.action_handlers),
                beliefs:          clone_string_array_to_identifiers(&old_agent.beliefsets),
                goals:            clone_string_array_to_identifiers(&old_agent.goals),
                initial_goals:    clone_string_array_to_identifiers(&old_agent.initial_goals),
                plans:            clone_string_array_to_identifiers(&old_agent.plans),
                resources:        clone_string_array_to_identifiers(&old_agent.resources),
                roles:            clone_string_array_to_identifiers(&old_agent.roles),
                tactics:          clone_string_array_to_identifiers(&old_agent.tactics),
                message_handlers: clone_string_array_to_identifiers(&old_agent.message_handlers),
                services:         clone_string_array_to_identifiers(&old_agent.services),
            };
            model.agents.push(agent);
        }

        println!("Migrating {} services ...", old_model.services.len());
        model.services.reserve_exact(old_model.services.len());
        for old_service in old_model.services.iter() {
            let service = model_v0_5_0::Service {
                id:              model_v0_5_0::Identifier { module: "".to_string(), name: old_service.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_service.name) },
                name:            old_service.name.clone(),
                note:            None,
                action_handlers: clone_string_array_to_identifiers(&old_service.action_handlers),
                messages:        clone_string_array_to_identifiers(&old_service.beliefsets),
            };
            model.services.push(service);
        }

        println!("Migrating {} messages ...", old_model.messages.len());
        model.messages.reserve_exact(old_model.messages.len());
        for old_message in old_model.messages.iter() {
            let mut message: model_v0_5_0::Message = Default::default();

            let mut message_name = old_message.name.clone();
            if old_message.protocol == model_v0_4_11::Protocol::action {
                message_name.push_str(" Request");
            } else if old_message.protocol == model_v0_4_11::Protocol::percept {
                message_name.push_str(" Reply");
            }
            message.id = model_v0_5_0::Identifier { module: "".to_string(), name: message_name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&message_name) };
            message.name = message.id.name.clone();

            message.fields.reserve_exact(old_message.fields.len());
            for old_field in old_message.fields.iter() {
                let mut field: model_v0_5_0::Field = Default::default();
                field.name = old_field.name.clone();
                field.r#type = match &old_field.r#type {
                    model_v0_4_11::FieldType::None      => model_v0_5_0::FieldType::None,
                    model_v0_4_11::FieldType::Bool      => model_v0_5_0::FieldType::Bool,
                    model_v0_4_11::FieldType::I8        => model_v0_5_0::FieldType::I8,
                    model_v0_4_11::FieldType::I16       => model_v0_5_0::FieldType::I16,
                    model_v0_4_11::FieldType::I32       => model_v0_5_0::FieldType::I32,
                    model_v0_4_11::FieldType::I64       => model_v0_5_0::FieldType::I64,
                    model_v0_4_11::FieldType::U8        => model_v0_5_0::FieldType::U8,
                    model_v0_4_11::FieldType::U16       => model_v0_5_0::FieldType::U16,
                    model_v0_4_11::FieldType::U32       => model_v0_5_0::FieldType::U32,
                    model_v0_4_11::FieldType::U64       => model_v0_5_0::FieldType::U64,
                    model_v0_4_11::FieldType::F32       => model_v0_5_0::FieldType::F32,
                    model_v0_4_11::FieldType::F64       => model_v0_5_0::FieldType::F64,
                    model_v0_4_11::FieldType::String    => model_v0_5_0::FieldType::String,
                    model_v0_4_11::FieldType::Custom(s) => model_v0_5_0::FieldType::Custom(model_v0_5_0::Identifier{name: s.clone(), module: "".to_string(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&s)}),
                };

                field.default  = old_field.default.as_ref().unwrap_or(&"".to_string()).into();
                field.is_array = old_field.is_array.unwrap_or(false);
                message.fields.push(field)
            }
            model.messages.push(message);
        }

        println!("Creating actions from messages ...");
        for old_message in old_model.messages.iter() {
            if old_message.protocol == model_v0_4_11::Protocol::action {
                let mut action: model_v0_5_0::Action = Default::default();
                action.id       = model_v0_5_0::Identifier { module: "".to_string(), name: old_message.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_message.name) };
                action.name     = action.id.name.clone();
                action.req.name = action.id.name.clone();
                action.rpy.name = action.id.name.clone();
                action.req.name.push_str(" Request");
                action.rpy.name.push_str(" Reply");
                model.actions.push(action);
            }
        }

        println!("Migrating {} goals ...", old_model.goals.len());
        model.goals.reserve_exact(old_model.goals.len());
        for old_goal in old_model.goals.iter() {
            let goal = model_v0_5_0::Goal {
                id:             model_v0_5_0::Identifier { module: "".to_string(), name: old_goal.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_goal.name) },
                name:           old_goal.name.clone(),
                heuristic:      old_goal.heuristic,
                query_messages: clone_string_array_to_identifiers(&old_goal.beliefsets),
                precondition:   model_v0_5_0::GoalPlanQuery { query: "".to_string(), custom: old_goal.precondition },
                dropcondition:  model_v0_5_0::GoalPlanQuery { query: "".to_string(), custom: old_goal.dropcondition },
                satisfied:      model_v0_5_0::GoalPlanQuery { query: "".to_string(), custom: old_goal.satisfied },
                resources:      clone_string_array_to_identifiers(&old_goal.resources),
                message:        model_v0_5_0::Identifier { module: "".to_string(), name: old_goal.message.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_goal.name) },
            };
            model.goals.push(goal);
        }

        println!("Migrating {} plans ...", old_model.plans.len());
        model.plans.reserve_exact(old_model.plans.len());
        for old_plan in old_model.plans.iter() {
            let mut plan: model_v0_5_0::Plan = Default::default();
            plan.id                   = model_v0_5_0::Identifier { module: "".to_string(), name: old_plan.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_plan.name) };
            plan.name                 = old_plan.name.clone();
            plan.precondition.custom  = old_plan.precondition;
            plan.dropcondition.custom = old_plan.dropcondition;
            plan.effects              = old_plan.effects;
            plan.handles              = Some(model_v0_5_0::Identifier{ module: "".to_string(), name: old_plan.handles.clone().unwrap_or_default(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_plan.name) });
            plan.query_messages       = clone_string_array_to_identifiers(&old_plan.beliefsets);

            plan.tasks.reserve_exact(old_plan.tasks.len());
            for old_task in old_plan.tasks.iter() {
                let mut task: model_v0_5_0::Task = Default::default();
                task.id            = old_task.id.clone();
                task.r#type        = match old_task.r#type {
                    model_v0_4_11::TaskType::action    => model_v0_5_0::TaskType::action,
                    model_v0_4_11::TaskType::sleep     => model_v0_5_0::TaskType::sleep,
                    model_v0_4_11::TaskType::goal      => model_v0_5_0::TaskType::goal,
                    model_v0_4_11::TaskType::node      => model_v0_5_0::TaskType::node,
                    model_v0_4_11::TaskType::wait      => model_v0_5_0::TaskType::wait,
                    model_v0_4_11::TaskType::condition => model_v0_5_0::TaskType::condition,
                    model_v0_4_11::TaskType::None      => model_v0_5_0::TaskType::None,
                };
                task.r#async = old_task.nowait.clone();

                let mut message_id : model_v0_5_0::Identifier = Default::default();
                if (task.r#type == model_v0_5_0::TaskType::goal || task.r#type == model_v0_5_0::TaskType::action) && old_task.message.is_some() {
                    message_id.name = old_task.message.as_ref().unwrap().clone();
                }
                task.message = Some(message_id);

                if old_task.mappings.is_some() {
                    let old_mappings_json = serde_json::to_value(old_task.mappings.clone()).unwrap();
                    task.mappings = Some(serde_json::from_value::<Vec<model_v0_5_0::Mapping>>(old_mappings_json).unwrap());
                } else {
                    task.mappings = Some(Vec::<model_v0_5_0::Mapping>::new());
                }

                task.duration      = old_task.duration;
                task.conditiontext = old_task.conditiontext.clone();

                plan.tasks.push(task)
            }

            let old_edges_json = serde_json::to_value(old_plan.edges.clone()).unwrap();
            plan.edges = serde_json::from_value::<Vec<model_v0_5_0::TaskEdge>>(old_edges_json).unwrap();

            model.plans.push(plan);
        }

        println!("Migrating {} roles ...", old_model.roles.len());
        model.roles.reserve_exact(old_model.roles.len());
        for old_role in old_model.roles.iter() {
            let mut role: model_v0_5_0::Role = Default::default();
            role.id     = model_v0_5_0::Identifier { module: "".to_string(), name: old_role.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_role.name) };
            role.name   = old_role.name.clone();
            role.goals  = clone_string_array_to_identifiers(&old_role.goals);

            let old_beliefsets_json = serde_json::to_value(old_role.beliefsets.clone()).unwrap();
            role.messages = serde_json::from_value::<Vec<model_v0_5_0::MessagePerm>>(old_beliefsets_json).unwrap();

            model.roles.push(role);
        }
    }

    // \note Deserialize upgraded model back into json
    *json = serde_json::to_value(model).unwrap();

    let model_path_path  = Path::new(&model_path);
    let file_name_no_ext = Path::file_stem(model_path_path).unwrap();
    let file_dir_buf     = PathBuf::from(model_path_path);
    let file_dir_parent  = file_dir_buf.parent();
    let file_dir         = file_dir_parent.unwrap().to_str().unwrap().to_string();

    let mut model_path_backup : String = Default::default();
    if file_dir.len() > 0 {
        model_path_backup = file_dir + "/"
    }
    model_path_backup.push_str(file_name_no_ext.to_str().unwrap());
    model_path_backup.push_str("_v0_4_11.jack");

    println!(r#"Previous model has been backed up to '{}'"#, model_path_backup);

    if let Err(error) = fs::copy(&model_path, &model_path_backup) {
        return Err(format!(r#"Failed to backup migrated model '{}' to disk: {}"#, model_path_backup, error).into());
    }

    // Pretty print the JSON model with an indent of 4 (this is the same as the editor)
    let mut buf = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser = serde_json::Serializer::with_formatter(&mut buf, formatter);
    json.serialize(&mut ser)?;

    if let Err(error) = fs::write(&model_path, String::from_utf8(buf).unwrap()) {
        return Err(format!(r#"Failed to overwrite old model '{}' with migrated model to disk: {}"#, model_path, error).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Model migrated to v0.5.0 successfully! @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    model_version.major = 0;
    model_version.minor = 5;
    model_version.patch = 0;

    Ok(())
}

pub fn migrate_0_5_0_to_0_5_1(model_version: &mut ModelVersion, model_path: &str, json: &mut serde_json::Value, validate: bool) -> Result<(), Box<dyn Error>>
{
    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Migrating model version to v0.5.1 @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    // \note Load the model under the old data scheme
    let old_model = match serde_json::from_value::<model_v0_5_0::Model>(json.clone()) {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure: {}"#, model_path, error).into()),
    };

    // \note Convert the model to next version
    if validate {
        let error_list = old_model.verify();
        for error in error_list.iter() {
            // Using the debug trait attached to the ModelError struct we can print easily
            // Can use other traits (format trait?) to print this out if we desite it to be nice
            println!("{:?}", error); // Print out the errors
        }

        if error_list.len() > 0 {
            return Err(format!("{} model errors found.", error_list.len()).into());
        }
    }

    // \note Project
    let mut model: model_v0_5_1::Model = Default::default();
    {
        model.project.name           = old_model.project.name;
        model.project.bumpy_case     = model_v0_5_1::convert_to_bumpy_case(&model.project.name);
        model.project.namespaces     = old_model.project.namespaces;
        model.project.major_version  = Some("0".to_string());
        model.project.minor_version  = Some("5".to_string());
        model.project.patch_version  = Some("1".to_string());
        model.project.generator      = Some("jack-edit".to_string());
    }

    // \note Direct migrate any compatible data structures
    {
        // \note We do this easily by serialising to JSON and deserialising it back into the
        // new model.
        let old_services_json = serde_json::to_value(old_model.services).unwrap();
        model.services = serde_json::from_value::<Vec<model_v0_5_1::Service>>(old_services_json).unwrap();

        let old_messages_json = serde_json::to_value(old_model.messages).unwrap();
        model.messages = serde_json::from_value::<Vec<model_v0_5_1::Message>>(old_messages_json).unwrap();

        let old_actions_json = serde_json::to_value(old_model.actions).unwrap();
        model.actions = serde_json::from_value::<Vec<model_v0_5_1::Action>>(old_actions_json).unwrap();

        let old_plans_json = serde_json::to_value(old_model.plans).unwrap();
        model.plans = serde_json::from_value::<Vec<model_v0_5_1::Plan>>(old_plans_json).unwrap();

        let old_roles_json = serde_json::to_value(old_model.roles).unwrap();
        model.roles = serde_json::from_value::<Vec<model_v0_5_1::Role>>(old_roles_json).unwrap();

        let old_resources_json = serde_json::to_value(old_model.resources).unwrap();
        model.resources = serde_json::from_value::<Vec<model_v0_5_1::Resource>>(old_resources_json).unwrap();

        let old_tactics_json = serde_json::to_value(old_model.tactics).unwrap();
        model.tactics = serde_json::from_value::<Vec<model_v0_5_1::Tactic>>(old_tactics_json).unwrap();
    }

    let clone_identifier = |id: &model_v0_5_0::Identifier| -> model_v0_5_1::Identifier {
        let result = model_v0_5_1::Identifier{
            module: id.module.clone(),
            name: id.name.clone(),
            bumpy_case: id.bumpy_case.clone(),
        };
        result
    };

    let clone_identifier_array = |array: &Vec<model_v0_5_0::Identifier>| -> Vec<model_v0_5_1::Identifier> {
        let mut result: Vec<model_v0_5_1::Identifier> = Default::default();
        result.reserve_exact(array.len());
        for item in array.iter() {
            result.push(clone_identifier(item));
        }
        result
    };

    println!("Migrating {} goals ...", old_model.goals.len());
    model.goals.reserve_exact(old_model.goals.len());
    for old_goal in old_model.goals.iter() {
        let goal = model_v0_5_1::Goal {
            id:             model_v0_5_1::Identifier { module: "".to_string(), name: old_goal.name.clone(), bumpy_case: model_v0_5_0::convert_to_bumpy_case(&old_goal.name) },
            name:           old_goal.name.clone(),
            heuristic:      old_goal.heuristic,
            query_messages: clone_identifier_array(&old_goal.query_messages),
            precondition:   model_v0_5_1::GoalPlanQuery { query: old_goal.precondition.query.clone(), custom: old_goal.precondition.custom },
            dropcondition:  model_v0_5_1::GoalPlanQuery { query: old_goal.dropcondition.query.clone(), custom: old_goal.dropcondition.custom },
            satisfied:      model_v0_5_1::GoalPlanQuery { query: old_goal.satisfied.query.clone(), custom: old_goal.satisfied.custom },
            resources:      clone_identifier_array(&old_goal.resources),
            message:        clone_identifier(&old_goal.message),
            policy:         model_v0_5_1::GoalPolicy { persistent: false, plan_order: Default::default(), plan_exclude: true, plan_loop: false },
        };
        model.goals.push(goal);
    }

    // \note Convert old data structures to new data structures
    {
        println!("Migrating {} teams ...", old_model.teams.len());
        model.teams.reserve_exact(old_model.teams.len());
        for old_team in old_model.teams.iter() {
            let mut team: model_v0_5_1::Agent = Default::default();
            team.id               = clone_identifier(&old_team.id);
            team.name             = old_team.name.clone();
            team.action_handlers  = clone_identifier_array(&old_team.action_handlers);
            team.beliefs          = clone_identifier_array(&old_team.beliefs);

            for (_, id) in old_team.goals.iter().enumerate() {
                let mut goal : model_v0_5_1::AgentGoal = Default::default();
                goal.id = clone_identifier(id);
                goal.startup_goal = old_team.initial_goals.contains(id);
                team.goals.push(goal);
            }

            team.plans            = clone_identifier_array(&old_team.plans);
            team.resources        = clone_identifier_array(&old_team.resources);
            team.roles            = clone_identifier_array(&old_team.roles);
            team.tactics          = clone_identifier_array(&old_team.tactics);
            team.message_handlers = clone_identifier_array(&old_team.message_handlers);
            team.services         = clone_identifier_array(&old_team.services);
            model.teams.push(team);
        }

        println!("Migrating {} agents ...", old_model.agents.len());
        model.agents.reserve_exact(old_model.agents.len());
        for old_agent in old_model.agents.iter() {
            let mut agent: model_v0_5_1::Agent = Default::default();
            agent.id               = clone_identifier(&old_agent.id);
            agent.name             = old_agent.name.clone();
            agent.action_handlers  = clone_identifier_array(&old_agent.action_handlers);
            agent.beliefs          = clone_identifier_array(&old_agent.beliefs);

            for (_, id) in old_agent.goals.iter().enumerate() {
                let mut goal : model_v0_5_1::AgentGoal = Default::default();
                goal.id = clone_identifier(id);
                goal.startup_goal = old_agent.initial_goals.contains(id);
                agent.goals.push(goal);
            }

            agent.plans            = clone_identifier_array(&old_agent.plans);
            agent.resources        = clone_identifier_array(&old_agent.resources);
            agent.roles            = clone_identifier_array(&old_agent.roles);
            agent.tactics          = clone_identifier_array(&old_agent.tactics);
            agent.message_handlers = clone_identifier_array(&old_agent.message_handlers);
            agent.services         = clone_identifier_array(&old_agent.services);
            model.agents.push(agent);
        }
    }

    // \note Deserialize upgraded model back into json
    *json = serde_json::to_value(model).unwrap();

    let model_path_path  = Path::new(&model_path);
    let file_name_no_ext = Path::file_stem(model_path_path).unwrap();
    let file_dir_buf     = PathBuf::from(model_path_path);
    let file_dir_parent  = file_dir_buf.parent();
    let file_dir         = file_dir_parent.unwrap().to_str().unwrap().to_string();

    let mut model_path_backup : String = Default::default();
    if file_dir.len() > 0 {
        model_path_backup = file_dir + "/"
    }
    model_path_backup.push_str(file_name_no_ext.to_str().unwrap());
    model_path_backup.push_str("_v0_5_0.jack");

    println!(r#"Previous model has been backed up to '{}'"#, model_path_backup);

    if let Err(error) = fs::copy(&model_path, &model_path_backup) {
        return Err(format!(r#"Failed to backup migrated model '{}' to disk: {}"#, model_path_backup, error).into());
    }

    // Pretty print the JSON model with an indent of 4 (this is the same as the editor)
    let mut buf = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser = serde_json::Serializer::with_formatter(&mut buf, formatter);
    json.serialize(&mut ser)?;

    if let Err(error) = fs::write(&model_path, String::from_utf8(buf).unwrap()) {
        return Err(format!(r#"Failed to overwrite old model '{}' with migrated model to disk: {}"#, model_path, error).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Model migrated to v0.5.1 successfully! @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    model_version.major = 0;
    model_version.minor = 5;
    model_version.patch = 1;

    Ok(())
}

pub fn migrate_0_5_1_to_0_5_2(model_version: &mut ModelVersion, model_path: &str, json: &mut serde_json::Value, validate: bool) -> Result<(), Box<dyn Error>>
{
    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Migrating model version to v0.5.2 @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    // load 0.5.1
    // add components

    // \note Load the model under the old data scheme
    let old_model = match serde_json::from_value::<model_v0_5_1::Model>(json.clone()) {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure: {}"#, model_path, error).into()),
    };

    // \note Validate the old model first
    if validate {
        let error_list = old_model.verify();
        for error in error_list.iter() {
            // Using the debug trait attached to the ModelError struct we can print easily
            // Can use other traits (format trait?) to print this out if we desite it to be nice
            println!("{:?}", error); // Print out the errors
        }

        if error_list.len() > 0 {
            return Err(format!("{} model errors found.", error_list.len()).into());
        }
    }

    // Create the new model
    let mut model: model_v0_5_2::Model = Default::default();
    {
        model.project.name           = old_model.project.name;
        model.project.bumpy_case     = model_v0_5_2::convert_to_bumpy_case(&model.project.name);
        model.project.namespaces     = old_model.project.namespaces;
        model.project.major_version  = Some("0".to_string());
        model.project.minor_version  = Some("5".to_string());
        model.project.patch_version  = Some("2".to_string());
        model.project.generator      = Some("jack-edit".to_string());
    }

    // Migrate any compatible data structures
    {
        // Serialise to JSON and deserialise back into the new model

        let old_goals_json = serde_json::to_value(old_model.goals).unwrap();
        model.goals = serde_json::from_value::<Vec<model_v0_5_2::Goal>>(old_goals_json).unwrap();

        let old_services_json = serde_json::to_value(old_model.services).unwrap();
        model.services = serde_json::from_value::<Vec<model_v0_5_2::Service>>(old_services_json).unwrap();

        let old_agents_json = serde_json::to_value(old_model.agents).unwrap();
        model.agents = serde_json::from_value::<Vec<model_v0_5_2::Agent>>(old_agents_json).unwrap();

        let old_teams_json = serde_json::to_value(old_model.teams).unwrap();
        model.teams = serde_json::from_value::<Vec<model_v0_5_2::Agent>>(old_teams_json).unwrap();

        let old_actions_json = serde_json::to_value(old_model.actions).unwrap();
        model.actions = serde_json::from_value::<Vec<model_v0_5_2::Action>>(old_actions_json).unwrap();

        let old_plans_json = serde_json::to_value(old_model.plans).unwrap();
        model.plans = serde_json::from_value::<Vec<model_v0_5_2::Plan>>(old_plans_json).unwrap();

        let old_roles_json = serde_json::to_value(old_model.roles).unwrap();
        model.roles = serde_json::from_value::<Vec<model_v0_5_2::Role>>(old_roles_json).unwrap();

        let old_resources_json = serde_json::to_value(old_model.resources).unwrap();
        model.resources = serde_json::from_value::<Vec<model_v0_5_2::Resource>>(old_resources_json).unwrap();

        let old_tactics_json = serde_json::to_value(old_model.tactics).unwrap();
        model.tactics = serde_json::from_value::<Vec<model_v0_5_2::Tactic>>(old_tactics_json).unwrap();
    }

    let clone_identifier = |id: &model_v0_5_1::Identifier| -> model_v0_5_2::Identifier {
        let result = model_v0_5_2::Identifier{
            module: id.module.clone(),
            name: id.name.clone(),
            bumpy_case: id.bumpy_case.clone(),
        };
        result
    };

    // Copy over the messages
    println!("Migrating {} messages ...", old_model.messages.len());
    model.messages.reserve_exact(old_model.messages.len());
    for old_message in old_model.messages.iter() {

        let old_fields_json = serde_json::to_value(&old_message.fields).unwrap();

        let mut message = model_v0_5_2::Message {
            id:             clone_identifier(&old_message.id),
            name:           old_message.name.clone(),
            note:           "".to_string(),
            component:      false,
            fields:         serde_json::from_value::<Vec<model_v0_5_2::Field>>(old_fields_json).unwrap()
        };

        if old_message.note.is_some() {
            message.note = old_message.note.clone().unwrap();
        }
        model.messages.push(message);
    }

    // \note Deserialize upgraded model back into json
    *json = serde_json::to_value(model).unwrap();

    let model_path_path  = Path::new(&model_path);
    let file_name_no_ext = Path::file_stem(model_path_path).unwrap();
    let file_dir_buf     = PathBuf::from(model_path_path);
    let file_dir_parent  = file_dir_buf.parent();
    let file_dir         = file_dir_parent.unwrap().to_str().unwrap().to_string();

    let mut model_path_backup : String = Default::default();
    if file_dir.len() > 0 {
        model_path_backup = file_dir + "/"
    }
    model_path_backup.push_str(file_name_no_ext.to_str().unwrap());
    model_path_backup.push_str("_v0_5_1.jack");

    println!(r#"Previous model has been backed up to '{}'"#, model_path_backup);

    if let Err(error) = fs::copy(&model_path, &model_path_backup) {
        return Err(format!(r#"Failed to backup migrated model '{}' to disk: {}"#, model_path_backup, error).into());
    }

    // Pretty print the JSON model with an indent of 4 (this is the same as the editor)
    let mut buf = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser = serde_json::Serializer::with_formatter(&mut buf, formatter);
    json.serialize(&mut ser)?;

    if let Err(error) = fs::write(&model_path, String::from_utf8(buf).unwrap()) {
        return Err(format!(r#"Failed to overwrite old model '{}' with migrated model to disk: {}"#, model_path, error).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Model migrated to v0.5.2 successfully! @ '{}'"#, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    model_version.major = 0;
    model_version.minor = 5;
    model_version.patch = 2;

    Ok(())
}

pub fn migrate_0_5_2_to_0_5_3(model_version: &mut ModelVersion, model_path: &str, json: &mut serde_json::Value, validate: bool) -> Result<(), Box<dyn Error>>
{
    use model_v0_5_2 as PrevModel;
    use model_v0_5_3 as NextModel;

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Migrating model version to v{}.{}.{} @ '{}'"#, NextModel::VERSION_MAJOR, NextModel::VERSION_MINOR, NextModel::VERSION_PATCH, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    // \note Load the model under the old data scheme
    let old_model = match serde_json::from_value::<PrevModel::Model>(json.clone()) {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure: {}"#, model_path, error).into()),
    };

    // \note Validate the old model first
    if validate {
        let error_list = old_model.verify();
        for error in error_list.iter() {
            println!("{:?}", error);
        }
        if error_list.len() > 0 {
            return Err(format!("{} model errors found.", error_list.len()).into());
        }
    }

    // Migrate any compatible data structures
    let mut model: NextModel::Model = Default::default();
    {
        // Serialise to JSON and deserialise back into the new model
        let old_project_json = serde_json::to_value(old_model.project).unwrap();
        model.project = serde_json::from_value::<NextModel::Project>(old_project_json).unwrap();
        model.project.major_version = NextModel::VERSION_MAJOR.to_string();
        model.project.minor_version = NextModel::VERSION_MINOR.to_string();
        model.project.patch_version = NextModel::VERSION_PATCH.to_string();

        let old_enums_json = serde_json::to_value(old_model.enums).unwrap();
        model.enums = serde_json::from_value::<Vec<NextModel::Enum>>(old_enums_json).unwrap();

        let old_teams_json = serde_json::to_value(old_model.teams).unwrap();
        model.teams = serde_json::from_value::<Vec<NextModel::Agent>>(old_teams_json).unwrap();

        let old_agents_json = serde_json::to_value(old_model.agents).unwrap();
        model.agents = serde_json::from_value::<Vec<NextModel::Agent>>(old_agents_json).unwrap();

        // \note Tactics model changed, migrated below, however we never used tactics, so
        // migration of old tactics is not supported.
        // let old_tactics_json = serde_json::to_value(old_model.tactics).unwrap();
        // model.tactics = serde_json::from_value::<Vec<NextModel::Tactic>>(old_tactics_json).unwrap();

        let old_roles_json = serde_json::to_value(old_model.roles).unwrap();
        model.roles = serde_json::from_value::<Vec<NextModel::Role>>(old_roles_json).unwrap();

        let old_resources_json = serde_json::to_value(old_model.resources).unwrap();
        model.resources = serde_json::from_value::<Vec<NextModel::Resource>>(old_resources_json).unwrap();

        // \note Migrated below, rpy -> reply and req -> request
        // let old_actions_json = serde_json::to_value(old_model.actions).unwrap();
        // model.actions = serde_json::from_value::<Vec<NextModel::Action>>(old_actions_json).unwrap();

        let old_goals_json = serde_json::to_value(old_model.goals).unwrap();
        model.goals = serde_json::from_value::<Vec<NextModel::Goal>>(old_goals_json).unwrap();

        let old_plans_json = serde_json::to_value(old_model.plans).unwrap();
        model.plans = serde_json::from_value::<Vec<NextModel::Plan>>(old_plans_json).unwrap();

        let old_messages_json = serde_json::to_value(old_model.messages).unwrap();
        model.messages = serde_json::from_value::<Vec<NextModel::Message>>(old_messages_json).unwrap();

        let old_services_json = serde_json::to_value(old_model.services).unwrap();
        model.services = serde_json::from_value::<Vec<NextModel::Service>>(old_services_json).unwrap();

        let old_entities_json = serde_json::to_value(old_model.entities).unwrap();
        model.entities = serde_json::from_value::<Vec<NextModel::Entity>>(old_entities_json).unwrap();

        let old_events_json = serde_json::to_value(old_model.events).unwrap();
        model.events = serde_json::from_value::<Vec<NextModel::Event>>(old_events_json).unwrap();
    }

    let clone_identifier = |id: &PrevModel::Identifier| -> NextModel::Identifier {
        let result = NextModel::Identifier{
            module: id.module.clone(),
            name: id.name.clone(),
            bumpy_case: id.bumpy_case.clone(),
        };
        result
    };

    println!("Migrating {} actions ...", old_model.actions.len());
    model.actions.reserve_exact(old_model.actions.len());
    for old_action in old_model.actions.iter() {
        let action = NextModel::Action {
            id:             clone_identifier(&old_action.id),
            name:           old_action.name.clone(),
            note:           old_action.note.clone(),
            request:        clone_identifier(&old_action.req),
            reply:          clone_identifier(&old_action.rpy),
        };
        model.actions.push(action);
    }

    // \note Deserialize upgraded model back into json
    *json = serde_json::to_value(model).unwrap();

    let model_path_path  = Path::new(&model_path);
    let file_name_no_ext = Path::file_stem(model_path_path).unwrap();
    let file_dir_buf     = PathBuf::from(model_path_path);
    let file_dir_parent  = file_dir_buf.parent();
    let file_dir         = file_dir_parent.unwrap().to_str().unwrap().to_string();

    let mut model_path_backup : String = Default::default();
    if file_dir.len() > 0 {
        model_path_backup = file_dir + "/"
    }
    model_path_backup.push_str(file_name_no_ext.to_str().unwrap());
    model_path_backup.push_str(format!("_v{}_{}_{}.jack", PrevModel::VERSION_MAJOR, PrevModel::VERSION_MINOR, PrevModel::VERSION_PATCH).as_str());

    println!(r#"Previous model has been backed up to '{}'"#, model_path_backup);

    if let Err(error) = fs::copy(&model_path, &model_path_backup) {
        return Err(format!(r#"Failed to backup migrated model '{}' to disk: {}"#, model_path_backup, error).into());
    }

    // Pretty print the JSON model with an indent of 4 (this is the same as the editor)
    let mut buf   = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser   = serde_json::Serializer::with_formatter(&mut buf, formatter);
    json.serialize(&mut ser)?;

    if let Err(error) = fs::write(&model_path, String::from_utf8(buf).unwrap()) {
        return Err(format!(r#"Failed to overwrite old model '{}' with migrated model to disk: {}"#, model_path, error).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Model migrated to v{}.{}.{} successfully! @ '{}'"#, NextModel::VERSION_MAJOR, NextModel::VERSION_MINOR, NextModel::VERSION_PATCH, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    model_version.major = NextModel::VERSION_MAJOR;
    model_version.minor = NextModel::VERSION_MINOR;
    model_version.patch = NextModel::VERSION_PATCH;

    Ok(())
}

trait HasModelV0_6_0 {
    fn get_id(&self) -> &model_v0_6_0::Identifier;
}

impl HasModelV0_6_0 for model_v0_6_0::Action {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Message {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Resource {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Goal {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Tactic {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Plan {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Role {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Service {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

impl HasModelV0_6_0 for model_v0_6_0::Enum {
    fn get_id(&self) -> &model_v0_6_0::Identifier {
        &self.id
    }
}

pub enum ModuleIdentifierKind
{
    Action,
    Message,
    Goal,
    Service,
    Entity,
}

#[allow(non_camel_case_types)]
#[derive(Default)]
pub struct ModuleIdentifiers_0_6_0
{
    pub messages: Vec<model_v0_6_0::Identifier>,
    pub actions:  Vec<model_v0_6_0::Identifier>,
    pub goals:    Vec<model_v0_6_0::Identifier>,
    pub services: Vec<model_v0_6_0::Identifier>,
    pub entities: Vec<model_v0_6_0::Identifier>,
}

/// Lookup an ID by 'name' and 'module' from the array of 'T's and extract the ID
fn lookup_id_from_array_0_6_0<T>(array: &[T], name: &str, module: &str) -> model_v0_6_0::Identifier
where
    T: HasModelV0_6_0,
{
    let result = array
        .iter()
        .find(|item| name == item.get_id().name && module == item.get_id().module)
        .map(|item| item.get_id().clone());

    if name.is_empty() {
        Default::default()
    } else {
        result.unwrap_or_else(Default::default)
    }
}

fn lookup_id_from_symbol_table_0_6_0(map: &BTreeMap<String/*module*/, ModuleIdentifiers_0_6_0>, kind: ModuleIdentifierKind, name: &str, module: &str) -> model_v0_6_0::Identifier
{
    let result: model_v0_6_0::Identifier = map
        .iter()
        .find(|(msg_module, _)| module == msg_module.as_str())
        .map( |(_,  module_identifiers)| {
            let msg_array = match kind {
                ModuleIdentifierKind::Action => {
                    &module_identifiers.actions
                },
                ModuleIdentifierKind::Message => {
                    &module_identifiers.messages
                }
                ModuleIdentifierKind::Goal => {
                    &module_identifiers.goals
                }
                ModuleIdentifierKind::Service => {
                    &module_identifiers.services
                }
                ModuleIdentifierKind::Entity => {
                    &module_identifiers.entities
                }
            };

            match msg_array.iter().find(|msg| msg.name == name && msg.module == module) {
                Some(item) => item.clone(),
                None       => Default::default(),
            }
        }).unwrap();
    result
}

pub fn migrate_0_5_3_to_0_6_0(model_version: &mut ModelVersion, model_path: &str, json: &mut serde_json::Value, validate: bool) -> Result<(), Box<dyn Error>>
{
    use model_v0_5_3 as PrevModel;
    use model_v0_6_0 as NextModel;

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Migrating model version to v{}.{}.{} @ '{}'"#, NextModel::VERSION_MAJOR, NextModel::VERSION_MINOR, NextModel::VERSION_PATCH, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    // \note Load the model under the old data scheme
    let mut old_model = match serde_json::from_value::<PrevModel::Model>(json.clone()) {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure: {}"#, model_path, error).into()),
    };

    // \note Update the concept IDs with the necessary values. Note this is
    // fixed in 0_6_0 in that a deserialisation from JSON will automatically set
    // the concept ID/uuid/name for you.
    {
        let update_identifiers_with_bumpy_case = |array: &mut Vec<PrevModel::Identifier>| {
            for item in array.iter_mut() {
                item.bumpy_case = PrevModel::convert_to_bumpy_case(&item.name);
            }
        };

        let update_agent_goal_identifiers_with_bumpy_case = |array: &mut Vec<PrevModel::AgentGoal>| {
            for item in array.iter_mut() {
                item.id.bumpy_case = PrevModel::convert_to_bumpy_case(&item.id.name);
            }
        };

        old_model.project.bumpy_case = old_model.project.name.replace(" ", "_");
        for team in old_model.teams.iter_mut() {
            team.id.name       = team.name.clone();
            team.id.bumpy_case = PrevModel::convert_to_bumpy_case(&team.id.name);
            update_identifiers_with_bumpy_case(&mut team.action_handlers);
            update_identifiers_with_bumpy_case(&mut team.beliefs);
            update_agent_goal_identifiers_with_bumpy_case(&mut team.goals);
            update_identifiers_with_bumpy_case(&mut team.plans);
            update_identifiers_with_bumpy_case(&mut team.resources);
            update_identifiers_with_bumpy_case(&mut team.roles);
            update_identifiers_with_bumpy_case(&mut team.message_handlers);
            update_identifiers_with_bumpy_case(&mut team.services);
        }
        for agent in old_model.agents.iter_mut() {
            agent.id.name       = agent.name.clone();
            agent.id.bumpy_case = PrevModel::convert_to_bumpy_case(&agent.id.name);
            update_identifiers_with_bumpy_case(&mut agent.action_handlers);
            update_identifiers_with_bumpy_case(&mut agent.beliefs);
            update_agent_goal_identifiers_with_bumpy_case(&mut agent.goals);
            update_identifiers_with_bumpy_case(&mut agent.plans);
            update_identifiers_with_bumpy_case(&mut agent.resources);
            update_identifiers_with_bumpy_case(&mut agent.roles);
            update_identifiers_with_bumpy_case(&mut agent.message_handlers);
            update_identifiers_with_bumpy_case(&mut agent.services);
        }
        for service in old_model.services.iter_mut() {
            service.id.name       = service.name.clone();
            service.id.bumpy_case = PrevModel::convert_to_bumpy_case(&service.id.name);
            update_identifiers_with_bumpy_case(&mut service.action_handlers);
            update_identifiers_with_bumpy_case(&mut service.messages);
        }
        for message in old_model.messages.iter_mut() {
            message.id.name       = message.name.clone();
            message.id.bumpy_case = PrevModel::convert_to_bumpy_case(&message.id.name);

            for field in message.fields.iter_mut() {
                field.bumpy_case = PrevModel::convert_to_bumpy_case(&field.name);
                if let PrevModel::FieldType::Custom(custom_type) = &mut field.r#type {
                    custom_type.bumpy_case = PrevModel::convert_to_bumpy_case(&custom_type.name);
                }
            }
        }
        for goal in old_model.goals.iter_mut() {
            goal.id.name       = goal.name.clone();
            goal.id.bumpy_case = PrevModel::convert_to_bumpy_case(&goal.id.name);
            goal.message.bumpy_case = PrevModel::convert_to_bumpy_case(&goal.message.name);
            update_identifiers_with_bumpy_case(&mut goal.query_messages);
            update_identifiers_with_bumpy_case(&mut goal.resources);
        }
        for plan in old_model.plans.iter_mut() {
            plan.id.name       = plan.name.clone();
            plan.id.bumpy_case = PrevModel::convert_to_bumpy_case(&plan.id.name);

            let mut handles    = plan.handles.clone();
            handles.bumpy_case = PrevModel::convert_to_bumpy_case(&handles.name);
            plan.handles       = handles.clone();
            update_identifiers_with_bumpy_case(&mut plan.query_messages);

            for task in plan.tasks.iter_mut() {
                if task.message.is_some() {
                    let mut message_id   = task.message.as_ref().unwrap().clone();
                    message_id.bumpy_case = PrevModel::convert_to_bumpy_case(&message_id.name);
                    task.message          = Some(message_id);
                }
            }
        }
        for role in old_model.roles.iter_mut() {
            role.id.name       = role.name.clone();
            role.id.bumpy_case = PrevModel::convert_to_bumpy_case(&role.id.name);
            update_identifiers_with_bumpy_case(&mut role.goals);

            for item in role.messages.iter_mut() {
                item.id.name = item.name.clone();
                item.id.bumpy_case = PrevModel::convert_to_bumpy_case(&item.id.name);
            }
        }
        for resource in old_model.resources.iter_mut() {
            resource.id.name       = resource.name.clone();
            resource.id.bumpy_case = PrevModel::convert_to_bumpy_case(&resource.id.name);
        }
        for tactic in old_model.tactics.iter_mut() {
            tactic.id.name         = tactic.name.clone();
            tactic.id.bumpy_case   = PrevModel::convert_to_bumpy_case(&tactic.id.name);
            tactic.goal.bumpy_case = PrevModel::convert_to_bumpy_case(&tactic.goal.name);
            update_identifiers_with_bumpy_case(&mut tactic.plan_list);
        }
        for action in old_model.actions.iter_mut() {
            action.id.name            = action.name.clone();
            action.id.bumpy_case      = PrevModel::convert_to_bumpy_case(&action.id.name);
            action.request.bumpy_case = PrevModel::convert_to_bumpy_case(&action.request.name);
            action.reply.bumpy_case   = PrevModel::convert_to_bumpy_case(&action.reply.name);
        }
        for enum_val in old_model.enums.iter_mut() {
            enum_val.id.name       = enum_val.name.clone();
            enum_val.id.bumpy_case = PrevModel::convert_to_bumpy_case(&enum_val.id.name);

            for enum_field in enum_val.fields.iter_mut() {
                enum_field.bumpy_case = PrevModel::convert_to_bumpy_case(&enum_field.name)
            }
        }

        for entity_val in old_model.entities.iter_mut() {
            entity_val.id.name       = entity_val.name.clone();
            entity_val.id.bumpy_case = PrevModel::convert_to_bumpy_case(&entity_val.id.name);

            for component in entity_val.components.iter_mut() {
                component.bumpy_case = PrevModel::convert_to_bumpy_case(&component.name);
            }

            for child in entity_val.children.iter_mut() {
                child.bumpy_case = PrevModel::convert_to_bumpy_case(&child.name);
            }
        }

        for event_val in old_model.events.iter_mut() {
            event_val.id.name            = event_val.name.clone();
            event_val.id.bumpy_case      = PrevModel::convert_to_bumpy_case(&event_val.id.name);
            event_val.message.bumpy_case = PrevModel::convert_to_bumpy_case(&event_val.message.name);
        }
    }

    // \note Validate the old model first
    if validate {
        let error_list = old_model.verify();
        for error in error_list.iter() {
            println!("{:?}", error);
        }
        if error_list.len() > 0 {
            return Err(format!("{} model errors found.", error_list.len()).into());
        }
    }

    // Migrate any compatible data structures
    let mut model: NextModel::Model = Default::default();
    {
        // Serialise to JSON and deserialise back into the new model
        let old_project_json = serde_json::to_value(old_model.project).unwrap();
        model.project = serde_json::from_value::<NextModel::Project>(old_project_json).unwrap();
        model.project.major_version = NextModel::VERSION_MAJOR.to_string();
        model.project.minor_version = NextModel::VERSION_MINOR.to_string();
        model.project.patch_version = NextModel::VERSION_PATCH.to_string();

        let old_enums_json = serde_json::to_value(old_model.enums).unwrap();
        model.enums = serde_json::from_value::<Vec<NextModel::Enum>>(old_enums_json).unwrap();

        let old_teams_json = serde_json::to_value(old_model.teams).unwrap();
        model.teams = serde_json::from_value::<Vec<NextModel::Agent>>(old_teams_json).unwrap();

        let old_agents_json = serde_json::to_value(old_model.agents).unwrap();
        model.agents = serde_json::from_value::<Vec<NextModel::Agent>>(old_agents_json).unwrap();

        let old_tactics_json = serde_json::to_value(old_model.tactics).unwrap();
        model.tactics = serde_json::from_value::<Vec<NextModel::Tactic>>(old_tactics_json).unwrap();

        let old_roles_json = serde_json::to_value(old_model.roles).unwrap();
        model.roles = serde_json::from_value::<Vec<NextModel::Role>>(old_roles_json).unwrap();

        let old_resources_json = serde_json::to_value(old_model.resources).unwrap();
        model.resources = serde_json::from_value::<Vec<NextModel::Resource>>(old_resources_json).unwrap();

        let old_actions_json = serde_json::to_value(old_model.actions).unwrap();
        model.actions = serde_json::from_value::<Vec<NextModel::Action>>(old_actions_json).unwrap();

        let old_goals_json = serde_json::to_value(old_model.goals).unwrap();
        model.goals = serde_json::from_value::<Vec<NextModel::Goal>>(old_goals_json).unwrap();

        let old_messages_json = serde_json::to_value(old_model.messages).unwrap();
        model.messages = serde_json::from_value::<Vec<NextModel::Message>>(old_messages_json).unwrap();
    }

    // \note Assign UUIDs
    for enum_val in model.enums.iter_mut() {
        enum_val.id.uuid = Uuid::new_v4().to_string();
    }

    for msg in model.messages.iter_mut() {
        msg.id.uuid = Uuid::new_v4().to_string();
    }

    for entity in model.entities.iter_mut() {
        entity.id.uuid = Uuid::new_v4().to_string();
        for child in entity.children.iter_mut() {
            child.uuid = Uuid::new_v4().to_string();
        }
    }

    for action in model.actions.iter_mut() {
        action.id.uuid = Uuid::new_v4().to_string();
    }

    for resource in model.resources.iter_mut() {
        resource.id.uuid = Uuid::new_v4().to_string();
    }

    for service in model.services.iter_mut() {
        service.id.uuid = Uuid::new_v4().to_string();
    }

    for goal in model.goals.iter_mut() {
        goal.id.uuid = Uuid::new_v4().to_string();
    }

    for plan in model.plans.iter_mut() {
        plan.id.uuid = Uuid::new_v4().to_string();
    }

    for role in model.roles.iter_mut() {
        role.id.uuid = Uuid::new_v4().to_string();
    }

    for tactic in model.tactics.iter_mut() {
        tactic.id.uuid = Uuid::new_v4().to_string();
    }

    for agent in model.agents.iter_mut() {
        agent.id.uuid = Uuid::new_v4().to_string();
    }

    for team in model.teams.iter_mut() {
        team.id.uuid = Uuid::new_v4().to_string();
    }

    // \note Convert old event structures to new data structures
    {
        let clone_identifier = |id: &PrevModel::Identifier| -> NextModel::Identifier {
            let result = NextModel::Identifier {
                uuid:       Uuid::new_v4().to_string(),
                name:       id.name.clone(),
                module:     id.module.clone(),
                bumpy_case: id.bumpy_case.clone(),
            };
            result
        };

        let clone_identifier_array = |array: &Vec<PrevModel::Identifier>| -> Vec<NextModel::Identifier> {
            let mut result: Vec<NextModel::Identifier> = Default::default();
            result.reserve_exact(array.len());
            for item in array.iter() {
                result.push(clone_identifier(item));
            }
            result
        };

        println!("Migrating {} plans ...", old_model.plans.len());
        model.plans.reserve_exact(old_model.plans.len());
        for old_plan in old_model.plans.iter() {
            let mut plan: NextModel::Plan = Default::default();
            plan.id                       = clone_identifier(&old_plan.id);
            plan.query_messages           = clone_identifier_array(&old_plan.query_messages);
            plan.handles                  = clone_identifier(&old_plan.handles);
            plan.effects                  = old_plan.effects;

            plan.tasks.reserve_exact(old_plan.tasks.len());
            for old_task in old_plan.tasks.iter() {
                let mut task: NextModel::Task = Default::default();
                task.id                       = old_task.id.clone();
                task.note                     = old_task.note.clone();
                task.r#type                   = match old_task.r#type {
                    PrevModel::TaskType::action    => { NextModel::TaskType::action },
                    PrevModel::TaskType::goal      => { NextModel::TaskType::goal },
                    PrevModel::TaskType::condition => { NextModel::TaskType::condition },
                    PrevModel::TaskType::sleep     => { NextModel::TaskType::sleep },
                    _                              => { NextModel::TaskType::None }
                };

                match old_task.r#type {
                    PrevModel::TaskType::action => { task.action = Some(clone_identifier(&old_task.message.as_ref().unwrap())); },
                    PrevModel::TaskType::goal   => { task.goal   = Some(clone_identifier(&old_task.message.as_ref().unwrap())); },
                    _ => { }
                }
                task.r#async  = old_task.r#async;
                if old_task.mappings.is_some() {
                    let mut mappings: Vec<NextModel::Mapping> = Default::default();
                    mappings.reserve_exact(old_task.mappings.as_ref().unwrap().len());
                    for old_mapping in old_task.mappings.as_ref().unwrap().iter() {
                        mappings.push(NextModel::Mapping{
                            from: old_mapping.from.clone(),
                            to:   old_mapping.to.clone(),
                        });
                    }
                    task.mappings = Some(mappings);
                }
                task.duration      = old_task.duration;
                task.conditiontext = old_task.conditiontext.clone();
                plan.tasks.push(task);
            }

            plan.edges.reserve_exact(old_plan.edges.len());
            for old_edge in old_plan.edges.iter() {
                plan.edges.push(NextModel::TaskEdge{
                    condition: old_edge.condition.clone(),
                    sourceid: old_edge.sourceid.clone(),
                    targetid: old_edge.targetid.clone(),
                });
            }

            plan.precondition.custom  = old_plan.precondition.custom;
            plan.precondition.query   = old_plan.precondition.query.clone();
            plan.dropcondition.custom = old_plan.dropcondition.custom;
            plan.dropcondition.query  = old_plan.dropcondition.query.clone();

            model.plans.push(plan);
        }

        println!("Migrating {} services ...", old_model.services.len());
        model.services.reserve_exact(old_model.services.len());
        for old_service in old_model.services.iter() {
            let mut service: NextModel::Service = Default::default();
            service.id              = clone_identifier(&old_service.id);
            service.id.uuid         = Uuid::new_v4().to_string();
            service.note            = old_service.note.clone();
            service.action_handlers = clone_identifier_array(&old_service.action_handlers);

            service.topics.reserve_exact(old_service.messages.len());
            for message in old_service.messages.iter() {
                let topic = NextModel::Topic {
                    name:    format!("{} Topic", message.qualified_name()),
                    message: clone_identifier(&message)
                };
                service.topics.push(topic);
            }

            model.services.push(service);
        }

        println!("Migrating {} events ...", old_model.events.len());
        model.events.reserve_exact(old_model.events.len());
        for old_event in old_model.events.iter() {
            let event = NextModel::Event {
                id:              clone_identifier(&old_event.id),
                note:            "".to_string(),
                requires_entity: false,
                message:         NextModel::EventMessage {
                    id:       clone_identifier(&old_event.message),
                    defaults: vec![],
                },
            };
            model.events.push(event);
        }

        println!("Migrating {} entities ...", old_model.entities.len());
        model.entities.reserve_exact(old_model.entities.len());
        for old_entity in old_model.entities.iter() {
            let mut entity: NextModel::Entity = Default::default();
            entity.id.name       = old_entity.name.clone();
            entity.id.uuid       = Uuid::new_v4().to_string();
            entity.id.bumpy_case = old_entity.id.bumpy_case.clone();
            entity.note          = "".to_string();
            for it in old_entity.children.iter() {
                entity.children.push(clone_identifier(&it));
            }

            let synthetic_component_suffix = " Component";
            for it in old_entity.components.iter() {
                if it.name == "AgentComponent" {
                    entity.agent = true;
                } else {
                    let mut is_message = true;

                    // Check if the component inside the old entity is a service
                    // by searching for the service name after stripping out the
                    // appended " Component" suffix
                    if it.name.ends_with(synthetic_component_suffix) {
                        let name_without_component_suffix = it.name.as_str().strip_suffix(synthetic_component_suffix).unwrap().to_string();
                        for old_service in old_model.services.iter() {
                            if old_service.name == name_without_component_suffix {
                                let mut id = clone_identifier(&it);
                                id.name    = name_without_component_suffix;
                                entity.services.push(id);
                                is_message = false;
                                break;
                            }
                        }
                    }

                    if is_message {
                        entity.messages.push(clone_identifier(&it));
                    }
                }
            }
            model.entities.push(entity);
        }
    }

    // \note Load all the actions/messages from descendant modules into a hash table
    //       (module name -> list of {messages, actions})
    let mut symbol_table: BTreeMap<String/*module*/, ModuleIdentifiers_0_6_0> = BTreeMap::new();
    {
        for module in model.project.modules.iter() {
            let module_path_buf  = super::module_filepath_to_abs_path(&model_path, module.filepath.as_str());
            let module_path      = module_path_buf.to_string_lossy();
            let file_read_result = fs::read_to_string(&*module_path);
            let file_data        = match file_read_result {
                Ok(file)   => file,
                Err(error) => return Err(format!(r#"Module file '{}' can not be read from model '{}': {}"#, module_path, model_path, error).into()),
            };

            // \note Deserialise into JSON data structure
            let json_read_result        = serde_json::from_str(&file_data);
            let json: serde_json::Value = match json_read_result {
                Ok(result) => result,
                Err(error) => return Err(format!(r#"Module file '{}' failed to be deserialized to json from model '{}': {}"#, module_path, model_path, error).into()),
            };

            let module_model = match serde_json::from_value::<NextModel::Model>(json.clone()) {
                Ok(result) => result,
                Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to model data structure from model '{}': {}"#, module_path, model_path, error).into()),
            };

            let mut module_identifiers: ModuleIdentifiers_0_6_0 = Default::default();

            // \note Collect all the messages for this module
            for module_message in module_model.messages.iter() {
                let mut id = module_message.id.clone();
                id.module  = module.name.clone();
                module_identifiers.messages.push(id);
            }

            // \note Collect all the actions for this module
            for module_action in module_model.actions.iter() {
                let mut id = module_action.id.clone();
                id.module  = module.name.clone();
                module_identifiers.actions.push(id);
            }

            // \note Collect all the goals for this module
            for module_goal in module_model.goals.iter() {
                let mut id = module_goal.id.clone();
                id.module  = module.name.clone();
                module_identifiers.goals.push(id);
            }

            // \note Collect all the services for this module
            for module_service in module_model.services.iter() {
                let mut id = module_service.id.clone();
                id.module  = module.name.clone();
                module_identifiers.services.push(id);
            }

            // \note Collect all the entitys for this module
            for module_entity in module_model.entities.iter() {
                let mut id = module_entity.id.clone();
                id.module  = module.name.clone();
                module_identifiers.entities.push(id);
            }

            symbol_table.insert(module_model.project.name, module_identifiers);
        }

        let mut module_identifiers: ModuleIdentifiers_0_6_0 = Default::default();

        // \note Insert our own messages into the map
        for item in model.messages.iter() {
            module_identifiers.messages.push(item.id.clone());
        }

        // \note Insert our own actions into the map
        for item in model.actions.iter() {
            module_identifiers.actions.push(item.id.clone());
        }

        // \note Insert our own goals into the map
        for item in model.goals.iter() {
            module_identifiers.goals.push(item.id.clone());
        }

        // \note Insert our own services into the map
        for item in model.services.iter() {
            module_identifiers.services.push(item.id.clone());
        }

        // \note Insert our own entities into the map
        for item in model.entities.iter() {
            module_identifiers.entities.push(item.id.clone());
        }

        symbol_table.insert("".to_string(), module_identifiers);
    }

    for msg in model.messages.iter_mut() {
        for field in msg.fields.iter_mut() {
            match &field.r#type {
                NextModel::FieldType::Custom(identifier) => {
                    field.r#type = NextModel::FieldType::Custom(lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, identifier.name.as_str(), identifier.module.as_str()));
                },
                NextModel::FieldType::Enum(identifier) => {
                    field.r#type = NextModel::FieldType::Enum(lookup_id_from_array_0_6_0(&model.enums, identifier.name.as_str(), identifier.module.as_str()));
                },
                _ => {
                }
            }
        }
    }

    for action in model.actions.iter_mut() {
        action.request = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, action.request.name.as_str(), action.request.module.as_str());
        action.reply   = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, action.reply.name.as_str(),   action.reply.module.as_str());
    }

    for service in model.services.iter_mut() {
        for topic in service.topics.iter_mut() {
            topic.message = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, topic.message.name.as_str(), topic.message.module.as_str());
        }
        for action in service.action_handlers.iter_mut() {
            *action = lookup_id_from_array_0_6_0(&model.actions, action.name.as_str(), action.module.as_str());
        }
    }

    for goal in model.goals.iter_mut() {
        for query_msg in goal.query_messages.iter_mut() {
            *query_msg = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, query_msg.name.as_str(), query_msg.module.as_str());
        }
        for resource in goal.resources.iter_mut() {
            *resource = lookup_id_from_array_0_6_0(&model.resources, resource.name.as_str(), resource.module.as_str());
        }
        goal.message = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, goal.message.name.as_str(), goal.message.module.as_str());
    }

    for plan in model.plans.iter_mut() {
        for query_msg in plan.query_messages.iter_mut() {
            *query_msg = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, query_msg.name.as_str(), query_msg.module.as_str());
        }

        // Update the goal in the plan "handles" which now have UUID's assigned to them
        let plan_goal = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Goal, plan.handles.name.as_str(), plan.handles.module.as_str());
        if plan_goal.uuid.len() > 0 {
            plan.handles = plan_goal;
        }

        for task in plan.tasks.iter_mut() {
            if task.goal.is_some() {
                let goal           = task.goal.as_ref().unwrap();
                let goal_with_uuid = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Goal, goal.name.as_str(), goal.module.as_str());
                if goal_with_uuid.uuid.len() > 0 {
                    task.goal = Some(goal_with_uuid);
                }
            }
            if task.action.is_some() {
                let action           = task.action.as_ref().unwrap();
                let action_with_uuid = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Action, action.name.as_str(), action.module.as_str());
                if action_with_uuid.uuid.len() > 0 {
                    task.action = Some(action_with_uuid);
                }
            }
        }
    }

    for role in model.roles.iter_mut() {
        for goal in role.goals.iter_mut() {
            *goal = lookup_id_from_array_0_6_0(&model.goals, goal.name.as_str(), goal.module.as_str());
        }
        for msg in role.messages.iter_mut() {
            msg.id = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, msg.id.name.as_str(), msg.id.module.as_str());
        }
    }

    for tactic in model.tactics.iter_mut() {
        tactic.goal    = lookup_id_from_array_0_6_0(&model.goals, tactic.goal.name.as_str(), tactic.goal.module.as_str());
        for plan in tactic.plan_list.iter_mut() {
            *plan = lookup_id_from_array_0_6_0(&model.plans, plan.name.as_str(), plan.module.as_str());
        }
    }

    for agent in model.agents.iter_mut() {
        for role in agent.roles.iter_mut() {
            *role = lookup_id_from_array_0_6_0(&model.roles, role.name.as_str(), role.module.as_str());
        }
        for belief in agent.beliefs.iter_mut() {
            *belief = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, belief.name.as_str(), belief.module.as_str());
        }
        for goal in agent.goals.iter_mut() {
            goal.id             = lookup_id_from_array_0_6_0(&model.goals, goal.id.name.as_str(), goal.id.module.as_str());
            goal.startup_tactic = lookup_id_from_array_0_6_0(&model.tactics, goal.startup_tactic.name.as_str(), goal.startup_tactic.module.as_str());
        }
        for service in agent.services.iter_mut() {
            *service = lookup_id_from_array_0_6_0(&model.services, service.name.as_str(), service.module.as_str());
        }
        for plan in agent.plans.iter_mut() {
            *plan = lookup_id_from_array_0_6_0(&model.plans, plan.name.as_str(), plan.module.as_str());
        }
        for msg in agent.message_handlers.iter_mut() {
            *msg = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, msg.name.as_str(), msg.module.as_str());
        }
        for resource in agent.resources.iter_mut() {
            *resource = lookup_id_from_array_0_6_0(&model.resources, resource.name.as_str(), resource.module.as_str());
        }
        for action in agent.action_handlers.iter_mut() {
            *action = lookup_id_from_array_0_6_0(&model.actions, action.name.as_str(), action.module.as_str());
        }
    }

    for team in model.teams.iter_mut() {
        for role in team.roles.iter_mut() {
            *role = lookup_id_from_array_0_6_0(&model.roles, role.name.as_str(), role.module.as_str());
        }
        for belief in team.beliefs.iter_mut() {
            *belief = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, belief.name.as_str(), belief.module.as_str());
        }
        for goal in team.goals.iter_mut() {
            goal.id             = lookup_id_from_array_0_6_0(&model.goals, goal.id.name.as_str(), goal.id.module.as_str());
            goal.startup_tactic = lookup_id_from_array_0_6_0(&model.tactics, goal.startup_tactic.name.as_str(), goal.startup_tactic.module.as_str());
        }
        for service in team.services.iter_mut() {
            *service = lookup_id_from_array_0_6_0(&model.services, service.name.as_str(), service.module.as_str());
        }
        for plan in team.plans.iter_mut() {
            *plan = lookup_id_from_array_0_6_0(&model.plans, plan.name.as_str(), plan.module.as_str());
        }
        for msg in team.message_handlers.iter_mut() {
            *msg = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, msg.name.as_str(), msg.module.as_str());
        }
        for resource in team.resources.iter_mut() {
            *resource = lookup_id_from_array_0_6_0(&model.resources, resource.name.as_str(), resource.module.as_str());
        }
        for action in team.action_handlers.iter_mut() {
            *action = lookup_id_from_array_0_6_0(&model.actions, action.name.as_str(), action.module.as_str());
        }
    }

    for entity in model.entities.iter_mut() {
        for service in entity.services.iter_mut() {
            let service_id = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Service, service.name.as_str(), service.module.as_str());
            if service_id.uuid.len() > 0 {
                *service = service_id;
            } else {
                service.uuid = Uuid::new_v4().to_string();
            }
        }

        for child in entity.children.iter_mut() {
            let child_id = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Entity, child.name.as_str(), child.module.as_str());
            if child_id.uuid.len() > 0 {
                *child = child_id;
            } else {
                child.uuid = Uuid::new_v4().to_string();
            }
        }

        for message in entity.messages.iter_mut() {
            let message_id = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, message.name.as_str(), message.module.as_str());
            if message_id.uuid.len() > 0 {
                *message = message_id;
            } else {
                message.uuid = Uuid::new_v4().to_string();
            }
        }
    }

    for event in model.events.iter_mut() {
        let msg_id = lookup_id_from_symbol_table_0_6_0(&symbol_table, ModuleIdentifierKind::Message, event.message.id.name.as_str(), event.message.id.module.as_str());
        if msg_id.uuid.len() > 0 {
            event.message.id = msg_id;
        } else {
            event.message.id.uuid = Uuid::new_v4().to_string();
        }
    }

    // \note Deserialize upgraded model back into json
    *json = serde_json::to_value(model).unwrap();

    let model_path_path  = Path::new(&model_path);
    let file_name_no_ext = Path::file_stem(model_path_path).unwrap();
    let file_dir_buf     = PathBuf::from(model_path_path);
    let file_dir_parent  = file_dir_buf.parent();
    let file_dir         = file_dir_parent.unwrap().to_str().unwrap().to_string();

    let mut model_path_backup : String = Default::default();
    if file_dir.len() > 0 {
        model_path_backup = file_dir + "/"
    }
    model_path_backup.push_str(file_name_no_ext.to_str().unwrap());
    model_path_backup.push_str(format!("_v{}_{}_{}.jack", PrevModel::VERSION_MAJOR, PrevModel::VERSION_MINOR, PrevModel::VERSION_PATCH).as_str());

    println!(r#"Previous model has been backed up to '{}'"#, model_path_backup);

    if let Err(error) = fs::copy(&model_path, &model_path_backup) {
        return Err(format!(r#"Failed to backup migrated model '{}' to disk: {}"#, model_path_backup, error).into());
    }

    // Pretty print the JSON model with an indent of 4 (this is the same as the editor)
    let mut buf   = Vec::new();
    let formatter = serde_json::ser::PrettyFormatter::with_indent(b"    ");
    let mut ser   = serde_json::Serializer::with_formatter(&mut buf, formatter);
    json.serialize(&mut ser)?;

    if let Err(error) = fs::write(&model_path, String::from_utf8(buf).unwrap()) {
        return Err(format!(r#"Failed to overwrite old model '{}' with migrated model to disk: {}"#, model_path, error).into());
    }

    println!("");
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!(r#"// Model migrated to v{}.{}.{} successfully! @ '{}'"#, NextModel::VERSION_MAJOR, NextModel::VERSION_MINOR, NextModel::VERSION_PATCH, model_path);
    println!("///////////////////////////////////////////////////////////////////////////////");
    println!("");

    model_version.major = NextModel::VERSION_MAJOR;
    model_version.minor = NextModel::VERSION_MINOR;
    model_version.patch = NextModel::VERSION_PATCH;

    Ok(())
}
