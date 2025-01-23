import React from 'react';
import { styled } from '@mui/material';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { CBDIEditorObject } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  width: 50,
  height: 50,
  borderRadius: 5,
  position: 'relative',
});

/* --------------------------------- Avatar --------------------------------- */
interface Props {
  selectedTreeNodeObj: CBDIEditorObject | undefined;
}

function Avatar({ selectedTreeNodeObj }: Props) {
  /* ------------------------------- Components ------------------------------- */
  if (!selectedTreeNodeObj) {
    return null;
  }
  return (
    <Root
      style={{ backgroundColor: nodeColor[selectedTreeNodeObj._objectType] }}
    >
      <img
        alt=""
        src={nodeIcon[selectedTreeNodeObj._objectType]}
        style={{ width: 50, height: 50, padding: 10 }}
      />
    </Root>
  );
}

export default React.memo(Avatar);
