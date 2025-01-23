import { ipcMain } from 'electron';
import Store from 'electron-store';

class ElectronStore {
  store = new Store();

  constructor() {
    this.start();
  }

  /**
   * Start listening to electron-store-get and electron-store-set events
   */
  start() {
    // IPC listener
    ipcMain.on('electron-store-get', async (event, val, defaultValue?) => {
      event.returnValue = this.store.get(val, defaultValue);
    });

    ipcMain.on('electron-store-set', async (_event, key, val) => {
      this.store.set(key, val);
    });
  }

  /**
   * Stop listening to electron-store-get and electron-store-set events
   */
  stop() {
    // IPC listener
    ipcMain.off('electron-store-get', async (event, val, defaultValue?) => {
      event.returnValue = this.store.get(val, defaultValue);
    });

    ipcMain.off('electron-store-set', async (_event, key, val) => {
      this.store.set(key, val);
    });
  }

  /**
   * Set value to key
   * @param key string
   * @param val any
   * @returns Electron Store
   */
  set = async (key: string, val: any) => {
    this.store.set(key, val);
    return this.store;
  };

  /**
   * Get value of key, return optional default value if does not have value
   * @param key string
   * @param defaultValue (optional) any
   * @returns value
   */
  get = async (key: string, defaultValue?: any) => {
    return this.store.get(key, defaultValue);
  };
}

/**
 * Singleton electron store
 */
const electronStore = new ElectronStore();
export default electronStore;
