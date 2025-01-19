/* eslint-disable no-param-reassign */
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';

/**
 * merge module projects into project
 * @param project
 * @param moduleProjects
 * @returns
 */
export const mergeModuleProjectsToProject = (project: CBDIEditorProject, moduleProjects: CBDIEditorProject[]) => {
  const newProject = { ...project };
  let cbdiObjects = project.cbdiObjects;
  moduleProjects.forEach((moduleProject) => {
    newProject.moduleProjectInfoDic[moduleProject.name] = Object.values(moduleProject.moduleProjectInfoDic)[0];

    cbdiObjects = { ...cbdiObjects, ...moduleProject.cbdiObjects };

    newProject.enums.push(...moduleProject.enums);
    newProject.teams.push(...moduleProject.teams);
    newProject.agents.push(...moduleProject.agents);
    newProject.tactics.push(...moduleProject.tactics);
    newProject.roles.push(...moduleProject.roles);
    newProject.goals.push(...moduleProject.goals);
    newProject.plans.push(...moduleProject.plans);
    newProject.resources.push(...moduleProject.resources);
    newProject.actions.push(...moduleProject.actions);
    newProject.messages.push(...moduleProject.messages);
    newProject.services.push(...moduleProject.services);
    newProject.entities.push(...moduleProject.entities);
    newProject.events.push(...moduleProject.events);
  });

  return { ...newProject, cbdiObjects };
};

/**
 * remove module project from project with module name
 * @param project
 * @param moduleName
 * @returns
 */
export const removeModuleProjectFromProject = (project: CBDIEditorProject, moduleName: string) => {
  const modulePaths = project.modulePaths.filter((el) => el.name !== moduleName);

  const moduleProjectInfos = { ...project.moduleProjectInfoDic };
  delete moduleProjectInfos[moduleName];
  const cbdiObjects = Object.fromEntries(Object.entries(project.cbdiObjects).filter(([uuid, cbdiObject]) => cbdiObject.module !== moduleName));

  const enums = project.enums.filter((el) => el.module !== moduleName);
  const teams = project.teams.filter((el) => el.module !== moduleName);
  const agents = project.agents.filter((el) => el.module !== moduleName);
  const tactics = project.tactics.filter((el) => el.module !== moduleName);
  const roles = project.roles.filter((el) => el.module !== moduleName);
  const goals = project.goals.filter((el) => el.module !== moduleName);
  const plans = project.plans.filter((el) => el.module !== moduleName);
  const resources = project.resources.filter((el) => el.module !== moduleName);
  const actions = project.actions.filter((el) => el.module !== moduleName);
  const messages = project.messages.filter((el) => el.module !== moduleName);
  const entities = project.entities.filter((el) => el.module !== moduleName);
  const events = project.events.filter((el) => el.module !== moduleName);
  const services = project.services.filter((el) => el.module !== moduleName);

  const newProject = {
    ...project,
    modulePaths,
    moduleProjectInfos,
    cbdiObjects,
    enums,
    teams,
    agents,
    tactics,
    roles,
    goals,
    plans,
    resources,
    actions,
    messages,
    entities,
    services,
    events,
  };

  return newProject;
};

type AnyObject = { [key: string]: any };
/**
 * Recursively replaces occurrences of a specified module value with a new value in an object.
 *
 * @param obj - The object to process.
 * @param oldModuleName - The module value to be replaced.
 * @param newModuleName - The new module value to replace with.
 * @returns The processed object with the module values replaced.
 */
function replaceCbdiObjectModuleName(obj: AnyObject, oldModuleName: string, newModuleName: string): AnyObject {
  // Check if the object is an array
  if (Array.isArray(obj)) {
    // If it's an array, loop through each element and apply the function recursively
    obj.forEach((item, index) => {
      obj[index] = replaceCbdiObjectModuleName(item, oldModuleName, newModuleName);
    });
    // Check if the object is a non-null object
  } else if (typeof obj === 'object' && obj !== null) {
    // Loop through each key-value pair in the object
    Object.entries(obj).forEach(([key, value]) => {
      // If the key is 'module' and the value matches oldModuleName, change it to newModuleName
      if (key === 'module' && value === oldModuleName) {
        obj[key] = newModuleName;
      } else {
        // Otherwise, apply the function recursively to the nested objects
        obj[key] = replaceCbdiObjectModuleName(value, oldModuleName, newModuleName);
      }
    });
  }
  // Return the processed object
  return obj;
}

/**
 * rename module project's name
 */
export const renameModuleProject = (project: CBDIEditorProject, oldModuleName: string, newModuleName: string) => {
  const modulePaths = project.modulePaths.map((el) => {
    if (el.name === oldModuleName) {
      return { ...el, name: newModuleName };
    }
    return el;
  });

  const moduleProjectInfoDic = { ...project.moduleProjectInfoDic };
  const newProjectInfo = { ...project.moduleProjectInfoDic[oldModuleName], name: newModuleName };
  delete moduleProjectInfoDic[oldModuleName];
  moduleProjectInfoDic[newModuleName] = newProjectInfo;
  const cbdiObjects = Object.fromEntries(
    Object.entries(project.cbdiObjects).map(([uuid, cbdiObject]) => [uuid, replaceCbdiObjectModuleName(cbdiObject, oldModuleName, newModuleName)]),
  );

  const enums = project.enums.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const teams = project.teams.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const agents = project.agents.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const tactics = project.tactics.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const roles = project.roles.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const goals = project.goals.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const plans = project.plans.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const resources = project.resources.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const actions = project.actions.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const messages = project.messages.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const entities = project.entities.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const events = project.events.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });
  const services = project.services.map((el) => {
    if (el.module === oldModuleName) {
      return { ...el, module: newModuleName };
    }
    return el;
  });

  const newProject = {
    ...project,
    modulePaths,
    moduleProjectInfoDic,
    cbdiObjects,
    enums,
    teams,
    agents,
    tactics,
    roles,
    goals,
    plans,
    resources,
    actions,
    messages,
    entities,
    services,
    events,
  };

  return newProject;
};
