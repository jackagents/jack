import React, { useCallback } from 'react';
import { useReactFlow, Node, Edge } from 'reactflow';
import { Delete, FileCopy } from '@mui/icons-material';
import { styled } from '@mui/material';
import { v4 } from 'uuid';
import { CONTEXT_MENU_HEIGHT, CONTEXT_MENU_WIDTH, NODE_FONT_SIZE } from '../reactFlowPlanEditorConstant';

/* --------------------------------- Styles --------------------------------- */

const ContextMenu = styled('div')({
  width: CONTEXT_MENU_WIDTH,
  height: CONTEXT_MENU_HEIGHT,
  background: 'white',
  borderStyle: 'solid',
  boxShadow: '10px 19px 20px rgba(0, 0, 0, 10%)',
  position: 'absolute',
  zIndex: 10,
  borderRadius: '5%',
  display: 'flex',
  flexDirection: 'column',
});

const ContextMenuButton = styled('button')({
  flex: 1,
  display: 'flex',
  alignItems: 'center',
  border: 'none',
  padding: '5%',
  width: '100%',
  borderRadius: 0,
  backgroundColor: 'transparent',
  gap: '5%',
  fontSize: NODE_FONT_SIZE,
  '&:hover': {
    backgroundColor: 'gray',
  },
});

/* ---------------------------------- Types --------------------------------- */
export interface ContextMenuProps {
  id: string;
  top?: number;
  left?: number;
  right?: number;
  bottom?: number;
}

interface Props extends ContextMenuProps {
  onClick: () => void;
  handleUpdatePlan: ({ newNodes, newEdges }: { newNodes?: Node[] | undefined; newEdges?: Edge[] | undefined }) => void;
}

export default function ReactFlowPlanEditorContextMenu({ id, top, left, right, bottom, onClick, handleUpdatePlan }: Props) {
  const { getNode, getNodes, getEdges } = useReactFlow();
  const duplicateNode = useCallback(() => {
    onClick();
    const nodes = getNodes();
    const node = getNode(id) as Node;
    const position = {
      x: node.position.x + node.width!,
      y: node.position.y + node.height!,
    };

    const duplicatedNode = {
      ...node,
      id: v4(),
      position,
      data: { ...node.data, updateTimestamp: Date.now() },
    };
    const newNodes = nodes.concat(duplicatedNode);
    handleUpdatePlan({ newNodes });
  }, [id, getNode, getNodes]);

  const deleteNode = useCallback(() => {
    onClick();
    const nodes = getNodes();
    const edges = getEdges();
    const newNodes = nodes.filter((node) => node.id !== id);
    const newEdges = edges.filter((edge) => edge.source !== id && edge.target !== id);
    handleUpdatePlan({ newNodes, newEdges });
  }, [id, getNodes, getEdges]);

  return (
    <ContextMenu
      style={{
        top,
        left,
        right,
        bottom,
      }}
    >
      <ContextMenuButton type="button" onClick={duplicateNode}>
        <FileCopy />
        Duplicate
      </ContextMenuButton>
      <ContextMenuButton type="button" onClick={deleteNode}>
        <Delete />
        Delete
      </ContextMenuButton>
    </ContextMenu>
  );
}
