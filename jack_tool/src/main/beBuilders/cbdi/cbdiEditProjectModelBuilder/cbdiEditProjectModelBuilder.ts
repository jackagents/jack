// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/* eslint-disable no-await-in-loop */
/* eslint-disable class-methods-use-this */
/* eslint-disable @typescript-eslint/no-shadow */
import {
  CBDIEditorProject,
  CBDIEditorModuleModel,
  ModulePathObjModel,
  FlatMetaData,
  CBDIEditorProjectModel,
  ModulePathObj,
} from 'types/cbdiEdit/cbdiEditModel';
import fs from 'fs';
import { copy } from 'misc/utils/cbdiEdit/Helpers';
import { saveCBDIJsonFromProject, convertModuleProjectFileToProject, saveCBDIProjectFromProject } from 'misc/utils/cbdiEdit/ModelTransform';
import path from 'path';
import { dialog } from 'electron';
import crypto from 'crypto';
import { request as editorRequest, response as editorResponse } from 'projectEvents/cbdiEdit/editEvents';
import electronStore from 'main/electronStore/ElectronStore';
import packageJson from 'package.json';
import { getMetaDataPath, readMetaDataFromCsv, saveMetaDataCsvFile } from 'main/listeners/cbdiEdit/CbdiMetaDataHandler';
import { mergeModuleProjectsToProject, removeModuleProjectFromProject, renameModuleProject } from './helper';

const fsPromise = fs.promises;

export interface EditModelBuildResult {
  editModel: CBDIEditorProject | null;
  filePath: string;
}

class CBDIEditProjectModelBuilder {
  /* ---------------------------- Private Property ---------------------------- */
  private _window: Electron.BrowserWindow;

  // .prj.jack.json file path
  private _projectFilePath: string | null = null;

  // is detect file change dialog open
  private _isDetectFileChangeDialogOpen: boolean = false;

  // file watcher (watching module files change)
  private _fileWatchers: { watcher: fs.StatWatcher; filePath: string; moduleName: string; fileHash: string }[] = [];

  /* ----------------------------- Public Property ---------------------------- */
  // static project model for scenario editor

  public cbdiEditorProject: CBDIEditorProject | null = null;

  /* ------------------------------- CONSTRUCTOR ------------------------------ */

  constructor(window: Electron.BrowserWindow) {
    this._window = window;
  }

  /* --------------------------------- PRIVATE -------------------------------- */

  private _handleSerialiseError = (error: any, modelFilePath: string | null, modelType: 'Project' | 'Module') => {
    const fileName = (() => {
      if (modelFilePath) {
        return path.basename(modelFilePath);
      }
      return '';
    })();
    console.log(error.stack);
    dialog.showMessageBox({
      type: 'error',
      title: `Model Serialisation at ${modelFilePath}`,
      message: `${modelType} ${fileName}: ${error.message}`,
      detail: error.stack,
    });
  };

  /**
   * dialog when detect change outside of cbdi edit
   */
  private _detectChangeFromOutside = () => {
    if (!this._isDetectFileChangeDialogOpen) {
      this._isDetectFileChangeDialogOpen = true;
      dialog
        .showMessageBox(this._window, {
          title: 'A change outside of JACK Editor has been detected',
          message: 'What do you want to do?',
          type: 'question',
          defaultId: 2,
          buttons: ['Overwrite', 'Save as', 'Reload'],
        })
        .then(async (result) => {
          switch (result.response) {
            case 0:
              this._window.webContents.send(editorRequest.project.overwrite);

              break;
            case 1:
              this._window.webContents.send(editorRequest.project.saveAs);

              break;
            case 2:
              await this.readProject(this._projectFilePath!);

              break;
            default:
              break;
          }
          this._isDetectFileChangeDialogOpen = false;
          return result;
        })
        .catch((e) => {
          console.log(e);
          dialog.showErrorBox('ERROR', e.stack || e.message);
        });
    }
  };

