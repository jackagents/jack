/* eslint-disable react/no-array-index-key */
import React from 'react';
import { Button, styled } from '@mui/material';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { FlexCol, FlexRow, Fluid, List } from 'components/common/base/BaseContainer';
import TextEdit from 'components/common/textEdit/TextEdit';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { Delete as DeleteIcon, AddCircleOutline as AddCircleOutlineIcon } from '@mui/icons-material';
import { v4 as uuid } from 'uuid';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedButton } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorRootConceptType, EnumField, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectEnum } from 'misc/types/cbdiEdit/cbdiEditModel';

/* --------------------------------- Styles --------------------------------- */
const ParamRoot = styled('div')({
  border: 'thin solid #ffffff96',
  marginBottom: 5,
  padding: '0 5px',
  fontSize: '.9em',
  listStyleType: 'none',
});

const FieldRow = styled(FlexRow)({
  width: '100%',
  marginBottom: 5,
});

const Header = styled(FlexRow)({
  width: '100%',
  height: 32,
  padding: '5px 0',
  borderBottom: 'thin solid #ffffff20',
});

const TextSlot = styled(FlexCol)({
  justifyContent: 'center',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

const FieldTextView = styled(TextView)({
  width: 45,
  minWidth: 55,
  lineHeight: '27px',
  marginRight: 5,
});

const IconSlot = styled(FlexCol)({
  minWidth: 22,
  width: '22px!important',
  justifyContent: 'center',
  alignItems: 'center',
});

const AddButton = styled(Button)(({ theme }) => ({
  width: '100%',
  border: `1px solid ${theme.editor.detailView.textColor}`,
  padding: 0,
  color: theme.editor.detailView.textColor,
}));

/* -------------------------------- Function -------------------------------- */
const getOtherNamesArr = (currentArr: (string | number)[], currentParamName: string | number) => {
  const index = currentArr.findIndex((name) => name === currentParamName);
  if (index > -1) {
    currentArr.splice(index, 1);
  }
  return currentArr;
};
/* --------------------------- ResourceDetailValue -------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
}

function EnumDetailValue({ moduleConcept }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */

  const onAddField = (oldEnumType: CBDIEditorProjectEnum) => {
    const newEnumType = copy(oldEnumType) as CBDIEditorProjectEnum;
    let lastField;
    if (newEnumType.fields.length > 0) {
      lastField = newEnumType.fields[newEnumType.fields.length - 1];
    }
    const field: EnumField = {
      name: `NewOption_${uuid().substring(0, 4)}`,
      value: lastField ? lastField.value + 1 : 0,
      note: '',
    };
    newEnumType.fields.push(field);

    dispatch(updateObjects(newEnumType));
  };

  const onRemoveField = (oldEnumType: CBDIEditorProjectEnum, index: number) => {
    const enumType = copy(oldEnumType) as CBDIEditorProjectEnum;
    enumType.fields.splice(index, 1);
    dispatch(updateObjects(enumType));
  };

  const onNameChange = (oldEnumType: CBDIEditorProjectEnum, index: number, name: string) => {
    const enumTypeField: EnumField = copy(oldEnumType.fields[index]);
    enumTypeField.name = name;
    const newEnumType: any = copy(oldEnumType) as CBDIEditorProjectEnum;
    newEnumType.fields[index] = enumTypeField;
    dispatch(updateObjects(newEnumType));
  };

  const onValueChange = (oldEnumType: CBDIEditorProjectEnum, index: number, value: string) => {
    const enumTypeField: EnumField = copy(oldEnumType.fields[index]);
    // eslint-disable-next-line radix
    enumTypeField.value = parseInt(value);
    const newEnumType: any = copy(oldEnumType) as CBDIEditorProjectEnum;
    newEnumType.fields[index] = enumTypeField;
    dispatch(updateObjects(newEnumType));
  };

  const onNoteChange = (oldEnumType: CBDIEditorProjectEnum, index: number, newNote: string) => {
    const enumTypeField: EnumField = copy(oldEnumType.fields[index]);
    enumTypeField.note = newNote;
    const newEnumType: any = copy(oldEnumType) as CBDIEditorProjectEnum;
    newEnumType.fields[index] = enumTypeField;
    dispatch(updateObjects(newEnumType));
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const [enumType, fieldNameArr, fieldValueArr] = React.useMemo(() => {
    const menumType = getObjectByModuleConcept(current!, moduleConcept) as CBDIEditorProjectEnum | undefined;
    let mfieldNameArr: string[] = [];
    let mfieldValueArr: number[] = [];
    if (menumType) {
      mfieldNameArr = menumType.fields.map((enumTypeField: EnumField) => enumTypeField.name);
      mfieldValueArr = menumType.fields.map((enumTypeField: EnumField) => enumTypeField.value);
    }
    return [menumType, mfieldNameArr, mfieldValueArr];
  }, [current, moduleConcept]);

  /* ------------------------------- Components ------------------------------- */
  if (!enumType) {
    return null;
  }
  return (
    <Fluid
      style={{
        padding: 4,
      }}
    >
      <AddButton
        style={{ margin: '8px 0' }}
        onClick={() => {
          onAddField(enumType);
        }}
      >
        <AddCircleOutlineIcon style={{ padding: 3 }} /> Add Option
      </AddButton>
      <List
        style={{
          overflow: 'auto',
          overflowX: 'hidden',
        }}
      >
        {enumType.fields.map((enumTypeField, index) => (
          <ParamRoot key={index}>
            <Header>
              <IconSlot>
                <img alt="" src={nodeIcon[CBDIEditorRootConceptType.EnumConceptType]} style={{ width: 22, height: 22, padding: 2 }} />
              </IconSlot>
              <TextSlot>
                <TextView>Option</TextView>
              </TextSlot>
              <ThemedButton
                onClick={() => {
                  onRemoveField(enumType, index);
                }}
              >
                <DeleteIcon style={{ padding: 2 }} />
              </ThemedButton>
            </Header>

            <FieldRow>
              <FieldTextView>Value</FieldTextView>
              <TextEdit
                text={enumTypeField.value}
                forbiddenValues={getOtherNamesArr(copy(fieldValueArr), enumTypeField.value)}
                onDoneEditing={(text: string | number) => onValueChange(enumType, index, text as string)}
              />
            </FieldRow>

            <FieldRow>
              <FieldTextView>Name</FieldTextView>
              <TextEdit
                text={enumTypeField.name}
                forbiddenValues={getOtherNamesArr(copy(fieldNameArr), enumTypeField.name)}
                onDoneEditing={(text: string | number) => onNameChange(enumType, index, text as string)}
              />
            </FieldRow>

            <FieldRow>
              <FieldTextView>Note</FieldTextView>
              <TextEdit
                text={enumTypeField.note}
                onDoneEditing={(text: string | number) => onNoteChange(enumType, index, text as string)}
                multiLine
              />
            </FieldRow>
          </ParamRoot>
        ))}
      </List>
    </Fluid>
  );
}

export default React.memo(EnumDetailValue);
