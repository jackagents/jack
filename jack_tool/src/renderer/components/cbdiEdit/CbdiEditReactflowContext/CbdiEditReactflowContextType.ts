import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Viewport } from 'reactflow';

export type CbdiEditReactflowViewportDataDic = {
  [uuid: string]: Viewport;
};

export type TCbdiEditReactflowContextProps = {
  viewportDataDic: CbdiEditReactflowViewportDataDic;
  setViewportDataDic: React.Dispatch<React.SetStateAction<CbdiEditReactflowViewportDataDic>>;
  selectedNodeId: string | undefined;
  setSelectedNodeId: React.Dispatch<React.SetStateAction<string | undefined>>;
  draggingModuleConcept: ModuleConcept | null;
  setDraggingModuleConcept: React.Dispatch<React.SetStateAction<ModuleConcept | null>>;
};
