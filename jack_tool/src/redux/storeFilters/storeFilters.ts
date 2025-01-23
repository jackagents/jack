import { AnyAction } from '@reduxjs/toolkit';

const cbdiExcludes = [
  'cbdiEditCurrent/setProjectError',
  'cbdiEditCurrent/setProjectWarning',
  'cbdiEditCurrent/setSelectedTreeNodeConcept',
  'cbdiEditCurrent/setGraphSelectedNode',
  'cbdiEditCurrent/updateExpandedProjectDirs',
  'cbdiEditCurrent/renameModuleName',
  'cbdiEditCurrent/setExplorerMode',
  'cbdiEditCurrent/setDebugConsoleIsVisible',
  'cbdiEditCurrent/setGraphDetailIsVisible',
  'cbdiEditCurrent/setTheme',
  'cbdiEditCurrent/resetCbdiEditor',
  'cbdiEditSavedProject/setSaved',
  'cbdiEditCurrent/updateCurrent',
  'cbdiEditCurrent/popSelectedTreeNodeConceptStack',
  'cbdiEditCurrent/pushSelectedTreeNodeConceptStack',
  'cbdiEditCurrent/setScrollToSelectedTreeNodeFlag',
  'cbdiEditCurrent/setActionNodesContextArr',
  'cbdiEditCurrent/setXaiLiveDebug',
];

const scenarioEditorExcludes = [
  'scenarioEditorTeams/setServices',
  'scenarioEditorTeams/setCurrentTeams',
  'scenarioEditorTeams/setLinks',
  'scenarioEditorEvents/addLinkAgentEvent',
  'scenarioEditorEvents/removeLinkAgentEvent',
  'scenarioEditorProject/setScenarioProject',
  'scenarioEditorEvents/setUserOptionsFilterEvent',
];

const iwdWebExcludes: string[] = [];

/**
 * Excludes actions from scenario editor and some cbdi editor actions
 * @param action
 * @returns
 */
export const filterCbdiActions = (action: AnyAction) => {
  const type = action.type as string;

  for (let i = 0; i < cbdiExcludes.length; i++) {
    const exclude = cbdiExcludes[i];
    if (exclude === type || !type.includes('cbdiEdit')) {
      return false;
    }
  }

  return true;
};

/**
 * Excludes actions not from scenario editor and some cbdi editor actions
 * @param action
 * @returns
 */
export const filterScenarioEditorActions = (action: AnyAction) => {
  const type = action.type as string;
  for (let i = 0; i < scenarioEditorExcludes.length; i++) {
    const exclude = scenarioEditorExcludes[i];
    if (exclude === type || !type.includes('scenarioEditor')) {
      return false;
    }
  }

  return true;
};

/**
 * Excludes actions not from iwd-web redux
 * @param action
 * @returns
 */
export const filterIwdWebActions = (action: AnyAction) => {
  const type = action.type as string;
  for (let i = 0; i < iwdWebExcludes.length; i++) {
    const exclude = iwdWebExcludes[i];
    if (exclude === type || !type.includes('iwdWeb')) {
      return false;
    }
  }

  return true;
};
