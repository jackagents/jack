/* eslint-disable react/no-array-index-key */
/* eslint-disable @typescript-eslint/naming-convention */
import React from 'react';
import { Between, Fluid } from 'components/common/base/BaseContainer';
import { styled, Stack } from '@mui/material';
import TextEdit from 'components/common/textEdit/TextEdit';
import { getObjectByModuleConcept, copy } from 'misc/utils/cbdiEdit/Helpers';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';

/* --------------------------------- Styles --------------------------------- */
const TextView = styled('div')({
  height: 25,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontSize: '.9em',
  lineHeight: '25px',
  paddingLeft: 5,
  marginRight: 5,
});

/* --------------------------- DetailUseMessagesAs -------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
  listItems: string[];
}

const queryList = ['Precondition', 'Drop Condition', 'Satisfied'];
function DetailUseMessagesAs({ moduleConcept: referConcept, listItems }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */
  const onToggleItem = (oldConcept: any, lower_item: string) => {
    const concept = copy(oldConcept);
    concept[lower_item] = !concept[lower_item];
    dispatch(updateObjects(concept));
  };

  const onToggleCustom = (oldConcept: any, lower_item: string) => {
    const concept = copy(oldConcept);
    concept[lower_item].custom = !concept[lower_item].custom;
    dispatch(updateObjects(concept));
  };

  const onQueryChange = (oldConcept: any, lower_item: string, newQueryText: string) => {
    const concept = copy(oldConcept);
    if (newQueryText) {
      const processedQueryText: string | undefined = newQueryText.replace(/\s+$/, '');
      concept[lower_item] = {
        ...concept[lower_item],
        query: processedQueryText,
      };
      dispatch(updateObjects(concept));
    } else {
      concept[lower_item] = { ...concept[lower_item], query: undefined };
      dispatch(updateObjects(concept));
    }
  };

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  const concept = React.useMemo(() => {
    const mconcept: any = getObjectByModuleConcept(current!, referConcept);

    return mconcept;
  }, [current, referConcept]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid sx={{ padding: 1 }}>
      {listItems
        .filter((item) => !queryList.includes(item))
        .map((item, index) => {
          const lower_item = item.replace(/ /g, '').toLowerCase();
          return (
            <BooleanValueToggler
              key={index as number}
              currentValue={concept[lower_item]}
              onToggle={() => {
                onToggleItem(concept, lower_item);
              }}
              label={item}
            />
          );
        })}
      {listItems
        .filter((item) => queryList.includes(item))
        .map((item, index) => {
          const lower_item = item.replace(/ /g, '').toLowerCase();

          return (
            <Stack key={index}>
              <TextView sx={{ fontSize: 16 }}>{item}</TextView>
              <div
                style={{
                  paddingLeft: 20,
                }}
              >
                <BooleanValueToggler
                  currentValue={concept[lower_item].custom}
                  onToggle={() => {
                    onToggleCustom(concept, lower_item);
                  }}
                  label="Custom"
                />
                <Between>
                  <TextView>Query &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TextView>
                  <TextEdit
                    text={concept[lower_item].query}
                    onDoneEditing={(text: string | number) => {
                      onQueryChange(concept, lower_item, text as string);
                    }}
                    multiLine
                  />
                </Between>
              </div>
            </Stack>
          );
        })}
    </Fluid>
  );
}

export default React.memo(DetailUseMessagesAs);
