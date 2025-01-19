import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { styled } from '@mui/material';
import React from 'react';
import { CustomNodeElementProps } from 'react-d3-tree';
import { CustomNodeData, useExplainabilityContext } from '../../../../context/explainabilityContext';
import TreeNodeUnit from './TreeNodeUnit';

export const EXPLAINABILITY_NODE_WIDTH = 300;
export const EXPLAINABILITY_NODE_HEIGHT = 250;

/* --------------------------------- Styles --------------------------------- */

const CustomNodeContainer = styled('div')({
  width: '100%',
  height: '100%',
  display: 'flex',
  flexDirection: 'column',
  borderRadius: '5%',
  overflow: 'hidden',
  border: '1px solid black',
  backgroundColor: '#fbfbfb',
});

const GroupNodeContainer = styled('div')({
  width: '100%',
  height: '100%',
  display: 'flex',
  justifyContent: 'center',
  flexWrap: 'wrap',
  overflowY: 'auto',
  flex: 1,
});

const NoDelegationPlaceHolder = styled('div')({
  width: '100%',
  height: '100%',
  fontSize: 40,
  color: 'gray',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
});

const ToggleButton = styled('span')({
  flex: 0,
  width: '100%',
  borderTop: '1px solid black',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  backgroundColor: '#efefef',
  zIndex: 9,
  fontSize: 24,
  padding: 5,
});

export default function AgentTreeCustomNode({ rd3tProps }: { rd3tProps: CustomNodeElementProps }) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { exploredTreeNodePaths, collapsedTreeNodePaths, setExploredTreeNodePaths, setCollapsedTreeNodePaths } = useExplainabilityContext();
  /* ------------------------------- Properties ------------------------------- */

  const { nodeDatum, toggleNode } = rd3tProps;

  const data = nodeDatum.attributes?.nodeData ? (JSON.parse(nodeDatum.attributes!.nodeData as string) as CustomNodeData) : undefined;
  const hasChildren = !!nodeDatum.attributes?.hasChildren;
  const nodePath = nodeDatum.attributes?.nodePath as string;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const collapsed = React.useMemo(() => {
    if (nodeDatum.children && nodeDatum.children.length > 0) {
      return nodeDatum.__rd3t.collapsed;
    }
    return true;
  }, [nodeDatum]);

  /* -------------------------------- Callbacks ------------------------------- */

  const handleToggleClick = () => {
    if (nodeDatum.children === undefined) {
      if (nodePath !== undefined && !exploredTreeNodePaths.includes(nodePath)) {
        setExploredTreeNodePaths((prev) => [...prev, nodePath]);
        setCollapsedTreeNodePaths((prev) => prev.filter((el) => el !== nodePath));
      }
    } else {
      if (collapsed === false && nodePath !== undefined && !collapsedTreeNodePaths.includes(nodePath)) {
        setCollapsedTreeNodePaths((prev) => [...prev, nodePath]);
        setExploredTreeNodePaths((prev) => prev.filter((el) => el !== nodePath));
      }
      toggleNode();
    }
  };

  if (data === undefined) {
    return null;
  }
  if (data.type === CBDIEditorRootConceptType.GoalConceptType) {
    return (
      <g>
        <foreignObject x={-EXPLAINABILITY_NODE_WIDTH * 0.75} width={EXPLAINABILITY_NODE_WIDTH * 1.5} height={EXPLAINABILITY_NODE_HEIGHT}>
          <CustomNodeContainer>
            <GroupNodeContainer>
              {data.children!.length > 0 ? (
                data.children?.map((child, index) => (
                  <div
                    key={index as number}
                    style={{
                      flex: '0 0 50%',
                      padding: 20,
                    }}
                  >
                    <TreeNodeUnit data={child} fontSize={24} type="children" />
                  </div>
                ))
              ) : (
                <NoDelegationPlaceHolder>There is no delegation</NoDelegationPlaceHolder>
              )}
            </GroupNodeContainer>
            <ToggleButton onClick={handleToggleClick}>{collapsed ? 'Expand' : 'Collapse'}</ToggleButton>
          </CustomNodeContainer>
        </foreignObject>
      </g>
    );
  }

  return (
    <g>
      <foreignObject x={-EXPLAINABILITY_NODE_WIDTH / 2} width={EXPLAINABILITY_NODE_WIDTH} height={EXPLAINABILITY_NODE_HEIGHT}>
        <CustomNodeContainer>
          <div
            style={{
              flexGrow: 1,
              overflow: 'hidden',
              textOverflow: 'ellipsis',
            }}
          >
            <TreeNodeUnit data={data} fontSize={40} type="root" />
          </div>
          {hasChildren ? (
            <ToggleButton
              onFocus={(e) => {
                e.preventDefault();
                e.stopPropagation();
              }}
              onClick={handleToggleClick}
            >
              {collapsed ? 'Expand' : 'Collapse'}
            </ToggleButton>
          ) : null}
        </CustomNodeContainer>
      </foreignObject>
    </g>
  );
}
