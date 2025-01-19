/* eslint-disable @typescript-eslint/naming-convention */
/* eslint-disable @typescript-eslint/no-use-before-define */
/* eslint-disable react/jsx-props-no-spreading */
import React, { ReactNode, useEffect } from 'react';
import { styled, IconButton } from '@mui/material';
import { TreeItem, useTreeItem } from '@mui/lab';
import { Timeline, Folder, DashboardCustomize, Add } from '@mui/icons-material';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { areModuleConceptsEqual, copyObj } from 'misc/utils/common/commonUtils';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import {
  createNewPlanWithGoal,
  createNewTacticWithGoal,
  createNewConcept,
  copy,
  getObjectByModuleConcept,
  createDuplicatedPlanObj,
} from 'misc/utils/cbdiEdit/Helpers';
import { v4 } from 'uuid';
import { editorRendererToRendererEvents, request } from 'misc/events/cbdiEdit/editEvents';
import { Prompt } from 'components/cbdiEdit/prompt/Prompt';
import { ThemedMenu, ThemedMenuItem } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorObject, CBDIEditorOtherCategoryType, CBDIEditorRootConceptType, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { BORDER_RADIUS, RECTANGLE_NODE_HEIGHT, RECTANGLE_NODE_WIDTH } from 'components/cbdiEdit/reactFlowPlanEditor/reactFlowPlanEditorConstant';
import { useCbdiReactflowContext } from 'components/cbdiEdit/CbdiEditReactflowContext/CbdiEditReactflowContext';
import { CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { TreeBasicData, getContextMenuItems } from '../helper';

const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  backgroundColor: 'transparent',
  '& .MuiTreeItem-content:hover': {
    backgroundColor: theme.editor.explorerView.hoveringBgColor,
  },
  '& .MuiTreeItem-content.Mui-selected': {
    color: theme.editor.explorerView.contrastTextColor,
    backgroundColor: theme.editor.explorerView.activeBgColor,
  },
  cursor: 'pointer',
  margin: '1px 0',
}));

const LabelRoot = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  whiteSpace: 'nowrap',
  fontSize: 18,
});

const SubRoot = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: 5,
});

interface Props extends TreeBasicData {
  children: ReactNode[] | undefined;
  isRenaming: boolean;
  setRenamingId: React.Dispatch<React.SetStateAction<string | undefined>>;
  setExpanded: React.Dispatch<React.SetStateAction<string[]>>;
}

