// Unit test for scenario-editor-map reducer
import '@testing-library/jest-dom';
import { EScenarioEditorMapType } from 'misc/enum/scenarioEditor/scenarioEditorEnum';
import scenarioEditorMapClientReducer, {
  scenarioMapActions,
} from 'projectRedux/reducers/scenarioEditor/scenarioEditorMap/scenarioEditorMapClientReducer';

const {
  setMap,
  resetMap,
  setLocalCoordinateThroughInput,
  setMapOriginThroughInput,
  setMapOriginThroughMap,
  setMapType,
} = scenarioMapActions;

const previousState = {
  map: {
    type: EScenarioEditorMapType.local,
  },
};

describe('scenarioEditorMap init', () => {
  it('returns the initial state', () => {
    expect(
      scenarioEditorMapClientReducer(undefined, {
        type: undefined,
      })
    ).toMatchSnapshot();
  });
});

describe('scenarioEditorMap setMap', () => {
  it('set map to new states', () => {
    expect(
      scenarioEditorMapClientReducer(
        previousState,
        setMap({
          type: EScenarioEditorMapType.global,
          localCoordinate: { x: 100, y: 100 },
          origin: { lat: 200, lng: 200 },
        })
      )
    ).toEqual({
      map: {
        type: EScenarioEditorMapType.global,
        localCoordinate: { x: 100, y: 100 },
        origin: { lat: 200, lng: 200 },
      },
    });
  });
});

describe('scenarioEditorMap setMapType', () => {
  it('set map type', () => {
    expect(
      scenarioEditorMapClientReducer(
        previousState,
        setMapType(EScenarioEditorMapType.global)
      )
    ).toEqual({
      map: {
        type: EScenarioEditorMapType.global,
      },
    });
  });
});

describe('scenarioEditorMap setMapOriginThroughMap', () => {
  it('set map origin', () => {
    expect(
      scenarioEditorMapClientReducer(
        previousState,
        setMapOriginThroughMap({ lat: 350, lng: 92 })
      )
    ).toEqual({
      map: {
        type: EScenarioEditorMapType.local,
        origin: { lat: 350, lng: 92 },
      },
    });
  });
});

describe('scenarioEditorMap setMapOriginThroughInput', () => {
  it('set map origin', () => {
    expect(
      scenarioEditorMapClientReducer(
        previousState,
        setMapOriginThroughInput({ lat: 350, lng: 92 })
      )
    ).toEqual({
      map: {
        type: EScenarioEditorMapType.local,
        origin: { lat: 350, lng: 92 },
      },
    });
  });
});

describe('scenarioEditorMap setLocalCoordinateThroughInput', () => {
  it('set map origin', () => {
    expect(
      scenarioEditorMapClientReducer(
        previousState,
        setLocalCoordinateThroughInput({ x: 29, y: 43 })
      )
    ).toEqual({
      map: {
        type: EScenarioEditorMapType.local,
        localCoordinate: { x: 29, y: 43 },
      },
    });
  });
});

describe('scenarioEditorMap resetMap', () => {
  it('reset map to initial state', () => {
    scenarioEditorMapClientReducer(
      previousState,
      setMap({ type: EScenarioEditorMapType.global })
    );
    expect(resetMap()).toMatchSnapshot();
  });
});