  /**
   * dialog when detect loaded file missing
   */
  private _detectLoadedFileMissing = () => {
    dialog
      .showMessageBox(this._window, {
        title: 'Project file is missing',
        message: 'What do you want to do?',
        type: 'question',
        defaultId: 0,
        buttons: ['Save as', 'Open a project'],
      })
      .then(async (result) => {
        switch (result.response) {
          case 0:
            this._window.webContents.send(editorRequest.project.saveAs);

            break;
          case 1:
            await this.openProject();

            break;
          default:
            break;
        }
        return result;
      })
      .catch((e) => {
        console.log(e);
        dialog.showErrorBox('ERROR', e.stack || e.message);
      });
  };

  /**
   * check if path exists
   */
  private _checkPathExists = (checkingPath: string) => {
    try {
      fs.accessSync(checkingPath);
      return true; // Path exists
    } catch (error) {
      return false; // Path does not exist
    }
  };

  /**
   * update module file watcher
   * @param moduleName updating watcher's module name
   * @param newFilePath new watching abosulte path
   */
  private async _updateModuleFileWatcher(moduleName: string, newFilePath: string | null) {
    // listener for the filePath change event
    const fileWatcherIndex = this._fileWatchers.findIndex((el) => el.moduleName === moduleName);

    if (fileWatcherIndex > -1) {
      const fileWatcher = this._fileWatchers[fileWatcherIndex];
      fileWatcher.watcher.removeAllListeners();
      this._fileWatchers.splice(fileWatcherIndex, 1);
      console.log(`Stopped watching file ${fileWatcher.filePath} for ${moduleName}`);
    }
    if (newFilePath) {
      const newWatcher = fs.watchFile(newFilePath, (curr, prev) => {
        try {
          const fileContents = fs.readFileSync(newFilePath);
          const currentFileHash = crypto.createHash('md5').update(fileContents).digest('hex');
          const oldFileHash = this._fileWatchers.find((el) => el.moduleName === moduleName)?.fileHash;

          if (oldFileHash && currentFileHash !== oldFileHash) {
            console.log(`File ${newFilePath} for ${moduleName} changed outside the cbdi edit`);
            this._detectChangeFromOutside();
          }
        } catch (error) {
          this._detectLoadedFileMissing();
          console.log(`File ${newFilePath} for ${moduleName} does not exit`);
        }
      });
      const fileContents = fs.readFileSync(newFilePath);
      const currentFileHash = crypto.createHash('md5').update(fileContents).digest('hex');
      this._fileWatchers.push({ watcher: newWatcher, filePath: newFilePath, moduleName, fileHash: currentFileHash });
      console.log(`Start watching file ${newFilePath} for ${moduleName}`);
    }
  }

  private _clearModuleFileWatchers() {
    this._fileWatchers.forEach((el) => {
      el.watcher.removeAllListeners();
    });
  }

  // TODO: need to detect project name not match
  /**
   * handle module project name does not match module name
   */
  private _handleModuleProjectNameNotMatch = async (
    modulePathObj: ModulePathObjModel,
    absoluteModulePath: string,
    isModulePathsChanged: boolean,
    fileDirPath: string,
  ): Promise<{ validModulePath: ModulePathObjModel | undefined; updatedModulePath: ModulePathObj; isModulePathsChangedResult: boolean }> => {
    const data = await fsPromise.readFile(absoluteModulePath, {
      encoding: 'utf8',
      flag: 'r',
    });
    const dataObj = JSON.parse(data);
    if (dataObj.project?.name === modulePathObj.name) {
      const relativePath = path.relative(fileDirPath, absoluteModulePath).replace(/\\/g, '/');
      return {
        validModulePath: { name: modulePathObj.name, path: relativePath },
        updatedModulePath: { name: modulePathObj.name, path: relativePath, valid: true },
        isModulePathsChangedResult: isModulePathsChanged,
      };
    }
    // open a dialog to ask if user wants to select new path / update module name for unmatched module
    const result = dialog.showMessageBoxSync(this._window, {
      title: `Module ${modulePathObj.name} does not match project name in file`,
      message: `Do you want to select a different path for module ${modulePathObj.name}?`,
      type: 'question',
      buttons: ['Yes', 'No'],
    });
    if (result === 0) {
      const paths = dialog.showOpenDialogSync(this._window, {
        title: 'Add a new module',
        filters: [
          { name: 'JACK Mod Model', extensions: ['mod.jack.json'] },
          { name: 'JACK Model', extensions: ['jack'] },
        ],
        properties: ['openFile'],
      });
      const seletedPath = paths ? paths[0] : undefined;
      if (seletedPath) {
        return this._handleModuleProjectNameNotMatch(modulePathObj, seletedPath, true, fileDirPath);
      }
    }
    return { validModulePath: undefined, updatedModulePath: { ...modulePathObj, valid: false }, isModulePathsChangedResult: false };
  };

