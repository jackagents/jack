import React from 'react';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { Report as ReportIcon } from '@mui/icons-material';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorModuleConceptWithId, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DraggableItemWrapper from 'components/cbdiEdit/draggableItemWrapper/DraggableItemWrapper';

export interface PlanListCommonProps {
  project: CBDIEditorProject;
  onRemovePlanItem: (deletingId: string) => void;
}

interface PlanProps {
  item: CBDIEditorModuleConceptWithId;
  itemSelected: number;
  dragHandleProps: object;
  commonProps: PlanListCommonProps;
}

export default class PlanListItem extends React.Component<PlanProps> {
  getDragHeight = () => {
    return 30;
  };

  render() {
    const { item, itemSelected, dragHandleProps, commonProps } = this.props;
    const { project, onRemovePlanItem } = commonProps;

    const planObj = getObjectByModuleConcept(project, item.moduleConcept);
    const prefix = `${item.moduleConcept.module}::`;

    const isMissing = !planObj || planObj._mod === Mod.Deletion;
    return (
      <DraggableItemWrapper
        dragHandleProps={dragHandleProps}
        itemSelected={itemSelected}
        onRemoveItem={() => onRemovePlanItem(item.id)}
        styles={{
          backgroundColor: 'white',
          height: 30,
          justifyContent: 'center',
        }}
      >
        <div className={isMissing ? 'itemText-missing' : 'itemText'}>
          <ReportIcon />
          <div title={planObj ? prefix + planObj.name : prefix + item.moduleConcept.name}>{planObj ? planObj.name : item.moduleConcept.name}</div>
        </div>
      </DraggableItemWrapper>
    );
  }
}
