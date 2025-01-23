/* eslint-disable react/no-array-index-key */
import React from 'react';
import { Button, styled } from '@mui/material';
import { Fluid, List } from 'components/common/base/BaseContainer';
import { AddCircleOutline as AddCircleOutlineIcon } from '@mui/icons-material';
import { v4 as uuid } from 'uuid';
import { copy, getAllObjOptionsForSingleItem, getObjectByModuleConcept, sortModuleConceptList } from 'misc/utils/cbdiEdit/Helpers';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import DraggableList from 'react-draggable-list';
import { RootState } from 'projectRedux/Store';
import {
  ModuleConcept,
  CBDIEditorObject,
  Mod,
  CBDIEditorRootConceptType,
  CBDIEditorTBasicMessageSchema,
  Vec2Type,
  EmptyModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import {
  CBDIEditorOverrideMessageFieldSchema,
  CBDIEditorProjectAction,
  CBDIEditorProjectEntity,
  CBDIEditorProjectGoal,
  CBDIEditorProjectMessage,
} from 'misc/types/cbdiEdit/cbdiEditModel';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import SingleSelectDetailListView from './SingleSelectMessageDetailView';
import DraggableMessageFieldItem, { MessageFieldItemCommonProps } from '../conceptDetail/message/DraggableMessageFieldItem/DraggableMessageFieldItem';

/* --------------------------------- Styles --------------------------------- */

export const ComponentRow = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  margin: '5px 0',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

const StickyHeader = styled('div')(({ theme }) => ({
  position: 'sticky',
  top: 0,
  backgroundColor: theme.editor.detailView.bgColor,
}));

const AddButonContainer = styled('div')({
  margin: '8px 4px',
});

const AddButton = styled(Button)(({ theme }) => ({
  width: '100%',
  border: `1px solid ${theme.editor.detailView.textColor}`,
  padding: 0,
  color: theme.editor.detailView.textColor,
  '&.Mui-disabled': {
    color: theme.editor.detailView.disableColor,
  },
}));

/* ---------------------------- DetailParamsView ---------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
  type: 'action_req' | 'action_rpy' | 'goal_message' | 'message_message' | 'action_feedback';
}

function DetailParamsView({ moduleConcept, type }: Props) {
  /* ------------------------------ useRef hooks ------------------------------ */
  const messageFieldListRef = React.useRef(null);
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */

  /**
   * msg concept refers to the message concept in the goal/action
   * msgObj is the message object of moduleConcept
   */
  const [oldObj, msgConcept, msgObj, messageFieldNameArr] = React.useMemo(() => {
    const moldObj = getObjectByModuleConcept(current!, moduleConcept);
    let mmsgConcept: ModuleConcept | undefined;
    switch (type) {
      case 'action_req':
        mmsgConcept = (moldObj as CBDIEditorProjectAction).request;
        break;
      case 'action_rpy':
        mmsgConcept = (moldObj as CBDIEditorProjectAction).reply;
        break;
      case 'action_feedback':
        mmsgConcept = (moldObj as CBDIEditorProjectAction).feedback;
        break;
      case 'goal_message':
        mmsgConcept = (moldObj as CBDIEditorProjectGoal).message;
        break;
      case 'message_message':
        mmsgConcept = moduleConcept;
        break;
      default:
        break;
    }

    const mmsgObj = getObjectByModuleConcept(current!, mmsgConcept) as CBDIEditorProjectMessage;

    let mmessageFieldNameArr: string[] = [];
    if (mmsgObj) {
      mmessageFieldNameArr = mmsgObj.fields.map((messageField: CBDIEditorOverrideMessageFieldSchema) => messageField.name);
    }

    return [moldObj, mmsgConcept, mmsgObj, mmessageFieldNameArr];
  }, [current!, moduleConcept, type]);

  // get all custom options, and exclude the editing message itself
  const allCustomOptions = React.useMemo(() => {
    let mallCustomOptions = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.MessageConceptType, current!);
    mallCustomOptions = mallCustomOptions.filter((item) => !areModuleConceptsEqual(item, msgConcept));
    const sortedAllCustomOptions = sortModuleConceptList(mallCustomOptions, current!);
    return sortedAllCustomOptions;
  }, [current, msgConcept]);

  // get all enum options
  const allEnumOptions = React.useMemo(() => {
    const mallEnumOptions = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.EnumConceptType, current!);

    return mallEnumOptions;
  }, [current!]);

  /* -------------------------------- Callbacks ------------------------------- */

  const onToggleDefaultValid = (id: string) => {
    if (msgObj) {
      const msg = copy(msgObj) as CBDIEditorProjectMessage;
      const messageFieldIndex = msg.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        msg.fields[messageFieldIndex] = {
          ...msg.fields[messageFieldIndex],
          isDefaultValid: !msg.fields[messageFieldIndex].isDefaultValid,
        };
        dispatch(updateObjects(msg));
      }
    }
  };

  const onToggleComponent = () => {
    if (msgObj) {
      const updatingObjs: CBDIEditorObject[] = [];
      const msg = copy(msgObj) as CBDIEditorProjectMessage;
      msg.component = !msg.component;
      updatingObjs.push(msg);
      if (!msg.component) {
        current!.entities.forEach((entityModuleConcept: ModuleConcept) => {
          const entityObj = getObjectByModuleConcept(current!, entityModuleConcept) as CBDIEditorProjectEntity | undefined;
          if (entityObj && entityObj._mod !== Mod.Deletion) {
            if (entityObj.messages.some((item) => areModuleConceptsEqual(item, moduleConcept))) {
              const newEntity = copy(entityObj) as CBDIEditorProjectEntity;
              newEntity.messages = entityObj.messages.filter((item) => !areModuleConceptsEqual(item, moduleConcept));
              updatingObjs.push(newEntity);
            }
          }
        });
      }

      dispatch(updateObjects(updatingObjs));
    }
  };

  const onAddParam = () => {
    if (msgObj) {
      const msg: any = copy(msgObj) as CBDIEditorProjectMessage;
      const id = uuid();
      const messageField: CBDIEditorOverrideMessageFieldSchema = {
        id,
        name: `NewField_${id.substring(0, 4)}`,
        note: '',
        type: CBDIEditorTBasicMessageSchema.I8Type,
        is_array: false,
        default: '0',
        hidden: false,
        isDefaultValid: false,
      };
      msg.fields.push(messageField);
      dispatch(updateObjects(msg));
    }
  };

  const onRemoveParam = (id: string) => {
    if (msgObj) {
      const msg = copy(msgObj) as CBDIEditorProjectMessage;
      const messageFieldIndex = msg.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        msg.fields.splice(messageFieldIndex, 1);
        dispatch(updateObjects(msg));
      }
    }
  };

  const onNameChange = (id: string, name: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.name = name;
        const msg: any = copy(msgObj) as CBDIEditorOverrideMessageFieldSchema;
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onDefaultChange = (id: string, defaultValue: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.default = defaultValue.trim();
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onBooleanDefaultChange = (id: string, defaultValue: boolean) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.default = defaultValue;
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onVec2DefaultChange = (id: string, field: 'x' | 'y', defaultValue: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.default = {
          ...(messageField.default as Vec2Type),
          [field]: defaultValue,
        };
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onNoteChange = (id: string, newNote: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const processedNote = newNote.replace(/\s+$/, '');
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.note = processedNote;
        const msg: any = copy(msgObj) as CBDIEditorProjectMessage;
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onToggleArray = (id: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.is_array = !messageField.is_array;
        const msg = copy(msgObj) as CBDIEditorProjectMessage;
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onToggleHidden = (id: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);
        messageField.hidden = !messageField.hidden;
        const msg = copy(msgObj) as CBDIEditorProjectMessage;
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onTypeChange = (id: string, mtype: CBDIEditorTBasicMessageSchema | 'Custom' | 'Enum') => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField: CBDIEditorOverrideMessageFieldSchema = copy(msgObj.fields[messageFieldIndex]);

        // give default value if the type is not custom or enum
        switch (mtype) {
          case CBDIEditorTBasicMessageSchema.I8Type:
          case CBDIEditorTBasicMessageSchema.I16Type:
          case CBDIEditorTBasicMessageSchema.I32Type:
          case CBDIEditorTBasicMessageSchema.I64Type:
          case CBDIEditorTBasicMessageSchema.U8Type:
          case CBDIEditorTBasicMessageSchema.U16Type:
          case CBDIEditorTBasicMessageSchema.U32Type:
          case CBDIEditorTBasicMessageSchema.U64Type:
            messageField.default = '0';
            messageField.type = mtype;

            break;
          case CBDIEditorTBasicMessageSchema.F32Type:
          case CBDIEditorTBasicMessageSchema.F64Type:
            messageField.default = '0.0';
            messageField.type = mtype;

            break;
          case CBDIEditorTBasicMessageSchema.Vec2Type:
            messageField.type = mtype;
            messageField.default = {
              x: 0,
              y: 0,
            };
            break;
          case CBDIEditorTBasicMessageSchema.BoolType:
            messageField.type = mtype;
            messageField.default = false;
            break;
          case CBDIEditorTBasicMessageSchema.StringType:
            messageField.type = mtype;
            messageField.default = '';
            break;
          case 'Enum':
            messageField.type = { Enum: EmptyModuleConcept };
            messageField.default = '';
            break;
          case 'Custom':
            messageField.type = { Custom: EmptyModuleConcept };
            messageField.default = '';
            break;
          default:
            break;
        }
        const msg = copy(msgObj) as CBDIEditorProjectMessage;

        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onCustomModuleConceptChange = (id: string, moduleConceptId: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField = copy(msgObj.fields[messageFieldIndex]) as CBDIEditorOverrideMessageFieldSchema;
        const mmoduleConcept: ModuleConcept = allCustomOptions.find((el) => el.uuid === moduleConceptId) || EmptyModuleConcept;
        messageField.type = { Custom: mmoduleConcept };
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onEnumTypeChange = (id: string, moduleConceptId: string) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const messageField = copy(msgObj.fields[messageFieldIndex]) as CBDIEditorOverrideMessageFieldSchema;
        const mmoduleConcept: ModuleConcept = allEnumOptions.find((el) => el.uuid === moduleConceptId) || EmptyModuleConcept;
        messageField.type = { Enum: mmoduleConcept };
        messageField.default = '';
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onEnumDefaultChange = (id: string, enumDefaultValue: number) => {
    if (msgObj) {
      const messageFieldIndex = msgObj.fields.findIndex((item) => item.id === id);
      if (messageFieldIndex > -1) {
        const msg: CBDIEditorProjectMessage = copy(msgObj);
        const messageField = copy(msgObj.fields[messageFieldIndex]) as CBDIEditorOverrideMessageFieldSchema;
        messageField.default = enumDefaultValue;
        msg.fields[messageFieldIndex] = messageField;
        dispatch(updateObjects(msg));
      }
    }
  };

  const onReorderParamList = (newParamList: CBDIEditorOverrideMessageFieldSchema[]) => {
    if (msgObj) {
      const msg: CBDIEditorProjectMessage = copy(msgObj);
      msg.fields = newParamList;
      dispatch(updateObjects(msg));
    }
  };

  /* ------------------------------- Components ------------------------------- */

  return (
    <Fluid>
      <StickyHeader>
        {/* the selector for goal/action */}
        {type !== 'message_message' ? <SingleSelectDetailListView oldObj={oldObj!} conceptType={type} currentConcept={msgConcept!} /> : null}
        <div style={{ padding: 5 }}>
          <BooleanValueToggler onToggle={onToggleComponent} label="Component" currentValue={msgObj?.component} />
          <TextView>Fields</TextView>
        </div>
        {msgObj ? (
          <AddButonContainer>
            <AddButton
              onClick={() => {
                onAddParam();
              }}
            >
              <AddCircleOutlineIcon style={{ padding: 3 }} /> Add Field
            </AddButton>
          </AddButonContainer>
        ) : null}
      </StickyHeader>
      <List
        ref={messageFieldListRef}
        style={{
          overflow: messageFieldListRef ? 'auto' : '',
          overflowX: 'hidden',
        }}
      >
        {msgObj ? (
          <DraggableList<CBDIEditorOverrideMessageFieldSchema, MessageFieldItemCommonProps, DraggableMessageFieldItem>
            itemKey="id"
            template={DraggableMessageFieldItem}
            list={msgObj.fields}
            springConfig={{ stiffness: 1200, damping: 50 }}
            onMoveEnd={(newList) => {
              onReorderParamList(newList as CBDIEditorOverrideMessageFieldSchema[]);
            }}
            padding={10}
            container={() => (messageFieldListRef ? messageFieldListRef.current! : document.body)}
            commonProps={{
              immutable: false,
              allCustomOptions,
              allEnumOptions,
              messageFieldNameArr,
              currentProject: current!,
              onRemoveParam,
              onNameChange,
              onTypeChange,
              onDefaultChange,
              onVec2DefaultChange,
              onCustomModuleConceptChange,
              onEnumTypeChange,
              onEnumDefaultChange,
              onToggleArray,
              onToggleHidden,
              onNoteChange,
              onToggleDefaultValid,
              onBooleanDefaultChange,
            }}
          />
        ) : null}
      </List>
    </Fluid>
  );
}

export default React.memo(DetailParamsView);
