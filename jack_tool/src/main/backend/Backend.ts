import { ipcMain, IpcMainInvokeEvent } from 'electron';
import BaseListener from 'listeners/base/BaseListener';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import { request, response } from 'misc/events/common/cmEvents';
import * as pkg from 'package.json';
import electronStore from 'main/electronStore/ElectronStore';

interface Props {
  window: Electron.BrowserWindow;
}

export default class Backend {
  private _window: Electron.BrowserWindow;

  private _appListener: BaseListener | null;

  private _client: string;

  constructor(props: Props) {
    this._window = props.window;

    this._client = 'cbdi-geo';

    this._appListener = null;
  }

  private _saveInfo = () => {
    this._window.webContents.send(request.project.saveInfo);
  };

  start() {
    this.deregisterEventHandlers();
    this.registerEventHandlers();
  }

  close() {
    this.deregisterEventHandlers();
    this._appListener = null;
  }

  /**
   * lazy import the listener depends on client
   * @param client client name
   */
  startClientListener() {
    import('listeners/cbdiEdit/CbdiEditListener')
      .then((editor) => {
        const CbdiEditListener = editor.default;

        this._appListener = new CbdiEditListener({
          window: this._window,
          appVersion: pkg.version,
          cbdiVersion: pkg.cbdiVersion,
          clientVersion: pkg.version,
        }).start();

        return this._window.setTitle('JACK Editor');
      })
      .catch((e: Error) => {
        LOGGER.error(e.stack ? e.stack : e);
      });
  }

  /**
   * Close window
   */
  private onCloseWindow = (_event: IpcMainInvokeEvent, ...args: any[]) => {
    // if client is editor, use cbdiEditListener to handle close window event
    if (this._client === 'editor') {
      const hasDialog = args[0];
      if (hasDialog !== undefined && typeof hasDialog === 'boolean') (this._appListener as any).onCloseWindow(...args);
    } else if (this._window.isClosable()) {
      this._window.close();
    }
  };

  /**
   * Maximise window
   */
  private onMaximizeWindow = () => {
    if (this._window.isMaximized()) {
      this._window.unmaximize();
    } else if (this._window.isMaximizable()) {
      this._window.maximize();
    }
  };

  /**
   * Minimise window
   */
  private onMinimizeWindow = () => {
    if (this._window.isMinimizable()) {
      this._window.minimize();
    }
  };

  /**
   * Callback when window maximised changed
   */
  private onWindowMaximizedChanged = () => {
    this._window.webContents.send(response.window.maximized, this._window.isMaximized());
  };

  /**
   * Callback when window resized
   */
  private onResizeWindow = () => {
    this._window.webContents.send(response.window.resized);
  };

  /**
   * Callback when user click about
   */
  private onAboutProject = () => {
    this._appListener?.showAbout(this._client);
  };

  /**
   * Takes in Electron event, an API string and set new link into Electron store then reload the application
   * @param _event Electron.IpcMainInvokeEvent
   * @param api API string
   */
  private onChangeAPISatellite = async (_event: Electron.IpcMainInvokeEvent, api: string) => {
    await electronStore.set('satelliteMapAPI', api);
    this._window.reload();
  };

  /**
   * Takes in Electron event, an API string and set new link into Electron store then reload the application
   * @param _event Electron.IpcMainInvokeEvent
   * @param api API string
   */
  private onChangeAPIStreetview = async (_event: Electron.IpcMainInvokeEvent, api: string) => {
    await electronStore.set('streetViewMapAPI', api);
    this._window.reload();
  };

  private onLandingStartApp = () => {
    this.startClientListener();
  };

  /**
   * Register event handlers with callbacks
   */
  private registerEventHandlers() {
    ipcMain.handle(request.window.minimize, this.onMinimizeWindow);
    ipcMain.handle(request.window.maximize, this.onMaximizeWindow);
    ipcMain.handle(request.window.close, this.onCloseWindow);
    ipcMain.handle(request.project.about, this.onAboutProject);
    ipcMain.handle(request.project.satellite, this.onChangeAPISatellite);
    ipcMain.handle(request.project.streetView, this.onChangeAPIStreetview);
    ipcMain.handle(request.startApp, this.onLandingStartApp);

    this._window.on('maximize', this.onWindowMaximizedChanged);
    this._window.on('unmaximize', this.onWindowMaximizedChanged);
    this._window.on('restore', this.onWindowMaximizedChanged);
    this._window.on('resized', this.onResizeWindow);
    this._window.on('close', this._saveInfo);
  }

  private deregisterEventHandlers() {
    ipcMain.removeHandler(request.window.minimize);
    ipcMain.removeHandler(request.window.maximize);
    ipcMain.removeHandler(request.window.close);
    ipcMain.removeHandler(request.project.about);
    ipcMain.removeHandler(request.project.satellite);
    ipcMain.removeHandler(request.project.streetView);
    ipcMain.removeHandler(request.startApp);

    this._window.off('maximize', this.onWindowMaximizedChanged);
    this._window.off('unmaximize', this.onWindowMaximizedChanged);
    this._window.off('restore', this.onWindowMaximizedChanged);
    this._window.off('resized', this.onResizeWindow);
    this._window.off('close', this._saveInfo);
  }
}
