import React from 'react';
import { styled, Tooltip } from '@mui/material';
import {
  ArrowDropDown as ExpandIcon,
  ArrowDropUp as CollapseIcon,
  CancelOutlined as ErrorIcon,
  ReportProblemOutlined as WarningIcon,
} from '@mui/icons-material';
import { FlexRow } from 'components/common/base/BaseContainer';
import { capitalize } from 'misc/utils/cbdiEdit/Helpers';

/* ************************************************************************************************
 * DetailField Styles
 * *********************************************************************************************** */
const Root = styled('div')({
  width: '100%',
  marginBottom: 10,
  position: 'relative',
});

const Title = styled('div')({
  width: '100%',
  height: 25,
  marginBottom: 3,
  whiteSpace: 'nowrap',
});

const TitleLeft = styled(FlexRow)({
  alignItems: 'left',
});

const TitleRight = styled('div')({
  position: 'absolute',
  top: 0,
  right: 0,
  width: 22,
  height: 22,
});

const RightSideButton = styled('div')(({ theme }) => ({
  height: '100%',
  width: '100%',
  color: theme.editor.detailView.textColor,
  cursor: 'pointer',
}));

const FieldContent = styled('div')(({ theme }) => ({
  width: '100%',
  minHeight: 22,
  margin: 3,
  border: `1px solid ${theme.editor.detailView.textColor}`,
}));

const TextView = styled('div')({
  height: 25,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  lineHeight: '25px',
  marginRight: 5,
});

const CollapsedView = styled('div')({
  width: '100%',
  height: 2,
  borderTop: 'thin solid #777777',
});

/* ************************************************************************************************
 * DetailField
 * *********************************************************************************************** */
interface Props {
  fieldName: string;
  tooltipText?: string;
  content: JSX.Element;
}

interface States {
  isExpanded: boolean;
  hasErrors: boolean;
  hasWarnings: boolean;
}

export default class DetailField extends React.Component<Props, States> {
  /* ********************************************************************************************
   * Constructor
   * ******************************************************************************************* */
  constructor(props: Props) {
    super(props);

    this.state = {
      isExpanded: true,
      hasErrors: false,
      hasWarnings: false,
    };
  }

  /* ********************************************************************************************
   * Callbacks
   * ******************************************************************************************* */
  onToggleExpand = () => {
    const { isExpanded } = this.state;
    this.setState({
      isExpanded: !isExpanded,
    });
  };

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  render() {
    return (
      <Root>
        <Title>
          <TitleLeft>
            {this.props.tooltipText ? (
              <Tooltip
                title={
                  <span style={{ fontSize: 14 }}>{this.props.tooltipText}</span>
                }
                placement="right"
                arrow
              >
                <TextView>{capitalize(this.props.fieldName)}</TextView>
              </Tooltip>
            ) : (
              <TextView>{capitalize(this.props.fieldName)}</TextView>
            )}

            {this.state.hasErrors ? (
              <ErrorIcon style={{ color: 'red' }} />
            ) : undefined}
            {this.state.hasWarnings ? (
              <WarningIcon style={{ color: 'yellow' }} />
            ) : undefined}
          </TitleLeft>
          <TitleRight>
            <RightSideButton onClick={this.onToggleExpand}>
              {this.state.isExpanded ? <CollapseIcon /> : <ExpandIcon />}
            </RightSideButton>
          </TitleRight>
        </Title>
        {this.state.isExpanded ? (
          <FieldContent>{this.props.content}</FieldContent>
        ) : (
          <CollapsedView />
        )}
      </Root>
    );
  }
}
