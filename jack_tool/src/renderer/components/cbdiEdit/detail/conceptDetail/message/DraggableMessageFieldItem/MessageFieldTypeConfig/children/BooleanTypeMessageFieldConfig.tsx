import { CBDIEditorOverrideMessageFieldSchema } from 'misc/types/cbdiEdit/cbdiEditModel';
import { SelectChangeEvent } from '@mui/material';
import {
  ThemedSelect,
  ThemedMenuItem,
} from 'components/cbdiEdit/themedComponents/ThemedComponents';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import {
  FieldRow,
  FieldTextView,
} from '../../styledComponents/StyledComponents';

interface Props {
  item: CBDIEditorOverrideMessageFieldSchema;
  onBooleanDefaultChange: (id: string, defaultValue: boolean) => void;
  immutable: boolean;
  onToggleDefaultValid: (id: string) => void;
}

const BooleanTypeMessageFieldConfig = ({
  item,
  immutable,
  onBooleanDefaultChange,
  onToggleDefaultValid,
}: Props) => {
  return (
    <>
      <BooleanValueToggler
        onToggle={() => {
          onToggleDefaultValid(item.id);
        }}
        label="Default Valid"
        currentValue={item.isDefaultValid}
      />
      {item.isDefaultValid && (
        <FieldRow>
          <FieldTextView>Default</FieldTextView>
          <ThemedSelect
            hasborder="true"
            disabled={immutable}
            value={item.default ? 'true' : 'false'}
            onChange={(event: SelectChangeEvent<unknown>) => {
              onBooleanDefaultChange(item.id, event.target.value === 'true');
            }}
          >
            <ThemedMenuItem value="" disabled>
              Please select the enum type
            </ThemedMenuItem>
            <ThemedMenuItem value="true">true</ThemedMenuItem>
            <ThemedMenuItem value="false">false</ThemedMenuItem>
          </ThemedSelect>
        </FieldRow>
      )}
    </>
  );
};

export default BooleanTypeMessageFieldConfig;
