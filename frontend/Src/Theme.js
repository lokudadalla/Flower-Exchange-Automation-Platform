import { createTheme } from '@mui/material/styles';

const theme = createTheme({
  palette: {
    primary: {
      main: '#1976d2', // blue
    },
    secondary: {
      main: '#d32f2f', // red
    },
    background: {
      default: '#f5f5f5',
    },
  },
  typography: {
    h4: {
      fontFamily: 'Roboto',
    },
  },
});

export default theme;
