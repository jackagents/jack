/* eslint-disable @typescript-eslint/no-use-before-define */
import React from 'react';
import 'allotment/dist/style.css';
import TitleMenu from 'components/common/titleMenu/TitleMenu';
import { ThemeProvider, styled } from '@mui/material';
import { IpcRendererEvent } from 'electron';
import { Fluid, Row } from 'components/common/base/BaseContainer';
import { useDispatch, useSelector } from 'react-redux';
import { editorRendererToRendererEvents, request, response } from 'projectEvents/cbdiEdit/editEvents';
import { IModelError, CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';
import { MODE_PROJECT } from 'constant/cbdiEdit/cbdiEditConstant';
import { getCbdiEditMenuTemplate } from 'misc/menuTemplate/cbdiEdit/cbdiEditMenuTemplate';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import Editor from 'components/cbdiEdit/editor/Editor';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { darkTheme, lightTheme } from 'theme';
import { RootState } from 'projectRedux/Store';
import { cbdiEditSavedProjectActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditSavedProjectClientReducer/cbdiEditSavedProjectClientReducer';
import { CBDI_CLEAR_HISTORY_TYPE, cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { Edge } from 'reactflow';
import { CBDIEditorRootConceptType, PlanEdgeData } from 'misc/types/cbdiEdit/cbdiEditTypes';
import UniversalSearch from '../universalSearch/UniversalSearch';
import { Prompt } from '../prompt/Prompt';
import { onSaveCbdiEdit } from './saveProjectHelper';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)({
  overflow: 'hidden',
});

const EditorRow = styled(Row)({
  top: 30,
  bottom: 0,
  overflow: 'hidden',
});

const MenubarRow = styled(Row)({
  height: 30,
  top: 0,
});

const InfoContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  gap: 20,
  color: 'black',
  fontSize: 14,
  width: 300,
});

const ShortcutTextDiv = (
  <InfoContainer>
    <div>Save Model: [Ctrl + S]</div>
    <div>Open Model: [Ctrl + O]</div>
    <div>New Model: [Ctrl + M]</div>
    <div>Search Concept: [Ctrl + G]</div>
    <div>Search Task: [Ctrl + P]</div>
  </InfoContainer>
);

/* --------------------------- Frontend --------------------------- */

