import MenuAction from 'types/common/cmTypes';
import { IModelError, CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';
import { request } from 'projectEvents/cbdiEdit/editEvents';
import { MODE_PROJECT } from 'constant/cbdiEdit/cbdiEditConstant';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';

interface Props {
  current: CBDIEditorProject | null;
  saved: CBDIEditorProject | null;
  mode: string;
  createModule: () => void;
  saveProject: () => void;
  saveAsProject: () => void;
  openProject: () => void;
  setProjectError: (error: IModelError[]) => void;
  setDebugConsoleIsVisible: (isVisible: boolean) => void;
  setProject: (project: CBDIEditorProject | null) => void;
  setIsShowingShortcutInfo: () => void;
}

export const getCbdiEditMenuTemplate = (props: Props) => {
  const { current, saved, mode, createModule, saveProject, openProject, setIsShowingShortcutInfo, saveAsProject } = props;

  const CbdiEditMenuActions: MenuAction = {
    submenu: [
      {
        label: 'File',
        disabled: mode !== MODE_PROJECT,
        submenu: [
          {
            label: 'New',
            onClick: () => {
              window.ipcRenderer.invoke(request.project.new);
            },
          },
          {
            label: 'Open',
            onClick: () => {
              openProject();
            },
          },
          {
            label: 'Save',
            onClick: () => {
              if (current !== null && saved) {
                saveProject();
                // init()
                //   .then(() => {
                //     const result: IModelError[] = JSON.parse(validate(JSON.stringify(saveCBDIJsonFromProject(saved), null, 2)));
                //     setProjectError(result);
                //     if (result.length > 0) {
                //       setDebugConsoleIsVisible(true);
                //     }
                //     return result;
                //   })
                //   .catch((e) => {
                //     console.log(e);
                //   });
              }
            },
          },
          {
            label: 'Save As',
            onClick: () => {
              if (current !== null) {
                saveAsProject();
              }
            },
          },

          {
            label: 'Close Project',
            onClick: () => {
              let hasDialog;
              if ((saved && current) === null || areObjectsEqual(saved, current)) {
                hasDialog = false;
              } else {
                hasDialog = true;
              }
              window.ipcRenderer.invoke(request.project.close, hasDialog);
            },
          },
        ],
      },

      {
        label: 'Edit',
        submenu: [
          {
            label: 'Create a module',
            onClick: () => {
              createModule();
            },
          },
          {
            label: 'Import a module',
            onClick: () => {
              if (window.mainWindowId) {
                window.ipcRenderer.invoke(request.project.importModule, current);
              }
            },
          },
        ],
      },
      {
        label: 'Help',
        submenu: [
          {
            label: 'Keyboard Shortcuts',
            onClick: () => {
              setIsShowingShortcutInfo();
            },
          },
        ],
      },
    ],
  };

  return CbdiEditMenuActions;
};
