/* eslint-disable no-case-declarations */
/* eslint-disable no-param-reassign */
/* eslint-disable no-underscore-dangle */
import { CBDIEditorProject, CBDIEditorProjectPlan, CBDIEditorProjectTactic } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorObject, Mod, CBDIEditorRootConceptType, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';

export function registerObject(project: CBDIEditorProject, object: CBDIEditorObject) {
  if (!project || !object) {
    console.error('Cannot register object, project or object is undefined');
    return;
  }
  const { module, name, uuid, ...others } = object;
  const moduleConcept = { module, name, uuid };
  switch (object._objectType) {
    case CBDIEditorRootConceptType.TeamConceptType: {
      project.teams.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.AgentConceptType: {
      project.agents.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.TacticConceptType: {
      project.tactics.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.RoleConceptType: {
      project.roles.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.ServiceConceptType: {
      project.services.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.GoalConceptType: {
      project.goals.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.PlanConceptType: {
      project.plans.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.ResourceConceptType: {
      project.resources.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.ActionConceptType: {
      project.actions.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.MessageConceptType: {
      project.messages.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.EnumConceptType: {
      project.enums.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.EntityConceptType: {
      project.entities.push(moduleConcept);
      break;
    }
    case CBDIEditorRootConceptType.EventConceptType: {
      project.events.push(moduleConcept);
      break;
    }
    default: {
      project.messages.push(moduleConcept);
      break;
    }
  }
}

export function deregisterObject(project: CBDIEditorProject, object: CBDIEditorObject) {
  if (!project || !object) {
    console.error('Cannot deregister object, project or object is undefined');
    return;
  }
  switch (object._objectType) {
    case CBDIEditorRootConceptType.TeamConceptType: {
      const teamIndex = project.teams.findIndex((el) => el.uuid === object.uuid);
      if (teamIndex > -1) {
        project.teams.splice(teamIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.AgentConceptType: {
      const agentIndex = project.agents.findIndex((el) => el.uuid === object.uuid);
      if (agentIndex > -1) {
        project.agents.splice(agentIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.TacticConceptType: {
      const tacticIndex = project.tactics.findIndex((el) => el.uuid === object.uuid);
      if (tacticIndex > -1) {
        project.tactics.splice(tacticIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.RoleConceptType: {
      const roleIndex = project.roles.findIndex((el) => el.uuid === object.uuid);
      if (roleIndex > -1) {
        project.roles.splice(roleIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.ServiceConceptType: {
      const serviceIndex = project.services.findIndex((el) => el.uuid === object.uuid);
      if (serviceIndex > -1) {
        project.services.splice(serviceIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.GoalConceptType: {
      const goalIndex = project.goals.findIndex((el) => el.uuid === object.uuid);
      if (goalIndex > -1) {
        project.goals.splice(goalIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.PlanConceptType: {
      const planIndex = project.plans.findIndex((el) => el.uuid === object.uuid);
      if (planIndex > -1) {
        project.plans.splice(planIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.ResourceConceptType: {
      const resourceIndex = project.resources.findIndex((el) => el.uuid === object.uuid);
      if (resourceIndex > -1) {
        project.resources.splice(resourceIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.ActionConceptType: {
      const actionIndex = project.actions.findIndex((el) => el.uuid === object.uuid);
      if (actionIndex > -1) {
        project.actions.splice(actionIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.MessageConceptType: {
      const messageIndex = project.messages.findIndex((el) => el.uuid === object.uuid);
      if (messageIndex > -1) {
        project.messages.splice(messageIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.EnumConceptType: {
      const enumIndex = project.enums.findIndex((el) => el.uuid === object.uuid);
      if (enumIndex > -1) {
        project.enums.splice(enumIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.EntityConceptType: {
      const entityIndex = project.entities.findIndex((el) => el.uuid === object.uuid);
      if (entityIndex > -1) {
        project.entities.splice(entityIndex, 1);
      }
      break;
    }
    case CBDIEditorRootConceptType.EventConceptType: {
      const eventIndex = project.events.findIndex((el) => el.uuid === object.uuid);
      if (eventIndex > -1) {
        project.events.splice(eventIndex, 1);
      }
      break;
    }
    default: {
      break;
    }
  }
}

export function putObject(current: CBDIEditorProject, cbdiObject: CBDIEditorObject) {
  const object: CBDIEditorObject = copy(cbdiObject);
  if (!object) {
    console.error('Cannot put object, object is undefined');
    return;
  }
  if (object._mod !== Mod.Addition) {
    object._mod = Mod.Addition;
  }
  if (!current) {
    console.error('Cannot put object, current project is undefined');
    return;
  }
  current.cbdiObjects[object.uuid] = object;
  registerObject(current, object);
}

export function deleteObject(current: CBDIEditorProject, objectId: string) {
  if (!current) {
    console.error('Cannot delete object, current project is undefined');
    return;
  }
  const object: CBDIEditorObject = copy(current.cbdiObjects[objectId]);
  if (!object) {
    console.error('Cannot delete object, object is undefined');
    return;
  }

  // make object mod to be Deletion
  object._mod = Mod.Deletion;
  current.cbdiObjects[objectId] = object;
}

export function saveObject(current: CBDIEditorProject, objectId: string) {
  if (!current) {
    console.error('Cannot save object, current or saved project is undefined');
    return;
  }
  const object: CBDIEditorObject = copy(current.cbdiObjects[objectId]);
  if (!object) {
    console.error('Cannot save object, object is undefined');
    return;
  }
  const prevMod = object._mod;
  if (prevMod !== Mod.Deletion) {
    object._mod = Mod.None;
  }
  // We also need to update the current for the change of `_mod` flag.
  current.cbdiObjects[objectId] = object;
}

export function changeObjectModule(current: CBDIEditorProject, changingModuleConcept: ModuleConcept, newModuleName: string) {
  if (changingModuleConcept.module === newModuleName) {
    return;
  }
  const obj = current.cbdiObjects[changingModuleConcept.uuid];
  if (obj) {
    const newChangingModuleConcept = { ...changingModuleConcept, module: newModuleName };
    // Update module name in cbdiObjects
    current.cbdiObjects[changingModuleConcept.uuid] = { ...obj, module: newModuleName };

    // Update module name in corresponding category
    switch (obj._objectType) {
      case CBDIEditorRootConceptType.TeamConceptType:
        const teamIndex = current.teams.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (teamIndex > -1) {
          current.teams[teamIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.AgentConceptType:
        const agentIndex = current.agents.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (agentIndex > -1) {
          current.agents[agentIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.EntityConceptType:
        const entityIndex = current.entities.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (entityIndex > -1) {
          current.entities[entityIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.ActionConceptType:
        const actionIndex = current.actions.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (actionIndex > -1) {
          current.actions[actionIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.GoalConceptType:
        const goalIndex = current.goals.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (goalIndex > -1) {
          current.goals[goalIndex] = newChangingModuleConcept;
        }

        break;

      case CBDIEditorRootConceptType.EnumConceptType:
        const enumIndex = current.enums.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (enumIndex > -1) {
          current.enums[enumIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.EventConceptType:
        const eventIndex = current.events.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (eventIndex > -1) {
          current.events[eventIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.MessageConceptType:
        const messageIndex = current.messages.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (messageIndex > -1) {
          current.messages[messageIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.PlanConceptType:
        const planIndex = current.plans.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (planIndex > -1) {
          current.plans[planIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.ResourceConceptType:
        const resourceIndex = current.resources.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (resourceIndex > -1) {
          current.resources[resourceIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.RoleConceptType:
        const roleIndex = current.roles.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (roleIndex > -1) {
          current.roles[roleIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.ServiceConceptType:
        const serviceIndex = current.services.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (serviceIndex > -1) {
          current.services[serviceIndex] = newChangingModuleConcept;
        }
        break;

      case CBDIEditorRootConceptType.TacticConceptType:
        const tacticIndex = current.tactics.findIndex((el) => el.uuid === changingModuleConcept.uuid);
        if (tacticIndex > -1) {
          current.tactics[tacticIndex] = newChangingModuleConcept;
        }
        break;

      default:
        break;
    }

    // COMMENT THIS FOR NOW
    // MAY USE IT AFTERWARDS
    // If it is goal
    // Change the module name of goals' plans and tactics
    // if (obj._objectType === CBDIEditorRootConceptType.GoalConceptType) {
    //   current.plans.forEach((planModuleConcept) => {
    //     const planObj = getObjectByModuleConcept(current, planModuleConcept) as CBDIEditorProjectPlan;
    //     if (planObj.handles.uuid === changingModuleConcept.uuid) {
    //       changeObjectModule(current, planModuleConcept, newModuleName);
    //     }
    //   });

    //   current.tactics.forEach((tacticModuleConcept) => {
    //     const tacticObj = getObjectByModuleConcept(current, tacticModuleConcept) as CBDIEditorProjectTactic;
    //     if (tacticObj.goal.uuid === changingModuleConcept.uuid) {
    //       changeObjectModule(current, tacticModuleConcept, newModuleName);
    //     }
    //   });
    // }
  }
}