  /**
   * select new path for missing module
   * @param module module object
   * @returns
   */
  private _selectPathForMissingModule = async (module: ModulePathObjModel) => {
    // open a dialog to ask if user wants to select new path for invalid module file
    const result = dialog.showMessageBoxSync(this._window, {
      title: `Can not find ${module.name} from the path`,
      message: `Module ${module.name} is missing, do you want to select a different path for module ${module.name}?`,
      type: 'question',
      buttons: ['Yes', 'No'],
    });
    if (result === 0) {
      const paths = dialog.showOpenDialogSync(this._window, {
        title: 'Add a new module',
        filters: [
          { name: 'JACK Mod Model', extensions: ['mod.jack.json'] },
          { name: 'JACK Model', extensions: ['jack'] },
        ],
        properties: ['openFile'],
      });
      const seletedPath = paths ? paths[0] : undefined;
      return seletedPath;
    }
    return undefined;
  };

  /**
   * find all valid module path dic
   * @param currentFilePath
   * @param projectModel
   * @returns
   */
  private _findAllValidModulePaths = async (currentFilePath: string, projectModel: CBDIEditorProjectModel) => {
    const validModulePathArr: ModulePathObjModel[] = [];
    let isModulePathsChanged = false;
    const updatedModulePathArr: ModulePathObj[] = [];
    // project file dir path
    const fileDirPath = path.dirname(currentFilePath);

    for (let i = 0; i < projectModel.modules.length; i++) {
      const modulePathObj = projectModel.modules[i];

      const absoluteModulePath = path.resolve(fileDirPath, modulePathObj.path);

      // if module path is relative to project root folder, and file exist
      if (fs.existsSync(absoluteModulePath)) {
        validModulePathArr.push(modulePathObj);
        updatedModulePathArr.push({ ...modulePathObj, valid: true });
      }
      // if cannot find module file with existing path, ask user to select a new path
      else {
        // eslint-disable-next-line no-await-in-loop
        const selectedPath = await this._selectPathForMissingModule(modulePathObj);
        // if select new path for missing module, update the path
        if (selectedPath) {
          const relativePath = path.relative(fileDirPath, selectedPath).replace(/\\/g, '/');
          validModulePathArr.push({ name: modulePathObj.name, path: relativePath });
          updatedModulePathArr.push({ name: modulePathObj.name, path: relativePath, valid: true });
          isModulePathsChanged = true;
        }
        // if do not select new path for missing module, use old path for module
        else {
          updatedModulePathArr.push({ ...modulePathObj, valid: false });
        }
      }
    }

    return { validModulePathArr, isModulePathsChanged, updatedModulePathArr };
  };

  /**
   * read module file with module path dic
   * @param currentFilePath
   * @param validModulePathArr
   * @returns
   */
  private _readModules = async (currentFilePath: string, validModulePathArr: ModulePathObjModel[]) => {
    const moduleProjects: CBDIEditorProject[] = [];
    // project file dir path
    const fileDirPath = path.dirname(currentFilePath);

    await Promise.all(
      validModulePathArr.map(async (modulePathObj) => {
        const modulePath = path.resolve(fileDirPath, modulePathObj.path);
        try {
          // check if metaData csv file exists in the same directory
          // if exists, read the meta data array
          // if does not exist, use empty array
          const desiredMetaDataFilePath = getMetaDataPath(modulePath);
          const flatMetaData = (() => {
            const hasMetaDataExist = fs.existsSync(desiredMetaDataFilePath);
            if (hasMetaDataExist) {
              return readMetaDataFromCsv(desiredMetaDataFilePath);
            }
            return undefined;
          })() as FlatMetaData | undefined;

          const data = await fsPromise.readFile(modulePath, {
            encoding: 'utf8',
            flag: 'r',
          });

          const moduleModel: CBDIEditorModuleModel = copy(JSON.parse(data.toString()));

          const moduleProject = convertModuleProjectFileToProject(modulePathObj.name, {
            moduleModel,
            flatMetaData,
          });
          moduleProjects.push(moduleProject);
          this._updateModuleFileWatcher(modulePathObj.name, modulePath);
        } catch (error) {
          console.error(`Failed to load module${modulePathObj}: ${error}`);
        }
      }),
    );

    return moduleProjects;
  };

