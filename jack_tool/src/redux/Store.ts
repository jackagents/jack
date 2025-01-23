import { configureStore, combineReducers, EmptyObject, PreloadedState } from '@reduxjs/toolkit';
import { cbdiEditClientReducer } from './reducers/cbdiEdit/cbdiEditClientReducer';

const reducers = combineReducers({
  cbdiEdit: cbdiEditClientReducer,
});

/**
 * Real application store
 */
export const store = configureStore({
  reducer: reducers,
  devTools: process.env.NODE_ENV !== 'production',
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: false,
      immutableCheck: false,
    }),
});

/**
 * setup store function to mock up redux store (for unit test)
 * @param preloadedState
 * @returns
 */
export const setupStore = (preloadedState?: PreloadedState<RootState>) =>
  configureStore({
    reducer: reducers,
    preloadedState,
    devTools: process.env.NODE_ENV !== 'production',
  });

export type RootState = ReturnType<typeof store.getState>;
export type RootStateWithoutEmpty = Omit<RootState, keyof EmptyObject>;
export type RootStateKeys = keyof RootStateWithoutEmpty;
export type AppStore = typeof store;
