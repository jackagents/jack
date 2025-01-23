import { CBDIEditorObject, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Node } from 'reactflow';

export type NodeData = CBDIEditorObject & {
  /**
   * if node is faded
   */
  faded?: boolean;
  /**
   * true if not layout using dagre
   */
  isNotReLayout?: boolean;
  /**
   * groupId for group not layout using dagre
   */
  groupId?: string;
  /**
   * group label which display on the group node
   */
  groupLabel?: string;
  /**
   * node badge type
   */
  nodeBadgeType?: 'exclamation' | 'question' | 'star' | 'agent';
  /**
   * relativePosition
   */
  relativePosition?: 'left' | 'right';
  /**
   * relative node id
   */
  relativeNodeId?: string;
  /**
   * custom node icon type
   */
  customNodeIconType?: 'beliefset' | 'messageField' | 'enumItem';
  /**
   * group children node object types
   */
  childObjectTypes?: CBDIEditorRootConceptType[];
  /**
   * group node y distance from center node
   */
  yDistanceFromCenter?: number;
  /**
   * center node id for group node's position
   */
  centerNodeId?: string;
};

export type CbdiEditTreeGraphNode = Node<NodeData>;
