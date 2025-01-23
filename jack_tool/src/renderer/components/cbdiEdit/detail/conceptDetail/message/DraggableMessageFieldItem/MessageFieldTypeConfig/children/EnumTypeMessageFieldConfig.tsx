import { SelectChangeEvent } from '@mui/material';
import { ThemedSelect, ThemedMenuItem } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { CBDIEditorEnumMessage, EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorOverrideMessageFieldSchema, CBDIEditorProjectEnum, CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import { FieldRow, FieldTextView } from '../../styledComponents/StyledComponents';

interface Props {
  immutable: boolean;
  item: CBDIEditorOverrideMessageFieldSchema;
  allEnumOptions: ModuleConcept[];
  currentProject: CBDIEditorProject;
  onEnumTypeChange: (id: string, referConceptString: string) => void;
  onEnumDefaultChange: (id: string, enumDefaultValue: number) => void;
  onToggleDefaultValid: (id: string) => void;
}

function EnumTypeMessageFieldDefault({
  item,
  immutable,
  allEnumOptions,
  currentProject,
  onEnumDefaultChange,
  onEnumTypeChange,
  onToggleDefaultValid,
}: Props) {
  const enumTypeFields = (() => {
    if (item.type instanceof Object && 'Enum' in item.type) {
      const enumObj = getObjectByModuleConcept(currentProject, item.type.Enum) as CBDIEditorProjectEnum | undefined;
      if (enumObj && enumObj._mod !== Mod.Deletion) {
        return enumObj.fields;
      }
    }
    return [];
  })();

  return (
    <>
      <FieldRow>
        <FieldTextView>Enum</FieldTextView>
        <ThemedSelect
          hasborder="true"
          disabled={immutable}
          value={(item.type as CBDIEditorEnumMessage).Enum.uuid}
          onChange={(event: SelectChangeEvent<unknown>) => {
            onEnumTypeChange(item.id, event.target.value as string);
          }}
        >
          <ThemedMenuItem value={EmptyModuleConcept.uuid}>Please select the enum type</ThemedMenuItem>
          {allEnumOptions.map((enumTypeConcept, mindex) => {
            const object = getObjectByModuleConcept(currentProject, enumTypeConcept);
            const prefix = `${enumTypeConcept.module}::`;

            if (object && object._mod !== Mod.Deletion) {
              return (
                <ThemedMenuItem key={mindex as number} value={enumTypeConcept.uuid}>
                  <span title={prefix + object.name}>{object.name}</span>
                </ThemedMenuItem>
              );
            }
            return null;
          })}
        </ThemedSelect>
      </FieldRow>
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
          <ThemedSelect
            hasborder="true"
            disabled={immutable}
            value={item.default}
            defaultValue={0}
            onChange={(event: SelectChangeEvent<unknown>) => {
              onEnumDefaultChange(item.id, event.target.value as number);
            }}
          >
            <ThemedMenuItem value="" disabled>
              Please select the default type
            </ThemedMenuItem>
            {enumTypeFields.map((enumOption, mindex) => (
              <ThemedMenuItem key={mindex as number} value={enumOption.value}>
                <span>{enumOption.name}</span>
              </ThemedMenuItem>
            ))}
          </ThemedSelect>
        </FieldRow>
      )}
    </>
  );
}

export default EnumTypeMessageFieldDefault;
