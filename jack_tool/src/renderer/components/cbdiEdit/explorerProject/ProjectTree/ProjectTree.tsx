/* eslint-disable react/jsx-props-no-spreading */
import { TreeView } from '@mui/lab';
import { ChevronRight, ExpandMore } from '@mui/icons-material';
import React from 'react';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { ThemedOverflowScrollContainer } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorOtherCategoryType, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { editorRendererToRendererEvents } from 'misc/events/cbdiEdit/editEvents';
import { isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import { TreeData, getExpandedTreeNodeId } from '../helper';
import ProjectTreeItem from './ProjectTreeItem';
import { findNodeById, findToggledNodeId } from './helper';

export default function ProjectTree({ treeData }: { treeData: TreeData }) {
  /* ------------------------------ useRef hooks ------------------------------ */
  const treeRef = React.useRef<HTMLDivElement | null>(null);
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const scrollToSelectedTreeNodeFlag = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.scrollToSelectedTreeNodeFlag);
  /* ----------------------------- useState hooks ----------------------------- */
  const [selected, setSelected] = React.useState<string[]>([CBDIEditorOtherCategoryType.OverviewType]);
  const [expanded, setExpanded] = React.useState<string[]>([CBDIEditorOtherCategoryType.RootType]);
  const [renamingId, setRenamingId] = React.useState<string>();
  /* -------------------------------- Callbacks ------------------------------- */
  const handleToggle = (event: React.SyntheticEvent<Element, Event>, nodeIds: string[]) => {
    // if click area is on expand/collapse icon
    // change expanded
    const target = event.target as HTMLElement;
    if (target.tagName === 'svg' || target.tagName === 'path') {
      setExpanded(nodeIds);
    } else {
      // Determine which node was toggled
      const toggledNodeId = findToggledNodeId(expanded, nodeIds);
      if (toggledNodeId) {
        const toggledNode = findNodeById(treeData, toggledNodeId);
        // if toggledNode is not goal
        // toggle children display
        if (toggledNode?.iconType !== CBDIEditorRootConceptType.GoalConceptType) {
          setExpanded(nodeIds);
        }
      }
    }
  };

  const onEditorAddToProjectTreeExpanded = React.useCallback(
    (event: Electron.IpcRendererEvent, addingNodeIds: string[]) => {
      const uniqueSet = new Set([...expanded, ...addingNodeIds]);
      const newExpanded = Array.from(uniqueSet);
      setExpanded(newExpanded);
    },
    [expanded],
  );

  /* ------------------------------ useEffect hooks ----------------------------- */
  React.useEffect(() => {
    // make selected tree node dir tree expand
    const obj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
    if (obj) {
      const newExpanded = getExpandedTreeNodeId(expanded, obj);
      setExpanded(newExpanded);
    }

    // change selected tree item
    if (selectedTreeNodeConcept) {
      if (isModuleConceptOverview(selectedTreeNodeConcept)) {
        // if select top level overview
        if (selectedTreeNodeConcept.module === '') {
          setSelected([CBDIEditorOtherCategoryType.OverviewType]);
        }
        // if select module level overview
        else {
          setSelected([CBDIEditorOtherCategoryType.OverviewType + selectedTreeNodeConcept.module]);
        }
      } else {
        setSelected([selectedTreeNodeConcept.uuid]);
      }
    } else {
      setSelected([]);
    }
  }, [selectedTreeNodeConcept]);

  // scroll to selected tree node
  React.useEffect(() => {
    // set 1 second timeout to make sure tree item is expanded before scroll
    setTimeout(() => {
      if (treeRef.current) {
        const selectedElement = treeRef.current.querySelector('.MuiTreeItem-content.Mui-selected');
        if (selectedElement) {
          selectedElement.scrollIntoView({
            behavior: 'smooth',
            block: 'center',
          });
        }
      }
    }, 1000);
  }, [scrollToSelectedTreeNodeFlag]);

  /* -------------------------------- functions ------------------------------- */
  const renderTree = ({ children, nodeId, ...other }: TreeData) => (
    <ProjectTreeItem
      key={nodeId}
      nodeId={nodeId}
      setExpanded={setExpanded}
      // if tree node has parentModuleConceptUuid
      // we use combination of parenetModuleConceptUuid and nodeId
      // to determin if it is renaming
      // this is for plan/tactic tree node under goal
      isRenaming={(other.parentModuleConceptUuid ? other.parentModuleConceptUuid + nodeId : nodeId) === renamingId}
      setRenamingId={setRenamingId}
      {...other}
    >
      {children && children.map((child: TreeData) => renderTree(child))}
    </ProjectTreeItem>
  );

  React.useEffect(() => {
    const onEditorAddToProjectTreeExpandedCleanup = window.ipcRenderer.setupIpcListener(
      editorRendererToRendererEvents.editorAddToProjectTreeExpanded,
      onEditorAddToProjectTreeExpanded,
    );

    return () => {
      onEditorAddToProjectTreeExpandedCleanup();
    };
  }, [onEditorAddToProjectTreeExpanded]);

  return (
    <ThemedOverflowScrollContainer style={{ overflowY: 'auto' }} ref={treeRef}>
      <TreeView
        key="cbdi-editor-left-directory-tree"
        id="cbdi-editor-left-directory-tree"
        selected={selected}
        expanded={expanded}
        onNodeToggle={handleToggle}
        defaultExpandIcon={<ChevronRight />}
        defaultCollapseIcon={<ExpandMore />}
      >
        {renderTree(treeData)}
      </TreeView>
    </ThemedOverflowScrollContainer>
  );
}
