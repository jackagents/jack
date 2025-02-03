// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

pub use crate::model::*;
use std::error::Error;
use std::path::PathBuf;

use askama::Template;

/// Trait to allow me to easily use all the below structs in the write_template function.
/// This trait supplies the filename for the template to be written to disk.
pub trait TemplateOutput {
    fn output_filename(&self) -> PathBuf;
}

/// Takes any struct that hast the template output and template traits and writes them to disk.
pub fn write_template<Z : TemplateOutput + Template>(template: &Z, output_path : &PathBuf, force : &bool, is_meta_file: bool) -> Result<(),Box<dyn Error>>
{
    let file_path = output_path.clone().join(template.output_filename());
    let mut dir   = file_path.clone();
    dir.pop();
    let dir = dir.as_path();

    if !dir.exists() && !dir.is_dir() {
        std::fs::create_dir_all(dir)?;
    }

    let dir = dir.canonicalize()?;
    if dir.is_dir()  {
        let mut write_template = *force;
        let new_contents       = template.render()?;
        if !write_template {

            // If we're writing meta files, we do not need to write the file if the contents are
            // the same, doing this we can save dirtying the file and causing C++ to recompile
            // everything.
            write_template = true;
            if is_meta_file {
                let file_read_result = std::fs::read_to_string(&file_path);
                match file_read_result {
                    Ok(old_contents) => {
                        write_template = new_contents != old_contents;
                    },
                    _ => { }
                }
            } else {
                write_template = !file_path.as_path().exists();
            }
        }

        if write_template {
            // \note Canonicalize will only work after the file is written,
            // fails if the path does not exist so we canonicalise at the very
            // last moment so that paths are printed with native OS paths.
            std::fs::write(&file_path, new_contents)?;
            println!("Writing {:?}", file_path.canonicalize().unwrap_or(file_path));
        }
    } else {
        return Err(format!("{:?} is not a directory", dir).into())
    }

    Ok(())
}

/******************************************************************************
 * ROS
 ******************************************************************************/
#[derive(Template)] // procedural macro to create the template creation code.
#[template(path = "ros2idl.msg.j2", escape = "none")] // template file and if there are any escape sequences.
pub struct Ros2IdlMetaMsg<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub message:   &'a Message,
}

impl TemplateOutput for Ros2IdlMetaMsg<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/msgs/{}.msg", self.message.id.bumpy_case));
        result
    }
}

/******************************************************************************
 * Service
 ******************************************************************************/
#[derive(Template)]
#[template(path = "serviceimpl.cpp.j2", escape = "none")]
pub struct ServiceImplCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub service:   Service,
}

impl TemplateOutput for ServiceImplCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/services/{}impl.cpp", self.service.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "serviceimpl.h.j2", escape = "none")]
pub struct ServiceImplH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub service:   Service,
    pub date:      String,
}

impl TemplateOutput for ServiceImplH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/services/{}impl.h", self.service.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "servicemeta.h.j2", escape = "none")]
pub struct ServiceMetaH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub service:   Service,
}

impl TemplateOutput for ServiceMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/services/{}meta.h", self.service.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "servicemeta.cpp.j2", escape = "none")]
pub struct ServiceMetaCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub service:   Service,
}

impl TemplateOutput for ServiceMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/services/{}meta.cpp", self.service.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Agent
 ******************************************************************************/
#[derive(Template)] // procedural macro to create the template creation code.
#[template(path = "agentimpl.cpp.j2", escape = "none")] // template file and if there are any escape sequences.
pub struct AgentImplCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for AgentImplCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/agents/{}impl.cpp", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "agentimpl.h.j2", escape = "none")]
pub struct AgentImplH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub agent:     Agent,
    pub date:      String,
    pub is_team:   bool,
}

impl TemplateOutput for AgentImplH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/agents/{}impl.h", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "agentmeta.h.j2", escape = "none")]
pub struct AgentMetaH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for AgentMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/agents/{}meta.h", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "agentmeta.cpp.j2", escape = "none")]
pub struct AgentMetaCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for AgentMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/agents/{}meta.cpp", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Team
 ******************************************************************************/
#[derive(Template)]
#[template(path = "agentimpl.cpp.j2", escape = "none")]
pub struct TeamImplCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for TeamImplCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let mut path = PathBuf::new();
        path.push(format!("impl/teams/{}impl.cpp", self.agent.id.bumpy_case.to_lowercase()));
        path
    }
}

#[derive(Template)]
#[template(path = "agentimpl.h.j2", escape = "none")]
pub struct TeamImplH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub agent:     Agent,
    pub date:      String,
    pub is_team:   bool,
}

impl TemplateOutput for TeamImplH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/teams/{}impl.h", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "agentmeta.h.j2", escape = "none")]
pub struct TeamMetaH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for TeamMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/teams/{}meta.h", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "agentmeta.cpp.j2", escape = "none")]
pub struct TeamMetaCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub agent:     Agent,
    pub is_team:   bool,
}

impl TemplateOutput for TeamMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/teams/{}meta.cpp", self.agent.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Message
 ******************************************************************************/
#[derive(Template)]
#[template(path = "beliefmeta.h.j2", escape = "none")]
pub struct BeliefMetaH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub beliefset: Message,
}

impl TemplateOutput for BeliefMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/messages/{}meta.h", self.beliefset.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "beliefmeta.cpp.j2", escape = "none")]
pub struct BeliefMetaCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub beliefset: Message,
}

impl TemplateOutput for BeliefMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/messages/{}meta.cpp", self.beliefset.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Goal
 ******************************************************************************/
