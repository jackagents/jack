import { List, SelectChangeEvent, styled } from '@mui/material';
import { getAddingModuleConceptOptions, getObjectByModuleConcept, sortModuleConceptList } from 'misc/utils/cbdiEdit/Helpers';
import DraggableList from 'react-draggable-list';
import React from 'react';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { ModuleConcept, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectEntity, CBDIEditorProjectMessage } from 'misc/types/cbdiEdit/cbdiEditModel';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import DraggableEntityComponentItem, { ComponentListCommonProps } from './DraggableEntityComponentItem/DraggableEntityComponentItem';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  fontSize: '.9em',
  gap: 20,
  padding: 5,
});

const ComponentList = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  gap: 10,
  border: `1px solid ${theme.editor.detailView.textColor}`,
  padding: 4,
}));

const ListContainer = styled(List)({
  paddingBottom: 5,
  width: 'calc(100%-20px)',
  margin: '0 10px',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  paddingLeft: 5,
});

/* ----------------------------- EntityDetailComponentsView ----------------------------- */

/* ------------------------------- Interfaces ------------------------------- */
interface Props {
  entityModuleConcept: ModuleConcept;
}

export default function EntityDetailComponentsView({ entityModuleConcept: entityReferConcept }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;
  /* ------------------------------- Properties ------------------------------- */
  const currentProject = current!;
  /* ------------------------------ useRef hooks ------------------------------ */
  const messageComponentListContainer = React.useRef(null);
  const serviceComponentListContainer = React.useRef(null);
  /* ------------------------------ useMemo hooks ----------------------------- */
  const [entity, messageComponentList, messageReferConceptToAdd, serviceComponentList, serviceModuleConceptToAdd] = React.useMemo(() => {
    let mmessageComponentList: ModuleConcept[] = [];
    let mmessageModuleConceptToAdd: ModuleConcept[] = [];
    let mserviceComponentList: ModuleConcept[] = [];
    let mserviceModuleConceptToAdd: ModuleConcept[] = [];

    const mentity = getObjectByModuleConcept(currentProject, entityReferConcept) as CBDIEditorProjectEntity | undefined;
    if (mentity && mentity._mod !== Mod.Deletion) {
      mmessageComponentList = mentity.messages;
      mmessageModuleConceptToAdd = getAddingModuleConceptOptions(mmessageComponentList, 'messages', currentProject);
      // filter the message with ture component flag
      mmessageModuleConceptToAdd = mmessageModuleConceptToAdd.filter((item) => {
        const itemObj = getObjectByModuleConcept(currentProject, item) as CBDIEditorProjectMessage;
        if (itemObj.component) {
          return true;
        }
        return false;
      });
      mmessageModuleConceptToAdd = [...new Set(mmessageModuleConceptToAdd)];
      mmessageModuleConceptToAdd = sortModuleConceptList(mmessageModuleConceptToAdd, current!);

      mserviceComponentList = mentity.services;
      mserviceModuleConceptToAdd = getAddingModuleConceptOptions(mserviceComponentList, 'services', currentProject);
      mserviceModuleConceptToAdd = [...new Set(mserviceModuleConceptToAdd)];
      mserviceModuleConceptToAdd = sortModuleConceptList(mserviceModuleConceptToAdd, current!);
    }

    return [mentity, mmessageComponentList, mmessageModuleConceptToAdd, mserviceComponentList, mserviceModuleConceptToAdd];
  }, [currentProject, entityReferConcept]);
  /* -------------------------------- Callbacks ------------------------------- */
  const onToggleAgentComponent = () => {
    if (entity) {
      const newEntity: CBDIEditorProjectEntity = {
        ...entity,
        agent: !entity?.agent,
      };
      dispatch(updateObjects(newEntity));
    }
  };

  const onAddComponentItem = (componentType: 'message' | 'service', addingPlanModuleConceptStr: string) => {
    if (entity) {
      const addingModuleConcept = JSON.parse(addingPlanModuleConceptStr) as ModuleConcept;
      if (componentType === 'message') {
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          messages: [...entity.messages, addingModuleConcept],
        };
        dispatch(updateObjects(newEntity));
      } else {
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          services: [...entity.services, addingModuleConcept],
        };
        dispatch(updateObjects(newEntity));
      }
    }
  };

  const onRemoveComponentItem = (componentType: 'message' | 'service', deletingComponent: ModuleConcept) => {
    if (entity) {
      if (componentType === 'message') {
        const newComponentList = entity.messages.filter((item) => !areModuleConceptsEqual(item, deletingComponent));
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          messages: newComponentList,
        };
        dispatch(updateObjects(newEntity));
      } else {
        const newComponentList = entity.services.filter((item) => !areModuleConceptsEqual(item, deletingComponent));
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          services: newComponentList,
        };
        dispatch(updateObjects(newEntity));
      }
    }
  };

  const onReorderComponentList = (componentType: 'message' | 'service', newComponentList: ModuleConcept[]) => {
    if (entity) {
      if (componentType === 'message') {
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          messages: newComponentList,
        };
        dispatch(updateObjects(newEntity));
      } else {
        const newEntity: CBDIEditorProjectEntity = {
          ...entity,
          services: newComponentList,
        };
        dispatch(updateObjects(newEntity));
      }
    }
  };
  /* ------------------------------- Components ------------------------------- */

  return (
    <Root>
      <BooleanValueToggler currentValue={!!entity?.agent} onToggle={onToggleAgentComponent} label="AgentComponent" />
      <ComponentList>
        <TextView>Message Component List</TextView>
        <ListContainer
          ref={messageComponentListContainer}
          style={{
            overflow: messageComponentListContainer ? 'auto' : '',
            overflowX: 'hidden',
            maxHeight: messageComponentListContainer ? '200px' : '',
          }}
        >
          <DraggableList<ModuleConcept, ComponentListCommonProps, DraggableEntityComponentItem>
            list={messageComponentList}
            itemKey={(item) => item.uuid}
            template={DraggableEntityComponentItem}
            padding={5}
            onMoveEnd={(newList) => {
              onReorderComponentList('message', newList as ModuleConcept[]);
            }}
            container={() => (messageComponentListContainer ? messageComponentListContainer.current! : document.body)}
            commonProps={{
              project: currentProject,
              onRemoveItem: (deletingComponent: ModuleConcept) => {
                onRemoveComponentItem('message', deletingComponent);
              },
              immutable: false,
            }}
          />
        </ListContainer>
        <ThemedSelect
          hasborder="true"
          value=""
          onChange={(event: SelectChangeEvent<unknown>) => {
            onAddComponentItem('message', event.target.value as string);
          }}
        >
          <ThemedMenuItem disabled value="">
            Add a new message component
          </ThemedMenuItem>
          {messageReferConceptToAdd.map((mmoduleConcept: ModuleConcept, index: number) => {
            const mobject = getObjectByModuleConcept(currentProject, mmoduleConcept);
            const prefix = `${mmoduleConcept.module}::`;
            if (mobject && mobject._mod !== Mod.Deletion) {
              return (
                <ThemedMenuItem key={index as number} value={JSON.stringify(mmoduleConcept)}>
                  <span title={prefix + mobject.name}>{mobject.name}</span>
                </ThemedMenuItem>
              );
            }
            return null;
          })}
        </ThemedSelect>
      </ComponentList>
      <ComponentList>
        <TextView>Service Component List</TextView>
        <ListContainer
          ref={serviceComponentListContainer}
          style={{
            overflow: serviceComponentListContainer ? 'auto' : '',
            overflowX: 'hidden',
            maxHeight: serviceComponentListContainer ? '200px' : '',
          }}
        >
          <DraggableList<ModuleConcept, ComponentListCommonProps, DraggableEntityComponentItem>
            list={serviceComponentList}
            itemKey={(item) => item.uuid}
            template={DraggableEntityComponentItem}
            padding={5}
            onMoveEnd={(newList) => {
              onReorderComponentList('service', newList as ModuleConcept[]);
            }}
            container={() => (serviceComponentListContainer ? serviceComponentListContainer.current! : document.body)}
            commonProps={{
              project: currentProject,
              onRemoveItem: (deletingComponent: ModuleConcept) => {
                onRemoveComponentItem('service', deletingComponent);
              },
              immutable: false,
            }}
          />
        </ListContainer>
        <ThemedSelect
          hasborder="true"
          value=""
          onChange={(event: SelectChangeEvent<unknown>) => {
            onAddComponentItem('service', event.target.value as string);
          }}
        >
          <ThemedMenuItem disabled value="">
            Add a new service component
          </ThemedMenuItem>
          {serviceModuleConceptToAdd.map((mmoduleConcept: ModuleConcept, index: number) => {
            const mobject = getObjectByModuleConcept(currentProject, mmoduleConcept);
            const prefix = `${mmoduleConcept.module}::`;
            if (mobject && mobject._mod !== Mod.Deletion) {
              return (
                <ThemedMenuItem key={index as number} value={JSON.stringify(mmoduleConcept)}>
                  <span title={prefix + mobject.name}>{mobject.name}</span>
                </ThemedMenuItem>
              );
            }
            return null;
          })}
        </ThemedSelect>
      </ComponentList>
    </Root>
  );
}
