import { CBDIEditorRootConceptType, ModuleConcept, CBDIEditorOtherCategoryType, Mod, CBDIEditorObject } from 'misc/types/cbdiEdit/cbdiEditTypes';

export interface TreeBasicData {
  nodeId: string;
  label: string;
  iconType: CBDIEditorRootConceptType | CBDIEditorOtherCategoryType;
  moduleName: string;
  categoryType?: CBDIEditorRootConceptType | CBDIEditorOtherCategoryType;
  moduleConcept?: ModuleConcept;
  mod?: Mod;
  disabled?: boolean;
  parentModuleConceptUuid?: string;
}

export interface TreeData extends TreeBasicData {
  children?: TreeData[];
}

export const getCategoryLabelByType = (type: CBDIEditorRootConceptType | CBDIEditorOtherCategoryType.ModuleType) => {
  if (type === CBDIEditorRootConceptType.EntityConceptType) {
    return 'entities';
  }
  return `${type}s`;
};

/**
 *
 * @param currentExpanded current expanded tree node id array
 * @param obj selected cbdi object
 * @returns new expanded tree node id array
 */
export const getExpandedTreeNodeId = (currentExpanded: string[], obj: CBDIEditorObject) => {
  // expand root node
  currentExpanded.push(CBDIEditorOtherCategoryType.RootType);
  // expand module root node
  currentExpanded.push(CBDIEditorOtherCategoryType.ModuleType + obj.module);

  // expand the category node
  const categoryNodeId = obj._objectType + obj.module;
  currentExpanded.push(categoryNodeId);

  const uniqueSet = new Set(currentExpanded);
  return Array.from(uniqueSet);
};

/**
 * get context menu item for tree item
 * @param treeBasicData
 * @returns
 */
export const getContextMenuItems = (treeBasicData: TreeBasicData) => {
  const { nodeId, categoryType, moduleConcept, iconType } = treeBasicData;
  if (nodeId === CBDIEditorOtherCategoryType.RootType) {
    return ['Rename'];
  }
  if (categoryType) {
    return [`New ${categoryType}`];
  }
  if (iconType === CBDIEditorOtherCategoryType.ModuleType) {
    return ['Rename', 'Remove'];
  }
  if (moduleConcept) {
    if (iconType === CBDIEditorRootConceptType.GoalConceptType) {
      return ['Rename', 'Duplicate', 'New plan', 'New tactic', 'Delete'];
    }
    if (iconType === CBDIEditorOtherCategoryType.OverviewType) {
      return [];
    }
    return ['Rename', 'Duplicate', 'Delete'];
  }
  return [];
};
