import '@testing-library/jest-dom/extend-expect';
import { fireEvent, screen } from '@testing-library/react';
import LeftToolBar from 'components/common/scenarioEditor/leftToolBar/LeftToolBar';
import {
  TScenarioEditorEntitiesClientState,
  TScenarioEditorMapClientState,
} from 'misc/types/scenarioEditor/seTypes';
import { initState, renderWithProviders } from 'root/mocks/testUtils/testUtils';
import {
  mockEntityTemplate,
  mockModules,
} from 'root/mocks/mockTemplate/mockTemplate';
import ScenarioMapView from 'components/common/scenarioEditor/scenarioMapView/ScenarioMapView';
import scenarioEditorMapClientReducer, {
  scenarioMapActions,
} from 'projectRedux/reducers/scenarioEditor/scenarioEditorMap/scenarioEditorMapClientReducer';
import scenarioEditorEntitiesClientReducer, {
  scenarioEntitiesActions,
} from 'projectRedux/reducers/scenarioEditor/scenarioEditorEntities/scenarioEditorEntitiesClientReducer';
import { createNewEntity } from 'components/common/scenarioEditor/utils/scenarioEditorUtils';
import { AppStore } from 'projectRedux/Store';
import {
  EBottomControlToolCheckboxes,
  EScenarioEditorMapType,
  EScenarioEditorTools,
} from 'misc/enum/scenarioEditor/scenarioEditorEnum';

const { setMap } = scenarioMapActions;
const { addNewEntity } = scenarioEntitiesActions;

const previousMapState: TScenarioEditorMapClientState = {
  map: {
    type: EScenarioEditorMapType.local,
  },
};

const previousEntitiesState: TScenarioEditorEntitiesClientState = {
  entities: { currentEntities: [] },
};

describe('add new entity from template', () => {
  let mStore: AppStore | undefined;
  const entity = createNewEntity(mockEntityTemplate, mockModules);

  it('add new physical entity', () => {
    scenarioEditorMapClientReducer(
      previousMapState,
      setMap({
        type: EScenarioEditorMapType.local,
        localCoordinate: { x: 0, y: 0 },
        origin: { lat: 200, lng: 200 },
      })
    );

    mStore = renderWithProviders(
      <>
        <LeftToolBar />
        <ScenarioMapView />
      </>,
      {
        context: {
          ...initState,
          currentTool: EScenarioEditorTools.ENTITIES,
          currentSelectedBottomCheckboxTool: EBottomControlToolCheckboxes.place,
          entitiesTemplate: [mockEntityTemplate],
          deserialisedModules: mockModules,
        },
      }
    ).reduxStore;

    expect(screen.getByText('MockTemplateEntity')).toBeInTheDocument();
    expect(screen.getByRole('presentation')).toBeInTheDocument();

    fireEvent.click(screen.getByText('MockTemplateEntity'));
    fireEvent.click(screen.getByRole('presentation'));

    expect(
      scenarioEditorEntitiesClientReducer(
        previousEntitiesState,
        addNewEntity(entity)
      )
    ).toEqual({
      entities: {
        currentEntities: [entity],
      },
    });

    // expect(mStore.dispatch).toHaveBeenCalledWith({
    //   type: 'scenarioEditor/addNewEntity',
    //   payload: { ...entity },
    // });
    expect(mStore.dispatch).toHaveBeenCalledTimes(1);
  });

  it('new entities in the current list on left menu', () => {
    if (!mStore) {
      return;
    }

    renderWithProviders(
      <>
        <LeftToolBar />
        <ScenarioMapView />
      </>,
      {
        context: {
          ...initState,
          currentTool: EScenarioEditorTools.ENTITIES,
        },
        preloadedState: {
          scenarioEditor: {
            ...mStore.getState().scenarioEditor,
            present: {
              ...mStore.getState().scenarioEditor.present,
              scenarioEditorEntities: {
                ...mStore.getState().scenarioEditor.present
                  .scenarioEditorEntities,
                entities: {
                  currentEntities: [entity],
                },
              },
            },
          },
        },
      }
    );

    fireEvent.click(screen.getByRole('tab', { name: 'Current' }));
    expect(screen.getByText('Current entities list')).toBeInTheDocument();
    expect(
      screen.getByRole('treeitem', { name: 'MockTemplateEntity' })
    ).toBeInTheDocument();
  });
});
