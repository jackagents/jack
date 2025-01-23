import React from 'react';
import { SelectChangeEvent, styled } from '@mui/material';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { FlexCol, FlexRow, Fluid } from 'components/common/base/BaseContainer';
import TextEdit from 'components/common/textEdit/TextEdit';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import {
  ThemedMenuItem,
  ThemedSelect,
} from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import {
  CBDIEditorTBasicMessageSchema,
  ModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectResource } from 'misc/types/cbdiEdit/cbdiEditModel';

/* --------------------------------- Styles --------------------------------- */
const FieldRow = styled(FlexRow)({
  width: '100%',
  justifyContent: 'space-between',
  '& :nth-of-type(2)': {
    flex: '0 0 80%',
  },
});

const TextSlot = styled(FlexCol)({
  justifyContent: 'center',
  paddingTop: 5,
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

const FieldTextView = styled(TextView)({
  width: 45,
  minWidth: 45,
  lineHeight: '27px',
  marginRight: 5,
});

/* --------------------------- ResourceDetailValue -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

function ResourceDetailValue({ moduleConcept: referConcept }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector(
    (state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current
  );
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */
  const onTypeChange = (
    oldRsc: CBDIEditorProjectResource,
    type: string | number
  ) => {
    const rsc = copy(oldRsc);
    rsc.type = type as string;

    dispatch(updateObjects(rsc));
  };

  const onMinChange = (
    oldRsc: CBDIEditorProjectResource,
    min: string | number
  ) => {
    const rsc = copy(oldRsc);
    rsc.min = Number(min) as number;

    dispatch(updateObjects(rsc));
  };

  const onMaxChange = (
    oldRsc: CBDIEditorProjectResource,
    max: string | number
  ) => {
    const rsc = copy(oldRsc);
    rsc.max = Number(max) as number;

    dispatch(updateObjects(rsc));
  };

  const rsc = getObjectByModuleConcept(
    current,
    referConcept
  ) as CBDIEditorProjectResource;
  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid
      style={{
        padding: 4,
      }}
    >
      <TextSlot>
        <FieldRow>
          <FieldTextView>Type</FieldTextView>
          <ThemedSelect
            hasborder="true"
            value={rsc.type || ''}
            onChange={(event: SelectChangeEvent<unknown>) => {
              onTypeChange(rsc, event.target.value as any);
            }}
          >
            <ThemedMenuItem disabled value="">
              Please select a type
            </ThemedMenuItem>
            <ThemedMenuItem value={CBDIEditorTBasicMessageSchema.I32Type}>
              Int 32
            </ThemedMenuItem>
          </ThemedSelect>
        </FieldRow>
      </TextSlot>
      <TextSlot>
        <FieldRow>
          <FieldTextView>Min</FieldTextView>
          <TextEdit
            inputType={CBDIEditorTBasicMessageSchema.I32Type}
            text={rsc.min}
            onDoneEditing={(text: string | number) => onMinChange(rsc, text)}
          />
        </FieldRow>
      </TextSlot>
      <TextSlot>
        <FieldRow>
          <FieldTextView>Max</FieldTextView>
          <TextEdit
            inputType={CBDIEditorTBasicMessageSchema.I32Type}
            text={rsc.max}
            onDoneEditing={(text: string | number) => onMaxChange(rsc, text)}
          />
        </FieldRow>
      </TextSlot>
    </Fluid>
  );
}

export default React.memo(ResourceDetailValue);
