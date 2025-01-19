import '@testing-library/jest-dom/extend-expect';
import { cleanup, fireEvent, screen } from '@testing-library/react';
import { BuiltinComponentNames } from 'components/common/scenarioEditor/builtIn/builtInComponents/BuiltInComponents';
import RightToolBar from 'components/common/scenarioEditor/rightToolBar/RightToolBar';
import ScenarioEditorTeamChart from 'components/common/scenarioEditor/scenarioEditorTeamChart/ScenarioEditorTeamChart';
import ScenarioEditorTeamsManager from 'components/common/scenarioEditor/scenarioEditorTeamsManager/ScenarioEditorTeamsManager';
import {
  getEntityByUuid,
  isScenarioEditorChildrenEntity,
} from 'components/common/scenarioEditor/utils/scenarioEditorUtils';
import { EScenarioEditorTools } from 'misc/enum/scenarioEditor/scenarioEditorEnum';
import { AppStore } from 'projectRedux/Store';
import { testSavedData } from 'root/mocks/exportData/testSavedData';
import { renderWithProviders, initState } from 'root/mocks/testUtils/testUtils';

describe('Edit team prop', () => {
  let mStore: AppStore = renderWithProviders(
    <>
      <ScenarioEditorTeamsManager />
      <ScenarioEditorTeamChart />
      <RightToolBar />
    </>,
    {
      context: { ...initState, currentTool: EScenarioEditorTools.TEAM },
    }
  ).reduxStore;

  beforeEach(cleanup);

  it('select team', () => {
    renderWithProviders(
      <>
        <ScenarioEditorTeamsManager />
        <ScenarioEditorTeamChart />
        <RightToolBar />
      </>,
      {
        context: {
          ...initState,
          currentTool: EScenarioEditorTools.TEAM,
          currentSelectedTeam: testSavedData.teams.currentTeams[0],
        },
        preloadedState: {
          scenarioEditor: {
            ...mStore.getState().scenarioEditor,
            present: {
              ...mStore.getState().scenarioEditor.present,
              scenarioEditorEntities: { entities: testSavedData.entities },
              scenarioEditorMap: { map: testSavedData.map },
              scenarioEditorEvents: { events: testSavedData.events },
              scenarioEditorTeams: { teams: testSavedData.teams },
            },
          },
        },
      }
    );

    expect(
      screen.getByText('WarehouseTeam (AgentComponent)')
    ).toBeInTheDocument();
    expect(screen.getByText('name')).toBeInTheDocument();
    expect(screen.getByText('templateName')).toBeInTheDocument();
    expect(screen.getByText('uuid')).toBeInTheDocument();
    expect(
      screen.getByDisplayValue('Warehouse Team Template')
    ).toBeInTheDocument();
    expect(mStore.dispatch).toBeCalledTimes(2);
  });

  it('edit team prop', () => {
    mStore = renderWithProviders(
      <>
        <ScenarioEditorTeamsManager />
        <ScenarioEditorTeamChart />
        <RightToolBar />
      </>,
      {
        context: {
          ...initState,
          currentTool: EScenarioEditorTools.TEAM,
          currentSelectedTeam: testSavedData.teams.currentTeams[0],
        },
        preloadedState: {
          scenarioEditor: {
            ...mStore.getState().scenarioEditor,
            present: {
              ...mStore.getState().scenarioEditor.present,
              scenarioEditorEntities: { entities: testSavedData.entities },
              scenarioEditorMap: { map: testSavedData.map },
              scenarioEditorEvents: { events: testSavedData.events },
              scenarioEditorTeams: { teams: testSavedData.teams },
            },
          },
        },
      }
    ).reduxStore;

    const templateNameElement: HTMLInputElement = screen.getByDisplayValue(
      'Warehouse Team Template'
    );
    const newValue = 'Test Template';

    fireEvent.change(templateNameElement, { target: { value: newValue } });
    fireEvent.blur(templateNameElement);

    expect(templateNameElement.value).toEqual(newValue);

    // 2 first times are from init
    // 3 later times are a batch from changing prop of team
    expect(mStore.dispatch).toHaveBeenCalledTimes(5);

    const entity = getEntityByUuid(
      testSavedData.entities.currentEntities,
      testSavedData.teams.currentTeams[0].entityUuid || ''
    );

    if (!entity || isScenarioEditorChildrenEntity(entity)) {
      return;
    }

    expect(mStore.dispatch).toHaveBeenCalledWith({
      type: 'scenarioEditorEntities/setEntity',
      payload: {
        ...entity,
        components: {
          AgentComponent: {
            ...entity.components[BuiltinComponentNames.AgentComponent],
            templateName: {
              ...entity.components[BuiltinComponentNames.AgentComponent]
                .templateName,
              value: newValue,
            },
          },
        },
      },
    });

    expect(mStore.dispatch).toHaveBeenCalledWith({
      type: 'scenarioEditorTeams/setCurrentTeams',
      payload: [
        { ...testSavedData.teams.currentTeams[0], templateName: newValue },
        ...testSavedData.teams.currentTeams.slice(1),
      ],
    });
  });
});
