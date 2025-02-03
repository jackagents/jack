// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

use argparse::{ArgumentParser, StoreTrue, StoreFalse, Store};

use std::fs;
use std::path::PathBuf;
use std::error::Error;
use std::env;

use chrono::Local;

use wasm_bindgen::prelude::*;

pub mod templates;
mod model_v0_4_11;
mod model_v0_5_0;
mod model_v0_5_1;
mod model_v0_5_2;
mod model_v0_5_3;
mod model_v0_6_0;

use model_v0_6_0 as model;

mod migration;
use crate::migration::migrate_0_4_11_to_0_5_0;
use crate::migration::migrate_0_5_0_to_0_5_1;
use crate::migration::migrate_0_5_1_to_0_5_2;
use crate::migration::migrate_0_5_2_to_0_5_3;
use crate::migration::migrate_0_5_3_to_0_6_0;
use crate::migration::ModelVersion;

pub fn module_filepath_to_abs_path(model_path: &str, module_path: &str) -> PathBuf {
    let mut result = PathBuf::from(module_path);
    if result.as_path().is_relative() {
        // \note Convert the path to an absolute path using the current model's file path
        let mut root_path = PathBuf::from(model_path);
        root_path.pop();
        result = PathBuf::from(format!("{}/{}", root_path.display(), module_path));
    }
    result
}

// WASM decorator
// bindgen creates JS/TS bndings to this function using wasm-pack.
#[wasm_bindgen]
/// WASM validation function.
/// Input is a JSON model as a string.
/// Output is a list of ModelError structs as a JSON string.
/// All errors are handled and exported as json (even if there is a serialisation error).
pub fn validate(data: &str) -> String {
    let mut errors : Vec<model::ModelError> = vec![];
    let mut result = "".to_string();

    // Deserialise JSON into model data structure
    let workspace = match load_model_at_buffer_and_migrate("from JSON buffer", data, /*validate*/ false, /*is_module*/ false) {
        Ok(workspace) => { workspace }
        Err(error) => {
            result = format!("[{{ \"keys\" : [], \"machine_keys\" : [], \"error_type\" : \"JsonSerialisationFailure\", \"error_string\" : \"{}\" }}]", error);
            return result
        }
    };

    if workspace.models.is_empty() {
        return result
    }

    // Verify the model
    let model = &workspace.models[0];
    errors.append(&mut model.verify(&workspace));

    // Return errors as json
    match serde_json::to_string_pretty(&errors) {
        Ok(err_string) => {
            result = err_string;
        },
        Err(e) => {
            // If serialisation should ever fail
            // We will hand write some JSON to let the GUI App know what happened
            // This is to stop any and all exceptions bubbling up into the JS app
            result = format!("[{{ \"keys\" : [], \"machine_keys\" : [], \"error_type\" : \"JsonSerialisationFailure\", \"error_string\" : \"{}\" }}]", e);
        }
    }

    result
}

