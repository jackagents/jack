/* eslint-disable class-methods-use-this */
import { dialog, ipcMain, IpcMainInvokeEvent } from 'electron';
import { request as editorRequest, response as editorResponse } from 'projectEvents/cbdiEdit/editEvents';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import WebSocketObj from 'main/websocketClient/WebSocketObj';
import MODEL_BUILDER from 'main/beBuilders/cbdi/cbdiModelBuilder/cbdiModelBuilderNonFlatBuffer';
import LOG_BUILDER from 'main/beBuilders/cbdi/logBuilder/cbdiLogBuilderNonFlatBuffer';
import INTENTION_BUILDER from 'main/beBuilders/cbdi/intentionBuilder/cbdiIntentionBuilderNonFlatBuffer';
import { response } from 'projectEvents/common/cmEvents';
import LOGGER from 'addons/logger/LoggerSingleton';
import electronStore from 'main/electronStore/ElectronStore';
import CBDIEditProjectModelBuilder from 'main/beBuilders/cbdi/cbdiEditProjectModelBuilder/cbdiEditProjectModelBuilder';
import { TListenerProps } from '../base/BaseListener';
import CbdiListener from '../cbdi/CbdiListener';
import { PLAN_INSPECTOR } from '../cbdi/planInspector/PlanInspector';

export default class CbdiEditListener extends CbdiListener {
  /* ********************************************************************************************
   * Properties
   * ******************************************************************************************* */

  recording: Event[] = [];

  CBDIEditProjectModelBuilder: CBDIEditProjectModelBuilder;

  /* ********************************************************************************************
   * Constructor
   * ******************************************************************************************* */
  constructor(props: TListenerProps) {
    super(props);
    this.CBDIEditProjectModelBuilder = new CBDIEditProjectModelBuilder(this._window);
    this.websocketObj = new WebSocketObj({
      window: this._window,
      address: 'ws://127.0.0.1:1313',
      builders: [MODEL_BUILDER, LOG_BUILDER, INTENTION_BUILDER],
      onConnected: () => {
        this._window.webContents.send(response.websocket.connected);
        this._window.webContents.send(response.window.setAppLoading, false);
      },
      onDisconneted: () => {
        this._window.webContents.send(response.websocket.disconnected);
        this._window.webContents.send(response.window.setAppLoading, false);
      },
      onMessage: (data) => {
        // On message received
      },
    });
  }

  cleanup() {
    LOGGER.info('cbdi-editor listener runs cleanup');
    this.CBDIEditProjectModelBuilder.clearFileWatcher();
    PLAN_INSPECTOR.stopInspectPlan();
    this.websocketObj?.cleanup();
  }

  override showAbout(clientName: string) {
    super.showAbout(clientName);
  }

  /**
   * start listener
   */
  override start() {
    super.start();
    this.deregisterEventHandlers();
    this.registerEventHandlers();

    // only for editor
    this._window.webContents.on('did-start-loading', this.cleanup.bind(this));
    return this;
  }

  /* ********************************************************************************************
   * Callbacks
   * ******************************************************************************************* */

