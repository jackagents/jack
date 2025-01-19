/* eslint-disable react/jsx-no-constructed-context-values */
import React, { PropsWithChildren, useContext } from 'react';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CbdiEditReactflowViewportDataDic, TCbdiEditReactflowContextProps } from './CbdiEditReactflowContextType';

export const CbdiEditReactflowContext = React.createContext<TCbdiEditReactflowContextProps>({
  viewportDataDic: {},
  setViewportDataDic: () => {},
  selectedNodeId: undefined,
  setSelectedNodeId: () => {},
  draggingModuleConcept: null,
  setDraggingModuleConcept: () => {},
});

export default function CbdiEditReactflowContextProvider({ children }: PropsWithChildren) {
  const [viewportDataDic, setViewportDataDic] = React.useState<CbdiEditReactflowViewportDataDic>({});

  const [selectedNodeId, setSelectedNodeId] = React.useState<string | undefined>();

  const [draggingModuleConcept, setDraggingModuleConcept] = React.useState<ModuleConcept | null>(null);

  return (
    <CbdiEditReactflowContext.Provider
      value={{
        viewportDataDic,
        setViewportDataDic,
        selectedNodeId,
        setSelectedNodeId,
        draggingModuleConcept,
        setDraggingModuleConcept,
      }}
    >
      {children}
    </CbdiEditReactflowContext.Provider>
  );
}

// Create a custom hook to make it easier to use the context
export const useCbdiReactflowContext = () => useContext(CbdiEditReactflowContext);