pub fn load_jack_model_from_json(json: serde_json::Value, is_module: bool) -> Result<model::Model, serde_json::Error>
{
    let update_identifiers_with_bumpy_case = |array: &mut Vec<model::Identifier>| {
        for item in array.iter_mut() {
            item.bumpy_case = model::convert_to_bumpy_case(&item.name);
        }
    };

    let update_agent_goal_identifiers_with_bumpy_case = |array: &mut Vec<model::AgentGoal>| {
        for item in array.iter_mut() {
            item.id.bumpy_case = model::convert_to_bumpy_case(&item.id.name);
        }
    };

    let mut result = match serde_json::from_value::<model::Model>(json) {
        Ok(model)  => model,
        Err(error) => return Err(error),
    };

    result.project.bumpy_case = result.project.name.replace(" ", "_");
    for team in result.teams.iter_mut() {
        team.id.bumpy_case = model::convert_to_bumpy_case(&team.id.name);
        update_identifiers_with_bumpy_case(&mut team.action_handlers);
        update_identifiers_with_bumpy_case(&mut team.beliefs);
        update_agent_goal_identifiers_with_bumpy_case(&mut team.goals);
        update_identifiers_with_bumpy_case(&mut team.plans);
        update_identifiers_with_bumpy_case(&mut team.resources);
        update_identifiers_with_bumpy_case(&mut team.roles);
        update_identifiers_with_bumpy_case(&mut team.message_handlers);
        update_identifiers_with_bumpy_case(&mut team.services);
    }
    for agent in result.agents.iter_mut() {
        agent.id.bumpy_case = model::convert_to_bumpy_case(&agent.id.name);
        update_identifiers_with_bumpy_case(&mut agent.action_handlers);
        update_identifiers_with_bumpy_case(&mut agent.beliefs);
        update_agent_goal_identifiers_with_bumpy_case(&mut agent.goals);
        update_identifiers_with_bumpy_case(&mut agent.plans);
        update_identifiers_with_bumpy_case(&mut agent.resources);
        update_identifiers_with_bumpy_case(&mut agent.roles);
        update_identifiers_with_bumpy_case(&mut agent.message_handlers);
        update_identifiers_with_bumpy_case(&mut agent.services);
    }
    for service in result.services.iter_mut() {
        service.id.bumpy_case = model::convert_to_bumpy_case(&service.id.name);
        update_identifiers_with_bumpy_case(&mut service.action_handlers);
        for topic in service.topics.iter_mut() {
            topic.message.bumpy_case = model::convert_to_bumpy_case(&topic.message.name);
        }
    }
    for message in result.messages.iter_mut() {
        message.id.bumpy_case = model::convert_to_bumpy_case(&message.id.name);

        for field in message.fields.iter_mut() {
            field.bumpy_case = model::convert_to_bumpy_case(&field.name);
            if let model::FieldType::Custom(custom_type) = &mut field.r#type {
                custom_type.bumpy_case = model::convert_to_bumpy_case(&custom_type.name);
            }
        }
    }

    for goal in result.goals.iter_mut() {
        goal.id.bumpy_case = model::convert_to_bumpy_case(&goal.id.name);
        goal.message.bumpy_case = model::convert_to_bumpy_case(&goal.message.name);
        update_identifiers_with_bumpy_case(&mut goal.query_messages);
        update_identifiers_with_bumpy_case(&mut goal.resources);

        // if we are loading as a module we will need to set our module name
        if is_module && goal.id.module.is_empty() {
            goal.id.module = result.project.name.clone();
        }
    }


    for plan in result.plans.iter_mut() {
        plan.id.bumpy_case = model::convert_to_bumpy_case(&plan.id.name);
        let mut handles    = plan.handles.clone();
        handles.bumpy_case = model::convert_to_bumpy_case(&handles.name);
        plan.handles       = handles.clone();
        update_identifiers_with_bumpy_case(&mut plan.query_messages);

        for task in plan.tasks.iter_mut() {
            if task.goal.is_some() {
                let mut goal_id   = task.goal.as_ref().unwrap().clone();
                goal_id.bumpy_case = model::convert_to_bumpy_case(&goal_id.name);
                task.goal          = Some(goal_id);
            }
            if task.action.is_some() {
                let mut action_id   = task.action.as_ref().unwrap().clone();
                action_id.bumpy_case = model::convert_to_bumpy_case(&action_id.name);
                task.action          = Some(action_id);
            }
        }
    }
    for role in result.roles.iter_mut() {
        role.id.bumpy_case = model::convert_to_bumpy_case(&role.id.name);
        update_identifiers_with_bumpy_case(&mut role.goals);
        for item in role.messages.iter_mut() {
            item.id.bumpy_case = model::convert_to_bumpy_case(&item.id.name);
        }
    }
    for resource in result.resources.iter_mut() {
        resource.id.bumpy_case = model::convert_to_bumpy_case(&resource.id.name);
    }
    for tactic in result.tactics.iter_mut() {
        tactic.id.bumpy_case   = model::convert_to_bumpy_case(&tactic.id.name);
        tactic.goal.bumpy_case = model::convert_to_bumpy_case(&tactic.goal.name);
        update_identifiers_with_bumpy_case(&mut tactic.plan_list);
    }
    for action in result.actions.iter_mut() {
        action.id.bumpy_case      = model::convert_to_bumpy_case(&action.id.name);
        action.request.bumpy_case = model::convert_to_bumpy_case(&action.request.name);
        action.reply.bumpy_case   = model::convert_to_bumpy_case(&action.reply.name);
    }
    for enum_val in result.enums.iter_mut() {
        enum_val.id.bumpy_case = model::convert_to_bumpy_case(&enum_val.id.name);
        for enum_field in enum_val.fields.iter_mut() {
            enum_field.bumpy_case = model::convert_to_bumpy_case(&enum_field.name)
        }
    }

    for entity_val in result.entities.iter_mut() {
        entity_val.id.bumpy_case = model::convert_to_bumpy_case(&entity_val.id.name);
        for service in entity_val.services.iter_mut() {
            service.bumpy_case = model::convert_to_bumpy_case(&service.name);
        }
        for message in entity_val.messages.iter_mut() {
            message.bumpy_case = model::convert_to_bumpy_case(&message.name);
        }
        for child in entity_val.children.iter_mut() {
            child.bumpy_case = model::convert_to_bumpy_case(&child.name);
        }
    }

    for event_val in result.events.iter_mut() {
        event_val.id.bumpy_case         = model::convert_to_bumpy_case(&event_val.id.name);
        event_val.message.id.bumpy_case = model::convert_to_bumpy_case(&event_val.message.id.name);
    }

    Ok(result)
}

