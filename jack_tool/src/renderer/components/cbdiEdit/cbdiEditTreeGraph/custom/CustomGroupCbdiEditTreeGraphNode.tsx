import { Handle, NodeProps, Position } from 'reactflow';
import { useTheme } from '@mui/material';
import { NodeData } from '../types';
import { CBDI_EDIT_TREE_GRAPH_FONT_SIZE, CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT, CBDI_EDIT_TREE_GRAPH_NODE_WIDTH } from '../constants';

function CustomGroupCbdiEditTreeGraphNode({ data }: NodeProps<NodeData>) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  return (
    <>
      <Handle type="target" position={Position.Top} />
      <div
        style={{
          color: theme.editor.graphView.textColor,
          position: 'absolute',
          left: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH / 4,
          top: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 4,
          fontSize: CBDI_EDIT_TREE_GRAPH_FONT_SIZE * 1.5,
          fontWeight: 'bolder',
        }}
      >
        {data.groupLabel}
      </div>
      <Handle type="source" position={Position.Bottom} />
    </>
  );
}

export default CustomGroupCbdiEditTreeGraphNode;
