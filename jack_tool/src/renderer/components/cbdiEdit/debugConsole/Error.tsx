import { styled } from '@mui/material';
import React from 'react';
import { IModelError } from 'types/cbdiEdit/cbdiEditModel';
import { FlexCol, FlexRow, List } from 'components/common/base/BaseContainer';
import { Error as ErrorIcon, ErrorOutline as ErrorOutlineIcon } from '@mui/icons-material';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */
const ListItem = styled('li')({
  whiteSpace: 'nowrap',
});

const ListRowHover = styled(ListItem)({
  '&:hover': {
    backgroundColor: '#e09e58',
  },
});

const IconSlot = styled(FlexCol)({
  marginLeft: 5,
  marginRight: 5,
  justifyContent: 'center',
  alignItems: 'center',
});

const ContentSlot = styled(FlexCol)({
  justifyContent: 'center',
});

const ContentSlotHover = styled('span')({
  cursor: 'pointer',
  flexFlow: 'column nowrap',
  justifyContent: 'center',
  flex: '0 1 auto',
  display: 'flex',
  '&:hover': {
    textDecoration: 'underline',
  },
});

const ContentSlotHelpText = styled('span')({
  flexFlow: 'column nowrap',
  justifyContent: 'center',

  alignItems: 'flex-end',
  flex: '1 1 auto',
  display: 'flex',
});
/* ---------------------------------- Error --------------------------------- */

interface Props {
  heightPx: number;
  error: IModelError;
}

function Error({ heightPx, error }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { setSelectedTreeNodeConcept, setGraphSelectedNode, setErrorConceptType } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */
  const onClick = (_e: React.MouseEvent<HTMLSpanElement, MouseEvent>, errorObjModuleConcepts: ModuleConcept[], conceptType: string) => {
    // TODO
    // select tree node concept and graph node concept
    dispatch(setSelectedTreeNodeConcept(errorObjModuleConcepts[0]));
    dispatch(setGraphSelectedNode(errorObjModuleConcepts[0]));
    dispatch(setErrorConceptType(conceptType));
  };

  /* ------------------------------- Components ------------------------------- */

  return (
    <List>
      <FlexRow>
        <IconSlot
          style={{
            width: heightPx,
            maxWidth: heightPx,
            minWidth: heightPx,
          }}
        >
          <ErrorOutlineIcon
            style={{
              fontSize: heightPx * 0.8,
            }}
          />
        </IconSlot>
        <ContentSlot style={{ height: heightPx }}>
          {error.error_type}: {error.error_string}
        </ContentSlot>
      </FlexRow>
      <ListRowHover style={{ paddingLeft: 25 }}>
        {error.keys.map((path: string, index: number) => {
          const errors = path.split('.');
          const firstMsg = errors[0];
          const conceptType = firstMsg.charAt(0).toUpperCase() + errors[0].slice(1);
          const errorObjName = errors[1];
          const errorObjModuleConcepts: ModuleConcept[] = [];
          const currentCbdiObjects = current?.cbdiObjects;

          if (currentCbdiObjects) {
            Object.keys(currentCbdiObjects).forEach((key) => {
              if (currentCbdiObjects[key].name === errorObjName) {
                errorObjModuleConcepts.push({
                  uuid: key,
                  module: '',
                  name: errorObjName,
                });
              }
            });
          }
          const key = index;
          return (
            <FlexRow key={key}>
              <ErrorIcon
                style={{
                  fontSize: heightPx,
                  padding: 5,
                  color: 'red',
                }}
              />
              <ContentSlotHover
                className="errorMsg"
                onClick={(e) => {
                  onClick(e, errorObjModuleConcepts, conceptType);
                }}
                style={{
                  height: heightPx,
                }}
              >
                {path}
              </ContentSlotHover>
              <ContentSlotHelpText>click message to track the error &nbsp;&nbsp;</ContentSlotHelpText>
            </FlexRow>
          );
        })}
      </ListRowHover>
    </List>
  );
}

export default React.memo(Error);
