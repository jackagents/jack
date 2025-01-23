import { IpcRenderer } from 'electron';

declare global {
  interface Window {
    /**
     * Current platform that application is running on
     */
    platform: string;
    /**
     * iwd/tewa...
     */
    client: string;
    /**
     * IpcRenderer process.
     */
    ipcRenderer: IpcRenderer & {
      setupIpcListener: (channel: string, cb: any) => () => void;
    };

    /**
     * varibles for open cbdi docs webpage
     */
    cbdiDocs: {
      shell: Electron.Shell;
      dirName: string;
    };

    /**
     * Electron store process.
     */
    electronStore: {
      /**
       * Get value from store, if not exist use defaultValue.
       */
      get: (key: string, defaultValue?: any) => any;

      /**
       * Set value to store using key string.
       */
      set: (key: string, val: any) => void;
    };

    /**
     * Main utility functions
     */
    mainUtils: {
      getCurrentDateStr: () => string;
    };

    mainWindowId: number | null;
  }
}