export default React.forwardRef((props: Props, ref: React.Ref<HTMLLIElement>) => {
  /* ---------------------------------- Props --------------------------------- */
  const { isRenaming, setRenamingId, setExpanded, children, ...treeBasicData } = props;
  const { nodeId, label, iconType, categoryType, moduleConcept, mod, moduleName, disabled, parentModuleConceptUuid } = treeBasicData;

  const contextMenuItems = getContextMenuItems(treeBasicData);
  /* ---------------------------- useTreeItem hook ---------------------------- */
  const { selected } = useTreeItem(nodeId);
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const dispatch = useDispatch();
  const {
    updateProjectName,
    putObjects,
    setSelectedTreeNodeConcept,
    setGraphSelectedNode,
    setGraphDetailIsVisible,
    deleteObjects,
    popSelectedTreeNodeConceptStack,
    updateObjects,
    renameModuleName,
  } = cbdiEditActions;
  /* ----------------------------- useContext hooks ----------------------------- */
  const { viewportDataDic, setDraggingModuleConcept } = useCbdiReactflowContext();
  /* ------------------------------ useRef hooks ------------------------------ */
  const inputRef = React.useRef(null);
  /* ----------------------------- useState hooks ----------------------------- */
  const [contextMenu, setContextMenu] = React.useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);
  const [isDeleting, setIsDeleting] = React.useState(false);
  const [duplicatedName, setDuplicatedName] = React.useState<string | null>(null);
  const [text, setText] = React.useState<string>(label);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleSelectTreeItem = (event: React.MouseEvent) => {
    event.preventDefault();
    event.stopPropagation();

    const target = event.target as HTMLElement;
    // if click the arrow to toggle expand/collapse
    // do not select the node
    if (target.tagName === 'svg' || target.tagName === 'path') {
      return;
    }
    if (moduleConcept) {
      if (!areModuleConceptsEqual(moduleConcept, selectedTreeNodeConcept)) {
        dispatch(setSelectedTreeNodeConcept(moduleConcept));
        dispatch(setGraphSelectedNode(moduleConcept));
      }
    }
  };

  const handleDoubleClick = (event: React.MouseEvent) => {
    event.stopPropagation();
    if (iconType === CBDIEditorRootConceptType.GoalConceptType) {
      setExpanded((prev) => {
        if (prev.includes(nodeId)) {
          return prev.filter((el) => el !== nodeId);
        }
        return [...prev, nodeId];
      });
    }
  };

  const handleContextMenu = (event: React.MouseEvent) => {
    handleSelectTreeItem(event);
    event.preventDefault();

    setContextMenu(
      contextMenu === null
        ? {
            mouseX: event.clientX + 2,
            mouseY: event.clientY - 6,
          }
        : null,
    );
  };

  const handleCloseContextMenu = (event: React.MouseEvent) => {
    event.stopPropagation();
    setContextMenu(null);
  };

  const handleDuplicateConcept = () => {
    const treeItemObj = getObjectByModuleConcept(current, moduleConcept);
    if (treeItemObj) {
      const id = v4();
      let newCbdiObject = copyObj(treeItemObj);
      newCbdiObject.uuid = id;
      newCbdiObject.name += ' copy';
      // if duplicate plan
      if (treeItemObj._objectType === CBDIEditorRootConceptType.PlanConceptType) {
        newCbdiObject = createDuplicatedPlanObj(treeItemObj as CBDIEditorProjectPlan, id);
      }

      dispatch(putObjects(newCbdiObject));

      // when duplicate concept, make it be selected
      const { _mod, _objectType, note, ...newModuleConcept } = newCbdiObject;
      dispatch(setSelectedTreeNodeConcept(newModuleConcept));
      dispatch(setGraphSelectedNode(newModuleConcept));
      dispatch(setGraphDetailIsVisible(true));

      // make newly duplicated concept renaming state
      setTimeout(() => {
        // if duplicate tactic/plan under goal
        if (parentModuleConceptUuid) {
          setRenamingId(parentModuleConceptUuid + id);
        }
        // if not duplicate tactic/plan under goal
        else {
          setRenamingId(id);
        }
      }, 50);
    }
  };

  const handleNewConcept = (
    newConceptType?: typeof CBDIEditorRootConceptType.PlanConceptType | typeof CBDIEditorRootConceptType.TacticConceptType,
  ) => {
    const id = v4();
    let newCbdiObject: CBDIEditorObject | CBDIEditorObject[] | null = null;
    // if not create new plan/tactic with goal
    if (!newConceptType) {
      if (!categoryType) {
        return;
      }
      // if create new module
      if (categoryType === CBDIEditorOtherCategoryType.ModuleType && current) {
        window.ipcRenderer.invoke(request.project.importModule, current);
        return;
      }
      // if create concept
      newCbdiObject = createNewConcept(id, categoryType as CBDIEditorRootConceptType, moduleName);
    }
    // if create new plan/tactic with goal
    else {
      if (!moduleConcept) {
        return;
      }
      if (newConceptType === CBDIEditorRootConceptType.PlanConceptType) {
        newCbdiObject = createNewPlanWithGoal(id, moduleConcept);
      } else if (newConceptType === CBDIEditorRootConceptType.TacticConceptType) {
        newCbdiObject = createNewTacticWithGoal(id, moduleConcept);
      }
    }

    if (newCbdiObject !== null) {
      dispatch(putObjects(newCbdiObject));
      // if it creates multiple concept, use the first concept to be selected
      const firstNewCbdiObject = Array.isArray(newCbdiObject) ? newCbdiObject[0] : newCbdiObject;
      // if the first concept is goal
      // make this goal expanded
      if (firstNewCbdiObject._objectType === CBDIEditorRootConceptType.GoalConceptType) {
        if (window.mainWindowId) {
          window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorAddToProjectTreeExpanded, [firstNewCbdiObject.uuid]);
        }
      }
      // when create new concept, make it be selected
      const { _mod, _objectType, note, ...newModuleConcept } = firstNewCbdiObject;
      dispatch(setSelectedTreeNodeConcept(newModuleConcept));
      dispatch(setGraphSelectedNode(newModuleConcept));
      dispatch(setGraphDetailIsVisible(true));
      // make newly created concept renaming state
      setTimeout(() => {
        // if create tactic/plan under goal
        if (newConceptType) {
          setRenamingId(nodeId + newModuleConcept.uuid);
        }
        // if not create tactic/plan under goal
        else {
          setRenamingId(newModuleConcept.uuid);
        }
      }, 50);
    }
  };

  const handleClickContextMenu = (event: React.MouseEvent<HTMLLIElement, MouseEvent>, menuLabel: string) => {
    event.stopPropagation();
    handleCloseContextMenu(event);

    switch (menuLabel) {
      case 'Rename':
        setTimeout(() => {
          setRenamingId(parentModuleConceptUuid ? parentModuleConceptUuid + nodeId : nodeId);
        }, 50);
        break;
      case 'Duplicate':
        handleDuplicateConcept();
        break;
      case `New ${categoryType}`:
        handleNewConcept();
        break;
      case 'New plan':
        handleNewConcept(CBDIEditorRootConceptType.PlanConceptType);
        break;
      case 'New tactic':
        handleNewConcept(CBDIEditorRootConceptType.TacticConceptType);
        break;
      case 'Delete':
        setIsDeleting(true);
        break;
      case 'Remove':
        setIsDeleting(true);
        break;
      default:
        break;
    }
  };

  const handleContextMenuBlured = () => {
    if (isRenaming && inputRef.current) {
      (inputRef.current as any).focus();
    }
  };

  const handleStartRenaming = (event: React.FocusEvent<HTMLInputElement>) => {
    event.currentTarget!.setSelectionRange(0, event.currentTarget!.value.length);
  };

  const handleRenaming = (event: React.ChangeEvent<HTMLInputElement>) => {
    setText(event.target.value);
  };

  const handleFinishRenaming = () => {
    if (text === '') {
      setText(label);
    }

    if (text !== label && current) {
      if (iconType === CBDIEditorOtherCategoryType.ModuleType) {
        if (!current.modulePaths.find((el) => el.name === text)) {
          dispatch(renameModuleName({ oldModuleName: moduleName, newModuleName: text }));
        } else {
          setText(label);
        }
      } else {
        const allCbdiObjectNames = Object.values(current.cbdiObjects)
          .filter((value) => value._mod !== Mod.Deletion)
          .map((value) => value.name);

        if (!allCbdiObjectNames.includes(text)) {
          if (nodeId === CBDIEditorOtherCategoryType.RootType) {
            dispatch(updateProjectName(text));
          } else {
            const cbdiObject: CBDIEditorObject = copy(current.cbdiObjects[nodeId]);
            cbdiObject.name = text;
            cbdiObject._mod = Mod.Update;
            dispatch(updateObjects(cbdiObject));
          }
        } else {
          setDuplicatedName(text);
          setText(label);
        }
      }
    }
    setRenamingId(undefined);
  };

  const getDeletePromptTitleAndContent = () => {
    let deleteTitle = 'Warning: Delete Concept';
    let deleteConcept = 'Are you sure you want to delete this concept?';
    if (iconType === CBDIEditorOtherCategoryType.ModuleType) {
      deleteTitle = 'Warning: Remove Module';
      deleteConcept = 'Are you sure you want to remove this module?';
    }
    return [deleteTitle, deleteConcept];
  };

  const handleDeleteTreeItem = () => {
    setIsDeleting(false);
    // deselect graph node
    dispatch(setGraphSelectedNode(null));
    // deselect tree node
    dispatch(setSelectedTreeNodeConcept(null));
    // if deleting node is a module, invoke removeModule event
    if (iconType === CBDIEditorOtherCategoryType.ModuleType) {
      window.ipcRenderer.invoke(request.project.removeModule, current, moduleName);
    }
    // delete object
    else {
      if (current) {
        const object = current.cbdiObjects[nodeId];
        dispatch(deleteObjects(object.uuid));
      }
      // make selected node concept to be the previous one
      dispatch(popSelectedTreeNodeConceptStack());
    }
  };

  const onDragStart = (event: React.DragEvent<HTMLLIElement>) => {
    event.stopPropagation();
    // if the item is not draggable
    // prevent default event
    if (!draggable) {
      event.preventDefault();
      return;
    }
    if (moduleConcept) {
      setDraggingModuleConcept(moduleConcept);
      // eslint-disable-next-line no-param-reassign
      event.dataTransfer.effectAllowed = 'copy';

      let graphZoom = 1;
      if (selectedTreeNodeConcept) {
        const viewport = viewportDataDic[selectedTreeNodeConcept.uuid];
        if (viewport) {
          graphZoom = viewport.zoom;
        }
      }

      const div = document.createElement('div');
      div.style.position = 'absolute';
      div.style.top = '-100%';
      div.style.width = `${RECTANGLE_NODE_WIDTH * graphZoom}px`;
      div.style.height = `${((RECTANGLE_NODE_HEIGHT * 5) / 6) * graphZoom}px`;
      div.style.border = '2px dashed black';
      div.style.borderRadius = `${BORDER_RADIUS * graphZoom}px`;
      div.classList.add('custom-drag-image');

      document.body.appendChild(div);

      event.dataTransfer.setDragImage(div, 0, 0);
    }
  };

  const onDragEnd = () => {
    const divs = document.querySelectorAll('.custom-drag-image');
    divs.forEach((div) => {
      div.remove();
    });
  };

  /* ------------------------------ useMemo hooks ----------------------------- */

  const icon = React.useMemo(() => {
    if (iconType === CBDIEditorOtherCategoryType.RootType) {
      return <DashboardCustomize />;
    }
    if (iconType === CBDIEditorOtherCategoryType.OverviewType) {
      return <Timeline />;
    }
    if (iconType === CBDIEditorOtherCategoryType.FolderType) {
      return <Folder />;
    }
    return (
      <img
        src={nodeIcon[iconType]}
        style={{
          height: 20,
          width: 20,
          display: 'block',
        }}
        alt=""
      />
    );
  }, [iconType]);

  const styledLabel = React.useMemo(() => {
    if (mod === Mod.Addition) {
      return <div style={{ color: '#4fd165', fontStyle: 'italic' }}>*{label}</div>;
    }
    if (mod === Mod.Update) {
      return <div style={{ color: '#e09e58', fontStyle: 'italic' }}>*{label}</div>;
    }
    return <div>{label}</div>;
  }, [mod, label]);

  const labelComponent = React.useMemo(() => {
    const [deleteTitle, deleteContent] = getDeletePromptTitleAndContent();
    return (
      <LabelRoot
        tabIndex={0}
        onKeyUp={(e) => {
          if (e.code === 'F2') {
            if (selected) {
              setRenamingId(parentModuleConceptUuid ? parentModuleConceptUuid + nodeId : nodeId);
              if (inputRef.current) {
                (inputRef.current as any).focus();
              }
            }
          }
        }}
        onContextMenu={handleContextMenu}
      >
        <SubRoot>
          {icon}
          {isRenaming ? (
            <input
              // eslint-disable-next-line jsx-a11y/no-autofocus
              autoFocus
              ref={inputRef}
              value={text}
              onBlur={handleFinishRenaming}
              onKeyUp={(evt) => {
                if (evt.key === 'Enter') {
                  handleFinishRenaming();
                }
              }}
              onClick={(e) => {
                e.preventDefault();
                e.stopPropagation();
              }}
              style={{ fontSize: 18 }}
              onChange={handleRenaming}
              onFocus={handleStartRenaming}
            />
          ) : (
            styledLabel
          )}
        </SubRoot>
        {categoryType && (
          <IconButton
            onClick={(event) => {
              event.stopPropagation();
              handleNewConcept();
            }}
            sx={{
              color: 'inherit',
              backgroundColor: 'transparent',
              boxShadow: 'none',
            }}
          >
            <Add />
          </IconButton>
        )}
        {contextMenu !== null && contextMenuItems.length > 0 && (
          <ThemedMenu
            open={contextMenu !== null && contextMenuItems.length > 0}
            onBlur={handleContextMenuBlured}
            onClose={handleCloseContextMenu}
            anchorReference="anchorPosition"
            anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
          >
            {contextMenuItems.map((el, index) => (
              <ThemedMenuItem key={index as number} onClick={(event) => handleClickContextMenu(event, el)}>
                {el}
              </ThemedMenuItem>
            ))}
          </ThemedMenu>
        )}
        {isDeleting && (
          <Prompt
            open={isDeleting}
            title={deleteTitle}
            content={deleteContent}
            onClose={() => {
              setIsDeleting(false);
            }}
            onConfirm={handleDeleteTreeItem}
          />
        )}
        {duplicatedName !== null && (
          <Prompt
            open={duplicatedName !== null}
            title="Warning: Rename Concept"
            content={`${duplicatedName} already exists, please use a different name`}
            onConfirm={() => {
              setDuplicatedName(null);
            }}
          />
        )}
      </LabelRoot>
    );
  }, [iconType, styledLabel, categoryType, contextMenu, isRenaming, text, isDeleting, duplicatedName, selected, mod, children]);

  // make tree item draggable when is selecting plan and tree item is goal/action
  const draggable = React.useMemo(() => {
    if (isRenaming) {
      return false;
    }
    const selectedTreeObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
    const treeItemObj = getObjectByModuleConcept(current, moduleConcept);
    if (selectedTreeObj && treeItemObj) {
      if (selectedTreeObj._objectType === CBDIEditorRootConceptType.PlanConceptType) {
        if (
          treeItemObj &&
          (treeItemObj._objectType === CBDIEditorRootConceptType.ActionConceptType ||
            treeItemObj._objectType === CBDIEditorRootConceptType.GoalConceptType)
        ) {
          return true;
        }
        return false;
      }
      return true;
    }
    return false;
  }, [moduleConcept, selectedTreeNodeConcept, current, isRenaming]);

  /* ------------------------------- useEffect hooks ------------------------------- */
  useEffect(() => {
    setText(label);
  }, [label]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <StyledTreeItem
      onClick={handleSelectTreeItem}
      onDoubleClick={handleDoubleClick}
      nodeId={nodeId}
      label={labelComponent}
      ref={ref}
      onDragStart={onDragStart}
      onDragEnd={onDragEnd}
      draggable={true}
      disabled={!!disabled}
    >
      {children}
    </StyledTreeItem>
  );
});
