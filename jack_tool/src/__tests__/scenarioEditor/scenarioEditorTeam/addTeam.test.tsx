import '@testing-library/jest-dom/extend-expect';
import { screen } from '@testing-library/react';
import LeftToolBar from 'components/common/scenarioEditor/leftToolBar/LeftToolBar';
import RightToolBar from 'components/common/scenarioEditor/rightToolBar/RightToolBar';
import ScenarioEditorTeamChart from 'components/common/scenarioEditor/scenarioEditorTeamChart/ScenarioEditorTeamChart';
import ScenarioEditorTeamsManager from 'components/common/scenarioEditor/scenarioEditorTeamsManager/ScenarioEditorTeamsManager';
import { EScenarioEditorTools } from 'misc/enum/scenarioEditor/scenarioEditorEnum';
import { AppStore } from 'projectRedux/Store';
import { renderWithProviders, initState } from 'root/mocks/testUtils/testUtils';

describe('Add new team', () => {
  it('add new pure team in team view', () => {
    const mStore: AppStore = renderWithProviders(
      <>
        <LeftToolBar />
        <ScenarioEditorTeamsManager />
        <ScenarioEditorTeamChart />
        <RightToolBar />
      </>,
      {
        context: { ...initState, currentTool: EScenarioEditorTools.TEAM },
      }
    ).reduxStore;

    expect(
      screen.getByRole('button', { name: 'Add new team' })
    ).toBeInTheDocument();
  });
});
