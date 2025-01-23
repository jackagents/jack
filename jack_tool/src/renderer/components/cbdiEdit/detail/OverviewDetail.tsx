import React, { useMemo } from 'react';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';

function OverviewDetail() {
  /* ---------------------------------- Redux --------------------------------- */
  const current: CBDIEditorProject | null = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);

  const projectInfoDic = useMemo(() => {
    const moduleName = selectedTreeNodeConcept!.module;
    if (!moduleName) {
      return current!.moduleProjectInfoDic;
    }
    return { [moduleName]: current!.moduleProjectInfoDic[moduleName] };
  }, [current, selectedTreeNodeConcept!]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: 5, padding: 10 }}>
      <h2>Project Info</h2>
      {Object.entries(projectInfoDic).map(([key, value]) => (
        <div style={{ display: 'flex', flexDirection: 'column', gap: 5 }} key={key}>
          <h3>{key}</h3>
          {/* TODO: update module name in overview when rename module */}
          {value &&
            Object.entries(value).map(([k, v]) => (
              <div key={key + k} style={{ display: 'flex', justifyContent: 'space-between' }}>
                <div>{k}</div>
                <div>{JSON.stringify(v)}</div>
              </div>
            ))}
        </div>
      ))}
    </div>
  );
}

export default OverviewDetail;
