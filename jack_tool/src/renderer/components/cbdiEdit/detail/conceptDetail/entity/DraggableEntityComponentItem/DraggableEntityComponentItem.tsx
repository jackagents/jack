import React from 'react';
import { CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { Report as ReportIcon } from '@mui/icons-material';
import { ModuleConcept, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DraggableItemWrapper from 'components/cbdiEdit/draggableItemWrapper/DraggableItemWrapper';

export interface ComponentListCommonProps {
  project: CBDIEditorProject;
  immutable: boolean;
  onRemoveItem: (deletingComponent: ModuleConcept) => void;
}

interface ComponentProps {
  item: ModuleConcept;
  itemSelected: number;
  dragHandleProps: object;
  commonProps: ComponentListCommonProps;
}

export default class DraggableEntityComponentItem extends React.Component<ComponentProps> {
  getDragHeight = () => 30;

  render() {
    const { item, itemSelected, dragHandleProps, commonProps } = this.props;
    const { project, immutable, onRemoveItem } = commonProps;

    const messageObj = getObjectByModuleConcept(project, item);
    const prefix = `${item.module}::`;

    const isMissing = !messageObj || messageObj._mod === Mod.Deletion;
    return (
      <DraggableItemWrapper
        dragHandleProps={dragHandleProps}
        immutable={immutable}
        itemSelected={itemSelected}
        onRemoveItem={() => onRemoveItem(item)}
        styles={{
          backgroundColor: 'white',
          height: 40,
          justifyContent: 'center',
        }}
      >
        <div className={isMissing ? 'itemText-missing' : 'itemText'}>
          <ReportIcon />
          <div title={messageObj ? prefix + messageObj.name : prefix + item.name}>{messageObj ? messageObj.name : item.name}</div>
        </div>
      </DraggableItemWrapper>
    );
  }
}
