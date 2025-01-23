import React, { PropsWithChildren } from 'react';
import { Box, Card, CardContent, Collapse, IconButton, ListItem, styled } from '@mui/material';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessOutlinedIcon from '@mui/icons-material/ExpandLessOutlined';

const ExpandIconContainer = styled(Box)(({ theme }) => ({
  display: 'flex',
  justifyContent: 'flex-end',
  padding: '0.5rem',
  paddingTop: 0,
  gap: '0.2rem',
}));

interface Props extends PropsWithChildren {
  id: number;
  selected?: boolean;
  expanded?: boolean;
  expandContent?: React.ReactNode;
  moreOptionsMenu?: React.ReactNode;
  moreOptionsMenuStyle?: React.CSSProperties;
  setExpanded?: React.Dispatch<React.SetStateAction<boolean>>;
  onSelectItem?: (id: number) => void;
}

/**
 * Card Item for List.
 * @returns
 */
export default function CardItem({
  id,
  selected,
  children,
  expanded = false,
  expandContent,
  moreOptionsMenu,
  moreOptionsMenuStyle,
  setExpanded,
  onSelectItem,
}: Props) {
  const handleExpandClick = () => {
    if (setExpanded) {
      setExpanded((prev) => !prev);
    }
  };

  const handleSelectItem = () => {
    if (onSelectItem) {
      onSelectItem(id);
    }
  };

  const stopPropagation = (evt: React.MouseEvent<HTMLDivElement>) => {
    evt.stopPropagation();
  };

  const showExpand = React.useMemo(() => setExpanded !== undefined && expanded !== undefined, []);

  return (
    <ListItem
      key={id}
      id={`card-content-item-${id}`}
      sx={{
        cursor: 'pointer',
        '&:hover': {
          backgroundColor: 'rgba(0, 0, 0, 0.04)',
        },
      }}
      onClick={handleSelectItem}
    >
      <Card
        sx={{
          width: '100%',
          backgroundColor: selected ? 'whitesmoke' : '#ddd',
          transition: 'background-color 0.3s',
        }}
      >
        <Box sx={{ position: 'relative' }}>
          {children}

          {/* Menu buttons */}
          {(Boolean(moreOptionsMenu) || showExpand) && (
            <ExpandIconContainer
              key="card-item-more-options"
              id={`card-item-more-options-${id}`}
              onClick={stopPropagation}
              sx={{
                position: 'absolute',
                top: 0,
                right: 0,
                height: '100%',
                alignItems: 'flex-end',
                ...moreOptionsMenuStyle,
              }}
            >
              {/* More extra components */}
              {moreOptionsMenu}

              {/* Expand button to expand content */}
              {showExpand && (
                <IconButton onClick={handleExpandClick} aria-expanded={expanded} aria-label="show more">
                  {expanded ? <ExpandLessOutlinedIcon /> : <ExpandMoreIcon />}
                </IconButton>
              )}
            </ExpandIconContainer>
          )}
        </Box>

        {/* Expand content */}
        {expandContent && (
          <Collapse in={expanded} timeout="auto" unmountOnExit>
            <CardContent>{expandContent}</CardContent>
          </Collapse>
        )}
      </Card>
    </ListItem>
  );
}
