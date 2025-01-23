import { SelectChangeEvent } from '@mui/material';
import { ThemedSelect, ThemedMenuItem } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { ModuleConcept, Mod, EmptyModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useMemo } from 'react';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';

interface Props {
  moduleConceptOptions: ModuleConcept[];
  immutable: boolean;
  /**
   * if ture, meaning it is using currentModuleConcept as select's value
   * if false, meaning select's value is always empty string
   */
  isSelectingItem: boolean;
  emptyOptionLabel: string;
  currentModuleConcept?: ModuleConcept;
  hasborder?: string;
  onChange: (selectingModuleConcept: ModuleConcept) => void;
}

function ModuleCocneptSelector({
  moduleConceptOptions,
  immutable,
  emptyOptionLabel,
  hasborder,
  onChange,
  isSelectingItem,
  currentModuleConcept,
}: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  /* ------------------------------ useMemo hooks ----------------------------- */
  const value = useMemo(() => {
    if (isSelectingItem) {
      return currentModuleConcept?.uuid || EmptyModuleConcept.uuid;
    }
    return EmptyModuleConcept.uuid;
  }, [isSelectingItem, currentModuleConcept]);

  const isShowingMissingConceptItem = useMemo(() => {
    if (isSelectingItem && currentModuleConcept && !areModuleConceptsEqual(currentModuleConcept, EmptyModuleConcept)) {
      const obj = getObjectByModuleConcept(current!, currentModuleConcept);
      if (!obj || obj._mod === Mod.Deletion) {
        return true;
      }
    }
    return false;
  }, [current, currentModuleConcept, isSelectingItem]);
  /* -------------------------------- Callbacks ------------------------------- */
  const handleChange = (event: SelectChangeEvent<unknown>) => {
    const moduleConceptId = event.target.value as string;
    const selectingModuleConcept = moduleConceptOptions.find((el) => el.uuid === moduleConceptId) || EmptyModuleConcept;
    onChange(selectingModuleConcept);
  };

  return (
    <ThemedSelect hasborder={hasborder} disabled={immutable} value={value} onChange={handleChange}>
      <ThemedMenuItem value={EmptyModuleConcept.uuid}>{emptyOptionLabel}</ThemedMenuItem>
      {moduleConceptOptions.map((mmoduleConcept: ModuleConcept, index: number) => {
        const mobject = getObjectByModuleConcept(current!, mmoduleConcept);
        const prefix = `${mmoduleConcept.module}::`;

        if (mobject && mobject._mod !== Mod.Deletion) {
          return (
            <ThemedMenuItem key={index as number} value={mmoduleConcept.uuid}>
              <span title={prefix + mobject.name}>{mobject.name}</span>
            </ThemedMenuItem>
          );
        }
        return null;
      })}
      {isShowingMissingConceptItem && (
        <ThemedMenuItem value={currentModuleConcept!.uuid} disabled>
          <span className="editor-missing">{currentModuleConcept!.name}</span>
        </ThemedMenuItem>
      )}
    </ThemedSelect>
  );
}

export default ModuleCocneptSelector;
