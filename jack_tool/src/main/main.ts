// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/* eslint global-require: off, no-console: off, promise/always-return: off */

/**
 * This module executes inside of electron's main process. You can start
 * electron renderer process from here and communicate with the other processes
 * through IPC.
 *
 * When running `npm run build` or `npm run build:main`, this file is compiled to
 * `./src/main.js` using webpack. This gives us some performance wins.
 */
import path from 'path';
import { app, BrowserWindow, session } from 'electron';
// import { autoUpdater } from 'electron-updater';
// import log from 'electron-log';
import { resolveHtmlPath } from 'main/util';
import Backend from 'main/backend/Backend';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import installExtension, { REDUX_DEVTOOLS } from 'electron-devtools-installer';

// class AppUpdater {
//   constructor() {
//     log.transports.file.level = 'info';
//     autoUpdater.logger = log;
//     autoUpdater.checkForUpdatesAndNotify();
//   }
// }

const isWin32 = process.platform === 'win32';

let mainWindow: BrowserWindow | null = null;

/**
 * Local react dev tools for manifest 2
 * TODO: Remove this when electron upgraded to use manifest 3
 */
const reactDevToolsPath = path.join(__dirname, '..', '..', 'ReactDevTools-manifestv2');

const RESOURCES_PATH = app.isPackaged ? path.join(process.resourcesPath, 'assets') : path.join(__dirname, '../../assets/common');

const getAssetPath = (...paths: string[]): string => path.join(RESOURCES_PATH, ...paths);

const createBrowserWindow = (webviewTag = false) =>
  new BrowserWindow({
    show: false,
    width: 1024,
    minWidth: 1024,
    height: 728,
    minHeight: 600,
    titleBarStyle: isWin32 ? 'default' : 'hidden',
    fullscreen: false,
    icon: getAssetPath('icon.png'),
    webPreferences: {
      sandbox: false,
      preload: app.isPackaged ? path.join(__dirname, 'preload.js') : path.join(__dirname, '../../.erb/dll/preload.js'),
      nodeIntegration: true,
      webSecurity: true,
      webviewTag,
    },
  });

if (process.env.NODE_ENV === 'production') {
  const sourceMapSupport = require('source-map-support');
  sourceMapSupport.install();
}

const isDebug = process.env.NODE_ENV === 'development' || process.env.DEBUG_PROD === 'true';

if (isDebug) {
  require('electron-debug')();
}
LOGGER.info(process.version);

const createWindow = async () => {
  if (isDebug) {
    if (reactDevToolsPath) {
      // Using manifest 2 React Developer tool because Electron 22 has not yet supported manifest version 3
      await session.defaultSession
        .loadExtension(reactDevToolsPath)
        .then((ext) => {
          LOGGER.info(`Added Extension:  ${ext.name}`);
        })
        .catch((err) => LOGGER.error('An error occurred: ', err));
    }

    // Install Redux Devtools
    await installExtension(REDUX_DEVTOOLS)
      .then((name) => LOGGER.info(`Added Extension:  ${name}`))
      .catch((err) => LOGGER.error('An error occurred: ', err));
  }

  mainWindow = createBrowserWindow();

  // remove default electron menu
  mainWindow.setMenu(null);

  // Init Backend
  let backend: Backend | null = new Backend({ window: mainWindow });
  backend.start();

  mainWindow.loadURL(resolveHtmlPath('index.html'));

  mainWindow.on('ready-to-show', () => {
    if (!mainWindow) {
      throw new Error('"mainWindow" is not defined');
    }
    if (process.env.START_MINIMIZED) {
      mainWindow.minimize();
    } else {
      mainWindow.show();
    }

    // Open Dev Tools (F12) on development env
    if (mainWindow && process.env.NODE_ENV === 'development') {
      mainWindow.webContents.openDevTools();
    }
  });

  mainWindow.on('closed', () => {
    mainWindow = null;

    if (backend) {
      backend.close();
      backend = null;
    }
  });
  // Handle window resize event
  mainWindow.on('resize', () => {
    const [width, height] = mainWindow!.getSize();
    const minWidth = 1024;
    const minHeight = 600;
    // Enforce the minimum width
    if (width < minWidth) {
      mainWindow!.setSize(minWidth, height);
    }
    // Enforce the minimum height
    if (height < minHeight) {
      mainWindow!.setSize(width, minHeight);
    }
  });

  // const menuBuilder = new MenuBuilder(mainWindow);
  // menuBuilder.buildMenu();

  // attach fullscreen(f11 and not 'maximized') && focus listeners
  // attachTitlebarToWindow(mainWindow);

  // Open urls in the user's browser
  // mainWindow.webContents.setWindowOpenHandler((edata) => {
  //   shell.openExternal(edata.url);
  //   return { action: 'deny' };
  // });

  // Remove this if your app does not use auto updates
  // eslint-disable-next-line
  // new AppUpdater();
};

/**
 * Add event listeners...
 */

app.on('window-all-closed', () => {
  // Respect the OSX convention of having the application in memory even
  // after all windows have been closed
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app
  .whenReady()
  .then(() => {
    createWindow();
    app.on('activate', () => {
      // On macOS it's common to re-create a window in the app when the
      // dock icon is clicked and there are no other windows open.
      if (mainWindow === null) createWindow();
    });
  })
  .catch(LOGGER.error);
