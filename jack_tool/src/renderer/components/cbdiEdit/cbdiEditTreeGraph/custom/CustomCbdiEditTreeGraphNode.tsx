import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { CBDIEditorOtherCategoryType, CBDIEditorRootConceptType, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import React from 'react';
import PriorityHighIcon from '@mui/icons-material/PriorityHigh';
import QuestionMarkIcon from '@mui/icons-material/QuestionMark';
import StarIcon from '@mui/icons-material/Star';
import { Handle, NodeProps, Position } from 'reactflow';
import { useCbdiReactflowContext } from 'components/cbdiEdit/CbdiEditReactflowContext/CbdiEditReactflowContext';
import { useTheme } from '@mui/material';
import {
  CBDI_EDIT_TREE_GRAPH_FONT_SIZE,
  CBDI_EDIT_TREE_GRAPH_NODE_GAP,
  CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
  CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
} from '../constants';
import { NodeData } from '../types';

function CustomCbdiEditTreeGraphNode({ data, id }: NodeProps<NodeData>) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  /* ----------------------------- useContext hooks ----------------------------- */
  const { selectedNodeId } = useCbdiReactflowContext();

  /* ------------------------------ useMemo hooks ----------------------------- */

  const [badgeIcon, badgeText] = React.useMemo(() => {
    if (data.nodeBadgeType) {
      switch (data.nodeBadgeType) {
        case 'exclamation':
          return [<PriorityHighIcon />, 'warning'];
        case 'question':
          return [<QuestionMarkIcon />, 'missing'];
        case 'star':
          return [<StarIcon />, 'startup'];
        case 'agent':
          return [
            <img
              style={{
                width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH / 2,
                height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 2,
              }}
              src={nodeIcon[CBDIEditorRootConceptType.AgentConceptType]}
              alt=""
            />,
            'Agent Component',
          ];
        default:
          return [null, ''];
      }
    }
    return [null, ''];
  }, [data.nodeBadgeType]);

  const customNodeIcon = React.useMemo(() => {
    let mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.TeamConceptType];

    if (data.customNodeIconType) {
      switch (data.customNodeIconType) {
        case 'beliefset':
          mcustomNodeIcon = nodeIcon.beliefset;
          break;

        case 'messageField':
          mcustomNodeIcon = nodeIcon[CBDIEditorOtherCategoryType.MessageFieldType];
          break;
        default:
          break;
      }
    } else {
      switch (data._objectType) {
        case CBDIEditorRootConceptType.TeamConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.TeamConceptType];
          break;

        case CBDIEditorRootConceptType.AgentConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.AgentConceptType];
          break;

        case CBDIEditorRootConceptType.RoleConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.RoleConceptType];
          break;

        case CBDIEditorRootConceptType.ActionConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.ActionConceptType];
          break;

        case CBDIEditorRootConceptType.GoalConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.GoalConceptType];
          break;

        case CBDIEditorRootConceptType.PlanConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.PlanConceptType];
          break;

        case CBDIEditorRootConceptType.TacticConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.TacticConceptType];
          break;

        case CBDIEditorRootConceptType.ResourceConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.ResourceConceptType];
          break;

        case CBDIEditorRootConceptType.MessageConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.MessageConceptType];
          break;

        case CBDIEditorRootConceptType.ServiceConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.ServiceConceptType];
          break;

        case CBDIEditorRootConceptType.EntityConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.EntityConceptType];
          break;

        case CBDIEditorRootConceptType.EventConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.EventConceptType];
          break;

        case CBDIEditorRootConceptType.EnumConceptType:
          mcustomNodeIcon = nodeIcon[CBDIEditorRootConceptType.EnumConceptType];
          break;

        default:
          break;
      }
    }
    return mcustomNodeIcon;
  }, [data._objectType, data.customNodeIconType]);

  const nodeLabel = React.useMemo(() => {
    let suffix = '';

    if (data._mod === Mod.Deletion) {
      suffix = ' (Missing)';
    }

    return `${data.name}${suffix}`;
  }, [data]);

  const nodeLabelColor = React.useMemo(() => {
    if (selectedNodeId === id) {
      return 'orange';
    }
    if (data._mod === Mod.Deletion) {
      return 'red';
    }
    return theme.editor.graphView.textColor;
  }, [data, selectedNodeId]);
  /* ------------------------------- Components ------------------------------- */
  return (
    <>
      <Handle type="target" position={Position.Top} id={Position.Top} />
      <Handle
        type="target"
        position={Position.Left}
        id={Position.Left}
        style={{
          top: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 2,
        }}
      />
      <div
        style={{
          width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
          display: 'flex',
          alignItems: 'center',
          flexDirection: 'column',
          gap: CBDI_EDIT_TREE_GRAPH_NODE_GAP,
          opacity: data.faded ? 0.4 : 1,
        }}
      >
        {/* badge */}
        {data.nodeBadgeType && (
          <div
            style={{
              position: 'absolute',
              top: 0,
              right: -CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 2,
              color: theme.editor.graphView.textColor,
            }}
            title={badgeText}
          >
            {badgeIcon}
          </div>
        )}

        {/* node image */}
        <img
          style={{
            width: CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
            height: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT,
          }}
          src={customNodeIcon}
          alt=""
          title={data.note}
        />
        {/* label */}
        <div
          className="nodeLabel"
          title={`${data.module}::${nodeLabel}`}
          style={{
            color: nodeLabelColor,
            textAlign: 'center',
            width: 2 * CBDI_EDIT_TREE_GRAPH_NODE_WIDTH,
            fontSize: CBDI_EDIT_TREE_GRAPH_FONT_SIZE,
            lineHeight: 1,
            height: 2 * CBDI_EDIT_TREE_GRAPH_FONT_SIZE,
            wordBreak: 'break-word',
            textOverflow: 'ellipsis',
            overflow: 'hidden',
            whiteSpace: 'normal',
            display: '-webkit-box',
            WebkitBoxOrient: 'vertical',
            WebkitLineClamp: 2,
          }}
        >
          {nodeLabel}
        </div>
      </div>
      <Handle type="source" position={Position.Bottom} id={Position.Bottom} />
      <Handle
        type="source"
        position={Position.Right}
        id={Position.Right}
        style={{
          top: CBDI_EDIT_TREE_GRAPH_NODE_HEIGHT / 2,
        }}
      />
    </>
  );
}

export default CustomCbdiEditTreeGraphNode;