  onCloseWindow = (hasDialog: boolean) => {
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
              if (this._window.isClosable()) {
                this._window.close();
              }
              break;
            case 1:
              if (this._window.isClosable()) {
                this._window.close();
              }
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
    } else if (this._window.isClosable()) {
      this._window.close();
    }
  };

  private onSetTheme = (_event: IpcMainInvokeEvent, theme: 'dark' | 'light') => {
    electronStore.set('editorTheme', theme);
    this._window.webContents.send(editorResponse.theme.themeChanged, theme);
  };

  private onGetTheme = () => {
    const currentTheme = electronStore.get('editorTheme', 'light');
    return currentTheme;
  };

  private onNewProject = () => {
    this.CBDIEditProjectModelBuilder.newProject();
  };

  private onOpenProject = async () => {
    await this.CBDIEditProjectModelBuilder.openProject();
  };

  private onOpenProjectWithPath = async (_event: IpcMainInvokeEvent, filePath: string) => {
    await this.CBDIEditProjectModelBuilder.openProjectWithPath(filePath);
  };

  private onImportModule = async (_event: IpcMainInvokeEvent, currentProject: CBDIEditorProject) => {
    await this.CBDIEditProjectModelBuilder.importModule(currentProject);
  };

  private onRemoveModule = async (_event: IpcMainInvokeEvent, currentProject: CBDIEditorProject, moduleName: string) => {
    await this.CBDIEditProjectModelBuilder.removeModule(currentProject, moduleName);
  };

  private onRenameModule = async (_event: IpcMainInvokeEvent, currentProject: CBDIEditorProject, oldModuleName: string, newModuleName: string) => {
    await this.CBDIEditProjectModelBuilder.renameModule(currentProject, oldModuleName, newModuleName);
  };

  private onSaveAsProject = async (_event: IpcMainInvokeEvent, ...args: any[]) => {
    if (args.length === 1) {
      const project = args[0];
      if (!project) {
        console.error('Project cannot be null or undefined on update project');
        return;
      }
      try {
        // await this.CBDIEditProjectModelBuilder.saveAsProject(project);
        // TODO
        // comment save as function
        // because relative path need to be updated
        this._window.webContents.send(editorResponse.project.saveSuccess);
      } catch (e: any) {
        dialog.showErrorBox('ERROR', e.stack || e.message);
      }
    }
  };

  private onUpdateProject = async (_event: IpcMainInvokeEvent, ...args: any[]) => {
    if (args.length === 1) {
      const project = args[0];
      if (!project) {
        console.error('Project cannot be null or undefined on update project');
        return;
      }
      try {
        await this.CBDIEditProjectModelBuilder.updateProject(project);
        this._window.webContents.send(editorResponse.project.saveSuccess);
      } catch (e: any) {
        dialog.showErrorBox('ERROR', e.stack || e.message);
      }
    }
  };

  private onCloseProject = (_event: IpcMainInvokeEvent, ...args: any[]) => {
    if (args.length === 1) {
      const hasDialog = args[0];
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
    }
  };

  private onClearFileWatcher = () => {
    this.CBDIEditProjectModelBuilder.clearFileWatcher();
  };

  private onGeneratePlan = async (_event: IpcMainInvokeEvent, dataUrl: string, selectedPlanName: string) => {
    await this.CBDIEditProjectModelBuilder.generatePlan(dataUrl, selectedPlanName);
  };

  /**
   * Register event handlers with callbacks
   */
  registerEventHandlers() {
    super.registerEventHandlers();
    /* ------------------------------ editor theme ------------------------------ */
    ipcMain.handle(editorRequest.theme.setTheme, this.onSetTheme);
    ipcMain.handle(editorRequest.theme.getTheme, this.onGetTheme);
    /* --------------------------------- editor --------------------------------- */
    ipcMain.handle(editorRequest.project.new, this.onNewProject);
    ipcMain.handle(editorRequest.project.open, this.onOpenProject);
    ipcMain.handle(editorRequest.project.openWithPath, this.onOpenProjectWithPath);
    ipcMain.handle(editorRequest.project.update, this.onUpdateProject);
    ipcMain.handle(editorRequest.project.saveAs, this.onSaveAsProject);
    ipcMain.handle(editorRequest.project.close, this.onCloseProject);
    ipcMain.handle(editorRequest.project.clearFileWatcher, this.onClearFileWatcher);
    ipcMain.handle(editorRequest.project.importModule, this.onImportModule);
    ipcMain.handle(editorRequest.project.removeModule, this.onRemoveModule);
    ipcMain.handle(editorRequest.project.renameModule, this.onRenameModule);
    ipcMain.handle(editorRequest.graph.generatePlan, this.onGeneratePlan);
  }

  /**
   * Deregister event handlers with callbacks
   */
  deregisterEventHandlers = () => {
    super.deregisterEventHandlers();
    this._window.webContents.removeAllListeners('did-start-loading');

    /* ------------------------------ editor theme ------------------------------ */
    ipcMain.removeHandler(editorRequest.theme.setTheme);
    ipcMain.removeHandler(editorRequest.theme.getTheme);
    /* --------------------------------- editor --------------------------------- */
    ipcMain.removeHandler(editorRequest.project.new);
    ipcMain.removeHandler(editorRequest.project.open);
    ipcMain.removeHandler(editorRequest.project.openWithPath);
    ipcMain.removeHandler(editorRequest.project.update);
    ipcMain.removeHandler(editorRequest.project.saveAs);
    ipcMain.removeHandler(editorRequest.project.close);
    ipcMain.removeHandler(editorRequest.project.clearFileWatcher);
    ipcMain.removeHandler(editorRequest.project.importModule);
    ipcMain.removeHandler(editorRequest.project.removeModule);
    ipcMain.removeHandler(editorRequest.project.renameModule);
    ipcMain.removeHandler(editorRequest.graph.generatePlan);
  };
}