  /* --------------------------------- PUBLIC --------------------------------- */

  /**
   * craete new project
   */
  public newProject = () => {
    const project: CBDIEditorProject = {
      name: 'NewProject',
      namespaces: [],
      jack_model_version: packageJson.cbdiVersion,
      modulePaths: [],
      moduleProjectInfoDic: {},
      cbdiObjects: {},
      enums: [],
      teams: [],
      agents: [],
      tactics: [],
      roles: [],
      goals: [],
      plans: [],
      resources: [],
      actions: [],
      messages: [],
      services: [],
      entities: [],
      events: [],
    };
    dialog
      .showSaveDialog(this._window, {
        filters: [{ name: 'JACK Project', extensions: ['prj.jack.json'] }],
      })
      .then((result) => {
        if (!result.canceled && result.filePath) {
          let resultFilePath = result.filePath;
          // if the return filePath has no extension (in Linux), it will add '.cbdi' as the default extension
          const hasExtension = /\.[^/\\]+$/.test(result.filePath);
          if (hasExtension === false) {
            const defaultExt = 'prj.jack.json';
            resultFilePath = `${result.filePath}.${defaultExt}`;
          }
          // update project fileName and project name
          const projectFileName = path.basename(resultFilePath).replace('.prj.jack.json', '');
          project.name = projectFileName;

          const projectModel = saveCBDIProjectFromProject(project);
          const formattedProjectModelStr = JSON.stringify(projectModel, null, 4);

          this.cbdiEditorProject = project;

          fs.writeFile(resultFilePath, formattedProjectModelStr, {}, () => {
            this._clearModuleFileWatchers();
            this._projectFilePath = resultFilePath;
            this._window.webContents.send(editorResponse.project.changed, project);
          });
        }
        return result;
      })
      .catch((error) => {
        console.error(error);
        dialog.showErrorBox('ERROR', error.stack || error.message);
      });
  };

  /**
   * read a cbdi project file
   * @param filename cbdi model file path
   */
  public readProject = async (filePath: string) => {
    try {
      const data = await fsPromise.readFile(filePath);
      this._projectFilePath = filePath;
      const model: CBDIEditorProjectModel = copy(JSON.parse(data.toString()));
      const { validModulePathArr, isModulePathsChanged, updatedModulePathArr } = await this._findAllValidModulePaths(filePath, model);
      try {
        const moduleProjects = await this._readModules(filePath, validModulePathArr);
        try {
          const project: CBDIEditorProject = {
            name: model.name,
            jack_model_version: model.jack_model_version,
            modulePaths: updatedModulePathArr,
            actions: [],
            agents: [],
            cbdiObjects: {},
            entities: [],
            enums: [],
            events: [],
            goals: [],
            messages: [],
            moduleProjectInfoDic: {},
            namespaces: [],
            plans: [],
            resources: [],
            roles: [],
            services: [],
            tactics: [],
            teams: [],
          };
          const newProject = mergeModuleProjectsToProject(project, moduleProjects);
          this.cbdiEditorProject = newProject;
          this._window.webContents.send(editorResponse.project.changed, newProject, isModulePathsChanged && project);
          // update recent open files
          const recentFiles = (await electronStore.get('editorRecentFiles', [])) as string[];
          const updatedRecentFiles = [filePath, ...recentFiles.filter((file) => file !== filePath)];
          electronStore.set('editorRecentFiles', updatedRecentFiles.slice(0, 10));

          return project;
        } catch (e) {
          this._handleSerialiseError(e, filePath, 'Project');
        }
      } catch (e) {
        // do nothing, the error has been handled by onReadModules
      }
    } catch (error: any) {
      console.error(`Failed to read file ${filePath}: ${error.message}`);
    }
    return undefined;
  };

