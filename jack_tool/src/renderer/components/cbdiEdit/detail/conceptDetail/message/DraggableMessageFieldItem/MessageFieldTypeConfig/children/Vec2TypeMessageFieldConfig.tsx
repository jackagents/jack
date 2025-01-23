import TextEdit from 'components/common/textEdit/TextEdit';
import {
  CBDIEditorTBasicMessageSchema,
  Vec2Type,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import { CBDIEditorOverrideMessageFieldSchema } from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  FieldTextView,
  FieldRow,
} from '../../styledComponents/StyledComponents';

interface Props {
  onVec2DefaultChange: (
    id: string,
    field: 'x' | 'y',
    defaultValue: string
  ) => void;
  immutable: boolean;
  item: CBDIEditorOverrideMessageFieldSchema;
  onToggleDefaultValid: (id: string) => void;
}

const Vec2TypeMessageFieldConfig = ({
  item,
  immutable,
  onVec2DefaultChange,
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
          <div
            style={{
              lineHeight: '27px',
              display: 'flex',
              flexDirection: 'column',
              gap: 20,
            }}
          >
            <div style={{ display: 'flex', flex: 1, gap: 10 }}>
              <div style={{ flexBasis: '22%', paddingLeft: 5 }}>x</div>
              <TextEdit
                inputType={CBDIEditorTBasicMessageSchema.F32Type}
                disabled={immutable}
                text={(item.default as Vec2Type).x}
                onDoneEditing={(text: string | number) =>
                  onVec2DefaultChange(item.id, 'x', text as string)
                }
              />
            </div>
            <div style={{ display: 'flex', flex: 1, gap: 10 }}>
              <div style={{ flexBasis: '22%', paddingLeft: 5 }}>y</div>
              <TextEdit
                inputType={CBDIEditorTBasicMessageSchema.F32Type}
                disabled={immutable}
                text={(item.default as Vec2Type).y}
                onDoneEditing={(text: string | number) =>
                  onVec2DefaultChange(item.id, 'y', text as string)
                }
              />
            </div>
          </div>
        </FieldRow>
      )}
    </>
  );
};

export default Vec2TypeMessageFieldConfig;
