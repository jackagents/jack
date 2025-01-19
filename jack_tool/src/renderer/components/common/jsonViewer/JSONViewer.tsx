import { Stack, styled } from '@mui/material';
import ReactJson from 'react-json-view';
import AutorenewIcon from '@mui/icons-material/Autorenew';

const LoadingRoot = styled('div')({
  position: 'absolute',
  width: '100%',
  height: '100%',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  zIndex: 9,
  fontSize: 18,
});

const Root = styled('div')({
  height: '100%',
  display: 'flex',
  flexDirection: 'column',
  '&>div:first-of-type': {
    flexBasis: 48,
  },
  backgroundColor: '#232C31',
});

const Title = styled('div')({
  display: 'flex',
  alignItems: 'center',
  paddingLeft: 10,
  backgroundColor: '#424242',
  color: 'white',
  height: 48,
});

const TitleStack = styled(Stack)({
  width: '100%',
  backgroundColor: '#424242',
  justifyContent: 'space-between',
  alignItems: 'center',
});

const RefreshBtn = styled(AutorenewIcon)({
  color: 'white',
  fontSize: '40px',
  cursor: 'pointer',
});

interface JSONViewerProps {
  title: string;
  dataName: string;
  nodeData?: Record<string, any>;
  isLoading?: boolean;
  timeoutText?: string;
  timeout?: boolean;
  onClickRefresh?: () => void;
}

export const INTERVAL_DURATION = 3000;
export const LOADINGTIMEOUT_DURATION = 10000;

export default function JSONViewer({
  title,
  dataName,
  nodeData,
  isLoading = false,
  timeout = false,
  timeoutText = '',
  onClickRefresh,
}: JSONViewerProps) {
  return (
    <Root>
      <TitleStack direction="row">
        <Title>{title}</Title>

        {onClickRefresh && (
          <RefreshBtn onClick={onClickRefresh}>Refresh</RefreshBtn>
        )}
      </TitleStack>

      {timeout && <LoadingRoot>{timeoutText}</LoadingRoot>}

      {isLoading && <span style={{ color: '#B5D8F6' }}>Loading...</span>}

      {!isLoading && nodeData && (
        <ReactJson
          src={nodeData}
          name={dataName}
          theme="codeschool"
          sortKeys
          enableClipboard={false}
          shouldCollapse={(field) => {
            if (field.type === 'object' && Object.keys(field.src).length > 3) {
              return true;
            }

            return false;
          }}
          groupArraysAfterLength={5}
          collapseStringsAfterLength={10}
          style={{
            overflow: 'auto',
            height: '100%',
            fontSize: 18,
            paddingTop: 10,
          }}
        />
      )}
    </Root>
  );
}