  /**
   * open project
   */
  public openProject = async () => {
    try {
      const result = await dialog.showOpenDialog(this._window, {
        filters: [{ name: 'JACK Project', extensions: ['prj.jack.json'] }],
        properties: ['openFile'],
      });
      if (!result.canceled && result.filePaths.length) {
        const project = await this.readProject(result.filePaths[0]);
        return project;
      }
      return undefined;
    } catch (error: any) {
      console.error(error);
      dialog.showErrorBox('ERROR', error.stack || error.message);
      return undefined;
    }
  };

  /**
   * open project with path
   */
  public openProjectWithPath = async (filePath: string) => {
    // get current recent files
    const recentFiles = (await electronStore.get('editorRecentFiles', [])) as string[];
    // remove the opened filePath
    const newRecentFiles = recentFiles.filter((item) => item !== filePath);
    if (this._checkPathExists(filePath)) {
      // put opened filePath on the index 0
      newRecentFiles.unshift(filePath);
      electronStore.set('editorRecentFiles', newRecentFiles);
      await this.readProject(filePath);
    } else {
      electronStore.set('editorRecentFiles', newRecentFiles);
      dialog.showMessageBox(this._window, {
        title: 'Path does not exist',
        message: `The path '${filePath}' does not exist on this computer.`,
        type: 'info',
        buttons: ['OK'],
      });
    }
  };

  /**
   * import module
   * @param _event
   * @param currentProject
   */
  public importModule = async (currentProject: CBDIEditorProject) => {
    let newProject = { ...currentProject };
    // open a dialog to get the path of adding module
    const result = dialog.showOpenDialogSync(this._window, {
      title: 'Add a new module',
      filters: [
        { name: 'JACK Mod Model', extensions: ['mod.jack.json'] },
        { name: 'JACK Model', extensions: ['jack'] },
      ],
      properties: ['openFile'],
    });
    // project file dir path
    const fileDirPath = path.dirname(this._projectFilePath!);

    // if path exists
    if (result && result.length > 0) {
      const newFilePath = result[0];
      const data = fs.readFileSync(newFilePath, {
        encoding: 'utf8',
        flag: 'r',
      });
      const moduleModel: CBDIEditorModuleModel = copy(JSON.parse(data.toString()));
      // add/update the new model name and relative path into current project
      const index = currentProject.modulePaths.findIndex((value) => value.name === moduleModel.project.name);
      const relativePath = path.relative(fileDirPath, newFilePath).replace(/\\/g, '/');
      const moduleProjects = await this._readModules(this._projectFilePath!, [{ name: moduleModel.project.name, path: relativePath }]);
      if (index > -1) {
        this._updateModuleFileWatcher(moduleModel.project.name, null);
        newProject = removeModuleProjectFromProject(currentProject, moduleModel.project.name);
      }
      this._updateModuleFileWatcher(moduleModel.project.name, newFilePath);
      newProject = mergeModuleProjectsToProject(newProject, moduleProjects);
      newProject.modulePaths = [...newProject.modulePaths, { name: moduleModel.project.name, path: relativePath, valid: true }];
      this._window.webContents.send(editorResponse.project.changed, newProject, currentProject);
    }
  };

  /**
   * delete module
   * @param currentProject
   * @param moduleName
   */
  public removeModule = async (currentProject: CBDIEditorProject, moduleName: string) => {
    const index = currentProject.modulePaths.findIndex((module) => module.name === moduleName);
    if (index > -1) {
      const newProject = removeModuleProjectFromProject(currentProject, moduleName);
      this._updateModuleFileWatcher(moduleName, null);
      this._window.webContents.send(editorResponse.project.changed, newProject, currentProject);
    }
  };

