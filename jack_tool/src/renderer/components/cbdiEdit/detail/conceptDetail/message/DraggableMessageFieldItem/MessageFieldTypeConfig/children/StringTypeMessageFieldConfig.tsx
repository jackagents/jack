import TextEdit from 'components/common/textEdit/TextEdit';
import { CBDIEditorOverrideMessageFieldSchema } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorTBasicMessageSchema } from 'misc/types/cbdiEdit/cbdiEditTypes';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import {
  FieldRow,
  FieldTextView,
} from '../../styledComponents/StyledComponents';

interface Props {
  item: CBDIEditorOverrideMessageFieldSchema;
  onDefaultChange: (id: string, defaultValue: string) => void;
  immutable: boolean;
  onToggleDefaultValid: (id: string) => void;
}

const StringTypeMessageFieldConfig = ({
  item,
  onDefaultChange,
  onToggleDefaultValid,
  immutable,
}: Props) => {
  return (
    <>
      <BooleanValueToggler
        label="Default Valid"
        onToggle={() => {
          onToggleDefaultValid(item.id);
        }}
        currentValue={item.isDefaultValid}
      />
      {item.isDefaultValid && (
        <FieldRow>
          <FieldTextView>Default</FieldTextView>
          <TextEdit
            inputType={item.type as CBDIEditorTBasicMessageSchema}
            disabled={immutable}
            text={item.default as string}
            onDoneEditing={(text: string | number) =>
              onDefaultChange(item.id, text as string)
            }
          />
        </FieldRow>
      )}
    </>
  );
};

export default StringTypeMessageFieldConfig;
