import React from 'react';
import { getAllObjOptionsForSingleItem } from 'misc/utils/cbdiEdit/Helpers';
import { Edit as EditIcon } from '@mui/icons-material';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  CBDIEditorRootConceptType,
  CBDIEditorServiceTopicSchema,
  ModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import DraggableItemWrapper from 'components/cbdiEdit/draggableItemWrapper/DraggableItemWrapper';
import TextEdit from 'components/common/textEdit/TextEdit';
import { ThemedButton } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { TextView } from 'components/common/base/BaseContainer';
import ModuleCocneptSelector from 'components/cbdiEdit/detail/children/ModuleCocneptSelector/ModuleCocneptSelector';
import { FieldRow } from '../../message/DraggableMessageFieldItem/styledComponents/StyledComponents';

export interface ServiceTopicCommonProps {
  current: CBDIEditorProject;
  allTopics: CBDIEditorServiceTopicSchema[];
  onRemoveTopicItem: (deletingId: string) => void;
  onTopicNameChange: (
    oldTopic: CBDIEditorServiceTopicSchema,
    name: string,
  ) => void;
  onTopicMessageChange: (
    oldTopic: CBDIEditorServiceTopicSchema,
    message: ModuleConcept,
  ) => void;
}

interface ServiceTopicProps {
  item: CBDIEditorServiceTopicSchema;
  itemSelected: number;
  dragHandleProps: object;
  commonProps: ServiceTopicCommonProps;
}

interface State {
  isEditingName: boolean;
}

export default class ServiceTopicItem extends React.Component<
  ServiceTopicProps,
  State
> {
  private nameInputRef: React.RefObject<HTMLInputElement>;

  constructor(props: ServiceTopicProps) {
    super(props);
    this.nameInputRef = React.createRef();
    this.state = { isEditingName: false };
  }

  startEditName = () => {
    this.nameInputRef.current?.focus();
    this.setState({ isEditingName: true });
  };

  endEditName = () => {
    this.setState({ isEditingName: false });
  };

  getDragHeight = () => {
    return 50;
  };

  render() {
    const { item, itemSelected, dragHandleProps, commonProps } = this.props;
    const {
      current,
      allTopics,
      onRemoveTopicItem,
      onTopicNameChange,
      onTopicMessageChange,
    } = commonProps;

    const forbiddenValues = allTopics
      .map((el) => el.name)
      .filter((el) => el !== item.name);
    const allMessageOptions = getAllObjOptionsForSingleItem(
      CBDIEditorRootConceptType.MessageConceptType,
      current,
    );
    return (
      <DraggableItemWrapper
        dragHandleProps={dragHandleProps}
        itemSelected={itemSelected}
        onRemoveItem={() => onRemoveTopicItem(item.name)}
      >
        <FieldRow>
          <div>Name</div>
          {this.state.isEditingName ? (
            <TextEdit
              text={item.name}
              forbiddenValues={forbiddenValues}
              onDoneEditing={(text: string | number) =>
                onTopicNameChange(item, text as string)
              }
              onBlur={this.endEditName}
              autoFocus
            />
          ) : (
            <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
              <TextView>{this.props.item.name}</TextView>
              <ThemedButton onClick={this.startEditName}>
                <EditIcon style={{ padding: 2 }} />
              </ThemedButton>
            </div>
          )}
        </FieldRow>
        <FieldRow>
          <div>Message</div>
          <ModuleCocneptSelector
            moduleConceptOptions={allMessageOptions}
            immutable={false}
            isSelectingItem={true}
            currentModuleConcept={item.message}
            emptyOptionLabel="Select a message"
            onChange={(selectingModuleConcept) =>
              onTopicMessageChange(item, selectingModuleConcept)
            }
            hasborder="true"
          />
        </FieldRow>
      </DraggableItemWrapper>
    );
  }
}
