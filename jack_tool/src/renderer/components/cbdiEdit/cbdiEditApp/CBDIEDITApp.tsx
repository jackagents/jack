import { Provider } from 'react-redux';
import { store } from 'projectRedux/Store';
import CbdiFrontEnd from 'components/cbdiEdit/frontend/CbdiFrontEnd';
import React from 'react';
import { response } from 'misc/events/common/cmEvents';
import { BaseBackdrop } from 'components/common/baseBackdrop/BaseBackdrop';

interface LoadingPage {
  isLoading: boolean;
  closable: boolean;
  closeBtnName?: string;
}

export default function CbdiEditApp() {
  const [loading, setLoading] = React.useState<LoadingPage>({
    isLoading: false,
    closable: false,
  });
  const [percentage, setPercentage] = React.useState<number>();

  const onSetMainWindowId = (evt: Electron.IpcRendererEvent, mainWindowId: number) => {
    window.mainWindowId = mainWindowId;
  };

  const onSetAppLoading = (evt: Electron.IpcRendererEvent, appLoading: boolean, closable = false, closeBtnName?: string) => {
    setLoading({ isLoading: appLoading, closable, closeBtnName });

    if (!appLoading) {
      setPercentage(undefined);
    }
  };

  const onSetPercentage = (evt: Electron.IpcRendererEvent, pc: number) => {
    setPercentage(pc);
  };

  React.useEffect(() => {
    const setMainWindowIdListenerCleanup = window.ipcRenderer.setupIpcListener(response.window.setMainWindowId, onSetMainWindowId);

    const setAppLoadingListenerCleanup = window.ipcRenderer.setupIpcListener(response.window.setAppLoading, onSetAppLoading);

    const setAppLoadingPercentageCleanup = window.ipcRenderer.setupIpcListener(response.window.setAppLoadingPercentage, onSetPercentage);

    return () => {
      setMainWindowIdListenerCleanup();
      setAppLoadingListenerCleanup();
      setAppLoadingPercentageCleanup();
    };
  }, []);

  return (
    <Provider store={store}>
      <CbdiFrontEnd />
      <BaseBackdrop
        key="base-loading"
        loading={loading.isLoading}
        closable={loading.closable}
        closeBtnName={loading.closeBtnName}
        percentage={percentage}
      />
    </Provider>
  );
}
