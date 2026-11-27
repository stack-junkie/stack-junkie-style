import { createContext, useContext, type CSSProperties, type HTMLAttributes, type ReactNode } from 'react';
import tokens from '../../../shared/tokens.json';

export type Direction = 'clean' | 'warm' | 'technical';
export type ThemeMode = 'light' | 'dark';
export const directions = tokens.directions;
export const spacing = tokens.spacing;
const ThemeContext = createContext<{ direction: Direction; mode: ThemeMode }>({ direction: 'clean', mode: 'light' });
export const useTheme = () => useContext(ThemeContext);

// Dark mode for Clean utility is true black (owner decision, 2026-11-26). Structure comes from thin borders, not gray fills,
// and text is near white. Kept here until the shared tokens.json moves over, so the current showcase stays unchanged.
// Light mode follows the same rule: one white ground, structure from borders, gray only for raised controls.
const modeOverrides: Record<ThemeMode, Partial<Record<Direction, Record<string, string>>>> = {
  dark: { clean: { canvas: '#000000', surface: '#000000', raised: '#0e0e0e', text: '#ededed', muted: '#8f8f8f', border: '#1f1f1f', selection: '#141414', onAccent: '#000000' } },
  light: { clean: { canvas: '#ffffff', surface: '#ffffff', raised: '#f4f5f7', border: '#d5dae1' } },
};

export function ThemeProvider({ direction = 'clean', mode = 'light', children, className = '', style, ...props }:
  HTMLAttributes<HTMLDivElement> & { direction?: Direction; mode?: ThemeMode; children: ReactNode }) {
  const theme = tokens.directions[direction];
  const vars: Record<string, string | number> = {};
  for (const [key, value] of Object.entries({ ...theme[mode], ...modeOverrides[mode][direction] })) vars[`--sj-${key}`] = value;
  for (const [key, value] of Object.entries(theme.metrics)) vars[`--sj-${key}`] = key === 'lineHeight' ? value : `${value}px`;
  for (const [key, value] of Object.entries(theme.fonts)) vars[`--sj-font-${key}`] = value === 'system-ui' ? '"Segoe UI Variable Text", "Segoe UI", system-ui, sans-serif' : value;
  for (const [key, value] of Object.entries(tokens.density)) vars[`--sj-density-${key}`] = `${value}px`;
  for (const [key, value] of Object.entries(tokens.typography)) vars[`--sj-${key}`] = key === 'readingMeasure' ? `${value}ch` : value;
  vars['--sj-fast'] = `${tokens.motion.fast}ms`;
  vars['--sj-normal'] = `${tokens.motion.normal}ms`;
  tokens.spacing.forEach((value, index) => { vars[`--sj-space-${index + 1}`] = `${value}px`; });
  for (const [key, value] of Object.entries(tokens.typeScale)) vars[`--sj-type-${key}`] = `${value}px`;
  return <ThemeContext.Provider value={{ direction, mode }}><div {...props} className={`sj-theme ${className}`} data-direction={direction} data-mode={mode} style={{ ...vars, colorScheme: mode, ...style } as CSSProperties}>{children}</div></ThemeContext.Provider>;
}