// See bin/jack-validator.rs for how it is used.
pub fn main_validate()  -> Result<(),Box<dyn Error>> {

    // CMD line interface variables with default values set.
    let mut json_error : bool   = false;
    let mut model_file : String = "".to_string();

    // Version information extracted from the build environment
    // env! is a macro to pull in an environment variable
    let version = env!("CARGO_PKG_VERSION");
    let app_name = format!("JACK Model Validation Tool v{}", version);

    // \note Parse arguments
    {
        let mut ap = ArgumentParser::new();
        ap.set_description(&app_name);
        ap.refer(&mut model_file).add_argument("source",        Store,     "Model file path").required();
        ap.refer(&mut json_error).add_option(&["-j", "--json"], StoreTrue, "output JSON Error report");
        ap.parse_args_or_exit();
    }

    // \note Deserialise JSON into model data structure
    let workspace = load_model_at_path_and_migrate(model_file.as_str(), /*validate*/false, /*is_module*/false)?;
    if workspace.models.is_empty() {
        return Err("No JACK models were loaded or able to be loaded from the input files".to_string().into());
    }
    let model  = &workspace.models[0];

    let errors = model.verify(&workspace);
    if json_error {
        println!("{}", serde_json::to_string_pretty(&errors)?);
    } else {
        for err in errors.iter() {
            println!("{:?}", err);
        }
    }

    println!("Model validated with {} errors.", errors.len());
    Ok(())
}