#[derive(Template)]
#[template(path = "goalimpl.h.j2", escape = "none")]
pub struct GoalImplH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub goal:      Goal,
    pub date:      String,
}

impl TemplateOutput for GoalImplH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/goals/{}impl.h", self.goal.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "goalimpl.cpp.j2", escape = "none")]
pub struct GoalImplCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub goal:      Goal,
}

impl TemplateOutput for GoalImplCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/goals/{}impl.cpp", self.goal.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "goalmeta.h.j2", escape = "none")]
pub struct GoalMetaH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub goal:      Goal,
}

impl TemplateOutput for GoalMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/goals/{}meta.h", self.goal.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "goalmeta.cpp.j2", escape = "none")]
pub struct GoalMetaCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub goal:      Goal,
}

impl TemplateOutput for GoalMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/goals/{}meta.cpp", self.goal.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Plan
 ******************************************************************************/
#[derive(Template)]
#[template(path = "planimpl.h.j2", escape = "none")]
pub struct PlanImplH<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub plan:      Plan,
    pub date:      String,
}

impl TemplateOutput for PlanImplH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/plans/{}impl.h", self.plan.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "planimpl.cpp.j2", escape = "none")]
pub struct PlanImplCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub plan:      Plan,
    pub date:      String,
}

impl TemplateOutput for PlanImplCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/plans/{}impl.cpp", self.plan.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "planmeta.h.j2", escape = "none")]
pub struct PlanMetaH<'a> {
    pub model:   &'a Model,
    pub project: &'a Project,
    pub plan:    Plan,
}

impl TemplateOutput for PlanMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/plans/{}meta.h", self.plan.id.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "planmeta.cpp.j2", escape = "none")]
pub struct PlanMetaCpp<'a> {
    pub model:   &'a Model,
    pub project: &'a Project,
    pub plan:    Plan,
}

impl TemplateOutput for PlanMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/plans/{}meta.cpp", self.plan.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Project
 ******************************************************************************/
#[derive(Template)]
#[template(path = "project.h.j2", escape = "none")]
pub struct ProjectH<'a> {
    pub project: &'a Project,
    pub model:   &'a Model,
}

impl TemplateOutput for ProjectH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}project.h", self.project.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "project.cpp.j2", escape = "none")]
pub struct ProjectCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
}

impl TemplateOutput for ProjectCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}project.cpp", self.project.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "project.cmake.j2", escape = "none")]
pub struct ProjectCMake<'a> {
    pub project:   &'a Project,
    pub model:     &'a Model,
    pub is_module: bool,
}

impl TemplateOutput for ProjectCMake<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}.cmake", self.project.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "sampleentrypoint.cpp.j2", escape = "none")]
pub struct MainCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
}

impl TemplateOutput for MainCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("impl/{}sampleentrypoint.cpp", self.workspace.project().bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Enum
 ******************************************************************************/
#[derive(Template)]
#[template(path = "enumsmeta.h.j2", escape = "none")]
pub struct EnumsMetaH<'a> {
    pub project: &'a Project,
    pub model:   &'a Model,
}

impl TemplateOutput for EnumsMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}enumsmeta.h", self.project.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "enumsmeta.cpp.j2", escape = "none")]
pub struct EnumsMetaCpp<'a> {
    pub project: &'a Project,
    pub model:   &'a Model,
}

impl TemplateOutput for EnumsMetaCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}enumsmeta.cpp", self.project.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Resource
 ******************************************************************************/
#[derive(Template)]
#[template(path = "resourcemeta.h.j2", escape = "none")]
pub struct ResourceMetaH<'a> {
    pub project:  &'a Project,
    pub model:    &'a Model,
    pub resource: Resource,
}

impl TemplateOutput for ResourceMetaH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/resources/{}meta.h", self.resource.id.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Event
 ******************************************************************************/
#[derive(Template)]
#[template(path = "events.h.j2", escape = "none")]
pub struct EventMetaJson<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
}

impl TemplateOutput for EventMetaJson<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}events.h", self.project.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Entity Templates
 ******************************************************************************/
#[derive(Template)]
#[template(path = "templates.json.j2", escape = "none")]
pub struct EntityTemplateMetaJson<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
}

impl TemplateOutput for EntityTemplateMetaJson<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}templates.json", self.project.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Modules
 ******************************************************************************/
#[derive(Template)]
#[template(path = "module.h.j2", escape = "none")]
pub struct ModuleH<'a> {
    pub project: &'a Project,
    pub model:   &'a Model,
}

impl TemplateOutput for ModuleH<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}module.h", self.project.bumpy_case.to_lowercase()));
        result
    }
}

#[derive(Template)]
#[template(path = "module.cpp.j2", escape = "none")]
pub struct ModuleCpp<'a> {
    pub workspace: &'a ProjectWorkspace,
    pub project:   &'a Project,
    pub model:     &'a Model,
}

impl TemplateOutput for ModuleCpp<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}module.cpp", self.project.bumpy_case.to_lowercase()));
        result
    }
}

/******************************************************************************
 * Service Sim Components
 ******************************************************************************/
#[derive(Template)]
#[template(path = "servicecomponents.h.j2", escape = "none")]
pub struct ServiceComponentsMeta<'a> {
    pub project: &'a Project,
    pub model:   &'a Model,
}

impl TemplateOutput for ServiceComponentsMeta<'_> {
    fn output_filename(&self) -> PathBuf {
        let result = PathBuf::from(format!("meta/{}servicecomponents.h", self.project.bumpy_case.to_lowercase()));
        result
    }
}
