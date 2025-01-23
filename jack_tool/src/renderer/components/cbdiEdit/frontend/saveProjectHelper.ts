import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { copy } from 'misc/utils/cbdiEdit/Helpers';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { saveObject } from 'projectRedux/reducers/cbdiEdit/Helpers';

export const onSaveCbdiEdit = (
  current: CBDIEditorProject,
  saved: CBDIEditorProject,
  forceUpdate = false
) => {
  // if it is not forceUpdate and project current and saved is same, return
  if (!forceUpdate && areObjectsEqual(current, saved)) {
    return null;
  }

  const mCurrent: CBDIEditorProject = copy(current);

  Object.keys(mCurrent.cbdiObjects).forEach((id) => {
    const object = copy(mCurrent.cbdiObjects[id]);
    saveObject(mCurrent, object.uuid);
  });

  return {
    current: mCurrent,
    saved: mCurrent,
  };
};
