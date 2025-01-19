import '@testing-library/jest-dom/extend-expect';
import { cleanup, screen } from '@testing-library/react';
import BottomToolEvents from 'components/common/scenarioEditor/bottomToolBar/bottomToolEvents/BottomToolEvents';
import { initState, renderWithProviders } from 'root/mocks/testUtils/testUtils';
import {
  mockEvents,
  mockComponents,
} from 'root/mocks/mockTemplate/mockTemplate';
import { createEventMessage } from 'components/common/scenarioEditor/utils/scenarioEditorUtils';

describe('EventTimeline', () => {
  beforeEach(cleanup);

  it('render event timeline', () => {
    renderWithProviders(<BottomToolEvents />, {
      context: {
        ...initState,
        eventsTemplate: mockEvents,
        componentsTemplate: mockComponents,
      },
    });

    const reactFlowChart = screen.getByTestId(
      'react-flow-event-timeline-chart'
    );
    const button = screen.getByRole('button', { name: 'TestEvent' });

    expect(reactFlowChart).toBeInTheDocument();
    expect(button).toBeInTheDocument();
  });

  it('get the dispatchTime correctly', () => {
    const messages = createEventMessage('TestComponent', mockComponents);
    expect(messages).toHaveProperty('dispatchTime');
  });
});
