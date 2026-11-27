import { createRoot } from 'react-dom/client';
import { ThemeProvider } from '@stack-junkie/ui-web';
// The config points this at page-with-stub.tsx or page-clean.tsx.
import Page from '#page';

createRoot(document.getElementById('root')!).render(<ThemeProvider mode="dark"><Page /></ThemeProvider>);
