import React from 'react';
import { styled } from '@mui/material';
import { FlexRow, Fluid, Row } from 'components/common/base/BaseContainer';
import { CBDIEditorProject, CBDIEditorProjectPlan, CBDIEditorProjectTactic } from 'types/cbdiEdit/cbdiEditModel';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { EDITOR_OVERVIEW_MODULECONCEPT, treeRootCategory } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import { CBDIEditorOtherCategoryType, CBDIEditorRootConceptType, Mod, ProjectConceptListType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import ProjectTree from './ProjectTree/ProjectTree';
import { TreeData, getCategoryLabelByType } from './helper';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)(({ theme }) => ({
  backgroundColor: theme.editor.explorerView.bgColor,
  color: theme.editor.explorerView.textColor,
}));

const TitleRow = styled(FlexRow)({
  height: '100%',
  padding: '0 10px',
  justifyContent: 'space-between',
  alignItems: 'center',
  fontSize: '.8em',
});

const Left = styled('div')({
  height: '100%',
  width: '100%',
  display: 'flex',
  justifyContent: 'flex-start',
  alignItems: 'center',
});

const TitleContainer = styled(Row)({
  height: 30,
  top: 0,
});

const ContentContainer = styled(Row)({
  top: 30,
  bottom: 0,
  fontSize: '.9em',
  fontWeight: 'lighter',
});

/* ---------------------------- Explorer Project ---------------------------- */
function ExplorerProject() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  /* -------------------------------- Functions ------------------------------- */

  const populateSubdir = (mproject: CBDIEditorProject, root: TreeData, conceptType: CBDIEditorRootConceptType, moduleName: string) => {
    const categoryLabel = getCategoryLabelByType(conceptType);
    const subdir: TreeData = {
      nodeId: conceptType + moduleName,
      label: categoryLabel,
      iconType: CBDIEditorOtherCategoryType.FolderType,
      categoryType: conceptType,
      children: [],
      moduleName,
    };

    const allPlans = mproject.plans
      .map((moduleConcept) => {
        const plan = getObjectByModuleConcept(current!, moduleConcept) as CBDIEditorProjectPlan;
        return plan;
      })
      .filter((plan) => plan !== undefined && plan._mod !== Mod.Deletion) as CBDIEditorProjectPlan[];
    const allTactics = mproject.tactics
      .map((moduleConcept) => {
        const tactic = getObjectByModuleConcept(current!, moduleConcept) as CBDIEditorProjectTactic;
        return tactic;
      })
      .filter((tactic) => tactic !== undefined && tactic._mod !== Mod.Deletion) as CBDIEditorProjectTactic[];

    mproject[categoryLabel as ProjectConceptListType].forEach((moduleConcept) => {
      if (moduleConcept.module !== moduleName) {
        return;
      }
      const { cbdiObjects } = mproject;
      const cbdiObject = cbdiObjects[moduleConcept.uuid];
      if (!cbdiObject || cbdiObject._mod === Mod.Deletion) {
        return;
      }
      const conceptTreeNode: TreeData = {
        moduleConcept: {
          uuid: cbdiObject.uuid,
          module: cbdiObject.module,
          name: cbdiObject.name,
        },
        nodeId: moduleConcept.uuid,
        label: cbdiObject.name,
        iconType: conceptType,
        mod: cbdiObject._mod,
        moduleName,
      };
      // if it is goals, add plans and tactics to its children
      if (conceptType === CBDIEditorRootConceptType.GoalConceptType) {
        // get sorted plan objects
        const childPlans = allPlans
          .filter((planObj) => planObj.handles.uuid === moduleConcept.uuid)
          .sort((a, b) => a.name.localeCompare(b.name, 'en'));
        // get sorted tactic objects
        const childTactics = allTactics
          .filter((tacticObj) => tacticObj.goal.uuid === moduleConcept.uuid)
          .sort((a, b) => a.name.localeCompare(b.name, 'en'));

        const goalChildTreeNodes: TreeData[] = [...childPlans, ...childTactics].map((obj) => ({
          moduleConcept: {
            uuid: obj.uuid,
            module: obj.module,
            name: obj.name,
          },
          nodeId: obj.uuid,
          label: obj.name,
          iconType: obj._objectType,
          mod: obj._mod,
          moduleName,
          parentModuleConceptUuid: cbdiObject.uuid,
        }));

        conceptTreeNode.children = goalChildTreeNodes;
      }
      subdir.children!.push(conceptTreeNode);
    });
    // sort tree item under subdir by label
    subdir.children!.sort((a, b) => a.label.localeCompare(b.label, 'en'));

    // if subdir has any child with addition or update mode, make subdir update mode
    if (subdir.children!.some((el) => el.mod === Mod.Addition || el.mod === Mod.Update)) {
      subdir.mod = Mod.Update;
    }
    root.children!.push(subdir);
  };

  const transformProjectModel = (mproject: CBDIEditorProject, moduleName: string) => {
    const root: TreeData = {
      nodeId: CBDIEditorOtherCategoryType.ModuleType + moduleName,
      label: moduleName,
      iconType: CBDIEditorOtherCategoryType.ModuleType,
      moduleName,
      children: [
        {
          nodeId: CBDIEditorOtherCategoryType.OverviewType + moduleName,
          label: CBDIEditorOtherCategoryType.OverviewType,
          iconType: CBDIEditorOtherCategoryType.OverviewType,
          moduleConcept: { ...EDITOR_OVERVIEW_MODULECONCEPT, module: moduleName },
          moduleName,
        },
      ],
    };

    treeRootCategory.forEach((type) => {
      populateSubdir(mproject, root, type, moduleName);
    });
    if (root.children!.some((el) => el.mod === Mod.Addition || el.mod === Mod.Update)) {
      root.mod = Mod.Update;
    }
    return root;
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const treeData = React.useMemo(() => {
    const root: TreeData = {
      nodeId: CBDIEditorOtherCategoryType.RootType,
      label: current!.name,
      iconType: CBDIEditorOtherCategoryType.RootType,
      children: [
        {
          nodeId: CBDIEditorOtherCategoryType.OverviewType,
          label: CBDIEditorOtherCategoryType.OverviewType,
          iconType: CBDIEditorOtherCategoryType.OverviewType,
          moduleConcept: EDITOR_OVERVIEW_MODULECONCEPT,
          moduleName: '',
        },
      ],
      moduleName: '',
    };
    current!.modulePaths.forEach((modulePath) => {
      // if module path is valid
      // populate concepts
      if (modulePath.valid) {
        root.children!.push(transformProjectModel(current!, modulePath.name));
      }
      // if module path is not valid
      // make module root node disabled
      else {
        const moduleRoot: TreeData = {
          nodeId: CBDIEditorOtherCategoryType.ModuleType + modulePath.name,
          label: modulePath.name,
          iconType: CBDIEditorOtherCategoryType.ModuleType,
          moduleName: modulePath.name,
          disabled: true,
        };
        root.children!.push(moduleRoot);
      }
    });
    return root;
  }, [current]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root key="explorer-project-root">
      <TitleContainer>
        <TitleRow>
          <Left>
            <div>PROJECT</div>
          </Left>
        </TitleRow>
      </TitleContainer>
      <ContentContainer>
        <ProjectTree key="project-tree" treeData={treeData} />
      </ContentContainer>
    </Root>
  );
}

export default React.memo(ExplorerProject);
