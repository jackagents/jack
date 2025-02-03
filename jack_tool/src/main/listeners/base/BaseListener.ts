// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import { dialog } from 'electron';
import { response } from 'projectEvents/common/cmEvents';

export interface TListenerProps {
  /**
   * Browser window
   */
  window: Electron.BrowserWindow;
  /**
   * Application version (iba/iwd/tewa)
   */
  appVersion: string;
  /**
   * CBDI-Geo version
   */
  clientVersion: string;
  /**
   * CBDI version
   */
  cbdiVersion: string;
}

/**
 * Abstract Class BaseListener
 * @class BaseListener
 */
export default abstract class BaseListener {
  protected _window: Electron.BrowserWindow;

  protected appVersion: string;

  protected clientVersion: string;

  protected cbdiVersion: string;

  /* -------------------------------------------------------------------------- */
  /*                                 CONSTRUCTOR                                */
  /* -------------------------------------------------------------------------- */

  constructor(props: TListenerProps) {
    this._window = props.window;
    this.appVersion = props.appVersion;
    this.cbdiVersion = props.cbdiVersion;
    this.clientVersion = props.clientVersion;
    // when do yarn start
    // window ready to show event is fired before listener is constructed
    // need to send front end the main window id after when listener is constructed
    this.setMainWindowId();
  }

  /* -------------------------------------------------------------------------- */
  /*                                  ABSTRACT                                  */
  /* -------------------------------------------------------------------------- */

  protected abstract onWindowClose(): void;

  protected abstract onWindowReadyToShow(): void;

  /* -------------------------------- protected ------------------------------- */
  /**
   * start listener
   */
  // eslint-disable-next-line class-methods-use-this
  protected start() {}

  protected async openFileDialogSync(
    options: Electron.OpenDialogSyncOptions,
    onSucess?: (filePaths: string[]) => void,
    onCancel?: () => void,
  ) {
    const filePaths = dialog.showOpenDialogSync(this._window, options);

    if (filePaths && onSucess) {
      onSucess(filePaths);
    }
    // Cancelled
    else if (onCancel) {
      onCancel();
    }
  }

  protected setMainWindowId() {
    this._window.webContents.send(
      response.window.setMainWindowId,
      this._window.webContents.id,
    );
  }

  /**
   * Register event handlers
   */
  protected registerEventHandlers() {
    /* ----------------- This is to close connection on refresh ----------------- */
    this._window.on('close', this.onWindowClose.bind(this));
    this._window.on('ready-to-show', this.onWindowReadyToShow.bind(this));
  }

  /**
   * Deregister event handlers
   */
  protected deregisterEventHandlers() {
    /* ----------------- This is to close connection on refresh ----------------- */
    this._window.off('close', this.onWindowClose.bind(this));
    this._window.off('ready-to-show', this.onWindowReadyToShow.bind(this));
  }

  /* -------------------------------------------------------------------------- */
  /*                                   PUBLIC                                   */
  /* -------------------------------------------------------------------------- */

  /**
   * Show about popup
   * @param clientName name of current client
   */
  showAbout(clientName: string) {
    dialog.showMessageBox(this._window, {
      title: 'About',
      message: `client: ${this.clientVersion} \n\rcbdi: ${this.cbdiVersion} \n\r${clientName}: ${this.appVersion}`,
    });
  }
}