function CbdiFrontEnd() {
  /* ----------------------------- Redux ----------------------------- */
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const theme = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.theme);
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const mode = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.explorer.mode);
  const saved = useSelector((state: RootState) => state.cbdiEdit.cbdiEditSavedProject.saved);
  const dispatch = useDispatch();
  const {
    updateCurrent,
    saveAsProject,
    setProjectError,
    setDebugConsoleIsVisible,
    setExplorerMode,
    setProject,
    setCurrentProject,
    popSelectedTreeNodeConceptStack,
    pushSelectedTreeNodeConceptStack,
    resetCbdiEditor,
    createModule,
  } = cbdiEditActions;
  const { setSaved } = cbdiEditSavedProjectActions;

  /* ----------------------------- useState hooks ----------------------------- */
  const [isShowingShortcutInfo, setIsShowingShortcutInfo] = React.useState(false);
  const [isUniversalSearchOpen, setIsUniversalSearchOpen] = React.useState(false);
  const [modalMode, setModalMode] = React.useState<'conceptSwitching' | 'taskAdding'>('conceptSwitching');
  const [addingTaskEdge, setAddingTaskEdge] = React.useState<Edge<PlanEdgeData> | undefined>(undefined);
  /* -------------------------------- Callbacks -------------------------------- */

  const saveCbdiEdit = (forceUpdate = false) => {
    if (!current || !saved) {
      return;
    }

    const data = onSaveCbdiEdit(current, saved, forceUpdate);
    if (data) {
      // Informs the backend to save the new project model into file.
      window.ipcRenderer.invoke(request.project.update, data.saved);
    }
  };

  const handleKeyPress = (e: KeyboardEvent) => {
    if (e.ctrlKey === true) {
      switch (e.key.toLowerCase()) {
        case 's':
          // if the project is opened, save the project
          if (saved && mode === MODE_PROJECT && current !== null) {
            saveCbdiEdit();
          }
          break;
        case 'p':
          handleOpenUniversalSearchForPlan((e.detail as any).planEdgeToAdd);
          break;
        case 'g':
          // if the project is opened,
          // open the universal search for concept switching
          if (current !== null && selectedTreeNodeConcept?.module !== '') {
            setModalMode('conceptSwitching');
            setIsUniversalSearchOpen(true);
          }
          break;
        case 'o':
          window.ipcRenderer.invoke(request.project.open);
          break;
        case 'n':
          window.ipcRenderer.invoke(request.project.new);
          break;
        default:
          break;
      }
    }
    if (e.altKey === true) {
      switch (e.key) {
        case 'ArrowLeft':
          dispatch(popSelectedTreeNodeConceptStack());
          break;
        case 'ArrowRight':
          dispatch(pushSelectedTreeNodeConceptStack());
          break;
        default:
          break;
      }
    }
  };

  /**
   * Run once only when project is open/created new
   * @param _event
   * @param currentProject
   * @param newProjectModuleNames
   */
  const onProjectChanged = (_event: IpcRendererEvent, currentProject: CBDIEditorProject, savedProject?: CBDIEditorProject) => {
    dispatch(setProject(currentProject));
    // if savedProject is valid, update saved project
    if (savedProject) {
      dispatch(setSaved(savedProject));
    }
    // if savedProject is not valid, use current project as saved project
    else {
      dispatch(setSaved(currentProject));
    }

    dispatch(setExplorerMode(MODE_PROJECT));
  };

  const onProjectSaveAs = (_event: IpcRendererEvent) => {
    dispatch(saveAsProject());
  };

  const onProjectOverwrite = (_event: IpcRendererEvent) => {
    saveCbdiEdit(true);
  };

  const onModuleChanged = (_event: IpcRendererEvent, currentProject: CBDIEditorProject) => {
    dispatch(setCurrentProject(currentProject));
    // validate projectModel
    // init()
    //   .then(() => {
    //     const result: IModelError[] = JSON.parse(
    //       validate(JSON.stringify(projectModel, null, 2))
    //     );
    //     dispatch(setProjectError(result));
    //     if (result.length > 0) {
    //       dispatch(setDebugConsoleIsVisible(true));
    //       dispatch(setExplorerMode(MODE_PROJECT));
    //     }

    //     dispatch(setCurrentProject(currentProject));

    //     return result;
    //   })
    //   .catch((e) => {
    //     console.log(e);
    //   });
  };

  const onProjectExitSaving = (_event: IpcRendererEvent, isSavingProject: boolean) => {
    if (isSavingProject) {
      saveCbdiEdit();
    }

    // Clear redux undo history
    dispatch({ type: CBDI_CLEAR_HISTORY_TYPE });

    // Reset redux cbdi editor state
    dispatch(resetCbdiEditor());

    window.ipcRenderer.invoke(request.project.clearFileWatcher);
  };

  const onProjectSaveSuccess = () => {
    if (!current || !saved) {
      return;
    }
    const data = onSaveCbdiEdit(current, saved);
    if (data) {
      dispatch(setSaved(data.saved));
      dispatch(updateCurrent(data.current));
    }
  };

  /* ---------------------------- useCallback hooks --------------------------- */
  // if the project is opened and it is plan concept selected,
  // open the universal search for task adding
  const handleOpenUniversalSearchForPlan = React.useCallback(
    (planEdgeToAdd: Edge<PlanEdgeData> | undefined) => {
      if (mode === MODE_PROJECT && current !== null) {
        const selectedObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
        if (selectedObj?._objectType === CBDIEditorRootConceptType.PlanConceptType) {
          setAddingTaskEdge(planEdgeToAdd);
          setModalMode('taskAdding');
          setIsUniversalSearchOpen(true);
        }
      }
    },
    [current, mode, selectedTreeNodeConcept],
  );

  const onEditorAddingPlanTask = React.useCallback(
    (event: Electron.IpcRendererEvent, planEdgeToAdd: Edge<PlanEdgeData> | undefined) => {
      handleOpenUniversalSearchForPlan(planEdgeToAdd);
    },
    [handleOpenUniversalSearchForPlan],
  );

  const handleConfirmShortcutInfo = React.useCallback(() => {
    setIsShowingShortcutInfo(false);
  }, []);

  const handleCloseUniversalSearch = React.useCallback(() => {
    setIsUniversalSearchOpen(false);
  }, []);
  /* -------------------------------- Functions ------------------------------- */
  const registerEventHandlers = () => {
    /* --------------------------------- request -------------------------------- */
    window.ipcRenderer.on(request.project.saveAs, onProjectSaveAs);
    window.ipcRenderer.on(request.project.overwrite, onProjectOverwrite);

    /* -------------------------------- response -------------------------------- */
    window.ipcRenderer.on(response.project.changed, onProjectChanged);
    window.ipcRenderer.on(response.project.moduleChanged, onModuleChanged);
    window.ipcRenderer.on(response.project.exitSaving, onProjectExitSaving);
    window.ipcRenderer.on(response.project.saveSuccess, onProjectSaveSuccess);
  };

  const deregisterEventHandlers = () => {
    /* --------------------------------- request -------------------------------- */
    window.ipcRenderer.removeAllListeners(request.project.saveAs);
    window.ipcRenderer.removeAllListeners(request.project.overwrite);
    /* -------------------------------- response -------------------------------- */
    window.ipcRenderer.removeAllListeners(response.project.changed);
    window.ipcRenderer.removeAllListeners(response.project.moduleChanged);
    window.ipcRenderer.removeAllListeners(response.project.exitSaving);
    window.ipcRenderer.removeAllListeners(response.project.saveSuccess);
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const [cbdiEditMenuTemplate, onCloseWindow] = React.useMemo(() => {
    const mcbdiEditMenuTemplate = getCbdiEditMenuTemplate({
      current,
      mode,
      saved,
      createModule: () => {
        dispatch(createModule());
      },
      saveProject: () => {
        saveCbdiEdit();
      },
      saveAsProject: () => {
        dispatch(saveAsProject());
      },
      openProject: () => {
        window.ipcRenderer.invoke(request.project.open);
      },
      setProjectError: (error: IModelError[]) => {
        dispatch(setProjectError(error));
      },
      setDebugConsoleIsVisible: (isVisible: boolean) => {
        dispatch(setDebugConsoleIsVisible(isVisible));
      },
      setProject: (mproject: CBDIEditorProject | null) => {
        dispatch(setProject(mproject));
        dispatch(setSaved(mproject));
      },
      setIsShowingShortcutInfo: () => {
        setIsShowingShortcutInfo(true);
      },
    });
    let hasDialog = true;
    if ((saved && current) === null || areObjectsEqual(saved, current)) {
      hasDialog = false;
    }
    const monCloseWindow = () => {
      /**
       * Scenario editor present data from redux
       */
      let data: string | undefined;

      window.ipcRenderer.invoke(request.window.close, hasDialog, data);
    };
    return [mcbdiEditMenuTemplate, monCloseWindow];
  }, [mode, current, saved]);
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    registerEventHandlers();
    document.addEventListener('keydown', handleKeyPress);
    return () => {
      deregisterEventHandlers();
      document.removeEventListener('keydown', handleKeyPress);
    };
  }, [current, saved, selectedTreeNodeConcept, mode]);

  React.useEffect(() => {
    const onEditorPlanUniveralSearchOpenWithEdgeCleanup = window.ipcRenderer.setupIpcListener(
      editorRendererToRendererEvents.editorOpenUniversalSearchWithPlanEdge,
      onEditorAddingPlanTask,
    );

    return () => {
      onEditorPlanUniveralSearchOpenWithEdgeCleanup();
    };
  }, [onEditorAddingPlanTask]);

  // Comment out because the validator does not work
  // React.useEffect(() => {
  //   if (saved) {
  //     init()
  //       .then(() => {
  //         const result: IModelError[] = JSON.parse(
  //           validate(JSON.stringify(saveCBDIJsonFromProject(saved), null, 2))
  //         );

  //         if (result.length > 0) {
  //           dispatch(setProjectError(result));
  //           dispatch(setDebugConsoleIsVisible(true));
  //         }
  //         return result;
  //       })
  //       .catch((err) => {
  //         setSaveError(err);
  //       });
  //   }
  //   // This should be in else clause otherwise we will have this one run first (sync) then the `dispatch(setProjectError(result));` in the promise's callback
  //   else {
  //     dispatch(setProjectError([]));
  //   }
  // }, [saved]);
  /* -------------------------------- Components ------------------------------- */
  return (
    <ThemeProvider theme={theme === 'dark' ? darkTheme : lightTheme}>
      <Root>
        <MenubarRow>
          <TitleMenu menuAction={cbdiEditMenuTemplate} onCloseWindow={onCloseWindow} />
        </MenubarRow>
        <EditorRow>
          <Editor />
        </EditorRow>

        {isShowingShortcutInfo && (
          <Prompt open={isShowingShortcutInfo} title="Keyboard Shortcuts" content={ShortcutTextDiv} onConfirm={handleConfirmShortcutInfo} />
        )}

        <UniversalSearch
          mode={modalMode}
          addingTaskEdge={addingTaskEdge}
          isModalOpen={isUniversalSearchOpen}
          onCloseModal={handleCloseUniversalSearch}
        />
      </Root>
    </ThemeProvider>
  );
}

export default React.memo(CbdiFrontEnd);
