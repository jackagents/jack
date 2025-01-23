import { combineReducers } from '@reduxjs/toolkit';
import { filterCbdiActions } from 'projectRedux/storeFilters/storeFilters';
import undoable from 'redux-undo';
import {
  cbdiEditCurrentClientSlice,
  CBDI_UNDO_TYPE,
  CBDI_REDO_TYPE,
  CBDI_CLEAR_HISTORY_TYPE,
} from './cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { cbdiEditSavedProjectClientSlice } from './cbdiEditSavedProjectClientReducer/cbdiEditSavedProjectClientReducer';

export const cbdiEditClientReducer = combineReducers({
  [cbdiEditCurrentClientSlice.name]: undoable(cbdiEditCurrentClientSlice.reducer, {
    limit: 10,
    filter: filterCbdiActions,
    undoType: CBDI_UNDO_TYPE,
    redoType: CBDI_REDO_TYPE,
    clearHistoryType: CBDI_CLEAR_HISTORY_TYPE,
    syncFilter: true,
  }),
  [cbdiEditSavedProjectClientSlice.name]: cbdiEditSavedProjectClientSlice.reducer,
});