  /**
   * rename module
   * @param currentProject
   * @param oldModuleName
   */
  public renameModule = async (currentProject: CBDIEditorProject, oldModuleName: string, newModuleName: string) => {
    const index = currentProject.modulePaths.findIndex((module) => module.name === oldModuleName);
    if (index > -1) {
      const newProject = renameModuleProject(currentProject, oldModuleName, newModuleName);
      const oldModuleFilePath = this._fileWatchers.find((el) => el.moduleName === oldModuleName)?.filePath;
      this._updateModuleFileWatcher(oldModuleName, null);
      if (oldModuleFilePath) {
        this._updateModuleFileWatcher(newModuleName, oldModuleFilePath);
      }
      this._window.webContents.send(editorResponse.project.changed, newProject, currentProject);
    }
  };

  /**
   * save project in new path
   * @param project
   * @returns
   */
  // public saveAsProject = async (project: CBDIEditorProject) => {
  //   const projectWithMetaDataModel = saveCBDIJsonFromProject(project);
  //   dialog
  //     .showSaveDialog(this._window, {
  //       filters: [
  //         { name: 'CBDI Model', extensions: ['cbdi'] },
  //         { name: 'JSON', extensions: ['json'] },
  //         { name: 'All Files', extensions: ['*'] },
  //       ],
  //     })
  //     .then((result) => {
  //       if (!result.canceled && result.filePath) {
  //         this._window.webContents.send(editorRequest.project.overwrite, true);

  //         // if the return filePath has no extension (in Linux), it will add '.cbdi' as the default extension
  //         // eslint-disable-next-line no-useless-escape
  //         const hasExtension = /\.[^\/\\]+$/.test(result.filePath);
  //         if (hasExtension === false) {
  //           const defaultExt = 'cbdi';
  //           result.filePath = `${result.filePath}.${defaultExt}`;
  //         }
  //         const jsonString = JSON.stringify(projectWithMetaDataModel.projectModel, null, 4);
  //         this.FileHash = crypto.createHash('md5').update(jsonString).digest('hex');

  //         this.cbdiEditorProject = project;

  //         saveMetaDataCsvFile(result.filePath!, projectWithMetaDataModel.flatMetaData);

  //         fs.writeFile(result.filePath!, jsonString, {}, async () => {
  //           await this._updateFilePath(result.filePath || null);
  //         });
  //       }
  //       return result;
  //     })
  //     .catch((error) => {
  //       console.error(error);
  //     });
  // };

  /**
   * get absolute path for module does not have file path
   */
  public _getNewModulePath = async (project: CBDIEditorProject, moduleName: string) => {
    const moduleObj = project.modulePaths.find((el) => el.name === moduleName);
    if (moduleObj) {
      // open a dialog to ask if user wants to save unsaved module project
      const result = dialog.showMessageBoxSync(this._window, {
        title: `${moduleName} unsaved`,
        message: `${moduleName} has not been saved yet, do you want to save ${moduleName}?`,
        type: 'question',
        buttons: ['Yes', 'No'],
      });
      if (result === 0) {
        const modulePathResult = await dialog.showSaveDialog({
          title: `Save ${moduleName}`,
          defaultPath: `${moduleName}.mod.jack.json`,
          filters: [
            { name: 'JACK Mod Model', extensions: ['mod.jack.json'] },
            { name: 'JACK Model', extensions: ['jack'] },
          ],
        });
        if (!modulePathResult.canceled && modulePathResult.filePath) {
          return modulePathResult.filePath;
        }
        return undefined;
      }
    }

    return undefined;
  };

