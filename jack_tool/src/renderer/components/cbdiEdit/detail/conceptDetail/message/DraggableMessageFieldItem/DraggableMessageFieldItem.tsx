import React, { Component } from 'react';
import { TextView } from 'components/common/base/BaseContainer';
import { Edit as EditIcon } from '@mui/icons-material';
import TextEdit from 'components/common/textEdit/TextEdit';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { ThemedButton, ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { CBDIEditorOverrideMessageFieldSchema, CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  ModuleConcept,
  CBDIEditorOtherCategoryType,
  CBDIEditorTBasicMessageSchema,
  CBDIEditorCustomMessage,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { FIELDTYPES } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import DraggableItemWrapper from 'components/cbdiEdit/draggableItemWrapper/DraggableItemWrapper';
import { FieldRow, CenterFlexCol, FieldTextView } from './styledComponents/StyledComponents';
import MessageFieldTypeConfig from './MessageFieldTypeConfig/MessageFieldTypeConfig';

export interface MessageFieldItemCommonProps {
  immutable: boolean;
  allCustomOptions: ModuleConcept[];
  allEnumOptions: ModuleConcept[];
  messageFieldNameArr: string[];
  currentProject: CBDIEditorProject;
  onRemoveParam: (id: string) => void;
  onNameChange: (id: string, name: string) => void;
  onTypeChange: (id: string, mtype: CBDIEditorTBasicMessageSchema | 'Custom' | 'Enum') => void;
  onDefaultChange: (id: string, defaultValue: string) => void;
  onVec2DefaultChange: (id: string, field: 'x' | 'y', defaultValue: string) => void;
  onCustomModuleConceptChange: (id: string, referConceptString: string) => void;
  onEnumTypeChange: (id: string, referConceptString: string) => void;
  onEnumDefaultChange: (id: string, enumDefaultValue: number) => void;
  onToggleHidden: (id: string) => void;
  onToggleArray: (id: string) => void;
  onNoteChange: (id: string, newNote: string) => void;
  onToggleDefaultValid: (id: string) => void;
  onBooleanDefaultChange: (id: string, defaultValue: boolean) => void;
}

interface Props {
  item: CBDIEditorOverrideMessageFieldSchema;
  itemSelected: number;
  dragHandleProps: object;
  commonProps: MessageFieldItemCommonProps;
}

interface State {
  isEditingName: boolean;
}

export default class MessageFieldItem extends Component<Props, State> {
  private nameInputRef: React.RefObject<HTMLInputElement>;

  constructor(props: Props) {
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

  getDragHeight = () => 50;

  render() {
    const { item, itemSelected, dragHandleProps, commonProps } = this.props;
    const {
      immutable,
      allCustomOptions,
      allEnumOptions,
      messageFieldNameArr,
      currentProject,
      onRemoveParam,
      onToggleHidden,
      onNameChange,
      onTypeChange,
      onDefaultChange,
      onVec2DefaultChange,
      onCustomModuleConceptChange,
      onEnumTypeChange,
      onEnumDefaultChange,
      onToggleArray,
      onNoteChange,
      onToggleDefaultValid,
      onBooleanDefaultChange,
    } = commonProps;

    const otherParamNames = messageFieldNameArr.filter((name) => name !== item.name);
    // item type string
    let itemTypeString: string;
    if (typeof item.type === 'object') {
      itemTypeString = (item.type as CBDIEditorCustomMessage).Custom ? 'Custom' : 'Enum';
    } else {
      itemTypeString = item.type;
    }

    return (
      <DraggableItemWrapper
        dragHandleProps={dragHandleProps}
        itemSelected={itemSelected}
        onRemoveItem={() => onRemoveParam(item.id)}
        immutable={immutable}
      >
        <FieldRow
          style={{
            height: 28,
            margin: 2,
            alignItems: 'center',
          }}
        >
          <img
            alt=""
            src={nodeIcon[CBDIEditorOtherCategoryType.MessageFieldType]}
            style={{
              display: 'block',
              width: 22,
              height: 22,
              padding: 2,
            }}
          />
          <CenterFlexCol style={{ flex: 1 }}>
            {this.state.isEditingName ? (
              <TextEdit
                disabled={immutable}
                text={item.name}
                forbiddenValues={otherParamNames}
                onDoneEditing={(text: string | number) => onNameChange(item.id, text as string)}
                onBlur={this.endEditName}
                autoFocus
              />
            ) : (
              <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
                <TextView>{this.props.item.name}</TextView>
                <ThemedButton disabled={immutable} onClick={this.startEditName}>
                  <EditIcon style={{ padding: 2 }} />
                </ThemedButton>
              </div>
            )}
          </CenterFlexCol>
        </FieldRow>
        <FieldRow>
          <FieldTextView>Type</FieldTextView>
          <div style={{ display: 'flex', gap: 20 }}>
            <div style={{ flex: 1 }}>
              <ThemedSelect
                hasborder="true"
                disabled={immutable}
                value={itemTypeString}
                onChange={(event) => {
                  onTypeChange(item.id, event.target.value as CBDIEditorTBasicMessageSchema | 'Custom' | 'Enum');
                }}
              >
                <ThemedMenuItem disabled value="">
                  Select a type
                </ThemedMenuItem>
                {FIELDTYPES.map((mtype, idx) => {
                  const typeName = mtype === 'Enum' ? 'Enumerator' : mtype;
                  return (
                    <ThemedMenuItem key={idx as number} value={mtype}>
                      {typeName}
                    </ThemedMenuItem>
                  );
                })}
              </ThemedSelect>
            </div>
            <div style={{ display: 'flex', flex: 0 }}>
              <BooleanValueToggler
                currentValue={item.is_array}
                onToggle={() => {
                  onToggleArray(item.id);
                }}
                disabled={immutable}
                label="Array"
              />
            </div>
          </div>
        </FieldRow>
        <MessageFieldTypeConfig
          item={item}
          immutable={immutable}
          allEnumOptions={allEnumOptions}
          currentProject={currentProject}
          allCustomOptions={allCustomOptions}
          onEnumDefaultChange={onEnumDefaultChange}
          onEnumTypeChange={onEnumTypeChange}
          onCustomModuleConceptChange={onCustomModuleConceptChange}
          onVec2DefaultChange={onVec2DefaultChange}
          onDefaultChange={onDefaultChange}
          onToggleDefaultValid={onToggleDefaultValid}
          onBooleanDefaultChange={onBooleanDefaultChange}
        />
        <BooleanValueToggler
          label="Hidden"
          onToggle={() => {
            onToggleHidden(item.id);
          }}
          currentValue={item.hidden}
        />
        <FieldRow>
          <FieldTextView>Note</FieldTextView>
          <TextEdit
            disabled={immutable}
            text={item.note}
            onDoneEditing={(text: string | number) => onNoteChange(item.id, text as string)}
            multiLine
          />
        </FieldRow>
      </DraggableItemWrapper>
    );
  }
}
