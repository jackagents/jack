// Unit test for scenario-editor-map reducer
import '@testing-library/jest-dom';
import { TScenarioEditorEntitiesClientState } from 'misc/types/scenarioEditor/seTypes';
import scenarioEditorEntitiesClientReducer, {
  scenarioEntitiesActions,
} from 'projectRedux/reducers/scenarioEditor/scenarioEditorEntities/scenarioEditorEntitiesClientReducer';
import { mockEntity } from 'root/mocks/mockEntity/mockEntity';

const {
  addNewEntity,
  removeEntity,
  resetEntities,
  setEntities,
  setEntity,
  setEntityPosition,
} = scenarioEntitiesActions;

describe('scenarioEditorEntities init', () => {
  it('returns the initial state', () => {
    expect(
      scenarioEditorEntitiesClientReducer(undefined, {
        type: undefined,
      })
    ).toMatchSnapshot();
  });
});

describe('scenarioEditorEntities setEntity', () => {
  it('set entity to new states', () => {
    const previousState: TScenarioEditorEntitiesClientState = {
      entities: {
        currentEntities: [mockEntity],
      },
    };

    expect(
      scenarioEditorEntitiesClientReducer(
        previousState,
        setEntity({
          ...mockEntity,
          templateName: 'set entity test',
        })
      )
    ).toEqual({
      entities: {
        currentEntities: [
          {
            ...mockEntity,
            templateName: 'set entity test',
          },
        ],
      },
    });
  });
});
