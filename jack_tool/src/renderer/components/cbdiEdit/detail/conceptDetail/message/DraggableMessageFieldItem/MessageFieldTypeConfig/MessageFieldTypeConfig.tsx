import { CBDIEditorOverrideMessageFieldSchema, CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorTBasicMessageSchema, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import Vec2TypeMessageFieldConfig from './children/Vec2TypeMessageFieldConfig';
import StringTypeMessageFieldConfig from './children/StringTypeMessageFieldConfig';
import EnumTypeMessageFieldDefault from './children/EnumTypeMessageFieldConfig';
import CustomTypeMessageFieldConfig from './children/CustomTypeMessageFieldConfig';
import BooleanTypeMessageFieldConfig from './children/BooleanTypeMessageFieldConfig';

interface Props {
  item: CBDIEditorOverrideMessageFieldSchema;
  immutable: boolean;
  allEnumOptions: ModuleConcept[];
  allCustomOptions: ModuleConcept[];
  currentProject: CBDIEditorProject;
  onCustomModuleConceptChange: (id: string, referConceptString: string) => void;
  onVec2DefaultChange: (id: string, field: 'x' | 'y', defaultValue: string) => void;
  onDefaultChange: (id: string, defaultValue: string) => void;
  onEnumTypeChange: (id: string, referConceptString: string) => void;
  onEnumDefaultChange: (id: string, enumDefaultValue: number) => void;
  onToggleDefaultValid: (id: string) => void;
  onBooleanDefaultChange: (id: string, defaultValue: boolean) => void;
}

function MessageFieldTypeConfig({
  item,
  immutable,
  currentProject,
  allEnumOptions,
  allCustomOptions,
  onCustomModuleConceptChange,
  onVec2DefaultChange,
  onDefaultChange,
  onEnumTypeChange,
  onEnumDefaultChange,
  onToggleDefaultValid,
  onBooleanDefaultChange,
}: Props) {
  if (typeof item.type === 'object') {
    if ('Enum' in item.type) {
      return (
        <EnumTypeMessageFieldDefault
          item={item}
          immutable={immutable}
          allEnumOptions={allEnumOptions}
          currentProject={currentProject}
          onEnumDefaultChange={onEnumDefaultChange}
          onEnumTypeChange={onEnumTypeChange}
          onToggleDefaultValid={onToggleDefaultValid}
        />
      );
    }
    if ('Custom' in item.type) {
      return (
        <CustomTypeMessageFieldConfig
          item={item}
          immutable={immutable}
          allCustomOptions={allCustomOptions}
          currentProject={currentProject}
          onCustomModuleConceptChange={onCustomModuleConceptChange}
        />
      );
    }
  }
  switch (item.type) {
    case CBDIEditorTBasicMessageSchema.Vec2Type:
      return (
        <Vec2TypeMessageFieldConfig
          onVec2DefaultChange={onVec2DefaultChange}
          immutable={immutable}
          item={item}
          onToggleDefaultValid={onToggleDefaultValid}
        />
      );
    case CBDIEditorTBasicMessageSchema.BoolType:
      return (
        <BooleanTypeMessageFieldConfig
          immutable={immutable}
          item={item}
          onToggleDefaultValid={onToggleDefaultValid}
          onBooleanDefaultChange={onBooleanDefaultChange}
        />
      );
    default:
      return (
        <StringTypeMessageFieldConfig
          onDefaultChange={onDefaultChange}
          immutable={immutable}
          item={item}
          onToggleDefaultValid={onToggleDefaultValid}
        />
      );
  }
}

export default MessageFieldTypeConfig;
