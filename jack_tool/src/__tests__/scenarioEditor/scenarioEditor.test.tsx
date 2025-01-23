import '@testing-library/jest-dom/extend-expect';
import { cleanup, fireEvent, screen } from '@testing-library/react';
import TopToolBar from 'components/common/scenarioEditor/topToolBar/TopToolBar';
import LeftToolBar from 'components/common/scenarioEditor/leftToolBar/LeftToolBar';
import BottomToolBar from 'components/common/scenarioEditor/bottomToolBar/BottomToolBar';
import BuiltInEvents from 'components/common/scenarioEditor/builtIn/builtInEvents/BuiltInEvents';
import { initState, renderWithProviders } from 'root/mocks/testUtils/testUtils';
import RightToolBar from 'srcRoot/renderer/components/common/scenarioEditor/rightToolBar/RightToolBar';
import { testSavedData } from 'root/mocks/exportData/testSavedData';
import { mockEntity } from 'root/mocks/mockEntity/mockEntity';
import { EScenarioEditorTools } from 'misc/enum/scenarioEditor/scenarioEditorEnum';

describe('TopToolBar', () => {
  const setCurrentTool = jest.fn();

  beforeEach(cleanup);

  it('render scenarioEditor top menu', () => {
    renderWithProviders(<TopToolBar />, {
      context: { ...initState, setCurrentTool },
    });

    expect(screen.getByRole('button', { name: 'Map' })).toBeInTheDocument();
    expect(
      screen.getByRole('button', { name: 'Entities' })
    ).toBeInTheDocument();
    expect(
      screen.getByRole('button', { name: 'CBDI Team' })
    ).toBeInTheDocument();
    expect(screen.getByRole('button', { name: 'Events' })).toBeInTheDocument();

    fireEvent.click(screen.getByRole('button', { name: 'Entities' }));
    expect(setCurrentTool).toBeCalledWith(EScenarioEditorTools.ENTITIES);
  });
});

describe('LeftToolBar', () => {
  beforeEach(cleanup);

  it('render scenarioEditor left menu map view', () => {
    renderWithProviders(<LeftToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.MAP },
    });

    expect(screen.getByTestId('left-tool-bar').hasChildNodes()).toBeFalsy();
  });

  it('render scenarioEditor left menu events view', () => {
    renderWithProviders(<LeftToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.EVENTS },
    });

    expect(screen.getByTestId('left-tool-bar').hasChildNodes()).toBeFalsy();
  });

  it('render scenarioEditor left menu template list', () => {
    renderWithProviders(<LeftToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.ENTITIES },
    });

    expect(screen.getByRole('tab', { name: 'Add new' })).toBeInTheDocument();
    expect(screen.getByRole('tab', { name: 'Current' })).toBeInTheDocument();
    expect(screen.getByText('Template list')).toBeInTheDocument();
  });

  it('render scenarioEditor left menu current entities list', () => {
    renderWithProviders(<LeftToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.ENTITIES },
    });

    fireEvent.click(screen.getByRole('tab', { name: 'Current' }));
    expect(screen.getByText('Current entities list')).toBeInTheDocument();
  });
});

describe('BottomToolBar', () => {
  beforeEach(cleanup);

  it('render scenarioEditor bottom menu on map view', () => {
    renderWithProviders(<BottomToolBar />, {
      context: { ...initState },
    });

    expect(screen.getByRole('button', { name: 'Save' })).toBeInTheDocument();
    expect(screen.getByRole('button', { name: 'Cancel' })).toBeInTheDocument();
    expect(screen.getByRole('radiogroup')).toBeInTheDocument();
  });

  it('render scenarioEditor bottom menu on entities view', () => {
    renderWithProviders(<BottomToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.ENTITIES },
    });

    expect(screen.getByTestId('entity-title-bar')).toBeInTheDocument();
  });

  it('should not render scenarioEditor bottom menu on teams view', () => {
    renderWithProviders(<BottomToolBar />, {
      context: { ...initState, currentTool: EScenarioEditorTools.TEAM },
    });

    expect(screen.getByTestId('bottom-tool-bar').hasChildNodes()).toBeFalsy();
  });

  it('render scenarioEditor bottom menu on events view', () => {
    renderWithProviders(<BottomToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.EVENTS,
        eventsTemplate: [...BuiltInEvents],
      },
    });

    expect(screen.getByText('dispatchTime')).toBeInTheDocument();
    expect(
      screen.getByRole('button', { name: 'AgentPursue' })
    ).toBeInTheDocument();
    expect(
      screen.getByRole('button', { name: 'LinkAgent' })
    ).toBeInTheDocument();
  });
});

describe('RightToolBar', () => {
  beforeEach(cleanup);

  it('render scenarioEditor right menu on map view', () => {
    renderWithProviders(<RightToolBar />, {
      context: { ...initState },
    });

    expect(
      screen.getByTestId('right-toolbar-root').hasChildNodes()
    ).toBeFalsy();
  });

  it('render scenarioEditor right menu on entities view', async () => {
    renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.ENTITIES,
        currentSelectedExistEntity: mockEntity,
      },
    });

    expect(
      screen.getByTestId('right-toolbar-root').hasChildNodes()
    ).toBeTruthy();

    expect(
      screen
        .getByTestId('right-tool-entities-components-treeview')
        .hasChildNodes()
    ).toBeTruthy();

    const emptyComponentE = screen.getByRole('treeitem', {
      name: 'EmptyComponent',
    });
    expect(emptyComponentE).toBeInTheDocument();

    const testPropE = screen.queryByText('testProp');
    expect(testPropE).toBeNull();

    // Expand the component
    fireEvent.click(screen.getByTestId('ChevronRightIcon'));
    expect(screen.getByDisplayValue('testinput')).toBeInTheDocument();
    // Is true
    expect(screen.getByTestId('CheckBoxIcon')).toBeInTheDocument();

    // Click checkbox's input element
    fireEvent.click(
      screen.getByTestId('component-property-checkbox').children[0]
    );
    // Is false
    expect(screen.getByTestId('CheckBoxOutlineBlankIcon')).toBeInTheDocument();
  });

  it('should not render scenarioEditor right menu on teams view', () => {
    renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.TEAM,
        currentSelectedTeam: testSavedData.teams.currentTeams[0],
      },
    });

    expect(
      screen.getByTestId('right-tool-team-properties').hasChildNodes()
    ).toBeTruthy();
    expect(
      screen.getByTestId('right-tool-team-link-member')
    ).toBeInTheDocument();
  });

  it('render scenarioEditor right menu on events view', () => {
    renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.EVENTS,
        eventsTemplate: [...BuiltInEvents],
        currentEvent: testSavedData.events.currentEvents[0],
      },
    });

    expect(
      screen.getByTestId('right-tool-event-properties').hasChildNodes()
    ).toBeTruthy();
  });

  it('render scenarioEditor right menu on service view', () => {
    renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.TEAM,
        currentService: testSavedData.teams.services[0],
      },
    });

    expect(
      screen.getByTestId('right-tool-service-properties').hasChildNodes()
    ).toBeTruthy();
  });
});
