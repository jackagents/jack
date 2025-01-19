import { SelectChangeEvent } from '@mui/material';
import { ThemedSelect, ThemedMenuItem } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { CBDIEditorCustomMessage, EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorOverrideMessageFieldSchema, CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { FieldRow, FieldTextView } from '../../styledComponents/StyledComponents';

interface Props {
  immutable: boolean;
  item: CBDIEditorOverrideMessageFieldSchema;
  allCustomOptions: ModuleConcept[];
  currentProject: CBDIEditorProject;
  onCustomModuleConceptChange: (id: string, referConceptString: string) => void;
}

function CustomTypeMessageFieldConfig({ item, immutable, currentProject, allCustomOptions, onCustomModuleConceptChange }: Props) {
  return (
    <FieldRow>
      <FieldTextView>Message</FieldTextView>
      <ThemedSelect
        hasborder="true"
        disabled={immutable}
        value={(item.type as CBDIEditorCustomMessage).Custom.uuid}
        onChange={(event: SelectChangeEvent<unknown>) => {
          onCustomModuleConceptChange(item.id, event.target.value as string);
        }}
      >
        <ThemedMenuItem value={EmptyModuleConcept.uuid}>Please select the custom type</ThemedMenuItem>
        {allCustomOptions.map((mreferConcept, mindex) => {
          const object = getObjectByModuleConcept(currentProject, mreferConcept);
          const prefix = `${mreferConcept.module}::`;

          if (object && object._mod !== Mod.Deletion) {
            return (
              <ThemedMenuItem key={mindex as number} value={mreferConcept.uuid}>
                <span title={prefix + object.name}>{object.name}</span>
              </ThemedMenuItem>
            );
          }
          return null;
        })}
      </ThemedSelect>
    </FieldRow>
  );
}

export default CustomTypeMessageFieldConfig;
