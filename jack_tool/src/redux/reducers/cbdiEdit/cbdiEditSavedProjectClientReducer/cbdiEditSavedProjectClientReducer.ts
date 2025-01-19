import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { CBDIEditorProject } from 'types/cbdiEdit/cbdiEditModel';

export interface CbdiEditSavedProjectClientState {
  saved: CBDIEditorProject | null;
}

const cbdiEditSavedProjectInitialState: CbdiEditSavedProjectClientState = {
  saved: null,
};

export const cbdiEditSavedProjectReducers = {
  setSaved: (
    state: CbdiEditSavedProjectClientState,
    action: PayloadAction<CBDIEditorProject | null>
  ) => {
    state.saved = action.payload;
  },
};

export const cbdiEditSavedProjectClientSlice = createSlice({
  name: 'cbdiEditSavedProject',
  initialState: cbdiEditSavedProjectInitialState,
  reducers: cbdiEditSavedProjectReducers,
});

export default cbdiEditSavedProjectClientSlice.reducer;

/* ************************************************************************************************
 * Actions
 * *********************************************************************************************** */
export const { actions: cbdiEditSavedProjectActions } =
  cbdiEditSavedProjectClientSlice;
