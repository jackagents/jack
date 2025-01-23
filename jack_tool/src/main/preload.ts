import { contextBridge, ipcRenderer, shell } from 'electron';
import * as util from 'main/util';

process.once('loaded', () => {
  contextBridge.exposeInMainWorld('ipcRenderer', {
    ...ipcRenderer,
    on: (channel: string, cb: any) => {
      ipcRenderer.on(channel, cb);

      return () => {
        ipcRenderer.removeListener(channel, cb);
      };
    },
    off: (channel: string, cb: any) => {
      ipcRenderer.off(channel, cb);
    },
    send: (channel: string, ...arg: any[]) => {
      ipcRenderer.invoke(channel, ...arg);
    },
    invoke: (channel: string, ...arg: any[]) => {
      ipcRenderer.invoke(channel, ...arg);
    },
    removeListener: (channel: string, listener: any) => {
      ipcRenderer.removeListener(channel, listener);
    },
    removeAllListeners: (channel: string) => {
      ipcRenderer.removeAllListeners(channel);
    },
    setupIpcListener: (channel: string, cb: any) => {
      ipcRenderer.on(channel, cb);

      return () => {
        ipcRenderer.removeListener(channel, cb);
      };
    },
  });

  contextBridge.exposeInMainWorld('electronStore', {
    get(key: string, defaultValue?: any) {
      return ipcRenderer.sendSync('electron-store-get', key, defaultValue);
    },
    set(property: string, val: any) {
      ipcRenderer.send('electron-store-set', property, val);
    },
    // Other method you want to add like has(), reset(), etc.
  });

  contextBridge.exposeInMainWorld('cbdiDocs', {
    shell,
    dirName: process.cwd(),
  });

  contextBridge.exposeInMainWorld('mainUtils', {
    getCurrentDateStr: () => {
      return util.getCurrentDateStr();
    },
  });

  contextBridge.exposeInMainWorld('platform', process.platform);
});
