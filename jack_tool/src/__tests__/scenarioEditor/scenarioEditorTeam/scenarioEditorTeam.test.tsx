import '@testing-library/jest-dom/extend-expect';
import { screen } from '@testing-library/react';
import { initState, renderWithProviders } from 'root/mocks/testUtils/testUtils';
import ScenarioEditorTeamsManager from 'components/common/scenarioEditor/scenarioEditorTeamsManager/ScenarioEditorTeamsManager';
import ScenarioEditorTeamChart from 'components/common/scenarioEditor/scenarioEditorTeamChart/ScenarioEditorTeamChart';
import { testSavedData } from 'root/mocks/exportData/testSavedData';
import { AppStore } from 'projectRedux/Store';
import { EScenarioEditorTools } from 'misc/enum/scenarioEditor/scenarioEditorEnum';

describe('Team view', () => {
  let mStore: AppStore | undefined;

  it('render scenarioEditor team view', () => {
    mStore = renderWithProviders(
      <>
        <ScenarioEditorTeamsManager />
        <ScenarioEditorTeamChart />
      </>,
      {
        context: { ...initState, currentTool: EScenarioEditorTools.TEAM },
      }
    ).reduxStore;

    expect(screen.getByTestId('rf__wrapper')).toBeInTheDocument();
  });

  it('render scenarioEditor team view with delivery robot model', () => {
    if (!mStore) {
      return;
    }

    renderWithProviders(
      <>
        <ScenarioEditorTeamsManager />
        <ScenarioEditorTeamChart />
      </>,
      {
        context: {
          ...initState,
          currentTool: EScenarioEditorTools.TEAM,
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

    expect(screen.getByText('WarehouseTeam')).toBeInTheDocument();

    expect(screen.getByText('MoonTrackTeam')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack01Platform')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack02Platform')).toBeInTheDocument();
    expect(screen.getByText('MoonTrackRepairer')).toBeInTheDocument();
    expect(screen.getByText('MoonTrackDeferToHuman')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack01Agent')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack02Agent')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack01Service')).toBeInTheDocument();
    expect(screen.getByText('MoonTrack02Service')).toBeInTheDocument();

    expect(screen.getByText('OzPostTeam')).toBeInTheDocument();
    expect(screen.getByText('OzPost01Platform')).toBeInTheDocument();
    expect(screen.getByText('OzPost02Platform')).toBeInTheDocument();
    expect(screen.getByText('OzPostRepairer')).toBeInTheDocument();
    expect(screen.getByText('OzPostDeferToHuman')).toBeInTheDocument();
    expect(screen.getByText('OzPost01Agent')).toBeInTheDocument();
    expect(screen.getByText('OzPost02Agent')).toBeInTheDocument();
    expect(screen.getByText('OzPost01Service')).toBeInTheDocument();
    expect(screen.getByText('OzPost02Service')).toBeInTheDocument();
  });
});