  /**
   * update project
   * @param project
   */
  public updateProject = async (project: CBDIEditorProject) => {
    let newProject = { ...project };
    const dirPath = path.dirname(this._projectFilePath!);
    // unsubscribe file watchers which does not exists in project module paths
    const unusefulFileWatchers = this._fileWatchers.filter((el) => !project.modulePaths.some((mp) => mp.name === el.moduleName));
    unusefulFileWatchers.forEach((el) => {
      this._updateModuleFileWatcher(el.moduleName, null);
    });
    if (project.modulePaths.length > 0) {
      // loop each module path
      for (let i = 0; i < project.modulePaths.length; i++) {
        const module = project.modulePaths[i];
        let validModuleAbosultePath: string | any;
        if (module.path) {
          const moduleAbosultePath = path.resolve(dirPath, module.path);
          if (fs.existsSync(moduleAbosultePath)) {
            validModuleAbosultePath = moduleAbosultePath;
          }
        } else {
          validModuleAbosultePath = await this._getNewModulePath(project, module.name);
          if (validModuleAbosultePath) {
            const relativeModulePath = path.relative(dirPath, validModuleAbosultePath).replace(/\\/g, '/');
            newProject.modulePaths[i] = { name: module.name, path: relativeModulePath, valid: true };
          } else {
            newProject = removeModuleProjectFromProject(newProject, module.name);
          }
        }

        if (validModuleAbosultePath) {
          const projectWithMetaDataModel = saveCBDIJsonFromProject(project, module.name);
          const moduleProjectString = JSON.stringify(projectWithMetaDataModel.moduleModel, null, 4);

          saveMetaDataCsvFile(validModuleAbosultePath, projectWithMetaDataModel.flatMetaData);

          this._updateModuleFileWatcher(module.name, null);
          fs.writeFile(validModuleAbosultePath, moduleProjectString, {}, () => {
            this._updateModuleFileWatcher(module.name, validModuleAbosultePath);
            console.log(`updated module ${module.name} to ${validModuleAbosultePath}`);
          });
        }

        if (i === project.modulePaths.length - 1) {
          const projectModel = saveCBDIProjectFromProject(newProject);
          const jsonString = JSON.stringify(projectModel, null, 4);
          fs.writeFile(this._projectFilePath!, jsonString, {}, () => {
            console.log(`updated file to ${this._projectFilePath}`);
          });
          this._window.webContents.send(editorResponse.project.changed, newProject);

          this.cbdiEditorProject = newProject;
        }
      }
    } else {
      const projectModel = saveCBDIProjectFromProject(newProject);
      const jsonString = JSON.stringify(projectModel, null, 4);
      fs.writeFile(this._projectFilePath!, jsonString, {}, () => {
        console.log(`updated file to ${this._projectFilePath}`);
      });
      this._window.webContents.send(editorResponse.project.changed, newProject);

      this.cbdiEditorProject = newProject;
    }
  };

  /**
   * close project
   * @param hasDialog if has dialog, asking if user want to save changes. Otherwise just close it
   */
  public closeProject = (hasDialog: boolean) => {
    if (hasDialog) {
      dialog
        .showMessageBox(this._window, {
          title: 'CBDI Editor',
          message: 'Do you want to save changes?',
          type: 'question',
          buttons: ['Yes', 'No', 'Cancel'],
        })
        .then((result) => {
          switch (result.response) {
            case 0:
              this._window.webContents.send(editorResponse.project.exitSaving, true);

              break;
            case 1:
              this._window.webContents.send(editorResponse.project.exitSaving, false);

              break;
            case 2:
              break;
            default:
              break;
          }
          return result;
        })
        .catch((e) => {
          console.log(e);
          dialog.showErrorBox('ERROR', e.stack || e.message);
        });
    } else {
      this._window.webContents.send(editorResponse.project.exitSaving, false);
    }
  };

  /**
   * generate plan graph and save as png
   * @param dataUrl
   * @param selectedPlanName
   * @returns
   */

  public generatePlan = async (dataUrl: string, selectedPlanName: string) => {
    const result = await dialog.showSaveDialog({
      defaultPath: `${selectedPlanName}.png`,
      filters: [{ name: 'PNG Files', extensions: ['png'] }],
    });

    if (!result.canceled && result.filePath) {
      const imageBuffer = Buffer.from(dataUrl.replace(/^data:image\/\w+;base64,/, ''), 'base64');
      await fs.writeFile(result.filePath, imageBuffer, (err: NodeJS.ErrnoException | null) => {
        if (err) {
          console.error('Error saving image:', err);
        } else {
          console.log('Image saved successfully!');
        }
      });
      return 'Image saved successfully!';
    }
    return 'Image save canceled.';
  };

  public clearFileWatcher = () => {
    this.cbdiEditorProject = null;
    this._clearModuleFileWatchers();
    this._fileWatchers = [];
  };
}

export default CBDIEditProjectModelBuilder;