pub fn load_model_at_buffer_and_migrate(path: &str, buffer: &str, validate: bool, is_module: bool) -> Result<model::ProjectWorkspace, Box<dyn std::error::Error>> {
    // \note Deserialise into JSON data structure
    let json_read_result = serde_json::from_str(&buffer);
    let mut json: serde_json::Value = match json_read_result {
        Ok(result) => result,
        Err(error) => return Err(format!(r#"Model file '{}' failed to be deserialized to json: {}"#, path, error).into()),
    };

    let mut result: model::ProjectWorkspace              = Default::default();
    let mut model_version                                = ModelVersion{major: 0, minor: 0, patch: 0};
    let mut sub_workspaces: Vec<model::ProjectWorkspace> = Vec::new();
    {
        let root_object = match json.as_object() {
            Some(result) => result,
            None => return Err(format!(r#"Model file '{}' JSON root must be an object"#, path).into()),
        };

        let project_object = match root_object.get("project") {
            Some(result) => result,
            None         => return Err(format!(r#"Model file '{}' JSON root object does not have required 'project' header for determining version"#, path).into()),
        };

        let minor_version = project_object.get("minor_version");
        let major_version = project_object.get("major_version");
        let patch_version = project_object.get("patch_version");

        let some_version_fields_missing = major_version.is_none() ||
                                          minor_version.is_none() ||
                                          patch_version.is_none();

        let all_version_fields_missing = major_version.is_none() &&
                                         minor_version.is_none() &&
                                         patch_version.is_none();

        if all_version_fields_missing {
            // \note We assume that a model file without any version fields represents the model
            // *prior* to us formally introducing version values to the model. This version is
            // model version v0.4.11
            model_version.major = 0;
            model_version.minor = 4;
            model_version.patch = 11;
        } else if some_version_fields_missing {
            // \note If some version fields are missing, *but*, not all of them, then this is just
            // a plain invalid model file.
            return Err(format!(r#"Model file "{}" is invalid: 'project' object is missing one of the following, "major_version", "minor_version" or "patch_version"#, path).into());
        } else {
            // \note All version values are specified, pull them out
            let major_version_string = major_version.unwrap();
            let minor_version_string = minor_version.unwrap();
            let patch_version_string = patch_version.unwrap();

            let version_fields_incorrect = !major_version_string.is_string() ||
                                           !minor_version_string.is_string() ||
                                           !patch_version_string.is_string();
            if version_fields_incorrect {
                return Err(format!(r#"Model file "{}" is invalid: 'project' object needs version fields to be a string"#, path).into());
            }

            match (major_version_string.as_str().unwrap().parse::<u32>(),
                   minor_version_string.as_str().unwrap().parse::<u32>(),
                   patch_version_string.as_str().unwrap().parse::<u32>()) {
                (Ok(major), Ok(minor), Ok(patch)) => {
                    model_version.major = major;
                    model_version.minor = minor;
                    model_version.patch = patch;
                },
                _ => {
                    return Err(format!(r#"Model file '{}' is invalid: 'project' object contains versions that are not u32's: "{}", "{}" or "{}"#,
                                       path,
                                       major_version_string,
                                       minor_version_string,
                                       patch_version_string).into());
                },
            }
        }

        let modules_array = project_object.get("modules");
        if modules_array.is_none() || !modules_array.unwrap().is_array() {
            return Err(format!(r#"Model file '{}' is invalid: 'project.modules' is not an array"#, path).into());
        }

        for module_array_item in modules_array.unwrap().as_array().unwrap().iter() {
            if !module_array_item.is_object() {
                return Err(format!(r#"Model file '{}' is invalid: 'project.modules' array does not contain a list of objects"#, path).into());
            }
        }

        // \todo: Extract modules and recursively load and update the modules
        for (index, module_array_item) in modules_array.unwrap().as_array().unwrap().iter().enumerate() {
            let module_name_option     = module_array_item.get("name");
            let module_filepath_option = module_array_item.get("filepath");

            if module_name_option.is_none() || !module_name_option.unwrap().is_string() {
                return Err(format!(r#"Model file '{}' is invalid: 'name' in module {} is not a string"#, path, index).into());
            }

            let module_name = module_name_option.unwrap().as_str().unwrap();
            if !module_filepath_option.unwrap().is_string() {
                return Err(format!(r#"Model file '{}' is invalid: 'filepath' in module '{}' is not a string"#, path, module_name).into());
            }

            let module_filepath     = module_filepath_option.unwrap();
            let module_abs_path_buf = module_filepath_to_abs_path(path, module_filepath.as_str().unwrap());
            let module_abs_path     = module_abs_path_buf.to_string_lossy();

            if module_abs_path_buf.exists() {
                sub_workspaces.push(load_model_at_path_and_migrate(&module_abs_path, validate, /*is_module*/true)?);
            } else {
                return Err(format!("Sub-module '{}' cannot be loaded: File does not exist. The path to the sub-module was created by combining base: '{}', module: '{}' path",
                                   module_abs_path,
                                   path,
                                   module_filepath.as_str().unwrap()).into());
            }
        }
    }

    println!("Model file version detected: v{}.{}.{} @ {}", model_version.major, model_version.minor, model_version.patch, path);

    // \note Migrate model version
    if model_version.major == 0 && model_version.minor == 4 {
        match migrate_0_4_11_to_0_5_0(&mut model_version, path, &mut json, validate) {
            Ok(result) => result,
            Err(error) => return Err(format!(r#"Failed to migrate: {} "#, error).into()),
        };
    }

    if model_version.major == 0 && model_version.minor == 5 && model_version.patch == 0 {
        match migrate_0_5_0_to_0_5_1(&mut model_version, path, &mut json, validate) {
            Ok(result) => result,
            Err(error) => return Err(format!(r#"Failed to migrate: {} "#, error).into()),
        }
    }

    if model_version.major == 0 && model_version.minor == 5 && model_version.patch == 1 {
        match migrate_0_5_1_to_0_5_2(&mut model_version, path, &mut json, validate) {
            Ok(result) => result,
            Err(error) => return Err(format!(r#"Failed to migrate: {} "#, error).into()),
        }
    }

    if model_version.major == 0 && model_version.minor == 5 && model_version.patch == 2 {
        match migrate_0_5_2_to_0_5_3(&mut model_version, path, &mut json, validate) {
            Ok(result) => result,
            Err(error) => return Err(format!(r#"Failed to migrate: {} "#, error).into()),
        }
    }

    if model_version.major == 0 && model_version.minor == 5 && model_version.patch == 3 {
        match migrate_0_5_3_to_0_6_0(&mut model_version, path, &mut json, validate) {
            Ok(result) => result,
            Err(error) => return Err(format!(r#"Failed to migrate: {} "#, error).into()),
        }
    }

    // \note Deserialise JSON into model data structure
    let root_model = match load_jack_model_from_json(json, is_module) {
        Ok(model)  => model,
        Err(error) => return Err(format!("{}|{}|{}: Model file failed to be deserialized to model data structure: {}", path, error.line(), error.column(), error).into()),
    };

    // NOTE: We add the root model first, the model at index 0 is the top-level
    // project's model, always
    result.add_models(vec![root_model]);
    for workspace in sub_workspaces.into_iter() {
        result.add_models(workspace.models)
    }

    if validate {
        let errors = result.models[0].verify(&result);
        for err in errors.iter() {
            println!("{:?}", err);
        }
        if errors.len() > 0 {
            return Err(format!("{} model errors found.", errors.len()).into());
        }
    }
    Ok(result)
}

pub fn load_model_at_path_and_migrate(path: &str, validate: bool, is_module: bool) -> Result<model::ProjectWorkspace, Box<dyn std::error::Error>> {

    // \note Read model file to string
    let file_read_result = fs::read_to_string(path);
    let file_data = match file_read_result {
        Ok(file)   => file,
        Err(error) => return Err(format!(r#"Model file '{}' can not be read: {}"#, path, error).into()),
    };

    let result = load_model_at_buffer_and_migrate(path, file_data.as_str(), validate, is_module);
    result
}

/// Main jack-make function.
/// See bin/jack-make.rs for how it is used.
pub fn main_make() -> Result<(), Box<dyn Error>> {
    let mut model_file     = "".to_string();
    let mut output_dir_arg = "".to_string();
    let mut force          = false;
    let mut validate       = true;
    let mut engine         = true;
    let mut ros            = false;

    // Version information extracted from the build environment
    // env! is a macro to pull in an environment variable
    let version = env!("CARGO_PKG_VERSION");
    let app_name = format!("JACK Make Tool v{}", version);

    {
        // \note Put in brackets to handle ownership and mutability contraints in rust
        let mut ap = ArgumentParser::new();
        ap.set_description(&app_name);
        ap.refer(&mut model_file)    .add_argument("source",             Store,      "Model file path").required();
        ap.refer(&mut output_dir_arg).add_option(&["--output-dir"],      Store,      "Destination directory for generated files");
        ap.refer(&mut force)         .add_option(&["--force"],           StoreTrue,  "Force files to be overwritten");
        ap.refer(&mut validate)      .add_option(&["--skip-validation"], StoreFalse, "Skip validation of the JACK model");
        ap.refer(&mut engine)        .add_option(&["--module"],          StoreFalse, "Create a JACK module instead of a JACK sim project");
        ap.refer(&mut ros)           .add_option(&["--ros"],             StoreTrue,  "Generate ROS messages from JACK messages");
        ap.parse_args_or_exit();
    }

    if model_file.len() == 0 {
        return Err("JACK model file (.jack) must be passed to jack-make\nSee -h or --help for more help".to_string().into());
    }

    println!("===================================================================================");
    println!("== jack-make v{} - JACK Code Generator", version);
    println!("== model: {}", model_file);
    println!("== module: {}", engine == false);
    println!("===================================================================================");

    let workspace = load_model_at_path_and_migrate(model_file.as_str(), validate, /*is_module*/engine == false)?;
    if workspace.models.is_empty() {
        return Err("No JACK models were loaded or able to be loaded from the input files".to_string().into());
    }
    let date                    = format!("{}", Local::today().format("%d-%m-%Y"));
    let model                   = &workspace.models[0];
    let mut output_dir: PathBuf = match output_dir_arg.is_empty() {
        true  => { env::current_dir()? }
        false => { PathBuf::from(output_dir_arg) }
    };
    output_dir.push(workspace.project().bumpy_case.to_lowercase());

    // \note Impl files
    {
        if engine {
            let cpp = templates::MainCpp{workspace: &workspace, project: workspace.project()};
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }

        for team in model.teams.iter() {
            let hpp = templates::TeamImplH  {workspace: &workspace, project: workspace.project(), agent: team.clone(), date: date.clone(), is_team: true};
            let cpp = templates::TeamImplCpp{workspace: &workspace, project: workspace.project(), agent: team.clone(), is_team: true};
            templates::write_template(&hpp, &output_dir, &force, false /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }

        for agent in model.agents.iter() {
            let hpp = templates::AgentImplH  {workspace: &workspace, project: workspace.project(), agent: agent.clone(), date: date.clone(), is_team: false};
            let cpp = templates::AgentImplCpp{workspace: &workspace, project: workspace.project(), agent: agent.clone(), is_team: false };
            templates::write_template(&hpp, &output_dir, &force, false /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }

        for service in model.services.iter() {
            let hpp = templates::ServiceImplH  {workspace: &workspace, project: workspace.project(), service: service.clone(), date: date.clone()};
            let cpp = templates::ServiceImplCpp{workspace: &workspace, project: workspace.project(), service: service.clone()};
            templates::write_template(&hpp, &output_dir, &force, false /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }

        for goal in model.goals.iter() {
            let hpp = templates::GoalImplH  {workspace: &workspace, project: workspace.project(), goal: goal.clone(), date: date.clone()};
            let cpp = templates::GoalImplCpp{workspace: &workspace, project: workspace.project(), goal: goal.clone()};
            templates::write_template(&hpp, &output_dir, &force, false /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }

        for plan in model.plans.iter() {
            let hpp = templates::PlanImplH  {workspace: &workspace, project: workspace.project(), plan: plan.clone(), date: date.clone()};
            let cpp = templates::PlanImplCpp{workspace: &workspace, project: workspace.project(), plan: plan.clone(), date: date.clone()};
            templates::write_template(&hpp, &output_dir, &force, false /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, false /*is_meta_file*/)?;
        }
    }

    // \note Meta files
    {
        let cmake     = templates::ProjectCMake{project: workspace.project(), model: &model, is_module: engine == false};
        templates::write_template(&cmake, &output_dir, &force, true /*is_meta_file*/)?;

        if engine {
            let hpp = templates::ProjectH  {                       project: workspace.project(), model: &model};
            let cpp = templates::ProjectCpp{workspace: &workspace, project: workspace.project(), model: &model};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        } else {
            let hpp = templates::ModuleH  {                       project: workspace.project(), model: &model};
            let cpp = templates::ModuleCpp{workspace: &workspace, project: workspace.project(), model: &model};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        // generate ROS messages if requested
        if ros {
            for msg in model.messages.iter() {
                let message_msg = templates::Ros2IdlMetaMsg{workspace: &workspace, project: workspace.project(), message: msg};
                templates::write_template(&message_msg, &output_dir, &force, true /*is_meta_file*/)?;
            }
        }

        for team in model.teams.iter() {
            let hpp = templates::TeamMetaH  {workspace: &workspace, project: workspace.project(), model: &model, agent: team.clone(), is_team: true };
            let cpp = templates::TeamMetaCpp{workspace: &workspace, project: workspace.project(), model: &model, agent: team.clone(), is_team: true };
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for agent in model.agents.iter() {
            let hpp = templates::AgentMetaH  {workspace: &workspace, project: workspace.project(), model: &model, agent: agent.clone(), is_team: false };
            let cpp = templates::AgentMetaCpp{workspace: &workspace, project: workspace.project(), model: &model, agent: agent.clone(), is_team: false };
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for service in model.services.iter() {
            let hpp = templates::ServiceMetaH  {workspace: &workspace, project: workspace.project(), model: &model, service: service.clone()};
            let cpp = templates::ServiceMetaCpp{workspace: &workspace, project: workspace.project(), model: &model, service: service.clone()};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for msg in model.messages.iter() {
            let hpp = templates::BeliefMetaH  {workspace: &workspace, project: workspace.project(), model: &model, beliefset: msg.clone()};
            let cpp = templates::BeliefMetaCpp{workspace: &workspace, project: workspace.project(), model: &model, beliefset: msg.clone()};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for resource in model.resources.iter() {
            let hpp = templates::ResourceMetaH{project: workspace.project(), model: &model, resource: resource.clone()};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for goal in model.goals.iter() {
            let hpp = templates::GoalMetaH  {workspace: &workspace, project: workspace.project(), model: &model, goal: goal.clone()};
            let cpp = templates::GoalMetaCpp{workspace: &workspace, project: workspace.project(), model: &model, goal: goal.clone()};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        for plan in model.plans.iter() {
            let hpp = templates::PlanMetaH  {project: workspace.project(), model: &model, plan: plan.clone()};
            let cpp = templates::PlanMetaCpp{project: workspace.project(), model: &model, plan: plan.clone()};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        {
            let hpp = templates::EnumsMetaH  {project: workspace.project(), model: &model};
            let cpp = templates::EnumsMetaCpp{project: workspace.project(), model: &model};
            templates::write_template(&hpp, &output_dir, &force, true /*is_meta_file*/)?;
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        {
            let cpp = templates::EntityTemplateMetaJson{workspace: &workspace, project: workspace.project()};
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        {
            let cpp = templates::EventMetaJson{workspace: &workspace, project: workspace.project(), model: &model};
            templates::write_template(&cpp, &output_dir, &force, true /*is_meta_file*/)?;
        }

        {
            let service_components_meta = templates::ServiceComponentsMeta{project: workspace.project(), model: &model};
            templates::write_template(&service_components_meta, &output_dir, &force, true /*is_meta_file*/)?;
        }
    }
    Ok(())
}
