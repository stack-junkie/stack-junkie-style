import { createContext, useContext, type CSSProperties, type HTMLAttributes, type ReactNode } from 'react';
import tokens from '../../../shared/tokens.json';

export type Direction = 'clean';
export type ThemeMode = 'light' | 'dark';
export type Density = 'compact' | 'comfortable';
// The shared density tokens are compact. Their comfortable block adds room for reading sites and lists only the values that change.
const { comfortable, ...compact } = tokens.density;
const comfortableValues: Partial<Record<string, number>> = comfortable;
export const directions = tokens.directions;
export const spacing = tokens.spacing;
const ThemeContext = createContext<{ direction: Direction; mode: ThemeMode }>({ direction: 'clean', mode: 'light' });
export const useTheme = () => useContext(ThemeContext);

export function ThemeProvider({ direction = 'clean', mode = 'light', density = 'compact', children, className = '', style, ...props }:
  HTMLAttributes<HTMLDivElement> & { direction?: Direction; mode?: ThemeMode; density?: Density; children: ReactNode }) {
  const theme = tokens.directions[direction] ?? tokens.directions.clean;
  const vars: Record<string, string | number> = {};
  for (const [key, value] of Object.entries(theme[mode])) vars[`--sj-${key}`] = value;
  for (const [key, value] of Object.entries(theme.metrics)) vars[`--sj-${key}`] = key === 'lineHeight' ? value : `${value}px`;
  for (const [key, value] of Object.entries(theme.fonts)) vars[`--sj-font-${key}`] = value === 'system-ui' ? '"Segoe UI Variable Text", "Segoe UI", system-ui, sans-serif' : value;
  for (const [key, value] of Object.entries(compact)) vars[`--sj-density-${key}`] = `${density === 'comfortable' ? comfortableValues[key] ?? value : value}px`;
  if (density === 'comfortable') vars['--sj-controlHeight'] = `${comfortable.controlHeight}px`;
  for (const [key, value] of Object.entries(tokens.typography)) vars[`--sj-${key}`] = key === 'readingMeasure' ? `${value}ch` : value;
  vars['--sj-fast'] = `${tokens.motion.fast}ms`;
  vars['--sj-normal'] = `${tokens.motion.normal}ms`;
  tokens.spacing.forEach((value, index) => { vars[`--sj-space-${index + 1}`] = `${value}px`; });
  for (const [key, value] of Object.entries(tokens.typeScale)) vars[`--sj-type-${key}`] = `${value}px`;
  return <ThemeContext.Provider value={{ direction, mode }}><div {...props} className={`sj-theme ${className}`} data-direction={direction} data-mode={mode} data-density={density} style={{ ...vars, colorScheme: mode, ...style } as CSSProperties}>{children}</div></ThemeContext.Provider>;
}
